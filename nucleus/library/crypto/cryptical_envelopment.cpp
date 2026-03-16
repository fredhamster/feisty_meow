/*****************************************************************************\
*                                                                             *
*  Name   : cryptical_envelopment                                             *
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

#include "cryptical_envelopment.h"
#include "ssl_init.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/mutex.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <mathematics/chaos.h>
#include <structures/static_memory_gremlin.h>

///#include <openssl/blowfish.h>
#include <openssl/err.h>
#include <openssl/evp.h>

using namespace basis;
using namespace loggers;
using namespace mathematics;
using namespace structures;

namespace crypto {

const int FUDGE = 1024;
  /* extra space for the cipher's block size.  blowfish, e.g., is only 8 bytes for
  the cipher block size, so we'd only ever need possibly 8 bytes padding?
  */
//hmmm: guarantee the fudge space is enough for other algorithms!

//#undef set_key
  // get rid of a macro we don't want.

#define DEBUG_CRYPTICAL_ENVELOPMENT
  // uncomment for noisier version.

// our logging via LOG is disabled unless the debugging flag above is turned on.
// but the ALWAYS_LOG macro is unfazed and will log regardless of the flag.
#undef ALWAYS_LOG
#define ALWAYS_LOG(t) CLASS_EMERGENCY_LOG(program_wide_logger::get(), t)
#ifdef DEBUG_CRYPTICAL_ENVELOPMENT
  #undef LOG
  #define LOG(t) CLASS_EMERGENCY_LOG(program_wide_logger::get(), t)
#else
  #undef LOG
  #define LOG(t) 
#endif

// helpful macro for the error string of last failure.
#define GET_SSL_ERROR() \
  ERR_error_string(ERR_get_error(), NULL_POINTER)

#ifdef DEBUG_CRYPTICAL_ENVELOPMENT
  // only cause a program stop if we're in debugging mode.
//hmmm: that's pretty loose, really; if they have a key size error, how can we keep going and just pretend that's okay?  we should not.
  #define ERROR_BAILOUT(a, b, c) deadly_error(a, b, c)
#else
  #define ERROR_BAILOUT(a, b, c) 
#endif

// this macro checks on the validity of the key sizes (in bits).
#define DISCUSS_KEY_SIZE(key_size) \
  if (key_size < minimum_key_size()) { \
    ERROR_BAILOUT(static_class_name(), func, \
        a_sprintf("key size (%d bits) is less than minimum key size %d.", \
            key_size, minimum_key_size())); \
    return false; \
  } \
  if (key_size > maximum_key_size()) { \
      ERROR_BAILOUT(static_class_name(), func, \
          a_sprintf("key size (%d bits) is greater than maximum key size %d.", \
              key_size, maximum_key_size())); \
    return false; \
  }

// this macro checks that the key in the byte array has enough bytes for
// the key size bits.
#define DISCUSS_PROVIDED_KEY(key_size, key) \
  if (key.length() * BITS_PER_BYTE < key_size) { \
    ERROR_BAILOUT(static_class_name(), func, \
        a_sprintf("key array length (%d) is less than required by key size " \
            "(%d bits).", key.length(), key_size)); \
    return false; \
  }

cryptical_envelopment::cryptical_envelopment(const EVP_CIPHER *cipher_type)
: _key_size(0),
  _key(new byte_array()),
  _cipher_type(cipher_type)
{
  FUNCDEF("ctor(int)");
  static_ssl_initializer();
}

cryptical_envelopment::cryptical_envelopment(const cryptical_envelopment &to_copy)
: root_object(),
  _key_size(to_copy._key_size),
  _key(new byte_array(*to_copy._key))
{
  FUNCDEF("copy ctor");
  static_ssl_initializer(); 
  LOG("after ssl static init");
}

cryptical_envelopment::~cryptical_envelopment()
{
  FUNCDEF("destructor");
  LOG("prior to key whack");
  WHACK(_key);
  LOG("after key whack");
}

bool cryptical_envelopment::set_key(int key_size)
{
  FUNCDEF("set_key(int)");
  DISCUSS_KEY_SIZE(key_size);
///  if (key_size < minimum_key_size())
///    _key_size = minimum_key_size();
///  if (key_size > maximum_key_size())
///    _key_size = maximum_key_size();
  LOG("prior to generate key");
  _key_size = key_size;
///  WHACK(_key);  // clear any existing key.
///  _key = new byte_array();
  bool to_return = generate_key(_key_size, *_key);
  LOG("after generate key");
  return to_return;
}

/*
bool cryptical_envelopment::set_key(const byte_array &key, int key_size)
{
  FUNCDEF("set_key(byte_array,int)");
////  static_ssl_initializer();
  DISCUSS_KEY_SIZE(key_size);
  DISCUSS_PROVIDED_KEY(key_size, key);
  _key_size = key_size;
  _key = new byte_array(key);
  return true;
}
*/

int cryptical_envelopment::key_size() const { return _key_size; }

const byte_array &cryptical_envelopment::get_key() const { return *_key; }

