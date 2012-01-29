#ifndef DOUBLE_PLUS_CLASS
#define DOUBLE_PLUS_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : double_plus (an extension for double floating point numbers)      *
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

#include "math_ops.h"

#include <basis/contracts.h>
#include <basis/enhance_cpp.h>
#include <basis/functions.h>

#include <stdio.h>//temp

//! An extension to floating point primitives providing approximate equality.
/*!
  Allows a programmer to ignore issues of rounding errors on floating point
  numbers by specifying that two floating point numbers are equivalent if
  they are equal within a small number "delta".  This can help to eliminate
  errors in floating point logic.
*/

namespace mathematics {

class double_plus : public basis::orderable
{
public:
  #define DEFAULT_DELTA 0.0001
    /*!< the delta is the acceptable amount of difference between two floating
    point numbers that are considered equivalent by this class.  if they
    differ by more than that, they are considered non-equivalent (and
    hence must be greater than or less than each other). */

  //! initializes using "init" as the initial value and equality within "delta".
  double_plus(double init = 0.0, double delta = DEFAULT_DELTA) : c_value(init), c_delta(delta) {}

  //! initializes this from "to_copy".
  double_plus(const double_plus &to_copy) : c_value(to_copy.c_value), c_delta(to_copy.c_delta) {}

  virtual ~double_plus() {}

  DEFINE_CLASS_NAME("double_plus");

  //! standard assignment operator.
  double_plus &operator = (const double_plus &cp)
      { c_value = cp.c_value; c_delta = cp.c_delta; return *this; }

  double value() const { return truncate(); }
    //!< observes the value held in this.
  operator double () const { return truncate(); }
    //!< observes the value held in this.

  double delta() const { return c_delta; }
    //!< observes the precision for equality comparisons.
  void delta(double new_delta) { c_delta = new_delta; }
    //!< modifies the precision for equality comparisons.

  double truncate() const { return math_ops::round_it(c_value / c_delta) * c_delta; }
    //!< returns a version of the number that is chopped off past the delta after rounding.

  //! returns true if this equals "f2" within the "delta" precision.
  virtual bool equal_to(const basis::equalizable &f2) const {
     const double_plus *cast = dynamic_cast<const double_plus *>(&f2);
     if (!cast) return false;
     return this->d_eq(*cast);
  }

  //!< returns true if this is less than "f2".
  virtual bool less_than(const basis::orderable &f2) const
  {
     const double_plus *cast = dynamic_cast<const double_plus *>(&f2);
     if (!cast) return false;
     return !this->d_eq(*cast) && (c_value < cast->c_value);
  }

private:
  double c_value;  //!< the contained floating point value.
  double c_delta;  //!< the offset within which equality is still granted.

  //! returns true if "to_compare" is within the delta of this.
  bool d_eq(const double_plus &to_compare) const {
    double diff = basis::absolute_value(c_value - to_compare.value());
    return diff < basis::absolute_value(c_delta); 
  }
};

} //namespace.

#endif

