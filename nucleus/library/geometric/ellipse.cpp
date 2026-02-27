


/*****************************************************************************\
*                                                                             *
*  Name   : ellipse                                                           *
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

#include "cartesian_objects.h"
#include "ellipse.h"
#include "line.h"
#include "rectangle.h"

#include <basis/functions.h>

#include <math.h>

using namespace basis;

namespace geometric {

ellipse::ellipse()
: _center(cartesian_point::origin()),
  _width_from_center(1),
  _height_from_center(1)
{}

ellipse::ellipse(const cartesian_point &a_center, double a_width_from_center,
    double a_height_from_center)
: _center(a_center),
  _width_from_center(a_width_from_center),
  _height_from_center(a_height_from_center)
{}

ellipse::ellipse(const ellipse &to_copy)
: _center(),
  _width_from_center(0),
  _height_from_center(0)
{ *this = to_copy; }

ellipse::~ellipse() {}

ellipse &ellipse::operator = (const ellipse &to_copy)
{
  if (this == &to_copy) return *this;
  _center = to_copy._center;
  _width_from_center = to_copy._width_from_center;
  _height_from_center = to_copy._height_from_center;
  return *this;
}

double ellipse::area() const
{ return absolute_value(PI_APPROX * _width_from_center * _height_from_center); }

double ellipse::perimeter() const
{
  double w = _width_from_center;
  double h = _height_from_center;
  double perim_temp = sqrt(square(h) + square(w)) / 2;
  return 2.0 * PI_APPROX * perim_temp;
}

cartesian_point ellipse::location(const double_angle &where) const
{
  double a = _width_from_center;
  double b = _height_from_center;
  double a_multiplier = square(where.tangent());
  double denom = sqrt(square(b) + square(a) * a_multiplier);
  double ab = a * b;
  double tango = where.tangent();
  cartesian_point to_return(ab / denom, ab * tango / denom);

  // the following negates the x component if the angle is in the appropriate
  // part of the ellipse.
  int ang = int(where.get(DEGREES));
  double adjustment = where.get(DEGREES) - double(ang);
  ang %= 360;
  double adjusted_ang = ang + adjustment;
  if ( (adjusted_ang < 270.0) && (adjusted_ang > 90.0) )
     to_return.set(to_return.x() * -1.0, to_return.y());
  to_return += _center;
  return to_return;
}

bool ellipse::inside(const cartesian_point &where) const
{
  double dist = where.distance(_center);
  double_angle to_point = double_angle(asin(where.y() / dist), RADIANS);
  cartesian_point intersector = location(to_point);
  return dist <= intersector.distance(_center)? true : false;
}

cartesian_point ellipse::center() const { return _center; }

double ellipse::width_from_center() const { return _width_from_center; }

double ellipse::height_from_center() const { return _height_from_center; }

void ellipse::center(const cartesian_point &to_set) { _center = to_set; }

void ellipse::width_from_center(double to_set)
{ _width_from_center = to_set; }

void ellipse::height_from_center(double to_set)
{ _height_from_center = to_set; }
  
} // namespace.




