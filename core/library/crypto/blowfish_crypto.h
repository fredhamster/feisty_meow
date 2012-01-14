#ifndef BLOWFISH_CRYPTO_CLASS
#define BLOWFISH_CRYPTO_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : blowfish encryption                                               *
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

namespace crypto {

//! Provides BlowFish encryption on byte_arrays using the OpenSSL package.

class blowfish_crypto : public virtual basis::root_object
{
public:
  blowfish_crypto(int key_size);
    //!< this will create a new random key of the "key_size", in bits.
    /*!< the valid sizes are from 64 bits to 448 bits (we are forcing a
    higher minimum than the published algorithm because we have found smaller
    keys to be unreliable during decryption.  keys of 168 bits and larger
    should be very secure.  it is said that if a billion computers each tried
    a billion keys a second, then a 168 bit key would take 10 * 10^24 years
    to break (using brute force).  this is essentially unbreakable since the
    age of the universe is only 10 * 10^9 years so far. */

  blowfish_crypto(const basis::byte_array &key, int key_size);
    //!< uses a pre-existing "key".

  blowfish_crypto(const blowfish_crypto &to_copy);  //!< copy constructor.

  virtual ~blowfish_crypto();

  blowfish_crypto &operator = (const blowfish_crypto &to_copy);

  DEFINE_CLASS_NAME("blowfish_crypto");

  int key_size() const;  // returns the size of our key, in bits.

  static int minimum_key_size();
    //!< returns the minimum key size (in bits) supported here.
  static int maximum_key_size();
    //!< returns the maximum key size (in bits) supported here.

  const basis::byte_array &get_key() const;  //!< returns our current key.

  bool set_key(const basis::byte_array &new_key, int key_size);
    //!< sets the encryption key to "new_key".

  static void generate_key(int size, basis::byte_array &new_key);
    //!< creates a "new_key" of the "size" (in bits) specified.

  bool encrypt(const basis::byte_array &source, basis::byte_array &target) const;
    //!< encrypts the "source" array into the "target" array.

  bool decrypt(const basis::byte_array &source, basis::byte_array &target) const;
    //!< decrypts the "target" array from the encrypted "source" array.

  // seldom-needed methods...

  static const basis::byte_array &init_vector();
    //!< returns the initialization vector that is used by this class.
    /*!< decryption of chunks that were encrypted by this class will require
    the same init vector as this function returns.  this is mainly provided
    for third-party applications that want to be able to decrypt interoperably
    with this class.  if you are creating such an application but for some
    reason cannot run this class in order to invoke this method, the vector
    is created by the algorithm in this class's implementation file
    (currently named blowfish_crypto.cpp). */

private:
  int _key_size;  //!< number of bits in the key.
  basis::byte_array *_key;  //!< our secret key.
};

} //namespace.

#endif

