#ifndef BLOWFISH_CRYPTO_CLASS
#define BLOWFISH_CRYPTO_CLASS

/*
*  Name   : borked_blowfish_crypto
*  Author : Chris Koeritz
*  Purpose: provides BlowFish encryption on byte_arrays using the OpenSSL package.
****
* Copyright (c) 2005-$now By Author.  This program is free software; you can
* redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either version 2 of
* the License or (at your option) any later version.  This is online at:
*     http://www.fsf.org/copyleft/gpl.html
* Please send any updates to: fred@gruntose.com
*/

/*
 * NOTE:
 * this class is currently failing with bad decrypt, which we actually blame the openssl
 * libraries for.  we have the same exact underlying usage of cryptical_envelopment class
 * to drive the EVP functions for our twofish implementation, and it never has that issue.
 * our blowfish implementation *used* to work fine, until openssl 3 or so.
 */

#include <basis/byte_array.h>
#include <basis/contracts.h>

#include "cryptical_envelopment.h"

namespace crypto {

class borked_blowfish_crypto : public cryptical_envelopment
{
public:
  borked_blowfish_crypto(int key_size);
    //!< this will create a new random key of the "key_size", in bits.
    /*!< the valid sizes are from 64 bits to 448 bits (we are forcing a
    higher minimum than the published algorithm because we have found smaller
    keys to be unreliable during decryption.  keys of 168 bits and larger
    should be very secure.  it is said that if a billion computers each tried
    a billion keys a second, then a 168 bit key would take 10 * 10^24 years
    to break (using brute force).  this is essentially unbreakable since the
    age of the universe is only 10 * 10^9 years so far. */

  borked_blowfish_crypto(const basis::byte_array &key, int key_size);
    //!< uses a pre-existing "key".

  borked_blowfish_crypto(const borked_blowfish_crypto &to_copy);  //!< copy constructor.

  virtual ~borked_blowfish_crypto();

  borked_blowfish_crypto &operator = (const borked_blowfish_crypto &to_copy);

  DEFINE_CLASS_NAME("borked_blowfish_crypto");

  // blowfish relevant values for appropriate key sizes.
  virtual int minimum_key_size_in_bits() const { return 64; }  // officially, this is 32.
  virtual int maximum_key_size_in_bits() const { return 448; }
};

} //namespace.

#endif

