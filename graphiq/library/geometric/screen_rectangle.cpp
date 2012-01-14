


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

#include "rectangle.h"
#include "screen_rectangle.h"

#include <basis/mutex.h>

#include <structures/static_memory_gremlin.h>

using namespace basis;

namespace geometric {

SAFE_STATIC_CONST(screen_point, screen_origin, (0, 0))

//////////////

#ifdef __WIN32__
screen_point::screen_point(const tagPOINT &original) : point<int>(original.x, original.y) {}

screen_point::operator tagPOINT()
{ POINT to_return; to_return.x = x(); to_return.y = y(); return to_return; }
#endif

//////////////

screen_rectangle::screen_rectangle(const rectangle<int> &init)
: rectangle<int>(init) {}

screen_rectangle::screen_rectangle(const screen_point &vertex_1,
    const screen_point &vertex_2)
: rectangle<int>(vertex_1, vertex_2) {}

screen_rectangle::screen_rectangle(int x_1, int y_1, int x_2, int y_2)
: rectangle<int>(x_1, y_1, x_2, y_2) {}

screen_rectangle screen_rectangle::order() const
{ return screen_rectangle(top_left(), bottom_right()); }

screen_point screen_rectangle::top_left() const
{ return rectangle<int>::bottom_left(); }

screen_point screen_rectangle::bottom_left() const
{ return rectangle<int>::top_left(); }

screen_point screen_rectangle::top_right() const
{ return rectangle<int>::bottom_right(); }

screen_point screen_rectangle::bottom_right() const
{ return rectangle<int>::top_right(); }

#ifdef __WIN32__
screen_rectangle::screen_rectangle(const tagRECT &original)
: rectangle<int>(original.left, original.top, original.right, original.bottom)
{}

screen_rectangle::operator tagRECT() const
{
  RECT to_return; to_return.left = left();
  to_return.top = top(); to_return.right = right();
  to_return.bottom = bottom(); return to_return;
}
#endif

} // namespace.



