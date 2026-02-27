#ifndef TRIANGLE_CLASS
#define TRIANGLE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : triangle                                                          *
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



// forward.
class cartesian_line;
class cartesian_point;

namespace geometric {

//! Represents a geometric triangle.

class triangle
{
public:
  triangle();
  triangle(const cartesian_point &vertex1, const cartesian_point &vertex2,
          const cartesian_point &vertex3);
  triangle(const triangle &to_copy);
  ~triangle();

  triangle &operator =(const triangle &to_copy);

  bool inside(const cartesian_point &where) const;

  double area() const;

  line<double> side_1_2() const;
  line<double> side_2_3() const;
  line<double> side_3_1() const;

  cartesian_point vertex_1() const;
  cartesian_point vertex_2() const;
  cartesian_point vertex_3() const;

  void vertex_1(const cartesian_point &to_set);
  void vertex_2(const cartesian_point &to_set);
  void vertex_3(const cartesian_point &to_set);

protected:
  cartesian_point _vertex_1;
  cartesian_point _vertex_2;
  cartesian_point _vertex_3;
};

} // namespace.

#endif

