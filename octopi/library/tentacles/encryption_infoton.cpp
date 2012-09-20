/*****************************************************************************\
*                                                                             *
*  Name   : encryption_infoton                                                *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2004-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "encryption_infoton.h"

#include <basis/byte_array.h>

#include <basis/mutex.h>
#include <basis/functions.h>
#include <crypto/blowfish_crypto.h>
#include <crypto/rsa_crypto.h>
#include <octopus/tentacle.h>
#include <structures/static_memory_gremlin.h>
#include <textual/byte_formatter.h>

using namespace basis;
using namespace crypto;
using namespace octopi;
using namespace structures;
using namespace textual;

namespace octopi {

const int encryption_infoton::BLOWFISH_KEY_SIZE = 314;
  // our key size is almost double the recommended key size (168 bits).
  // this would take a very long time to crack using brute force.

const int encryption_infoton::RSA_KEY_SIZE = 1480;
  // a little bit larger than the 1024 bit threshold.

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s);

encryption_infoton::encryption_infoton(const byte_array &pub_key,
    const byte_array &secret_blowfish)
: infoton(encryption_classifier()),
  _public_key(pub_key),
  _encrypted_blowfish_key(secret_blowfish),
  _success(tentacle::NOT_FOUND)
{}

encryption_infoton::encryption_infoton(const encryption_infoton &to_copy)
: root_object(),
  infoton(to_copy),
  _public_key(to_copy._public_key),
  _encrypted_blowfish_key(to_copy._encrypted_blowfish_key),
  _success(to_copy._success)
{
}

encryption_infoton::~encryption_infoton() {}

clonable *encryption_infoton::clone() const
{ return cloner<encryption_infoton>(*this); }

encryption_infoton &encryption_infoton::operator =
    (const encryption_infoton &to_copy)
{
  if (this == &to_copy) return *this;
  _public_key = to_copy._public_key;
  _encrypted_blowfish_key = to_copy._encrypted_blowfish_key;
  _success = to_copy._success;
  return *this;
}

const char *encryption_class_constant = "#octcod";

SAFE_STATIC_CONST(string_array, encryption_infoton::encryption_classifier,
    (1, &encryption_class_constant))

int encryption_infoton::packed_size() const
{
  return sizeof(int)  // packed outcome.
      + _public_key.length() + sizeof(int)  // public key array.
      + _encrypted_blowfish_key.length() + sizeof(int);  // secret key array.
}

void encryption_infoton::pack(byte_array &packed_form) const
{
  structures::attach(packed_form, _success.value());
  structures::attach(packed_form, _public_key);
  structures::attach(packed_form, _encrypted_blowfish_key);
}

bool encryption_infoton::unpack(byte_array &packed_form)
{
  int value;
  if (!structures::detach(packed_form, value)) return false;
  _success = outcome(value);
  if (!structures::detach(packed_form, _public_key)) return false;
  if (!structures::detach(packed_form, _encrypted_blowfish_key)) return false;
  return true;
}

outcome encryption_infoton::prepare_blowfish_key(blowfish_crypto &new_key)
{
  FUNCDEF("prepare_blowfish_key");
  _encrypted_blowfish_key.reset();  // clean out stuff to create.
  if (!_public_key.length()) {
    // wrong type of request being seen or something.
    _success = tentacle::BAD_INPUT;
    return _success;
  }

  rsa_crypto pub(_public_key);  // suck in the provided key.
  blowfish_crypto agreed_key(BLOWFISH_KEY_SIZE);  // random blowfish key.
  new_key = agreed_key;

  // now encrypt the new key for transit.
  bool worked = pub.public_encrypt(agreed_key.get_key(),
      _encrypted_blowfish_key);
  if (!worked) _success = tentacle::GARBAGE;  // lacking a better description.
  else _success = tentacle::OKAY;
  return _success;
}

outcome encryption_infoton::prepare_both_keys(rsa_crypto &private_key)
{
  rsa_crypto priv(RSA_KEY_SIZE);  // generate random key.
  outcome to_return = prepare_public_key(priv);
  if (to_return == tentacle::OKAY) private_key = priv;
  return to_return;
}

outcome encryption_infoton::prepare_public_key(const rsa_crypto &private_key)
{
  bool worked = private_key.public_key(_public_key);
  if (!worked) return tentacle::DISALLOWED;  // why would that ever fail?
  return tentacle::OKAY;
}

outcome encryption_infoton::extract_response(const rsa_crypto &private_key,
    blowfish_crypto &new_key) const
{
  FUNCDEF("extract_response");
  if (_success != tentacle::OKAY) return _success;
  byte_array decrypted;
  bool worked = private_key.private_decrypt(_encrypted_blowfish_key, decrypted);
  if (!worked) return tentacle::BAD_INPUT;  // that one we hope is accurate.
  new_key.set_key(decrypted, BLOWFISH_KEY_SIZE);
  return tentacle::OKAY;
}

} //namespace.

