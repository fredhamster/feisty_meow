#ifndef OBJECT_PACKERS_CLASS
#define OBJECT_PACKERS_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : object_packers                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1995-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/byte_array.h>
#include <basis/definitions.h>

namespace structures {

// the sizes in bytes of common objects.  if the compiler doesn't match these, there will
// probably be severe tire damage.
const int PACKED_SIZE_BYTE = 1;
const int PACKED_SIZE_INT16 = 2;
const int PACKED_SIZE_INT32 = 4;

// these functions pack and unpack popular data types.

void attach(basis::byte_array &packed_form, bool to_attach);
  //!< Packs a bool "to_attach" into "packed_form".
bool detach(basis::byte_array &packed_form, bool &to_detach);
  //!< Unpacks a bool "to_detach" from "packed_form".

void attach(basis::byte_array &packed_form, basis::abyte to_attach);
  //!< Packs a byte "to_attach" into "packed_form".
bool detach(basis::byte_array &packed_form, basis::abyte &to_detach);
  //!< Unpacks a byte "to_detach" from "packed_form".

int packed_size(const basis::byte_array &packed_form);
  //!< Reports the size required to pack a byte array into a byte array.
void attach(basis::byte_array &packed_form, const basis::byte_array &to_attach);
  //!< Packs a byte_array "to_attach" into "packed_form".
bool detach(basis::byte_array &packed_form, basis::byte_array &to_detach);
  //!< Unpacks a byte_array "to_detach" from "packed_form".

void attach(basis::byte_array &packed_form, char to_attach);
  //!< Packs a char "to_attach" into "packed_form".
bool detach(basis::byte_array &packed_form, char &to_detach);
  //!< Unpacks a char "to_detach" from "packed_form".

int packed_size(double to_pack);
  //!< Reports how large the "to_pack" will be as a stream of bytes.
void attach(basis::byte_array &packed_form, double to_pack);
  //!< Packs a double precision floating point "to_attach" into "packed_form".
bool detach(basis::byte_array &packed_form, double &to_unpack);
  //!< Unpacks a double precision floating point "to_attach" from "packed_form".

void attach(basis::byte_array &packed_form, float to_pack);
  //!< Packs a floating point "to_attach" into "packed_form".
bool detach(basis::byte_array &packed_form, float &to_unpack);
  //!< Unpacks a floating point "to_attach" from "packed_form".

void attach(basis::byte_array &packed_form, int to_attach);
  //!< Packs an integer "to_attach" into "packed_form".
  /*!< This method and the other simple numerical storage methods use a little
  endian ordering of the bytes.  They are platform independent with respect to
  endianness and will reassemble the number properly on any platform. */
bool detach(basis::byte_array &packed_form, int &to_detach);
  //!< Unpacks an integer "to_attach" from "packed_form".

void obscure_attach(basis::byte_array &packed_form, basis::un_int to_attach);
  //!< like the normal attach but shifts in some recognizable sentinel data.
  /*!< this is slightly more sure than a simple integer attachment.  it can
  be used to make sure upcoming data is probably a valid int. */
bool obscure_detach(basis::byte_array &packed_form, basis::un_int &to_detach);
  //!< shifts the number back and checks validity, false returned if corrupted.

/*
void attach(basis::byte_array &packed_form, long to_attach);
  //!< Packs a long integer "to_attach" into "packed_form".
bool detach(basis::byte_array &packed_form, long &to_detach);
  //!< Unpacks a long integer "to_attach" from "packed_form".
*/

void attach(basis::byte_array &packed_form, short to_attach);
  //!< Packs a short integer "to_attach" into "packed_form".
bool detach(basis::byte_array &packed_form, short &to_detach);
  //!< Unpacks a short integer "to_attach" from "packed_form".

void attach(basis::byte_array &packed_form, basis::un_int to_attach);
  //!< Packs an unsigned integer "to_attach" into "packed_form".
bool detach(basis::byte_array &packed_form, basis::un_int &to_detach);
  //!< Unpacks an unsigned integer "to_attach" from "packed_form".

/*
void attach(basis::byte_array &packed_form, basis::un_long to_attach);
  //!< Packs an unsigned long integer "to_attach" into "packed_form".
bool detach(basis::byte_array &packed_form, basis::un_long &to_detach);
  //!< Unpacks an unsigned long integer "to_attach" from "packed_form".
*/

void attach(basis::byte_array &packed_form, basis::un_short to_attach);
  //!< Packs an unsigned short integer "to_attach" into "packed_form".
bool detach(basis::byte_array &packed_form, basis::un_short &to_detach);
  //!< Unpacks an unsigned short integer "to_attach" from "packed_form".

//////////////

// helpful template functions for packing.

//! provides a way to pack any array that stores packable objects.
template <class contents>
void pack_array(basis::byte_array &packed_form, const basis::array<contents> &to_pack) {
  obscure_attach(packed_form, to_pack.length());
  for (int i = 0; i < to_pack.length(); i++) to_pack[i].pack(packed_form);
}

//! provides a way to unpack any array that stores packable objects.
template <class contents>
bool unpack_array(basis::byte_array &packed_form, basis::array<contents> &to_unpack) {
  to_unpack.reset();
  basis::un_int len;
  if (!obscure_detach(packed_form, len)) return false;
  basis::array<contents> swappy_array(len, NIL, to_unpack.flags());
    // we create an array of the specified length to see if it's tenable.
  if (!swappy_array.observe()) return false;  // failed to allocate.
  for (int i = 0; i < (int)len; i++) {
    if (!swappy_array[i].unpack(packed_form))
      return false;
  }
  // now that we've got exactly what we want, plunk it into the result array.
  swappy_array.swap_contents(to_unpack);
  return true;
}

//! provides space estimation for the objects to be packed.
template <class contents>
int packed_size_array(const basis::array<contents> &to_pack) {
  int to_return = sizeof(int) * 2;  // obscure version uses double int size.
  for (int i = 0; i < to_pack.length(); i++)
    to_return += to_pack[i].packed_size();
  return to_return;
}

//! Packs flat objects into an array of bytes.
/*! Similar to pack above, but operates on arrays with simple
objects that do not support functional pack and unpack. */
template <class contents>
void pack_simple(basis::byte_array &packed_form, const basis::array<contents> &to_pack) {
  obscure_attach(packed_form, to_pack.length());
  for (int i = 0; i < to_pack.length(); i++)
    attach(packed_form, to_pack[i]);
}

//! Unpacks flat objects from an array of bytes.
/*! Similar to unpack above, but operates on arrays with simple
objects that do not support functional pack and unpack. */
template <class contents>
bool unpack_simple(basis::byte_array &packed_form, basis::array<contents> &to_unpack) {
  to_unpack.reset();
  basis::un_int len;
  if (!obscure_detach(packed_form, len)) return false;
  basis::array<contents> swappy_array(len, NIL, to_unpack.flags());
  if (!swappy_array.observe()) return false;  // failed to allocate.
  for (int i = 0; i < len; i++) {
    if (!detach(packed_form, swappy_array[i]))
      return false;
  }
  swappy_array.swap_contents(to_unpack);
  return true;
}

} // namespace

#endif

