#ifndef TWOFISH_CRYPTO_CLASS
#define TWOFISH_CRYPTO_CLASS

/*
* Name   : twofish_crypto
* Author : Chris Koeritz
* Purpose: provides twofish encryption using the openssl libraries.
****
* Copyright (c) 2005-$now By Author.  This program is free software; you can
* redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either version 2 of
* the License or (at your option) any later version.  This is online at:
*     http://www.fsf.org/copyleft/gpl.html
* Please send any updates to: fred@gruntose.com
*/

#include <basis/byte_array.h>
#include <basis/contracts.h>

#include "cryptical_envelopment.h"

namespace crypto {

class twofish_crypto : public cryptical_envelopment
{
public:
  twofish_crypto(int key_size);
    //!< this will create a new random key of the "key_size", in bits.
    /*!< the valid sizes are from 128 bits to 256 bits.  AES apparently only
    requires that 128, 192 and 256 actually work, but the intervening key
    sizes should be fine also. */

  twofish_crypto(const basis::byte_array &key, int key_size);
    //!< uses a pre-existing "key".

  twofish_crypto(const twofish_crypto &to_copy);  //!< copy constructor.

  virtual ~twofish_crypto();

  twofish_crypto &operator = (const twofish_crypto &to_copy);

  DEFINE_CLASS_NAME("twofish_crypto");

  // twofish relevant values for appropriate key sizes.
  virtual int minimum_key_size_in_bits() const { return static_minimum_key_size_in_bits(); }
  virtual int maximum_key_size_in_bits() const { return static_maximum_key_size_in_bits(); }
  static int static_minimum_key_size_in_bits() { return 92; }
    /*
     note that the lower bound above was discovered for openssl library through experimentation,
     and is not officially documented for the twofish algorithm anywhere we can find (yet).
     really contradictory results happened with lower key sizes, where many encrypt/decrypt cycles
     would succeed on the same key length but then would trigger an 'invalid key length' error.
     not the kind of deterministic behavior we might expect.
    */
  static int static_maximum_key_size_in_bits() { return 256; }
};

} //namespace.

#endif

