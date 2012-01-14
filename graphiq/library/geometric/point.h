#ifndef POINT_CLASS
#define POINT_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : point                                                             *
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

#include "angle.h"
#include "math_bits.h"
#include "point.h"

#include <basis/astring.h>
#include <basis/contracts.h>
#include <basis/functions.h>
#include <structures/object_packers.h>

#include <math.h>

//! Contains all of our objects for geometry and avoids name clashes.

namespace geometric {

//! Represents a geometric point.

template <class numeric_type>
class point : public basis::packable, public virtual basis::root_object
{
public:
  point(numeric_type x = 0, numeric_type y = 0);
  point(numeric_type r, double_angle theta);

  DEFINE_CLASS_NAME("point");

  void set(numeric_type x, numeric_type y);
  void set(numeric_type r, double_angle theta);

  numeric_type x() const { return _x; }
  numeric_type y() const { return _y; }

  numeric_type r() const;
  double_angle theta() const;

  point rotate(const double_angle &theta) const;
    //!< Rotates the point by the angle "theta".
    /*!< This rotates the position of the point around the origin in the
    trigonometric standard manner; zero degrees is at the right, increasing
    degree angles are counterclockwise from the x axis to the y to the
    -x to the -y .... */

  numeric_type distance(const point &p2) const;
    //!< Returns the distance between `this' and the second point `p2'.

  point operator - () const { return point<numeric_type>(-_x, -_y); }
    //!< return the additive inverse of the vector

  numeric_type magnitude() const;
    //!< return the distance from the origin to this point.

  point operator + (const point &arg2) const;
  point operator - (const point &arg2) const;
  point &operator += (const point &arg2);
  point &operator -= (const point &arg2);
  bool operator == (const point &arg2) const;

  basis::astring text_form() const;
    //!< Prints out the two values (x and y) held in the point.

  bool from_text(const basis::astring &text);
    //!< returns true if the "text" is successfully pulled into this point.

  virtual void pack(basis::byte_array &packed_form) const;
  virtual bool unpack(basis::byte_array &packed_form);
  int packed_size() const;

private:
  numeric_type _x;
  numeric_type _y;
};

//////////////

// implementations below...

// notes:
//
// - there is an odd breaking up of the expressions where we're taking a
//   square root because ms visual studio 7 has a bug of some sort that
//   convinces it that angle<int> is being used in there, although it's not.
//   these lines use a temporary named "sumsquar" to deconfuse the compiler. 

template <class numeric_type>
point<numeric_type>::point(numeric_type x, numeric_type y) { set(x, y); }

template <class numeric_type>
point<numeric_type>::point(numeric_type r, double_angle theta)
{ set(r, theta); }

template <class numeric_type>
basis::astring point<numeric_type>::text_form() const
{
  numeric_type temp = 0;
  basis::astring specifier(numeric_specifier(temp));
  basis::astring sprintf_template(basis::astring::SPRINTF, "(%s, %s)", specifier.s(), specifier.s());
  return basis::astring(basis::astring::SPRINTF, sprintf_template.s(), x(), y());
}

template <class numeric_type>
void point<numeric_type>::set(numeric_type x, numeric_type y)
{ _x = x; _y = y; }

template <class numeric_type>
numeric_type point<numeric_type>::r() const
{
  const double sumsquar = square(x()) + square(y());
  return numeric_type(sqrt(sumsquar)); 
}

template <class numeric_type>
void point<numeric_type>::set(numeric_type r, double_angle theta)
{ set(numeric_type(r * theta.cosine()), numeric_type(r * theta.sine())); }

template <class numeric_type>
numeric_type point<numeric_type>::distance(const point &p2) const
{
  const double sumsquar = square(p2.x() - x()) + square(p2.y() - y());
  return numeric_type(sqrt(sumsquar));
}

template <class numeric_type>
double_angle point<numeric_type>::theta() const
{
  basis::outcome retval;
  return double_angle::arctangent(y(), x(), retval);
}

template <class contents>
int point<contents>::packed_size() const
{
  basis::byte_array temp;
//hmmm: inefficient!
  pack(temp);
  return temp.length();
}

template <class contents>
void point<contents>::pack(basis::byte_array &packed_form) const
{
  structures::attach(packed_form, _x);
  structures::attach(packed_form, _y);
}

template <class contents>
bool point<contents>::unpack(basis::byte_array &packed_form)
{
  if (!structures::detach(packed_form, _x)) return false;
  if (!structures::detach(packed_form, _y)) return false;
  return true;
}

template <class numeric_type>
numeric_type point<numeric_type>::magnitude() const
{
  const double sumsquar = square(x()) + square(y());
  return numeric_type(sqrt(sumsquar)); 
}

template <class numeric_type>
point<numeric_type> point<numeric_type>::operator + (const point &arg2) const
{ return point<numeric_type>(x() + arg2.x(), y() + arg2.y()); }

template <class numeric_type>
point<numeric_type> point<numeric_type>::operator - (const point &arg2) const
{ return point<numeric_type>(x() - arg2.x(), y() - arg2.y()); }

template <class numeric_type>
point<numeric_type> &point<numeric_type>::operator += (const point &arg2)
{ _x += arg2.x(); _y += arg2.y(); return *this; }

template <class numeric_type>
point<numeric_type> &point<numeric_type>::operator -= (const point &arg2)
{ _x -= arg2.x(); _y -= arg2.y(); return *this; }

template <class numeric_type>
bool point<numeric_type>::operator == (const point &arg2) const
{
// this bit should be part of the floating point stuff...
  double epsilon = 1e-10;
  return (absolute_value(x() - arg2.x()) <= epsilon)
      && (absolute_value(y() - arg2.y()) <= epsilon);
}

template <class numeric_type>
point<numeric_type> point<numeric_type>::rotate
    (const double_angle &theta) const
{
  numeric_type tempX = x();
  numeric_type tempY = y();
  numeric_type temp1 = numeric_type(tempX * theta.cosine()
      - tempY * theta.sine());
  numeric_type temp2 = numeric_type(tempX * theta.sine() 
      + tempY * theta.cosine());
  return point<numeric_type>(temp1, temp2);
}

template <class numeric_type>
bool point<numeric_type>::from_text(const basis::astring &_text)
{
  numeric_type x = 0, y = 0;
  basis::astring text(_text);
  // chop junk off the front.
  text = crop_non_numeric(text);
  // scan the string for values.
  x = text.convert(x);
  // remove the number.
  text = crop_numeric(text);
  // chop off more junk.
  text = crop_non_numeric(text);
  // get the next number.
  y = text.convert(y);
  set(x, y);
  return true;
}

} // namespace.

#endif

