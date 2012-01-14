#ifndef CARTESIAN_OBJECTS_GROUP
#define CARTESIAN_OBJECTS_GROUP

/*****************************************************************************\
*                                                                             *
*  Name   : cartesian objects                                                 *
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
#include "line.h"
#include "point.h"
#include "rectangle.h"

namespace geometric {

//! Provides a geometric point that use double floating points numbers.

class cartesian_point : public point<double>
{
public:
  cartesian_point(double x = 0, double y = 0) : point<double>(x, y) {}
  cartesian_point(double r, double_angle theta) : point<double>(r, theta) {}
  cartesian_point(const point<double> &to_copy) : point<double>(to_copy) {}
  DEFINE_CLASS_NAME("cartesian_point");

  static cartesian_point origin() { return cartesian_point(0.0, 0.0); }
    //!< the origin of the two-dimensional system.
};

//////////////

//! Provides a geometric line that use double floating points numbers.

class cartesian_line : public line<double>
{
public:
  cartesian_line(const cartesian_point &endpoint_1,
          const cartesian_point &endpoint_2)
          : line<double>(endpoint_1, endpoint_2) {}
  cartesian_line(double x_1 = 0, double y_1 = 0,
          double x_2 = 0, double y_2 = 0)
          : line<double>(x_1, y_1, x_2, y_2) {}
};

//////////////

//! Provides a geometric rectangle that use double floating points numbers.

class cartesian_rectangle : public rectangle<double>
{
public:
  cartesian_rectangle(const cartesian_point &vertex_1,
          const cartesian_point &vertex_2)
          : rectangle<double>(vertex_1, vertex_2) {}
  cartesian_rectangle(double x_1 = 0, double y_1 = 0,
          double x_2 = 0, double y_2 = 0)
          : rectangle<double>(x_1, y_1, x_2, y_2) {}
  cartesian_rectangle(const rectangle<double> &rect)
          : rectangle<double>(rect) {}
};

} // namespace.

#endif