cryptical_envelopment &cryptical_envelopment::operator = (const cryptical_envelopment &to_copy)
{
  if (this == &to_copy) return *this;
  _key_size = to_copy._key_size;
  *_key = *to_copy._key;
  _cipher_type = to_copy._cipher_type;
  return *this;
}

bool cryptical_envelopment::set_key(const byte_array &new_key, int key_size)
{
  FUNCDEF("set_key(byte_array,int)");
//  if (!new_key.length()) return false;
  DISCUSS_KEY_SIZE(key_size);
  DISCUSS_PROVIDED_KEY(key_size, new_key);
//  if ( (key_size < minimum_key_size()) || (key_size > maximum_key_size()) )
//    return false;
//  if (new_key.length() * BITS_PER_BYTE < key_size) return false;
////  if (!_key)
////    _key = new byte_array();
  _key_size = key_size;
  *_key = new_key;
  return true;
}

//hmmm: move someplace more useful.
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte) \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 

bool cryptical_envelopment::generate_key(int size, byte_array &new_key)
{
  FUNCDEF("generate_key");
  static_ssl_initializer();
////  if (!_key)
////    _key = new byte_array();
  DISCUSS_KEY_SIZE(size);
//  if (size < minimum_key_size())
//    size = minimum_key_size();
//  else if (size > maximum_key_size())
//    size = maximum_key_size();
  int bytes = size / BITS_PER_BYTE;  // calculate the number of bytes needed.
  if (size % BITS_PER_BYTE) bytes++;  // add one for non-integral portion.
  new_key.reset(bytes);
  for (int i = 0; i < bytes; i++)
    new_key[i] = static_ssl_initializer().randomizer().inclusive(0, 255);

//code that we thought might be necessary but which doesn't help blowfish not suck.
//#define WIPE_UNUSED_BITS
#ifdef WIPE_UNUSED_BITS
  // clear the bits that cannot be non-zero for a key whose bits are not evenly divisible by 8.
  int bits_to_wipe = BITS_PER_BYTE - (size % BITS_PER_BYTE);
ALWAYS_LOG(a_sprintf("saying we need to zap %d bits from result.", bits_to_wipe));
  abyte mask = 0xFF;
//hmmm: if we leave non-zero stuff in the last byte, that's not quite right!
//      also a question of endian-ness of where to zap those bits.  argh!

  if (bits_to_wipe) {
    // rotate our mask to cover the number of bits we want to actually use.
    mask <<= bits_to_wipe;
ALWAYS_LOG(a_sprintf("mask now: " BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(mask)));
    new_key[bytes - 1] &= mask;
ALWAYS_LOG(a_sprintf("last byte now: " BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(new_key[bytes - 1])));
  }
#endif //wipe.

  return true;
}

SAFE_STATIC(mutex, __vector_init_lock, )

/*
hmmm: this seems like a bad security situation, because we are using a single algorithm for
      creating init vectors, so it's super easy to guess these (by reading the code, for
      example).  is this still secure, given that the keys are not known to an attacker?
*/
const byte_array &cryptical_envelopment::init_vector()
{
  FUNCDEF("init_vector");
  static_ssl_initializer();
  auto_synchronizer locking(__vector_init_lock());
  static byte_array to_return(EVP_MAX_IV_LENGTH);
  static bool initted = false;
  if (!initted) {
    LOG(">> actually creating init_vector >>");
//hmmm: are we okay with the wrap-around on the byte type (going negative) if the init vector length is longer than 214?
    for (int i = 0; i < to_return.length(); i++)
      to_return[i] = abyte(214 - i);
    initted = true;
    LOG("<< finished init_vector creation <<");
  }
  return to_return;
}

