


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

#include "polygon.h"

#include <basis/array.h>

using namespace basis;

namespace geometric {

bool polygon::inside(const cartesian_point &to_check)
{
  int right_intersect_count = 0;
  for (int i = 0; i < length(); i++) {
    cartesian_point vert_1 = get(i);
    cartesian_point vert_2 = get( (i + 1) % length() );
    if ( (to_check.y() < minimum(vert_1.y(), vert_2.y()))
        || (to_check.y() > maximum(vert_1.y(), vert_2.y())) ) continue;
    double x_intersect;
    if (vert_2.x() == vert_1.x()) {
      x_intersect = vert_2.x();
    } else {
      double m = (vert_2.y() - vert_1.y()) / (vert_2.x() - vert_1.x());
      x_intersect = 1.0 / m * (to_check.y() - vert_1.y() + m * vert_1.x());
    }
    if (x_intersect > to_check.x()) right_intersect_count++;
  }
  return !!(right_intersect_count % 2);
}

}




