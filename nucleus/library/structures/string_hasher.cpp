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

#include "string_hasher.h"

#include <basis/functions.h>
#include <basis/astring.h>

using namespace basis;

namespace structures {

const int MAX_STRING_CHARS_USED = 3;
  // we use this many characters from the string in question (if they
  // exist) at each end of the string.

//////////////

hashing_algorithm *string_hasher::clone() const
{ return new string_hasher; }

basis::un_int string_hasher::hash(const void *key_data, int key_length_in) const
{
  if (!key_data) return 0;  // error!
  if (key_length_in <= 1) return 0;  // ditto!

  abyte *our_key = (abyte *)key_data;
  abyte hashed[4] = { 0, 0, 0, 0 };

  int key_length = minimum(key_length_in - 1, MAX_STRING_CHARS_USED);

  int fill_posn = 0;
  // add the characters from the beginning of the string.
  for (int i = 0; i < key_length; i++) {
    // add to the primary area.
    hashed[fill_posn] = hashed[fill_posn] + our_key[i];
    fill_posn++;
    if (fill_posn >= 4) fill_posn = 0;
    // add to the secondary area (the next in rotation after primary).
    hashed[fill_posn] = hashed[fill_posn] + (our_key[i] / 4);
  }
  // add the characters from the end of the string.
  for (int k = key_length_in - 2;
      (k >= 0) && (k >= key_length_in - 1 - key_length); k--) {
    // add to the primary area.
    hashed[fill_posn] = hashed[fill_posn] + our_key[k];
    fill_posn++;
    if (fill_posn >= 4) fill_posn = 0;
    // add to the secondary area (the next in rotation after primary).
    hashed[fill_posn] = hashed[fill_posn] + (our_key[k] / 4);
  }
  
  basis::un_int to_return = 0;
  for (int j = 0; j < 4; j++) to_return = (to_return << 8) + hashed[j];
  return to_return;
}

//////////////

hashing_algorithm *astring_hasher::clone() const
{ return new astring_hasher; }

basis::un_int astring_hasher::hash(const void *key_data, int key_length_in) const
{
  if (!key_data) return 0;  // error.
  const astring *real_key = (const astring *)key_data;
  if (real_key->length() + 1 != key_length_in) {
//    printf("differing key lengths, string len=%d, key len=%d\n",
//        real_key->length() + 1, key_length_in);
  }
  return string_hasher().hash((const void *)real_key->observe(),
      real_key->length() + 1);
}

} //namespace.

