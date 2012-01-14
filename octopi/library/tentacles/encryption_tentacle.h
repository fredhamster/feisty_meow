#ifndef ENCRYPTION_TENTACLE_CLASS
#define ENCRYPTION_TENTACLE_CLASS

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

#include "encryption_infoton.h"

#include <octopus/tentacle_helper.h>

namespace octopi {

// forward.
class key_repository;

//! Processes the encryption_infoton object for setting up an encrypted channel.

/*!
  NOTE:
      to use encryption, both the client and the server need to have an
  encryption_tentacle added as a filter.  it should be the first filter
  added by users and it must be before any security tentacles (otherwise,
  the security info would not be encrypted).
  further, an unwrapping_tentacle (see encryption_wrapper.h) must also
  be added.  it must *not* be added as a filter.  this is what allows the
  octopus to reconstitute the encoded infotons when encryption is active.
*/

class encryption_tentacle
: public tentacle_helper<encryption_infoton>
{
public:
  encryption_tentacle();
    //!< this tentacle will implement the server side.
    /*!< it will expect only to see public keys from clients and to respond
    with encrypted blowfish keys. */

  encryption_tentacle(const basis::byte_array &rsa_key);
    //!< this is the client side tentacle.
    /*!< it will only deal with unwrapping a server's response with the
    encrypted blowfish key.  the "rsa_key" is the private key that will be
    used for decrypting the key response. */

  encryption_tentacle(int key_size);
    //!< automatically creates a private key of the "key_size".
    /*!< this is for use by the client side's encryption needs. */

  virtual ~encryption_tentacle();

  DEFINE_CLASS_NAME("encryption_tentacle");

  virtual basis::outcome reconstitute(const structures::string_array &classifier,
          basis::byte_array &packed_form, infoton * &reformed);
    //!< recreates a "reformed" infoton from a packed form.
    /*!< the "classifier" is provided as well as the packed infoton data
    in "packed_form".  this will only succeed if the classifier's first name
    is understood here. */

  virtual basis::outcome consume(infoton &to_chow, const octopus_request_id &item_id,
          basis::byte_array &transformed);
    //!< the base class handles the processing of the request in "to_chow".
    /*!< it will generally perform all the services needed to start
    the encrypted connection up.  the "transformed" array will be filled
    with the actual infoton if decryption is successful.  if the outcome
    is ENCRYPTION_MISMATCH, then the infoton is not encrypted but was
    expected to be. */

  virtual void expunge(const octopus_entity &to_remove);
    //!< throws out any keys we were maintaining for this entity.

  key_repository &keys() const;
    //!< provides access to our list of keys.
    /*!< this is very private info, but it's needed for encrypting items
    going back to the client. */

  const crypto::rsa_crypto &private_key() const;
    //!< provides access to the key held here.
    /*!< this is an important object; do not expose it externally. */

private:
  bool _server_side;  //!< true if we're acting as a server.
  key_repository *_keys;  //!< our table of keys that we've agreed on.
  crypto::rsa_crypto *_rsa_private;  //!< the private key for a client side.
};

} //namespace.

#endif  // outer guard.

