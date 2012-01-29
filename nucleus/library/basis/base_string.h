#ifndef BASE_STRING_CLASS
#define BASE_STRING_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : base_string                                                       *
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

//hmmm: some of these methods could be pulled out to a base_array class.
//      that would be a nice further abstraction.

#include "contracts.h"

namespace basis {

//! Defines the base class for all string processing objects in hoople.

class base_string : public virtual orderable
{
public:
  virtual int length() const = 0;
    //!< Returns the current length of the string.
    /*!< The length returned does not include the terminating null character
    at the end of the string. */

  virtual const char *observe() const = 0;
    //!< observes the underlying pointer to the zero-terminated string.
    /*!< this does not allow the contents to be modified.  this method should
    never return NIL. */

  virtual char *access() = 0;
    //!< provides access to the actual string held.
    /*!< this should never return NIL.  be very careful with the returned
    pointer: don't destroy or corrupt its contents (e.g., do not mess with
    its zero termination). */

  virtual char get(int index) const = 0;
    //!< a constant peek at the string's internals at the specified index.

  virtual void put(int position, char to_put) = 0;
    //!< stores the character "to_put" at index "position" in the string.

  virtual bool sub_compare(const base_string &to_compare, int start_first,
          int start_second, int count, bool case_sensitive) const = 0;
    //!< Compares "this" string with "to_compare".
    /*!< The "start_first" is where the comparison begins in "this" string,
    and "start_second" where it begins in the "to_compare".  The "count" is
    the number of characters to compare between the two strings.  If either
    index is out of range, or "count"-1 + either index is out of range, then
    compare returns false.  If the strings differ in that range, false is
    returned.  Only if the strings have identical contents in the range is
    true returned.  If case-sensitive is false, then matches will not require
    the caps and lower-case match. */

  virtual base_string &assign(const base_string &s) = 0;
    //!< Sets the contents of this string to "s".

  virtual base_string &upgrade(const char *s) = 0;
    //!< Sets the contents of this string to "s".

  virtual void insert(int position, const base_string &to_insert) = 0;
    //!< Copies "to_insert" into "this" at the "position".
    /*!<  Characters at the index "position" and greater are moved over. */

  virtual void zap(int start, int end) = 0;
    //!< Deletes the characters between "start" and "end" inclusively.
    /*!< C++ array conventions are used (0 through length()-1 are valid).  If
    either index is out of bounds, then the string is not modified. */

  virtual base_string &concatenate_string(const base_string &s) = 0;
    //!< Modifies "this" by concatenating "s" onto it.

  virtual base_string &concatenate_char(char c) = 0;
    //!< concatenater for single characters.

  virtual bool sub_string(base_string &target, int start, int end) const = 0;
    //!< Gets the segment of "this" between the indices "start" and "end".
    /*!< false is returned if the range is invalid. */

  //! sets this string's contents equal to the contents of "to_copy".
  /*! this assignment ensures that setting the base class to derived versions will
  succeed; otherwise, a base class copy does very little. */
  virtual base_string &operator =(const base_string &to_copy) { return assign(to_copy); }
};

} //namespace.

#endif

