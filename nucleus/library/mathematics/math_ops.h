#ifndef MATH_OPS_GROUP
#define MATH_OPS_GROUP

/*****************************************************************************\
*                                                                             *
*  Name   : mathematical operations                                           *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/definitions.h>

namespace mathematics {

//! A grab-bag of mathematical functions that are frequently useful.

class math_ops
{
public:
  //! returns the rounded integer value for "to_round".
  static int round_it(float to_round)
  {
    int to_return = int(to_round);
    // this uses a simplistic view of rounding.
    if (to_round - float(to_return) > 0.5) to_return++;
    return to_return;
  }

  //! returns the rounded integer value for "to_round".
  static int round_it(double to_round)
  {
    int to_return = int(to_round);
    // this uses a simplistic view of rounding.
    if (to_round - double(to_return) > 0.5) to_return++;
    return to_return;
  }

/*
  //! returns the number two to the power "raise_to" (i.e. 2^raise_to).
  static basis::u_int pow_2(const basis::u_int raise_to)
  {
    if (!raise_to) return 1;
    basis::u_int to_return = 2;
    for (basis::u_int i = 1; i < raise_to; i++)
      to_return *= 2;
    return to_return;
  }
*/

  //! returns n! (factorial), which is n * (n - 1) * (n - 2) ...
  static basis::un_int factorial(int n)
  { return (n < 2)? 1 : n * factorial(n - 1); }
};

} //namespace.

#endif

