/*****************************************************************************\
*                                                                             *
*  Name   : checksums group                                                   *
*  Authors: Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "checksums.h"

#include <basis/definitions.h>

using namespace basis;

namespace structures {

const int HIGHEST_MOD_VALUE = 32014;

unsigned int checksums::bizarre_checksum(const abyte *data, int length)
{
  int sum = 0;
  for (int i = 0; i < length; i++) sum += data[i] % 23 + i % 9;
  sum = (sum % (HIGHEST_MOD_VALUE - 1)) + 1;
  return (unsigned int)sum;
}

// fletcher checksum is from Dr. Dobbs Journal May 1992, pp. 32-38.

basis::un_short checksums::fletcher_checksum(const abyte *data, int length)
{
  int sum1 = 0;
  basis::un_int sum2 = 0;
  const abyte *buffer = data;

  while (length--) {
    sum1 += int(*buffer++);
    // check for overflow into high byte.
    if (sum1 > 255) sum1++;
    sum1 &= 0xFF;  // remove any bits in high byte for next sum.
    sum2 += basis::un_int(sum1);
  }
  if (sum1 == 255) sum1 = 0;
  unsigned int fletch = basis::un_int(sum2 & 0xFF);
  fletch <<= 8;
  fletch |= basis::un_int(sum1 & 0xFF);

  return basis::un_short(fletch);
}

basis::un_short checksums::rolling_fletcher_checksum(basis::un_short previous, const abyte *data,
    int len)
{ return previous ^ fletcher_checksum(data, len); }

abyte checksums::byte_checksum(const abyte *data, int length)
{
  abyte to_return = 0;
  for (int i = 0; i < length; i++) to_return += abyte(data[i]);
  return to_return;
}

basis::un_int checksums::short_checksum(const abyte *data, int length)
{
  basis::un_int to_return = 0;
  for (int i = 0; i < length; i++) to_return += data[i];
  return to_return;
}

basis::un_int checksums::hash_bytes(const void *key_data, int key_length)
{
  if (!key_data) return 0;  // error!
  if (!key_length) return 0;  // ditto!

  abyte *our_key = (abyte *)key_data;
  abyte hashed[4] = { 0, 0, 0, 0 };

  int fill_posn = 0;
  for (int i = 0; i < key_length; i++) {
    // add to the primary area.
    hashed[fill_posn] = hashed[fill_posn] + our_key[i];
    fill_posn++;
    if (fill_posn >= 4) fill_posn = 0;
    // add to the secondary area (the next in rotation after primary).
    hashed[fill_posn] = hashed[fill_posn] + (our_key[i] / 4);
  }

  basis::un_int to_return = 0;
  for (int j = 0; j < 4; j++) to_return = (to_return << 8) + hashed[j];
  return to_return;
}

} //namespace.

