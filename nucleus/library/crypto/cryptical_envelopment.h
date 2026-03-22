#ifndef CRYPTICAL_ENVELOPMENT_CLASS
#define CRYPTICAL_ENVELOPMENT_CLASS

/*
*  Name   : cryptical_envelopment
*  Author : Chris Koeritz
*
*  Purpose: Implements the majority of encryption processing using the
*  OpenSSL EVP methods, aka the digital envelope routines.
*****
* Copyright (c) 2005-$now By Author.  This program is free software; you can
* redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either version 2 of
* the License or (at your option) any later version.  This is online at:
*     http://www.fsf.org/copyleft/gpl.html
* Please send any updates to: fred@gruntose.com
*/

#include <basis/byte_array.h>
#include <basis/contracts.h>

// forward.
struct evp_cipher_st;
typedef struct evp_cipher_st EVP_CIPHER;

namespace crypto {

class cryptical_envelopment : public virtual basis::root_object
{
public:
  cryptical_envelopment(const EVP_CIPHER *cipher_type);
    //!< constructor requires the type of encryption to use.
  cryptical_envelopment(const cryptical_envelopment &to_copy);  //!< copy constructor.

  virtual ~cryptical_envelopment();

  cryptical_envelopment &operator = (const cryptical_envelopment &to_copy);

  bool set_key(int key_size);
    //!< this will create a new random key of the "key_size", in bits.
    /*!< the valid sizes for keys depend on the algorithm in question. */

  bool set_key(const basis::byte_array &new_key, int key_size);
    //!< sets the encryption key to "new_key" with a "key_size" in bits.

  DEFINE_CLASS_NAME("cryptical_envelopment");

  int key_size() const;  // returns the size of our key, in bits.

  // derived classes must provide the min and max for key sizes.
  virtual int minimum_key_size_in_bits() const = 0;
    //!< returns the minimum key size in bits supported here.
  virtual int maximum_key_size_in_bits() const = 0;
    //!< returns the maximum key size in bits supported here.

  const basis::byte_array &get_key() const;  //!< returns our current key.

  bool generate_key(int size, basis::byte_array &new_key);
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
    (currently named cryptical_envelopment.cpp). */

private:
  int _key_size;  //!< number of bits in the key.
  basis::byte_array *_key;  //!< our secret key.
  const EVP_CIPHER *_cipher_type;  //!< what kind of encryption are we using?
};

} //namespace.

#endif

