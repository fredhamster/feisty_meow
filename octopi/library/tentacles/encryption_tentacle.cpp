/*****************************************************************************\
*                                                                             *
*  Name   : encryption_tentacle                                               *
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

#include "encryption_tentacle.h"
#include "encryption_wrapper.h"
#include "key_repository.h"

#include <crypto/blowfish_crypto.h>
#include <crypto/rsa_crypto.h>
#include <loggers/program_wide_logger.h>
#include <structures/symbol_table.h>
#include <textual/byte_formatter.h>

using namespace basis;
using namespace crypto;
using namespace loggers;
using namespace structures;
using namespace textual;

namespace octopi {

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

#define DEBUG_ENCRYPTION_TENTACLE
  // uncomment for noisier code.

//////////////

encryption_tentacle::encryption_tentacle()
: tentacle_helper<encryption_infoton>
    (encryption_infoton::encryption_classifier(), false),
  _server_side(true),
  _keys(new key_repository),
  _rsa_private(NULL_POINTER)
{
}

encryption_tentacle::encryption_tentacle(const byte_array &private_key)
: tentacle_helper<encryption_infoton>
    (encryption_infoton::encryption_classifier(), false),
  _server_side(false),
  _keys(new key_repository),
  _rsa_private(new rsa_crypto(private_key))
{
}

encryption_tentacle::encryption_tentacle(int key_size)
: tentacle_helper<encryption_infoton>
    (encryption_infoton::encryption_classifier(), false),
  _server_side(false),
  _keys(new key_repository),
  _rsa_private(new rsa_crypto(key_size))
{
}

encryption_tentacle::~encryption_tentacle()
{
  WHACK(_rsa_private);
  WHACK(_keys);
}

key_repository &encryption_tentacle::keys() const { return *_keys; }

const rsa_crypto &encryption_tentacle::private_key() const
{ return *_rsa_private; }

outcome encryption_tentacle::reconstitute(const string_array &classifier,
    byte_array &packed_form, infoton * &reformed)
{
  if (classifier != encryption_infoton::encryption_classifier()) 
    return NO_HANDLER;

  return reconstituter(classifier, packed_form, reformed,
      (encryption_infoton *)NULL_POINTER);
}

void encryption_tentacle::expunge(const octopus_entity &formal(to_remove))
{
////  _keys->whack(to_remove);
//we need a better approach.  it seems there are places where an entity
//can get reused and it still expects its key to be present.
}

outcome encryption_tentacle::consume(infoton &to_chow,
    const octopus_request_id &item_id, byte_array &transformed)
{
  FUNCDEF("consume");
  transformed.reset();
  encryption_infoton *inf = dynamic_cast<encryption_infoton *>(&to_chow);
  if (!inf) {
    // this package is not explicitly an encryption infoton.  we need to
    // decrypt it using what we already know.

    encryption_wrapper *wrap = dynamic_cast<encryption_wrapper *>(&to_chow);
    if (!wrap) {
#ifdef DEBUG_ENCRYPTION_TENTACLE
//      LOG(astring("got a stray infoton that was not encrypted: ")
//          + to_chow.text_form());
#endif
      // this signals that we were expecting an encrypted package.
      return ENCRYPTION_MISMATCH;
    }

    octenc_key_record record;
    octenc_key_record *rec = _keys->lock(item_id._entity);
    if (!rec) {
#ifdef DEBUG_ENCRYPTION_TENTACLE
      LOG(astring("no key stored for entity ")
          + item_id._entity.mangled_form()
          + "; rejecting packet.");
#endif
      return DISALLOWED;
    }
    record = *rec;
    _keys->unlock(rec);

    byte_array decro;
    bool decrypts_properly = record._key.decrypt(wrap->_wrapped, decro);
    if (decrypts_properly) {
      // this package seems to be intact.  we need to reconstitute the
      // original infoton.
      transformed = decro;  // set the decrypted blob.
      return PARTIAL;
    }

#ifdef DEBUG_ENCRYPTION_TENTACLE
    LOG(astring("denying client ") + item_id._entity.mangled_form()
        + " due to erroneous decryption");
#endif

    // the infoton's client is not authorized; it needs to be dropped.
    return DISALLOWED;
  }

  // reaching here means this is explicitly an encryption startup request.

  if (!_server_side) {
    // client's side must track the key we were given for decryption.  we'll
    // use that from now on.
    blowfish_crypto new_key(blowfish_crypto::minimum_key_size());  // bogus.
    outcome ret = inf->extract_response(*_rsa_private, new_key);
    if (ret != OKAY) {
#ifdef DEBUG_ENCRYPTION_TENTACLE
      LOG(astring("client failed to process encrypted blowfish key for ")
          + item_id._entity.mangled_form());
#endif
    } else {
      _keys->add(item_id._entity, new_key);  // add our key for this guy.
    }
    // we do not store a copy of the infoton; it's just done now.
    return ret;
  } else {
    // server's side need to process a key request and send it back using
    // the public key the requester provided.
    blowfish_crypto agreed_key(blowfish_crypto::minimum_key_size());
      // initialized with junk.
    outcome worked = inf->prepare_blowfish_key(agreed_key);
    if (worked != OKAY) {
#ifdef DEBUG_ENCRYPTION_TENTACLE
      LOG(astring("server failed to encrypt blowfish key for ")
          + item_id._entity.mangled_form());
#endif
    } else {
      _keys->add(item_id._entity, agreed_key);  // add our key for this guy.
    }
  }

  if (!store_product(dynamic_cast<infoton *>(inf->clone()), item_id))
    return NO_SPACE;
  return OKAY;
}

} //namespace.

