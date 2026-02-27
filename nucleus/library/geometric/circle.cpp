/*****************************************************************************\
*                                                                             *
*  Name   : circle                                                            *
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

#include "circle.h"
#include "cartesian_objects.h"
#include "line.h"
#include "rectangle.h"

#include <basis/functions.h>

#include <math.h>

using namespace basis;

namespace geometric {

circle::circle() : _radius(1), _center(cartesian_point::origin()) {}

circle::circle(double a_radius, const cartesian_point &a_center)
: _radius(a_radius), _center(a_center) {}

circle::~circle() {}

double circle::area() const { return PI_APPROX * square(_radius); }

double circle::diameter() const { return 2.0 * _radius; }

double circle::circumference() const { return 2.0 * PI_APPROX * _radius; }

cartesian_point circle::location(const double_angle &where) const
{
  double rotation = where.get(RADIANS);
  cartesian_point second(cos(rotation) * _radius, sin(rotation) * _radius);
  return _center + second;
}

bool circle::inside(const cartesian_point &where) const
{
  double dist = where.distance(_center);
  return dist <= _radius? true : false;
}

cartesian_rectangle circle::dimensions() const
{
  const double deg0 = 0;
  const double deg90 = 0.5 * PI_APPROX;
  const double deg180 = PI_APPROX;
  const double deg270 = 1.5 * PI_APPROX;

  cartesian_point right(location(deg0));
  cartesian_point top(location(deg90));
  cartesian_point left(location(deg180));
  cartesian_point bottom(location(deg270));
  return cartesian_rectangle(left.x(), bottom.y(), right.x(), top.y());
}

double circle::radius() const { return _radius; }

void circle::radius(double to_set) { _radius = to_set; }

cartesian_point circle::center() const { return _center; }

void circle::center(const cartesian_point &to_set) { _center = to_set; }

} // namespace.

