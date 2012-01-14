#ifndef POLYGON_CLASS
#define POLYGON_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : polygon                                                           *
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

//! Represents a multi-sided geometric figure made of line segments.
/*!
  The polygon is a list of points that are assumed to be connected.  It will
  have as many sides as its point count minus one.  Thus there is no valid
  polygon with less than three points.  A function that tests whether a point
  is inside or outside the polygon is provided.
*/

#include "cartesian_objects.h"

#include <basis/array.h>

//hmmm: it might be nice to structuralize this.
//hmmm: also deriving from an array of points is not so great.

namespace geometric {

class polygon : public basis::array<geometric::cartesian_point>
{
public:
  polygon() {}
  ~polygon() {}

  void add(const cartesian_point &to_add);
    //!< adds a new point to the list that represents the polygon's sides.

  int points() const { return length(); }
  int sides() const { return points() - 1; }
  
  cartesian_point &operator [] (int index);
    //!< retrieves the index-th point in the list.
    /*!< this is valid for indices between zero and points() - 1. */

  bool inside(const cartesian_point &to_check);
    //!< Returns true if the point "to_check" is inside of this polygon.
    /*!< This function assumes that the polygon is closed when performing
    the check. */
};

} // namespace.

#endif