bool cryptical_envelopment::encrypt(const byte_array &source,
    byte_array &target) const
{
  FUNCDEF("encrypt");
ALWAYS_LOG(">>encrypt>>");
  target.reset();
  if (!_key->length() || !source.length()) return false;
  bool to_return = true;

LOG(a_sprintf("  encrypting %d bytes", source.length()));

  // initialize an encoding session.
  EVP_CIPHER_CTX *session = EVP_CIPHER_CTX_new();
  EVP_CIPHER_CTX_init(session);

  //new rules!
  // EVP_EncryptInit to set the cipher, but leave key and IV null and unset
  // EVP_CIPHER_CTX_set_key_length and EVP_CTRL_AEAD_SET_IVLEN
  // EVP_EncryptInit again. This time leave cipher null, because you've already set it, and set the key and IV.

  int initret = EVP_EncryptInit_ex(session, _cipher_type, NULL_POINTER, NULL_POINTER, NULL_POINTER);
  if (!initret) {
    // zero means a failure of the initialization.
    deadly_error(class_name(), func, a_sprintf("failure in calling EVP_EncryptInit_ex, with error %s", GET_SSL_ERROR()));
  }
  LOG(a_sprintf("  calling set key len with key size of %d", _key_size));
  // new fancy footwork needed to keep openssl from blowing up and claiming we didn't set the key.
//hmmm: check returns on these setters?
  EVP_CIPHER_CTX_set_key_length(session, _key_size);
  EVP_CIPHER_CTX_ctrl(session, EVP_CTRL_AEAD_SET_IVLEN, init_vector().length(), NULL);
  // and round and round we go...
  initret = EVP_EncryptInit_ex(session, NULL_POINTER, NULL_POINTER, _key->observe(), init_vector().observe());
  if (!initret) {
    // zero means a failure of the initialization.
    deadly_error(class_name(), func, a_sprintf("second phase failure in calling EVP_EncryptInit_ex, with error %s", GET_SSL_ERROR()));
  }

  // allocate temporary space for encrypted data.
  byte_array encoded(source.length() + FUDGE);

  // encrypt the entire source buffer.
  int encoded_len = 0;
  int enc_ret = EVP_EncryptUpdate(session, encoded.access(), &encoded_len,
      source.observe(), source.length());
  if (enc_ret != 1) {
    deadly_error(class_name(), func, a_sprintf("encryption failed, "
        "result=%d with error=%s.", enc_ret, GET_SSL_ERROR()));
    to_return = false;
  } else {
    // chop any extra space off.
    LOG(a_sprintf("  chopping extra bytes %d to %d.", encoded_len, encoded.last()));
    encoded.zap(encoded_len, encoded.last());
    target = encoded;
  }

  // only add padding if we succeeded with the encryption.
  if (enc_ret == 1) {
    // finalize the encryption.
    encoded.reset(FUDGE);  // reinflate for padding.
    int pad_len = 0;
    enc_ret = EVP_EncryptFinal_ex(session, encoded.access(), &pad_len);
    if (enc_ret != 1) {
      deadly_error(class_name(), func, a_sprintf("finalizing encryption "
          "failed, result=%d with error=%s.", enc_ret, GET_SSL_ERROR()));
      to_return = false;
    } else {
      LOG(a_sprintf("  encryption padding added %d bytes.", pad_len));
      encoded.zap(pad_len, encoded.last());
      target += encoded;
    }
  }

  EVP_CIPHER_CTX_cleanup(session);
  EVP_CIPHER_CTX_free(session);
ALWAYS_LOG("<<encrypt<<");
  return to_return;
}

bool cryptical_envelopment::decrypt(const byte_array &source,
    byte_array &target) const
{
  FUNCDEF("decrypt");
ALWAYS_LOG(">>decrypt>>");
  target.reset();
  if (!_key->length() || !source.length()) return false;
  bool to_return = true;
  EVP_CIPHER_CTX *session = EVP_CIPHER_CTX_new();
  EVP_CIPHER_CTX_init(session);
  LOG(a_sprintf("  using key size with %d bits.", _key_size));
  int initret = EVP_DecryptInit_ex(session, _cipher_type, NULL_POINTER, NULL_POINTER, NULL_POINTER);
  if (!initret) {
    // zero means a failure of the initialization.
    deadly_error(class_name(), func, a_sprintf("failure in calling EVP_DecryptInit_ex, with error %s", GET_SSL_ERROR()));
  }
  // more fancy fupwork.
//hmmm: check returns on these setters?
  EVP_CIPHER_CTX_set_key_length(session, _key_size);
  EVP_CIPHER_CTX_ctrl(session, EVP_CTRL_AEAD_SET_IVLEN, init_vector().length(), NULL);
  initret = EVP_DecryptInit_ex(session, NULL_POINTER, NULL_POINTER, _key->observe(), init_vector().observe());
  if (!initret) {
    // zero means a failure of the initialization.
    deadly_error(class_name(), func, a_sprintf("second phase failure in calling EVP_DecryptInit_ex, with error %s", GET_SSL_ERROR()));
  }

  // allocate enough space for decoded bytes.
  byte_array decoded(source.length() + FUDGE);

  int decoded_len = 0;
  int dec_ret = EVP_DecryptUpdate(session, decoded.access(), &decoded_len,
      source.observe(), source.length());
  if (dec_ret != 1) {
    deadly_error(class_name(), func, a_sprintf("decryption failed with error=%s", GET_SSL_ERROR()));
    to_return = false;
  } else {
    LOG(a_sprintf("  first part decrypted size in bytes is %d.", decoded_len));
    decoded.zap(decoded_len, decoded.last());
    target = decoded;
  }

  // only process padding if the first part of decryption succeeded.
  if (dec_ret == 1) {
    decoded.reset(FUDGE);  // reinflate for padding.
    int pad_len = 0;
    dec_ret = EVP_DecryptFinal_ex(session, decoded.access(), &pad_len);
    if (dec_ret != 1) {
      deadly_error(class_name(), func, a_sprintf("finalizing decryption "
          "failed, result=%d, padlen=%d, target had %d bytes, error=%s.", dec_ret,
          pad_len, target.length(), GET_SSL_ERROR()));
      to_return = false;
    } else {
      LOG(a_sprintf("  decryption final had %d bytes padding.", pad_len));
      decoded.zap(pad_len, decoded.last());
      target += decoded;
    }
  }

  EVP_CIPHER_CTX_cleanup(session);
  EVP_CIPHER_CTX_free(session);
ALWAYS_LOG("<<decrypt<<");
  return to_return;
}

} //namespace.


