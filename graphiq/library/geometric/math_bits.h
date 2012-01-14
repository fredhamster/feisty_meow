#ifndef MATHEMATICAL_OPERATIONS_GROUP
#define MATHEMATICAL_OPERATIONS_GROUP

/*****************************************************************************\
*                                                                             *
*  Name   : mathematical operations group                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1996-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

/*! @file math_bits.h
  @brief Provides some fairly low-level math support.
*/

#include <basis/astring.h>

namespace geometric {

basis::astring crop_numeric(const basis::astring &input);
  //!< Eats the numeric characters from the front of "input".
  /*!< This and crop_non_numeric() return a string that is constructed from
  "input" by chopping the numerical (or non-numerical) characters off of the
  beginning.  The types of numbers supported are floating point, but it will
  generally work for integers also (problems arise when mixing in
  the period, e, E, +, and - characters with integer numbers). */
basis::astring crop_non_numeric(const basis::astring &input);
  //!< Eats the non-numeric characters from the front of "input".

// type identification functions:

//! returns true if the specified numeric_type is a floating_point type.
/*! this is useful in templates where one wants to know about the templated
type. */
template <class numeric_type>
bool is_floating_point(numeric_type t)
    { t = numeric_type(5.1); t = numeric_type(t * 3.0);
          return 0.001 < float(absolute_value(numeric_type(t - 15.0))); }

//! returns true if the instantiation type is an integer.
template <class numeric_type>
bool is_integral(numeric_type t) { return !is_floating_point(t); }

// the following functions (is_short, is_signed and is_unsigned) are only
// useful for integral types.

//! returns true if the specified type is short on the PC.
template <class numeric_type>
bool is_short(numeric_type) { return sizeof(numeric_type) == 2; }

//! returns true if the templated type is unsigned.
template <class numeric_type>
bool is_unsigned(numeric_type t) { t = -1; return t > 0; }
//! returns true if the templated type is signed.
template <class numeric_type>
bool is_signed(numeric_type t) { return !is_unsigned(t); }

//! Guesses the formatting string needed for the type provided.
/*! Returns a string that is appropriate as the format specifier of a printf
(or the astring constructor) given the template type.  templates can rely
on this to print numerical types correctly. */
template <class numeric_type>
basis::astring numeric_specifier(numeric_type t) {
  basis::astring to_return("%d");
  if (is_floating_point(t))
    to_return = basis::astring("%f");
  else {  // integral.
    if (is_unsigned(t))
      to_return = basis::astring("%u");
    if (is_short(t))
      to_return.insert(1, "h");
  }
  return to_return;
}

}

#endif // outer guard.

