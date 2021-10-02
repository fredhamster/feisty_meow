#ifndef FUNCTIONS_GROUP
#define FUNCTIONS_GROUP

/*****************************************************************************\
*                                                                             *
*  Name   : functions                                                         *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

/*! @file functions.h
  Provides a set of useful mini-functions.
*/

#include "definitions.h"

namespace basis {

template <class type> type maximum(type a, type b)
        { return (a > b)? a : b; }
  //!< minimum returns the lesser of two values.
template <class type> type minimum(type a, type b)
        { return (a < b)? a : b; }
  //!< maximum returns the greater of two values.

template <class type> type absolute_value(type a)
        { return (a >= 0)? a : -a; }
  //!< Returns a if a is non-negative, and returns -a otherwise.

//////////////

template <class type> bool positive(const type &a) { return a > 0; }
  //!< positive returns true if "a" is greater than zero, or false otherwise.
template <class type> bool non_positive(const type a) { return a <= 0; }
  //!< non_positive returns true if "a" is less than or equal to zero.
template <class type> bool negative(const type &a) { return a < 0; }
  //!< negative returns true if "a" is less than zero.
template <class type> bool non_negative(const type &a) { return a >= 0; }
  //!< non_negative returns true if "a" is greater than or equal to zero.

//////////////

// the following comparisons are borrowed from the STL.  they provide the full set of comparison
// operators for any object that implements the equalizable and orderable base classes.
template <class T1, class T2>
bool operator != (const T1 &x, const T2 &y) { return !(x == y); }

template <class T1, class T2>
bool operator > (const T1 &x, const T2 &y) { return y < x; }

template <class T1, class T2>
bool operator <= (const T1 &x, const T2 &y) { return !(y < x); }

template <class T1, class T2>
bool operator >= (const T1 &x, const T2 &y) { return !(x < y); }

//////////////

//! dynamically converts a type to a target type, or throws an exception if it cannot.
template <class target_type, class source_type>
target_type *cast_or_throw(source_type &to_cast, const target_type &ignored)
{
//  if (!&ignored) {}  // do nothing.
  target_type *cast = dynamic_cast<target_type *>(&to_cast);
  if (!cast) throw "error: casting problem, unknown RTTI cast.";
  return cast;
}

//! const version of the cast_or_throw template.
template <class target_type, class source_type>
const target_type *cast_or_throw(const source_type &to_cast, const target_type &ignored)
{
// if (!&ignored) {}  // do nothing.
  const target_type *cast = dynamic_cast<const target_type *>(&to_cast);
  if (!cast) throw "error: casting problem, unknown RTTI cast.";
  return cast;
}

//////////////

template <class type> bool range_check(const type &c, const type &low,
    const type &high) { return (c >= low) && (c <= high); }
  //!< Returns true if "c" is between "low" and "high" inclusive.

template <class type> type square(const type &a) { return a * a; }
  //!< Returns the square of the object (which is a * a).

template <class type> void flip_increasing(type &a, type &b)
      { if (b < a) { type tmp = a; a = b; b = tmp; } }
  //!< Makes sure that two values are in increasing order (a < b).

template <class type> void flip_decreasing(type &a, type &b)
      { if (b > a) { type tmp = a; a = b; b = tmp; } }
  //!< Makes sure that two values are in decreasing order (a > b).

template <class type> void swap_values(type &a, type &b)
      { type tmp = a; a = b; b = tmp; }
  //!< Exchanges the values held by "a" & "b".

template <class type> type sign(type a)
      { if (a < 0) return -1; else if (a > 0) return 1; else return 0; }
  //!< Returns the numerical sign of a number "a".

//////////////

// helpful coding / debugging macros:

//! deletion with clearing of the pointer.
/*! this function simplifies the two step process of deleting a pointer and
then clearing it to NULL_POINTER.  this makes debugging a bit easier since an access
of NULL_POINTER should always cause a fault, rather than looking like a possibly
valid object. */
template<class contents>
void WHACK(contents * &ptr) { if (ptr) { delete ptr; ptr = NULL_POINTER; } }

//! Returns an object that is defined statically.
/*! Thus the returned object will never be recreated once this function
is called within the same scope of memory (within a dynamic library or
application).  This is useful for templates that want to have access to a
bogus element whose contents don't matter.  NOTE: bogonic is not
thread safe! */
template <class type> type &bogonic() {
   static type local_bogon;
   return local_bogon;
}

//////////////

template <class type>
type number_of_packets(type message_size, type packet_size)
{ return message_size / packet_size + ((message_size % packet_size) != 0); }
  //!< Reports number of packets needed given a total size and the packet size.
  /*!< This returns the number of packets needed to contain a contiguous array
  of characters with size "message_size" when the number of characters
  per packet is "packet_size". */

template <class type>
type last_packet_size(type message_size, type packet_size)
{ return message_size % packet_size? message_size % packet_size : packet_size; }
  //!< Tells how many bytes are used within last packet.
  /*< The companion call to number_of_packets; it returns the size of the last
  packet in the sequence of packets, taking into account the special case
  where the message_size divides evenly. */

//////////////

} //namespace.

#endif

