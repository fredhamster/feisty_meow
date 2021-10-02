/*
*  Name   : RSA public key encryption
*  Author : Chris Koeritz
*  Purpose:
*    Supports public (and private) key encryption and decryption using the
*    OpenSSL package's support for RSA encryption.
****
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

//note: rsa crypto provides a nice printing method...  RSA_print_fp(stdout, private_key, 0);

// notes from openssl docs: length to be encrypted in a chunk must be less than
// RSA_size(rsa) - 11 for the PKCS #1 v1.5 based padding modes, less than
// RSA_size(rsa) - 41 for RSA_PKCS1_OAEP_PADDING and exactly RSA_size(rsa)
// for RSA_NO_PADDING.

#include "rsa_crypto.h"
#include "ssl_init.h"

#include <basis/functions.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <mathematics/chaos.h>
#include <structures/object_packers.h>
#include <structures/static_memory_gremlin.h>

#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/rsa.h>

using namespace basis;
using namespace loggers;
using namespace mathematics;
using namespace structures;

namespace crypto {

//#define DEBUG_RSA_CRYPTO
  // uncomment for noisier version.

#ifdef DEBUG_RSA_CRYPTO
  #undef LOG
  #define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)
#else
  #undef LOG
  #define LOG(s)
#endif

SAFE_STATIC(mutex, __single_stepper, )
  // protects unsafe areas of rsa crypto from access by multiple threads at once.

rsa_crypto::rsa_crypto(int key_size)
: _key(NULL_POINTER)
{
  FUNCDEF("ctor(int)");
  LOG("prior to generating key");
  _key = generate_key(key_size);  // generate_key initializes ssl for us.
  LOG("after generating key");
}

rsa_crypto::rsa_crypto(const byte_array &key)
: _key(NULL_POINTER)
{
  FUNCDEF("ctor(byte_array)");
  static_ssl_initializer();
  byte_array key_copy = key;
  LOG("prior to set key");
  set_key(key_copy);
  LOG("after set key");
}

rsa_crypto::rsa_crypto(RSA *key)
: _key(NULL_POINTER)
{
  FUNCDEF("ctor(RSA)");
  static_ssl_initializer();
  LOG("prior to set key");
  set_key(key);
  LOG("after set key");
}

rsa_crypto::rsa_crypto(const rsa_crypto &to_copy)
: root_object(),
  _key(NULL_POINTER)
{
  FUNCDEF("copy ctor");
  static_ssl_initializer();
  LOG("prior to set key");
  set_key(to_copy._key);
  LOG("after set key");
}

rsa_crypto::~rsa_crypto()
{
  FUNCDEF("destructor");
  LOG("prior to rsa free");
  auto_synchronizer mutt(__single_stepper());
  RSA_free(_key);
  LOG("after rsa free");
}

const rsa_crypto &rsa_crypto::operator = (const rsa_crypto &to_copy)
{
  if (this == &to_copy) return *this;
  set_key(to_copy._key);
  return *this;
}

RSA *rsa_crypto::generate_key(int key_size)
{
  FUNCDEF("generate_key");
  if (key_size < 4) key_size = 4;  // laughable lower default.
  static_ssl_initializer();
  LOG("into generate key");
  auto_synchronizer mutt(__single_stepper());
  RSA *to_return = RSA_new();
  BIGNUM *e = BN_new();
  BN_set_word(e, 65537);
//hmmm: only one value of e?
  int ret = RSA_generate_key_ex(to_return, key_size, e, NULL_POINTER);
  if (!ret) {
    continuable_error(static_class_name(), func,
        a_sprintf("failed to generate a key of %d bits: error is %ld.", key_size, ERR_get_error()));
    BN_free(e);
    RSA_free(to_return);
    return NULL;
  }
  LOG("after key generated");
  BN_free(e);
  return to_return;
}

bool rsa_crypto::check_key(RSA *key)
{
  auto_synchronizer mutt(__single_stepper());
  return RSA_check_key(key) == 1;
}

bool rsa_crypto::set_key(byte_array &key)
{
  FUNCDEF("set_key [byte_array]");
  if (!key.length()) return false;
  auto_synchronizer mutt(__single_stepper());
  if (_key) RSA_free(_key);
  _key = RSA_new();
  abyte type;
  if (!structures::detach(key, type)) return false;
  if ( (type != 'r') && (type != 'u') ) return false;
  // get the public key bits first.
  byte_array n;
  if (!structures::detach(key, n)) return false;
  BIGNUM *the_n = BN_bin2bn(n.access(), n.length(), NULL_POINTER);
  if (!the_n) return false;
  byte_array e;
  if (!structures::detach(key, e)) return false;
  BIGNUM *the_e = BN_bin2bn(e.access(), e.length(), NULL_POINTER);
  if (!the_e) return false;

  if (type == 'u') {
     // done with public key.
#ifdef NEWER_OPENSSL
     RSA_set0_key(_key, the_n, the_e, NULL_POINTER);
#else
     _key->n = the_n; _key->e = the_e;
#endif
     return true;
  }

  // the rest is for a private key.
  byte_array d;
  if (!structures::detach(key, d)) return false;
  BIGNUM *the_d = BN_bin2bn(d.access(), d.length(), NULL_POINTER);
  if (!the_d) return false;

  byte_array p;
  if (!structures::detach(key, p)) return false;
  BIGNUM *the_p = BN_bin2bn(p.access(), p.length(), NULL_POINTER);
  if (!the_p) return false;
  byte_array q;
  if (!structures::detach(key, q)) return false;
  BIGNUM *the_q = BN_bin2bn(q.access(), q.length(), NULL_POINTER);
  if (!the_q) return false;
  byte_array dmp1;
  if (!structures::detach(key, dmp1)) return false;
  BIGNUM *the_dmp1 = BN_bin2bn(dmp1.access(), dmp1.length(), NULL_POINTER);
  if (!the_dmp1) return false;
  byte_array dmq1;
  if (!structures::detach(key, dmq1)) return false;
  BIGNUM *the_dmq1 = BN_bin2bn(dmq1.access(), dmq1.length(), NULL_POINTER);
  if (!the_dmq1) return false;
  byte_array iqmp;
  if (!structures::detach(key, iqmp)) return false;
  BIGNUM *the_iqmp = BN_bin2bn(iqmp.access(), iqmp.length(), NULL_POINTER);
  if (!the_iqmp) return false;

  // we can set the n, e and d now.
#ifdef NEWER_OPENSSL
  int ret = RSA_set0_key(_key, the_n, the_e, the_d);
  if (ret != 1) return false;
  ret = RSA_set0_factors(_key, the_p, the_q);
  if (ret != 1) return false;
  ret = RSA_set0_crt_params(_key, the_dmp1, the_dmq1, the_iqmp);
  if (ret != 1) return false;
#else
  _key->n = the_n; _key->e = the_e; _key->d = the_d;
  _key->p = the_p; _key->q = the_q;
  _key->dmp1 = the_dmp1; _key->dmq1 = the_dmq1; _key->iqmp = the_iqmp;
#endif

  int check = RSA_check_key(_key);
  if (check != 1) {
    continuable_error(static_class_name(), func, "failed to check the private "
        "portion of the key!");
    return false;
  }

  return true;
}

bool rsa_crypto::set_key(RSA *key)
{
  FUNCDEF("set_key [RSA]");
  if (!key) return NULL_POINTER;
  // test the incoming key.
  auto_synchronizer mutt(__single_stepper());
  int check = RSA_check_key(key);
  if (check != 1) return false;
  // clean out the old key.
  if (_key) RSA_free(_key);
  _key = RSAPrivateKey_dup(key);
  if (!_key) {
    continuable_error(static_class_name(), func, "failed to create a "
        "duplicate of the key!");
    return false;
  }
  return true;
}

bool rsa_crypto::public_key(byte_array &pubkey) const
{
  FUNCDEF("public_key");
  if (!_key) return false;
  structures::attach(pubkey, abyte('u'));  // signal a public key.
  // convert the two public portions into binary.
  BIGNUM **the_n = new BIGNUM *, **the_e = new BIGNUM *, **the_d = new BIGNUM *;
#ifdef NEWER_OPENSSL
  RSA_get0_key(_key, (const BIGNUM **)the_n, (const BIGNUM **)the_e, (const BIGNUM **)the_d);
#else
  *the_n = _key->n; *the_e = _key->e; *the_d = _key->d;
#endif
  byte_array n(BN_num_bytes(*the_n));
  int ret = BN_bn2bin(*the_n, n.access());
  byte_array e(BN_num_bytes(*the_e));
  ret = BN_bn2bin(*the_e, e.access());
  // pack those two chunks.
  structures::attach(pubkey, n);
  structures::attach(pubkey, e);
  WHACK(the_n); WHACK(the_e); WHACK(the_d);

  return true;
}

bool rsa_crypto::private_key(byte_array &privkey) const
{
  FUNCDEF("private_key");
  if (!_key) return false;
  int posn = privkey.length();
  bool worked = public_key(privkey);  // get the public pieces first.
  if (!worked) return false;
  privkey[posn] = abyte('r');  // switch public key flag to private.
  // convert the multiple private portions into binary.
  //const BIGNUM **the_n = NULL_POINTER, **the_e = NULL_POINTER, **the_d = NULL_POINTER;
  BIGNUM **the_n = new BIGNUM *, **the_e = new BIGNUM *, **the_d = new BIGNUM *;
  BIGNUM **the_p = new BIGNUM *, **the_q = new BIGNUM *;
  BIGNUM **the_dmp1 = new BIGNUM *, **the_dmq1 = new BIGNUM *, **the_iqmp = new BIGNUM *;
#ifdef NEWER_OPENSSL
  RSA_get0_key(_key, (const BIGNUM **)the_n, (const BIGNUM **)the_e, (const BIGNUM **)the_d);
  RSA_get0_factors(_key, (const BIGNUM **)the_p, (const BIGNUM **)the_q);
  RSA_get0_crt_params(_key, (const BIGNUM **)the_dmp1, (const BIGNUM **)the_dmq1, (const BIGNUM **)the_iqmp);
#else
  *the_n = _key->n; *the_e = _key->e; *the_d = _key->d;
  *the_p = _key->p; *the_q = _key->q;
  *the_dmp1 = _key->dmp1; *the_dmq1 = _key->dmq1; *the_iqmp = _key->iqmp;
#endif
  byte_array d(BN_num_bytes(*the_d));
  int ret = BN_bn2bin(*the_d, d.access());
  byte_array p(BN_num_bytes(*the_p));
  ret = BN_bn2bin(*the_p, p.access());
  byte_array q(BN_num_bytes(*the_q));
  ret = BN_bn2bin(*the_q, q.access());
  byte_array dmp1(BN_num_bytes(*the_dmp1));
  ret = BN_bn2bin(*the_dmp1, dmp1.access());
  byte_array dmq1(BN_num_bytes(*the_dmq1));
  ret = BN_bn2bin(*the_dmq1, dmq1.access());
  byte_array iqmp(BN_num_bytes(*the_iqmp));
  ret = BN_bn2bin(*the_iqmp, iqmp.access());
  // pack all those in now.
  structures::attach(privkey, d);
  structures::attach(privkey, p);
  structures::attach(privkey, q);
  structures::attach(privkey, dmp1);
  structures::attach(privkey, dmq1);
  structures::attach(privkey, iqmp);
  return true;
}

bool rsa_crypto::public_encrypt(const byte_array &source,
    byte_array &target) const
{
  FUNCDEF("public_encrypt");
  target.reset();
  if (!source.length()) return false;

  auto_synchronizer mutt(__single_stepper());
  const int max_chunk = RSA_size(_key) - 12;

  byte_array encoded(RSA_size(_key));
  for (int i = 0; i < source.length(); i += max_chunk) {
    int edge = i + max_chunk - 1;
    if (edge > source.last())
      edge = source.last();
    int next_chunk = edge - i + 1;
    RSA_public_encrypt(next_chunk, &source[i],
        encoded.access(), _key, RSA_PKCS1_PADDING);
    target += encoded;
  }
  return true;
}

bool rsa_crypto::private_decrypt(const byte_array &source,
    byte_array &target) const
{
  FUNCDEF("private_decrypt");
  target.reset();
  if (!source.length()) return false;

  auto_synchronizer mutt(__single_stepper());
  const int max_chunk = RSA_size(_key);

  byte_array decoded(max_chunk);
  for (int i = 0; i < source.length(); i += max_chunk) {
    int edge = i + max_chunk - 1;
    if (edge > source.last())
      edge = source.last();
    int next_chunk = edge - i + 1;
    int dec_size = RSA_private_decrypt(next_chunk, &source[i],
        decoded.access(), _key, RSA_PKCS1_PADDING);
    if (dec_size < 0) return false;  // that didn't work.
    decoded.zap(dec_size, decoded.last());
    target += decoded;
    decoded.reset(max_chunk);
  }
  return true;
}

bool rsa_crypto::private_encrypt(const byte_array &source,
    byte_array &target) const
{
  FUNCDEF("private_encrypt");
  target.reset();
  if (!source.length()) return false;

  auto_synchronizer mutt(__single_stepper());
  const int max_chunk = RSA_size(_key) - 12;

  byte_array encoded(RSA_size(_key));
  for (int i = 0; i < source.length(); i += max_chunk) {
    int edge = i + max_chunk - 1;
    if (edge > source.last())
      edge = source.last();
    int next_chunk = edge - i + 1;
    RSA_private_encrypt(next_chunk, &source[i],
        encoded.access(), _key, RSA_PKCS1_PADDING);
    target += encoded;
  }
  return true;
}

bool rsa_crypto::public_decrypt(const byte_array &source,
    byte_array &target) const
{
  FUNCDEF("public_decrypt");
  target.reset();
  if (!source.length()) return false;

  auto_synchronizer mutt(__single_stepper());
  const int max_chunk = RSA_size(_key);

  byte_array decoded(max_chunk);
  for (int i = 0; i < source.length(); i += max_chunk) {
    int edge = i + max_chunk - 1;
    if (edge > source.last())
      edge = source.last();
    int next_chunk = edge - i + 1;
    int dec_size = RSA_public_decrypt(next_chunk, &source[i],
        decoded.access(), _key, RSA_PKCS1_PADDING);
    if (dec_size < 0) return false;  // that didn't work.
    decoded.zap(dec_size, decoded.last());
    target += decoded;
    decoded.reset(max_chunk);
  }
  return true;
}

} //namespace.

