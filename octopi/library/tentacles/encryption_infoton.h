#ifndef ENCRYPTION_INFOTON_CLASS
#define ENCRYPTION_INFOTON_CLASS

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

#include <crypto/blowfish_crypto.h>
#include <crypto/rsa_crypto.h>
#include <octopus/entity_defs.h>
#include <octopus/infoton.h>

namespace octopi {

//! Encapsulates the chit-chat necessary to establish an encrypted connection.
/*!
  This is framed in terms of a client and a server, where the client creates
  a private key and gives the server the public key.  The server side creates
  a blowfish key and encrypts it using the public key.
*/

class encryption_infoton : public infoton
{
public:
  basis::byte_array _public_key;
    //!< valid during the request stage of encryption.
    /*!< this is used when the client is telling the server how to talk to
    it to provide the key. */
  basis::byte_array _encrypted_blowfish_key;
    //!< valid during the response stage of encryption.
    /*!< this is used when the server reports a blowfish key that it will
    use on this connection with the client. */

  basis::outcome _success;  //!< did the request succeed?

  encryption_infoton(const basis::byte_array &public_key = basis::byte_array::empty_array(),
          const basis::byte_array &encrypted_blowfish_key = basis::byte_array::empty_array());
  encryption_infoton(const encryption_infoton &to_copy);

  virtual ~encryption_infoton();

  DEFINE_CLASS_NAME("encryption_infoton");

  static const int RSA_KEY_SIZE;
    //!< this key size should be used for all RSA private keys.
  static const int BLOWFISH_KEY_SIZE;
    //!< this will be used for blowfish keys that this object generates.

  void text_form(basis::base_string &fill) const {
    fill.assign(basis::astring(class_name()));  // low exposure for vital held info.
  }

  encryption_infoton &operator =(const encryption_infoton &to_copy);

  basis::outcome prepare_blowfish_key(crypto::blowfish_crypto &new_key);
    //!< performs the server side's job on the current key.
    /*!< the public key had better be set already or this will fail.  the
    "new_key" will always be used to communicate with the client after this.
    */

  basis::outcome prepare_public_key(const crypto::rsa_crypto &private_key);
    //!< prepares the request side for a client.
    /*!< the rsa public key will be generated from the "private_key". */

  basis::outcome prepare_both_keys(crypto::rsa_crypto &private_key);
    //!< sets up both keys by randomly generating the "private_key".

  basis::outcome extract_response(const crypto::rsa_crypto &private_key,
          crypto::blowfish_crypto &new_key) const;
    //!< used by the client to extract the shared blowfish key from the server.
    /*!< using the private key, the server's response is decrypted and stored
    in "new_key".  note that this will only succeed if the _success member
    is OKAY.  otherwise it means the server has beefed on the request. */

  static const structures::string_array &encryption_classifier();
    //!< returns the classifier for this type of infoton.

  virtual void pack(basis::byte_array &packed_form) const;
  virtual bool unpack(basis::byte_array &packed_form);

  virtual clonable *clone() const;

  virtual int packed_size() const;
};

} //namespace.

#endif  // outer guard.

