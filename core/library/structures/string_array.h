#ifndef STRING_ARRAY_CLASS
#define STRING_ARRAY_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : string_array                                                      *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1993-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "object_packers.h"

#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/functions.h>

namespace structures {

//! An array of strings with some additional helpful methods.

class string_array
: public basis::array<basis::astring>,
  public virtual basis::packable,
  public virtual basis::equalizable
{
public:
  string_array(int number = 0, const basis::astring *initial_contents = NIL)
          : basis::array<basis::astring>(number, initial_contents,
                EXPONE | FLUSH_INVISIBLE) {}
    //!< Constructs an array of "number" strings.
    /*!< creates a list of strings based on an initial "number" of entries and
    some "initial_contents", which should be a regular C array of astrings
    with at least as many entries as "number". */

  //! a constructor that operates on an array of char pointers.
  /*! be very careful with the array to ensure that the right number of
  elements is provided. */
  string_array(int number, const char *initial_contents[])
      : basis::array<basis::astring>(number, NIL, EXPONE | FLUSH_INVISIBLE) {
    for (int i = 0; i < number; i++) {
      put(i, basis::astring(initial_contents[i]));
    }
  }

  string_array(const basis::array<basis::astring> &to_copy)
      : basis::array<basis::astring>(to_copy) {}
    //!< copy constructor that takes a templated array of astring.

  DEFINE_CLASS_NAME("string_array");

  //! Prints out a formatted view of the contained strings and returns it.
  basis::astring text_format(const basis::astring &separator = ",",
          const basis::astring &delimiter = "\"") const {
    basis::astring to_return;
    for (int i = 0; i < length(); i++) {
      to_return += delimiter;
      to_return += get(i);
      to_return += delimiter;
      if (i < last())
        to_return += separator;
    }
    return to_return;
  }

  basis::astring text_form() const { return text_format(); }
    //!< A synonym for the text_format() method.

//hmmm: extract as re-usable equality operation.

  //! Compares this string array for equality with "to_compare".
  bool equal_to(const equalizable &to_compare) const {
    const string_array *cast = cast_or_throw(to_compare, *this);
    if (length() != cast->length())
      return false;
    for (int i = 0; i < length(); i++)
      if (cast->get(i) != get(i))
        return false;
    return true;
  }

  //! locates string specified and returns its index, or negative if missing.
  int find(const basis::astring &to_find) const {
    for (int i = 0; i < length(); i++) {
      if (to_find.equal_to(get(i))) return i;
    }
    return basis::common::NOT_FOUND;
  }

  //! Returns true if all of the elements in this are the same in "second".
  /*! The array "second" can have more elements, but must have all of the
  items listed in this string array. */
  bool prefix_compare(const string_array &second) const {
    if (second.length() < length()) return false;
    if (!length()) return false;
    for (int i = 0; i < length(); i++)
      if ((*this)[i] != second[i]) return false;
    return true;
  }

  //! Packs this string array into the "packed_form" byte array.
  virtual void pack(basis::byte_array &packed_form) const
      { pack_array(packed_form, *this); }
  
  //! Unpacks a string array from the "packed_form" byte array.
  virtual bool unpack(basis::byte_array &packed_form)
      { return unpack_array(packed_form, *this); }

  //! Returns the number of bytes this string array would consume if packed.
  virtual int packed_size() const {
    int to_return = sizeof(int) * 2;  // length packed in, using obscure.
    for (int i = 0; i < length(); i++)
      to_return += get(i).length() + 1;
    return to_return;
  }
};

} //namespace.

#endif

