/*****************************************************************************\
*                                                                             *
*  Name   : blowfish encryption                                               *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "blowfish_crypto.h"
#include "ssl_init.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/mutex.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <mathematics/chaos.h>
#include <structures/static_memory_gremlin.h>

#include <openssl/blowfish.h>
#include <openssl/evp.h>

using namespace basis;
using namespace loggers;
using namespace mathematics;
using namespace structures;

namespace crypto {

const int FUDGE = 128;
  // extra space for the cipher's block size.  blowfish is only 8 bytes for
  // the cipher block size, but we ensure there will definitely be no
  // problems.

#undef set_key
  // get rid of a macro we don't want.

//#define DEBUG_BLOWFISH
  // uncomment for noisier version.

#ifdef DEBUG_BLOWFISH
  #undef LOG
  #define LOG(t) CLASS_EMERGENCY_LOG(program_wide_logger::get(), t)
#else
  #undef LOG
  #define LOG(t) 
#endif

#ifdef DEBUG_BLOWFISH
  // this macro checks on the validity of the key sizes (in bits).
  #define DISCUSS_KEY_SIZE(key_size) \
    if (key_size < minimum_key_size()) { \
      continuable_error(static_class_name(), func, \
          a_sprintf("key size (%d bits) is less than minimum key size %d.", \
              key_size, minimum_key_size())); \
      } \
  if (key_size > maximum_key_size()) { \
      continuable_error(static_class_name(), func, \
          a_sprintf("key size (%d bits) is greater than maximum key size %d.", \
              key_size, maximum_key_size())); \
    }
  
  // this macro checks that the key in the byte array has enough bytes for
  // the key size bits.
  #define DISCUSS_PROVIDED_KEY(key_size, key) \
    if (key.length() * BITS_PER_BYTE < key_size) { \
      continuable_error(static_class_name(), func, \
          a_sprintf("key array length (%d) is less than required by key size " \
              "(%d bits).", key.length(), key_size)); \
    }
#else
  #define DISCUSS_PROVIDED_KEY(key_size, key) 
  #define DISCUSS_KEY_SIZE(key_size) 
#endif

blowfish_crypto::blowfish_crypto(int key_size)
: _key_size(key_size),
  _key(new byte_array)
{
  FUNCDEF("ctor(int)");
  static_ssl_initializer();
  LOG("prior to key size discuss");
  DISCUSS_KEY_SIZE(key_size);
  if (key_size < minimum_key_size())
    _key_size = minimum_key_size();
  if (key_size > maximum_key_size())
    _key_size = maximum_key_size();
  LOG("prior to generate key");
  generate_key(_key_size, *_key);
  LOG("after generate key");
}

blowfish_crypto::blowfish_crypto(const byte_array &key, int key_size)
: _key_size(key_size),
  _key(new byte_array(key))
{
  FUNCDEF("ctor(byte_array,int)");
  // any problems with the key provided are horrid.  they will yield a
  // non-working blowfish object.
  LOG("prior to key size discuss");
  DISCUSS_KEY_SIZE(key_size);
  LOG("prior to provided key discuss");
  DISCUSS_PROVIDED_KEY(key_size, key);
  LOG("prior to ssl static init");
  static_ssl_initializer();
  LOG("after ssl static init");
}

blowfish_crypto::blowfish_crypto(const blowfish_crypto &to_copy)
: root_object(),
  _key_size(to_copy._key_size),
  _key(new byte_array(*to_copy._key))
{
  FUNCDEF("copy ctor");
  static_ssl_initializer(); 
  LOG("after ssl static init");
}

blowfish_crypto::~blowfish_crypto()
{
  FUNCDEF("dtor");
  LOG("prior to key whack");
  WHACK(_key);
  LOG("after key whack");
}

int blowfish_crypto::key_size() const { return _key_size; }

const byte_array &blowfish_crypto::get_key() const { return *_key; }

int blowfish_crypto::minimum_key_size() { return 64; }

int blowfish_crypto::maximum_key_size() { return 448; }

blowfish_crypto &blowfish_crypto::operator = (const blowfish_crypto &to_copy)
{
  if (this == &to_copy) return *this;
  _key_size = to_copy._key_size;
  *_key = *to_copy._key;
  return *this;
}

bool blowfish_crypto::set_key(const byte_array &new_key, int key_size)
{
  FUNCDEF("set_key");
  if (!new_key.length()) return false;
  DISCUSS_KEY_SIZE(key_size);
  DISCUSS_PROVIDED_KEY(key_size, new_key);
  if ( (key_size < minimum_key_size()) || (key_size > maximum_key_size()) )
    return false;
  if (new_key.length() * BITS_PER_BYTE < key_size) return false;
  _key_size = key_size;
  *_key = new_key;
  return true;
}

void blowfish_crypto::generate_key(int size, byte_array &new_key)
{
  FUNCDEF("generate_key");
  DISCUSS_KEY_SIZE(size);
  if (size < minimum_key_size())
    size = minimum_key_size();
  else if (size > maximum_key_size())
    size = maximum_key_size();
  int bytes = size / BITS_PER_BYTE;  // calculate the number of bytes needed.
  if (size % BITS_PER_BYTE) bytes++;  // add one for non-integral portion.
  new_key.reset(bytes);
  for (int i = 0; i < bytes; i++)
    new_key[i] = static_ssl_initializer().randomizer().inclusive(0, 255);
}

SAFE_STATIC(mutex, __vector_init_lock, )

const byte_array &blowfish_crypto::init_vector()
{
  FUNCDEF("init_vector");
  auto_synchronizer locking(__vector_init_lock());
  static byte_array to_return(EVP_MAX_IV_LENGTH);
  static bool initted = false;
  LOG("prior to initted check");
  if (!initted) {
    LOG("actually doing init");
    for (int i = 0; i < EVP_MAX_IV_LENGTH; i++)
      to_return[i] = 214 - i;
    initted = true;
  }
  LOG("leaving init check");
  return to_return;
}

bool blowfish_crypto::encrypt(const byte_array &source,
    byte_array &target) const
{
  FUNCDEF("encrypt");
  target.reset();
  if (!_key->length() || !source.length()) return false;
  bool to_return = true;

  // initialize an encoding session.
  EVP_CIPHER_CTX session;
  EVP_CIPHER_CTX_init(&session);
  EVP_EncryptInit_ex(&session, EVP_bf_cbc(), NIL, _key->observe(),
      init_vector().observe());
  EVP_CIPHER_CTX_set_key_length(&session, _key_size);

  // allocate temporary space for encrypted data.
  byte_array encoded(source.length() + FUDGE);

  // encrypt the entire source buffer.
  int encoded_len = 0;
  int enc_ret = EVP_EncryptUpdate(&session, encoded.access(), &encoded_len,
      source.observe(), source.length());
  if (enc_ret != 1) {
    continuable_error(class_name(), func, a_sprintf("encryption failed, "
        "result=%d.", enc_ret));
    to_return = false;
  } else {
    // chop any extra space off.
    LOG(a_sprintf("chopping bytes %d to %d.\n", encoded_len, encoded.last()));
    encoded.zap(encoded_len, encoded.last());
    target = encoded;
  }

  // only add padding if we succeeded with the encryption.
  if (enc_ret == 1) {
    // finalize the encryption.
    encoded.reset(FUDGE);  // reinflate for padding.
    int pad_len = 0;
    enc_ret = EVP_EncryptFinal_ex(&session, encoded.access(), &pad_len);
    if (enc_ret != 1) {
      continuable_error(class_name(), func, a_sprintf("finalizing encryption "
          "failed, result=%d.", enc_ret));
      to_return = false;
    } else {
      LOG(a_sprintf("padding added %d bytes.\n", pad_len));
      encoded.zap(pad_len, encoded.last());
      target += encoded;
    }
  }

  EVP_CIPHER_CTX_cleanup(&session);
  return to_return;
}

bool blowfish_crypto::decrypt(const byte_array &source,
    byte_array &target) const
{
  FUNCDEF("decrypt");
  target.reset();
  if (!_key->length() || !source.length()) return false;
  bool to_return = true;
  EVP_CIPHER_CTX session;
  EVP_CIPHER_CTX_init(&session);
  LOG(a_sprintf("key size %d bits.\n", BITS_PER_BYTE * _key->length()));
  EVP_DecryptInit_ex(&session, EVP_bf_cbc(), NIL, _key->observe(),
      init_vector().observe());
  EVP_CIPHER_CTX_set_key_length(&session, _key_size);

  // allocate enough space for decoded bytes.
  byte_array decoded(source.length() + FUDGE);

  int decoded_len = 0;
  int dec_ret = EVP_DecryptUpdate(&session, decoded.access(), &decoded_len,
      source.observe(), source.length());
  if (dec_ret != 1) {
    continuable_error(class_name(), func, "decryption failed.");
    to_return = false;
  } else {
    LOG(a_sprintf("  decrypted size in bytes is %d.\n", decoded_len));
    decoded.zap(decoded_len, decoded.last());
    target = decoded;
  }

  // only process padding if the first part of decryption succeeded.
  if (dec_ret == 1) {
    decoded.reset(FUDGE);  // reinflate for padding.
    int pad_len = 0;
    dec_ret = EVP_DecryptFinal_ex(&session, decoded.access(), &pad_len);
    LOG(a_sprintf("padding added %d bytes.\n", pad_len));
    if (dec_ret != 1) {
      continuable_error(class_name(), func, a_sprintf("finalizing decryption "
          "failed, result=%d, padlen=%d, target had %d bytes.", dec_ret,
          pad_len, target.length()));
      to_return = false;
    } else {
      int dec_size = pad_len;
      decoded.zap(dec_size, decoded.last());
      target += decoded;
    }
  }

  EVP_CIPHER_CTX_cleanup(&session);
  return to_return;
}

} //namespace.


