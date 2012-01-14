#ifndef ROTATING_BYTE_HASHER_CLASS
#define ROTATING_BYTE_HASHER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : rotating_byte_hasher                                              *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2001-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "byte_hasher.h"
#include "checksums.h"
#include "hash_table.h"

#include <basis/definitions.h>

namespace structures {

//! Implements a hashing algorithm based on the contents stored in an object.
/*!
  This will only be usable for key types that have flat members; keys with
  pointers limit the meaning of the hash value, or destroy the meaning if the
  pointer value can change between lookups.  Note that objects based on RTTI
  will probably never work with this either since the compiler stores extra
  data as part of the binary form for those objects.
*/

class rotating_byte_hasher : public virtual hashing_algorithm
{
public:
  virtual ~rotating_byte_hasher() {}

  virtual basis::un_int hash(const void *key_data, int key_length) const
      { return checksums::hash_bytes(key_data, key_length); }
    //!< returns a value that can be used for indexing into a hash table.
    /*!< the returned value is loosely based on the "key_data" and the
    "key_length" we are provided with.  note: do not use a huge key length
    for this or your hash table will be very slow; the key should probably
    be limited to 16 or less. */

  virtual hashing_algorithm *clone() const
      { return new rotating_byte_hasher; }
    //!< implements cloning of the algorithm object.
};

} //namespace.

#endif

