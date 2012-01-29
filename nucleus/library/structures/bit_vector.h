#ifndef BIT_VECTOR_CLASS
#define BIT_VECTOR_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : bit_vector                                                        *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1990-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <basis/definitions.h>

namespace structures {

//! An array of bits with operations for manipulating and querying individual bits.

class bit_vector
{
public:
  bit_vector();
    //!< creates a zero length bit_vector.

  bit_vector(int size, const basis::abyte *initial = NIL);
    //!< creates a bit_vector able to store "size" bits.
    /*!< if initial is NIL, the vector is initialized to zero.  otherwise, the
    bits are copied from "initial".  "initial" must be large enough for the
    copying to succeed. */

  bit_vector(const bit_vector &to_copy);

  ~bit_vector();

  bit_vector &operator =(const bit_vector &to_copy);

  int bits() const;
    //!< returns the number of bits in the vector.

  bool operator [] (int position) const;
    //!< returns the value of the bit at the position specified.

  bool on(int position) const;
    //!< returns true if the bit at "position" is set.

  bool off(int position) const;
    //!< returns true if the bit at "position" is clear.

  void set_bit(int position, bool value);
    //!< sets the bit at "position" to a particular "value".

  bool whole() const;
    //!< returns true if entire vector is set.

  bool empty() const;
    //!< returns true if entire vector is unset.

  int find_first(bool to_find) const;
    //!< Seeks the first occurrence of "to_find".
    /*!< locates the position at which a bit equal to to_find is located or it
    returns common::NOT_FOUND if no bit of that value is in the vector. */

  void light(int position);
    //!< sets the value of the bit at "position".

  void clear(int position);
    //!< clears the value of the bit at "position".

  void resize(int size);
    //!< Changes the size of the bit_vector to "size" bits.
    /*!< This keeps any bits that still fit.  Any new bits are set to zero. */

  void reset(int size);
    //!< resizes the bit_vector and clears all bits in it.

  bool compare(const bit_vector &that, int start, int stop) const;
    //!< true if "this" is the same as "that" between "start" and "stop".

  bool operator == (const bit_vector &that) const;
    //!< returns true if "this" is equal to "that".
    /*!< neither vector is changed.  if the vectors do not have the same size,
    false is returned. */

  basis::astring text_form() const;
    //!< prints a nicely formatted list of bits to a string.

  bit_vector subvector(int start, int end) const;
    //!< Extracts a chunk of the vector between "start" and "end".
    /*!< Returns a bit_vector that is copied from this one starting at "start"
    and running until "end".  An empty bit vector is returned if the indices
    are out of range. */

  bool overwrite(int start, const bit_vector &to_write);
    //!< Stores bits from "to_write" into "this" at "start".
    /*!< overwrites the contents of this bit_vector with the contents of
    "to_write", beginning at "start" in this bit_vector.  true is returned
    for a successful write.  false will be returned if the "start" is out of
    range or if "to_write" is too large. */

  basis::un_int get(int start, int size) const;
    //!< gets a portion of the vector as an unsigned integer.
    /*!< returns an integer (as interpreted by the operating system) where the
    pattern of bits in that integer is identical to the bits in this
    bit_vector, beginning at "start" and including enough bits for an
    integer of "size" bits. */

  bool set(int start, int size, basis::un_int source);
    //!< puts the "source" value into the vector at "start".
    /*!< sets the pattern of bits in this bit_vector beginning at "start"
    identically to how they are stored in the integer "source", where the
    integer is expected to be using "size" of its bits.  the bits are copied
    starting from the low end of "source", where the operating system
    defines what the low end is.  true is returned for a successful copying. */

  operator const basis::byte_array &() const;
    //!< returns a copy of the low-level implementation of the bit vector.
    /*!< the first bit is stored at the bit in first byte, and so forth. */

private:
  basis::byte_array *_implementation;  //!< holds the real state of the bits.
  int _number_of_bits;  //!< the total number of bits possible in this vector.

  struct two_dim_location { int c_byte; int c_offset; };
    //!< a two-dimensional position given by byte number and offset within byte.

  two_dim_location into_two_dim(int position) const;
    /*!< turns a bit position in the vector into a two dimensional position
    of the byte number and a bit offset within that byte. */
  bool get_bit(const two_dim_location &pos_in2) const;
    //!< returns the value of the bit given its two dimensional location.
  void set_bit(const two_dim_location &pos_in2, bool value);
    //!< sets the value of the bit if "value", and clears it if not.
};

//////////////

// NOTE: these are operations on numbers, NOT on bit_vectors.

//! returns a number based on "to_modify" but with "bits" turned on.
template <class type>
void SET(type &to_modify, type bits) { to_modify |= bits; }

//! returns a number based on "to_modify" but with "bits" turned off.
template <class type>
void CLEAR(type &to_modify, type bits) { to_modify &= (type)~bits; }

//! returns non-zero if the "bits" bit pattern is turned on in "to_test".
template <class type>
bool TEST(type to_test, type bits) { return bool(!(!(to_test & bits))); }

//////////////

} //namespace.

#endif

