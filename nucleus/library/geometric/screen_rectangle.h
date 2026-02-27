#ifndef SCREEN_RECTANGLE_CLASS
#define SCREEN_RECTANGLE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : screen_rectangle                                                  *
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

#include <application/windoze_helper.h>

#include "rectangle.h"

#ifdef __WIN32__
  // forward.
  struct tagPOINT; struct tagRECT;
#endif

namespace geometric {

// forward.
class double_angle;

//! a simple class used to describe points on a graphics screen.

class screen_point : public point<int>
{
public:
  screen_point(int x = 0, int y = 0) : point<int>(x, y) {}
  screen_point(int r, double_angle theta) : point<int>(r, theta) {}
  screen_point(const point<int> &original) : point<int>(original) {}
  DEFINE_CLASS_NAME("screen_point");

#ifdef __WIN32__
  screen_point(const tagPOINT &original);
    //!< helpful conversions from basic ms-windows type.
  operator tagPOINT();
    //!< helpful conversions to basic ms-windows type.
#endif
};

const screen_point &screen_origin();
  //!< the origin of the screen coordinate system (which is top-left here).

//////////////

//! Represents a rectangle as interpreted on display screens.
/*!
  The origin is the top-left corner of the rectangle and the y coordinate
  gets larger as one goes downwards.  This class is primarily useful in
  conjunction with a windowing environment.
*/

class screen_rectangle : public rectangle<int>
{
public:
  screen_rectangle(const screen_point &vertex_1, const screen_point &vertex_2);
  screen_rectangle(int x_1 = 0, int y_1 = 0, int x_2 = 0, int y_2 = 0);
  screen_rectangle(const rectangle<int> &init);

  screen_rectangle order() const;
    //!< Re-orders the vertices to match expectations.
    /*!< This is just like rectangle::order() except that the first vertex
    will be closest to the top-left of the screen. */

  screen_point top_left() const;
  screen_point bottom_left() const;
  screen_point top_right() const;
  screen_point bottom_right() const;

  int left() const { return top_left().x(); }
  int top() const { return top_left().y(); }
  int right() const { return bottom_right().x(); }
  int bottom() const { return bottom_right().y(); }

#ifdef __WIN32__
  screen_rectangle(const tagRECT &original);
    //!< helpful conversion from basic ms-windows type.
  operator tagRECT() const;
    //!< helpful conversion to basic ms-windows type.
#endif
};

} // namespace.

#endif

