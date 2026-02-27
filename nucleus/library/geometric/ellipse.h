#ifndef ELLIPSE_CLASS
#define ELLIPSE_CLASS

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

namespace geometric {

// forward.
class cartesian_point;
class double_angle;

//! Represents a geometric ellipse.
/*! An ellipse is specified by its height, width and center. */

class ellipse
{
public:
  ellipse();
  ellipse(const cartesian_point &center, double width_from_center,
          double height_from_center);
  ellipse(const ellipse &to_copy);

  ~ellipse();

  ellipse &operator = (const ellipse &to_copy);

  double area() const;
    //!< Returns the area occupied by the ellipse.

  double perimeter() const;
    //!< Returns the perimeter for the ellipse.
    /*!< This is the length of the virtual string around the ellipse.  The
    returned value is an approximation. */

  bool inside(const cartesian_point &where) const;
    //!< Returns true if the point is inside the ellipse.

  cartesian_point location(const double_angle &where) const;
    //!< Describes the locus of the ellipse (basically, where it lives).
    /*!< Returns the point on the ellipse that lies on a ray from the center
    of the ellipse directed at an angle "where". */

  cartesian_point center() const;
  double width_from_center() const;
  double height_from_center() const;

  void center(const cartesian_point &to_set);
  void width_from_center(double to_set);
  void height_from_center(double to_set);

protected:
  cartesian_point _center;
  double _width_from_center;
  double _height_from_center;
};

} // namespace.

#endif

