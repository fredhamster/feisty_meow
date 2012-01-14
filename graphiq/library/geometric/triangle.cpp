


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

#include "cartesian_objects.h"
#include "line.h"
#include "rectangle.h"
#include "triangle.h"

namespace geometric {

triangle::triangle()
: _vertex_1(cartesian_point::origin()),
  _vertex_2(cartesian_point::origin()),
  _vertex_3(cartesian_point::origin())
{}

triangle::triangle(const cartesian_point &vertex_1,
    const cartesian_point &vertex_2, const cartesian_point &vertex_3)
: _vertex_1(vertex_1),
  _vertex_2(vertex_2),
  _vertex_3(vertex_3)
{}

triangle::triangle(const triangle &to_copy)
: _vertex_1(to_copy._vertex_1),
  _vertex_2(to_copy._vertex_2),
  _vertex_3(to_copy._vertex_3)
{}

triangle::~triangle() {}

triangle &triangle::operator =(const triangle &to_copy)
{
  if (this == &to_copy) return *this;
  _vertex_1 = to_copy._vertex_1;
  _vertex_2 = to_copy._vertex_2;
  _vertex_3 = to_copy._vertex_3;
  return *this;
}

line<double> triangle::side_1_2() const
{ return line<double>(_vertex_1, _vertex_2); }

line<double> triangle::side_2_3() const
{ return line<double>(_vertex_2, _vertex_3); }

line<double> triangle::side_3_1() const
{ return line<double>(_vertex_3, _vertex_1); }

cartesian_point triangle::vertex_1() const { return _vertex_1; }

cartesian_point triangle::vertex_2() const { return _vertex_2; }

cartesian_point triangle::vertex_3() const { return _vertex_3; }

void triangle::vertex_1(const cartesian_point &to_set) { _vertex_1 = to_set; }

void triangle::vertex_2(const cartesian_point &to_set) { _vertex_2 = to_set; }

void triangle::vertex_3(const cartesian_point &to_set) { _vertex_3 = to_set; }

bool triangle::inside(const cartesian_point &where) const
{
//cerr << "triangle::inside: not implemented" << endl << flush;
if (where.x()) where.y();  // bogus.
  return false;
}

double triangle::area() const
{
//cerr << "triangle::area: not implemented" << endl << flush;
  return 5;
}

} // namespace.

/*
//temp
#include "warper.h"
using namespace geometric;
typedef rectangle_warper<double> chuzzo;
chuzzo beanburp = chuzzo(rectangle<double>(0, 23, 39, 1012),
  rectangle<double>(8, 19, 92982, -2), chuzzo::BOTTOM_RIGHT,
  chuzzo::TOP_LEFT);
typedef rectangle_warper<double>::horizontal_component horzo;
typedef rectangle_warper<double>::vertical_component verzo;
int frunk() {
  horzo peen;
  beanburp.separate_horizontal(chuzzo::BOTTOM_RIGHT, peen);
  verzo neep;
  beanburp.separate_vertical(chuzzo::TOP_RIGHT, neep);
}
*/





