#ifndef CIRCLE_CLASS
#define CIRCLE_CLASS

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

#include "cartesian_objects.h"

namespace geometric {

//! Represents a geometric circle.
/*!
  A circle is specified by its center and its radius.  The angles are
  measured in radians.
*/

class circle
{
public:
  circle();
  circle(double radius, const cartesian_point &center);
  ~circle();

  double area() const;
    //!< Returns the area occupied by the circle.

  double circumference() const;
    //!< Returns the perimeter for the circle.
    /*!< The circumference is the length of a virtual string around the
    circle. */

  double diameter() const;
    //!< Returns the length of the circle's bisecting line.
    /*!< This is the length of a line segment that is circumscribed by the
    circle and which passes through the center of the circle. */

  bool inside(const cartesian_point &where) const;
    //!< Returns true if the point is inside the circle.

  cartesian_point location(const double_angle &where) const;
    //!< Returns the point on the circle that is at the angle "where".

  cartesian_rectangle dimensions() const;
    //!< Returns a bounding box around the circle.

  double radius() const;  //!< Half of the circle's diameter.
  void radius(double to_set);  //!< Sets the radius of the circle.

  cartesian_point center() const;  //!< The point at the center of the circle.
  void center(const cartesian_point &to_set);  //!< Resets the circle's center.

private:
  double _radius;  //!< Records the current radius.
  cartesian_point _center;  //!< Records the current center.
};

} // namespace.

#endif

