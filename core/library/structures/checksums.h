#ifndef CHECKSUMS_GROUP
#define CHECKSUMS_GROUP

/*****************************************************************************\
*                                                                             *
*  Name   : checksums group                                                   *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/definitions.h>

/* @file checksums.h
  A collection of useful checksum calculators.
*/

namespace structures {

class checksums
{
public:

  static basis::abyte byte_checksum(const basis::abyte *data, int length);
    //!< simple byte-sized checksum based on additive roll-over.

  static basis::un_int short_checksum(const basis::abyte *data, int length);
    //!< simple shorty checksum based on additive roll-over.

  static basis::un_short fletcher_checksum(const basis::abyte *data, int length);
    //!< A positionally computed error detection value.

  static basis::un_short rolling_fletcher_checksum(basis::un_short previous, const basis::abyte *data, int len);
    //!< Fletcher checksums applied to streaming data.
    /*!< this is not strictly a fletcher checksum, but it uses the normal
    fletcher checksum on the specified data and XORs it with the "previous"
    value of the checksum.  this leads to a regenerable number that should
    always be the same if done on the same data using the same chunking
    factor (the "len"), although of course the last piece of data does not
    have to be "len" bytes. */

  static unsigned int bizarre_checksum(const basis::abyte *data, int length);
    //!< A different type of checksum with somewhat unknown properties.
    /*!< It attempts to be incorporate positioning of the bytes. */

  static basis::un_int hash_bytes(const void *key_data, int key_length);
    //!< returns a value that can be used for indexing into a hash table.
    /*!< the returned value is loosely based on the "key_data" and the
    "key_length" we are provided with. */
};

} //namespace.

#endif

