#ifndef STRING_HASH_CLASS
#define STRING_HASH_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : string_hash                                                       *
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
#include "string_hasher.h"

#include <basis/astring.h>

namespace structures {

//! Implements a hash table indexed on character strings.

template <class contents>
class string_hash : public hash_table<basis::astring, contents>
{
public:
  string_hash(int estimated_elements)
      : hash_table<basis::astring, contents>(astring_hasher(), estimated_elements) {}

  ~string_hash() {}
};

} //namespace.

#endif // outer guard.

