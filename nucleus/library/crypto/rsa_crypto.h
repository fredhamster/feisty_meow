#ifndef RSA_CRYPTO_CLASS
#define RSA_CRYPTO_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : RSA public key encryption                                         *
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

#include <basis/byte_array.h>
#include <basis/contracts.h>

// forward.
struct rsa_st;

namespace crypto {

//! Supports public key encryption and decryption.
/*!
  This class uses the OpenSSL package's support for RSA encryption.
*/

class rsa_crypto : public virtual basis::nameable
{
public:
  rsa_crypto(int key_size);
    //!< constructs using a randomized private key of the "key_size".
    /*!< the "key_size" must be at least 1024 bits for acceptable security.
    smaller keys are considered insecure. */

  rsa_crypto(const basis::byte_array &key);
    //!< constructs with the specified "key" as our private key.
    /*!< the "key" is used for encryption rather than generating a random one.
    the key is only valid if it was created with this class.  also, if the key
    is a public key, then only the public_encryption and public_decryption
    methods will be available. */

  rsa_crypto(rsa_st *key);
    //!< starts with a pre-existing "key" in the low-level form.

  rsa_crypto(const rsa_crypto &to_copy);

  virtual ~rsa_crypto();

  const rsa_crypto &operator = (const rsa_crypto &to_copy);

  DEFINE_CLASS_NAME("rsa_crypto");

  bool set_key(basis::byte_array &key);
    //!< resets this object's key to "key".
    /*!< the key is only valid if this class created it.  note: the "key"
    is destructively consumed during the set method; do not pass in your
    only copy. */

  bool set_key(rsa_st *key);
    //!< sets our new "key".
    /*!< this must be a valid key created via the RSA algorithms. */

  bool check_key(rsa_st *key);
    //!< checks the RSA "key" provided for validity.

  bool public_encrypt(const basis::byte_array &source, basis::byte_array &target) const;
    //!< encrypts "source" using our public key and stores it in "target".
    /*!< public_encrypt and private_decrypt are a pair.  an untrusted user can
    encrypt with the public key and only the possessor of the private key
    should be able to decrypt it. */
  bool private_decrypt(const basis::byte_array &source, basis::byte_array &target) const;
    //!< decrypts "source" using our private key and stores it in "target".

  bool private_encrypt(const basis::byte_array &source, basis::byte_array &target) const;
    //!< encrypts "source" using our private key and stores it in "target".
    /*!< private_encrypt and public_decrypt are also a pair.  the trusted
    user with the private key can create encrypted chunks that anyone with
    the public key can decrypt. */
  bool public_decrypt(const basis::byte_array &source, basis::byte_array &target) const;
    //!< decrypts "source" using our public key and stores it in "target".

  bool public_key(basis::byte_array &pubkey) const;
    //!< makes a copy of the public key held here.
  bool private_key(basis::byte_array &privkey) const;
    //!< makes a copy of the private key held here.
    /*!< the private key should never be exposed to anyone else. */

  static rsa_st *generate_key(int key_size);
    //!< creates a random RSA key using the lower-level openssl methods.

private:
  rsa_st *_key;  //!< our internal key.
};

} //namespace.

#endif

