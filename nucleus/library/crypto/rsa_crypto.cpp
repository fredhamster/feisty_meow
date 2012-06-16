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
: _key(NIL)
{
  FUNCDEF("ctor(int)");
  LOG("prior to generating key");
  _key = generate_key(key_size);  // generate_key initializes ssl for us.
  LOG("after generating key");
}

rsa_crypto::rsa_crypto(const byte_array &key)
: _key(NIL)
{
  FUNCDEF("ctor(byte_array)");
  static_ssl_initializer();
  byte_array key_copy = key;
  LOG("prior to set key");
  set_key(key_copy);
  LOG("after set key");
}

rsa_crypto::rsa_crypto(rsa_st *key)
: _key(NIL)
{
  FUNCDEF("ctor(rsa_st)");
  static_ssl_initializer();
  LOG("prior to set key");
  set_key(key);
  LOG("after set key");
}

rsa_crypto::rsa_crypto(const rsa_crypto &to_copy)
: root_object(),
  _key(NIL)
{
  FUNCDEF("copy ctor");
  static_ssl_initializer();
  LOG("prior to set key");
  set_key(to_copy._key);
  LOG("after set key");
}

rsa_crypto::~rsa_crypto()
{
  FUNCDEF("dtor");
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

rsa_st *rsa_crypto::generate_key(int key_size)
{
  FUNCDEF("generate_key");
  if (key_size < 4) key_size = 4;  // laughable lower default.
  static_ssl_initializer();
  LOG("prior to generate key");
  auto_synchronizer mutt(__single_stepper());
  rsa_st *to_return = RSA_generate_key(key_size, 65537, NIL, NIL);
  if (!to_return) {
    continuable_error(static_class_name(), func,
        a_sprintf("failed to generate a key of %d bits.", key_size));
  }
  LOG("after generate key");
  return to_return;
}

bool rsa_crypto::check_key(rsa_st *key)
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
  _key->n = BN_bin2bn(n.access(), n.length(), NIL);
  if (!_key->n) return false;
  byte_array e;
  if (!structures::detach(key, e)) return false;
  _key->e = BN_bin2bn(e.access(), e.length(), NIL);
  if (!_key->e) return false;
  if (type == 'u') return true;  // done with public key.

  // the rest is for a private key.
  byte_array d;
  if (!structures::detach(key, d)) return false;
  _key->d = BN_bin2bn(d.access(), d.length(), NIL);
  if (!_key->d) return false;
  byte_array p;
  if (!structures::detach(key, p)) return false;
  _key->p = BN_bin2bn(p.access(), p.length(), NIL);
  if (!_key->p) return false;
  byte_array q;
  if (!structures::detach(key, q)) return false;
  _key->q = BN_bin2bn(q.access(), q.length(), NIL);
  if (!_key->q) return false;
  byte_array dmp1;
  if (!structures::detach(key, dmp1)) return false;
  _key->dmp1 = BN_bin2bn(dmp1.access(), dmp1.length(), NIL);
  if (!_key->dmp1) return false;
  byte_array dmq1;
  if (!structures::detach(key, dmq1)) return false;
  _key->dmq1 = BN_bin2bn(dmq1.access(), dmq1.length(), NIL);
  if (!_key->dmq1) return false;
  byte_array iqmp;
  if (!structures::detach(key, iqmp)) return false;
  _key->iqmp = BN_bin2bn(iqmp.access(), iqmp.length(), NIL);
  if (!_key->iqmp) return false;
  int check = RSA_check_key(_key);
  if (check != 1) {
    continuable_error(static_class_name(), func, "failed to check the private "
        "portion of the key!");
    return false;
  }

  return true;
}

bool rsa_crypto::set_key(rsa_st *key)
{
  FUNCDEF("set_key [rsa_st]");
  if (!key) return NIL;
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
  byte_array n(BN_num_bytes(_key->n));
  int ret = BN_bn2bin(_key->n, n.access());
  byte_array e(BN_num_bytes(_key->e));
  ret = BN_bn2bin(_key->e, e.access());
  // pack those two chunks.
  structures::attach(pubkey, n);
  structures::attach(pubkey, e);
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
  byte_array d(BN_num_bytes(_key->d));
  int ret = BN_bn2bin(_key->d, d.access());
  byte_array p(BN_num_bytes(_key->p));
  ret = BN_bn2bin(_key->p, p.access());
  byte_array q(BN_num_bytes(_key->q));
  ret = BN_bn2bin(_key->q, q.access());
  byte_array dmp1(BN_num_bytes(_key->dmp1));
  ret = BN_bn2bin(_key->dmp1, dmp1.access());
  byte_array dmq1(BN_num_bytes(_key->dmq1));
  ret = BN_bn2bin(_key->dmq1, dmq1.access());
  byte_array iqmp(BN_num_bytes(_key->iqmp));
  ret = BN_bn2bin(_key->iqmp, iqmp.access());
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

