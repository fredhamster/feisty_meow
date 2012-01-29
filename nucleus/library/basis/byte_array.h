#ifndef BYTE_ARRAY_CLASS
#define BYTE_ARRAY_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : byte_array                                                        *
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

#include "array.h"
#include "base_string.h"
#include "contracts.h"
#include "definitions.h"

#include <string.h>  // for memcmp.

namespace basis {

//! A very common template for a dynamic array of bytes.
/*!
  byte_array provides a simple wrapper around array<byte>, but with the
  exponential growth and simple copy modes automatically enabled.
  Note that it is almost always best to forward declare byte_arrays in ones
  own headers rather than including this header.
*/

class byte_array : public array<abyte>, public virtual orderable
{
public:
  byte_array(int number = 0, const abyte *initial_contents = NIL)
      : array<abyte>(number, initial_contents, SIMPLE_COPY | EXPONE) {}
    //!< constructs an array of "number" bytes from "initial_contents".

  byte_array(const byte_array &to_copy)
      : root_object(), array<abyte>(to_copy) {}
    //!< constructs an array bytes by copying the "to_copy" array.

  byte_array(const array<abyte> &to_copy) : array<abyte>(to_copy) {}
    //!< constructs an array bytes by copying the "to_copy" array.

  virtual ~byte_array() {}

  DEFINE_CLASS_NAME("byte_array");

  //!< returns an array of zero bytes.
  /*!< note that this is implemented in the opsystem library to avoid bad
  issues with static objects mixed into multiple dlls from a static
  library. */
  static const byte_array &empty_array()
      { static byte_array g_empty; return g_empty; }

  // these implement the orderable and equalizable interfaces.
  virtual bool equal_to(const equalizable &s2) const {
    const byte_array *s2_cast = dynamic_cast<const byte_array *>(&s2);
    if (!s2_cast) throw "error: byte_array::==: unknown type";
    return comparator(*s2_cast) == 0;
  }
  virtual bool less_than(const orderable &s2) const {
    const byte_array *s2_cast = dynamic_cast<const byte_array *>(&s2);
    if (!s2_cast) throw "error: byte_array::<: unknown type";
    return comparator(*s2_cast) < 0;
  }

  int comparator(const byte_array &s2) const { 
    return memcmp(observe(), s2.observe(), length());
  }
};

//////////////

//! A base class for objects that can pack into an array of bytes.
/*!
  A packable is an abstract object that represents any object that can
  be transformed from a potentially deep form into an equivalent flat
  form.  The flat form is a simple extent of memory stored as bytes.
*/

class packable : public virtual root_object
{
public:
  virtual void pack(byte_array &packed_form) const = 0;
    //!< Creates a packed form of the packable object in "packed_form".
    /*!< This must append to the data in "packed_form" rather than clearing
    prior contents. */

  virtual bool unpack(byte_array &packed_form) = 0;
    //!< Restores the packable from the "packed_form".
    /*!< This object becomes the unpacked form, and therefore must lose any of
    its prior contents that depend on the data in "packed_form".  This is up to
    the derived unpack function to figure out.  The "packed_form" is modified
    by extracting all of the pieces that are used for this object; the
    remainder stays in "packed_form".  true is returned if the unpacking was
    successful. */

  virtual int packed_size() const = 0;
    //!< Estimates the space needed for the packed structure.
};

//////////////

// the two templates below can be used to add or remove objects from an array
// of bytes.  NOTE: the functions below will only work with objects that are
// already platform-independent.  it's better to make structures packable by
// using the attach and detach functions in the "packable" library.

//! attach_flat() places a copy of "attachment" onto the array of bytes.
template <class contents>
void attach_flat(byte_array &target, const contents &attachment)
{ target.concatenate(byte_array(sizeof(attachment), (abyte *)&attachment)); }

//! detach_flat() pulls the "detached" object out of the array of bytes.
template <class contents>
bool detach_flat(byte_array &source, contents &detached)
{
  if (sizeof(detached) > source.length()) return false;
  detached = *(contents *)source.observe();
  source.zap(0, sizeof(detached) - 1);
  return true;
}

} // namespace.

#endif

