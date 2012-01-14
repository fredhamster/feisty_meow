#ifndef LINE_CLASS
#define LINE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : line                                                              *
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

#include "point.h"

namespace geometric {

//! Represents a geometric line segment.

template <class numeric_type>
class line
{
public:
  line(const point<numeric_type> &endpoint1,
          const point<numeric_type> &endpoint2);
  line(numeric_type end1_x = 0, numeric_type end1_y = 0,
          numeric_type end2_x = 0, numeric_type end2_y = 0);

  point<numeric_type> center() const;
    //!< Returns the point at the center of the line segment.

  line operator + (const point<numeric_type> &to_add) const;
  line operator - (const point<numeric_type> &to_subtract) const;
    //!< Returns this line with "to_add" added to it.
    /*!< This returns a line that is the result of adding or subtracting a
    point to the endpoints of this line. */

  line &operator += (const point<numeric_type> &to_add);
  line &operator -= (const point<numeric_type> &to_subtract);
    //!< Adds or subtracts a point from `this' line.

  point<numeric_type> endpoint_1() const;
  point<numeric_type> endpoint_2() const;

  void endpoint_1(const point<numeric_type> &to_set);
  void endpoint_2(const point<numeric_type> &to_set);

  basis::astring text_form() const;
    //!< returns a string form of the points defining the line.

protected:
  point<numeric_type> _endpoint_1;
  point<numeric_type> _endpoint_2;
};

//////////////

// implementations below...

template <class numeric_type>
line<numeric_type>::line(const point<numeric_type> &p1,
    const point<numeric_type> &p2)
: _endpoint_1(p1), _endpoint_2(p2) {}

template <class numeric_type>
line<numeric_type>::line(numeric_type x1, numeric_type y1, numeric_type x2,
    numeric_type y2)
: _endpoint_1(point<numeric_type>(x1, y1)),
  _endpoint_2(point<numeric_type>(x2, y2))
{}

template <class numeric_type>
point<numeric_type> line<numeric_type>::center() const
{
  return point<numeric_type>(_endpoint_1.x() / 2.0 + _endpoint_2.x() / 2.0,
      _endpoint_1.y() / 2.0 + _endpoint_2.y() / 2.0);
}

template <class numeric_type>
basis::astring line<numeric_type>::text_form() const
{
  return basis::astring("<") + _endpoint_1.text_form() + basis::astring(" ")
      + _endpoint_2.text_form() + basis::astring(">");
}

template <class numeric_type>
line<numeric_type> &line<numeric_type>::operator +=
    (const point<numeric_type> &to_add)
{ _endpoint_1 += to_add; _endpoint_2 += to_add; return *this; }

template <class numeric_type>
line<numeric_type> &line<numeric_type>::operator -=
    (const point<numeric_type> &to_subtract)
{ _endpoint_1 -= to_subtract; _endpoint_2 -= to_subtract; return *this; }

template <class numeric_type>
line<numeric_type> line<numeric_type>::operator +
    (const point<numeric_type> &to_add) const
{ line<numeric_type> to_return(*this); to_return += to_add; return to_return; }

template <class numeric_type>
line<numeric_type> line<numeric_type>::operator -
    (const point<numeric_type> &to_subtract) const
{
  line<numeric_type> to_return(*this);
  to_return -= to_subtract;
  return to_return;
}

template <class numeric_type>
point<numeric_type> line<numeric_type>::endpoint_1() const
{ return _endpoint_1; }

template <class numeric_type>
point<numeric_type> line<numeric_type>::endpoint_2() const
{ return _endpoint_2; }

template <class numeric_type>
void line<numeric_type>::endpoint_1(const point<numeric_type> &to_set)
{ _endpoint_1 = to_set; }

template <class numeric_type>
void line<numeric_type>::endpoint_2(const point<numeric_type> &to_set)
{ _endpoint_2 = to_set; }

} // namespace.

#endif

