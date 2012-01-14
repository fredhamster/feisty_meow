#ifndef STRING_HASHER_CLASS
#define STRING_HASHER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : string_hasher                                                     *
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

#include "hash_table.h"

namespace structures {

//! Implements a simple hashing algorithm for strings.
/*! This uses a portion of the string's contents to create a hash value. */

class string_hasher : public virtual hashing_algorithm
{
public:
  virtual basis::un_int hash(const void *key_data, int key_length) const;
    //!< returns a value that can be used to index into a hash table.
    /*!< the returned value is loosely based on the "key_data" and the
    "key_length" we are provided with.  it is expected that the "key_data"
    really is a 'char' pointer whose length is "key_length" (including the
    zero terminator at the end). */

  virtual hashing_algorithm *clone() const;
    //!< implements cloning of the algorithm object.
};

//////////////

class astring_hasher : public virtual hashing_algorithm
{
public:
  virtual basis::un_int hash(const void *key_data, int key_length) const;
    //!< similar to string_hasher, but expects "key_data" as an astring pointer.

  virtual hashing_algorithm *clone() const;
    //!< implements cloning of the algorithm object.
};

} //namespace.

#endif

