#ifndef RECTANGLE_WARPER_CLASS
#define RECTANGLE_WARPER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : rectangle_warper                                                  *
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

//! Warps points in one frame of reference to a different one.
/*!
  This class encapsulates the notion of a rectangular region that is
  referred to from two different points of view.  This relates two
  two-dimensional frames of reference to each other.  Each frame of reference
  is specified by two rectangles.  A point that is measured in one frame of
  reference can be transformed into a point that is measured in the other,
  and vice-versa.
*/

#include "rectangle.h"

#include <basis/astring.h>

namespace geometric {

template <class numeric_type>
class rectangle_warper
{
public:
  //! describes where a rectangle's origin is located on the rectangle.
  /*! our model is to consider the first vertex point of the rectangle as its
  origin and the second vertex point (diagonally opposite the first point) as
  its extent.  since it may make sense for that first vertex point to be
  located at any one of the vertices of the rectangle (as in windowing
  coordinate system conversions), the enumeration below allows any one of the
  rectangle's vertices to be chosen as its origin. */
  enum origin_vertex { BOTTOM_LEFT, TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT };

  rectangle_warper(const rectangle<numeric_type> &system_1,
          const rectangle<numeric_type> &system_2,
          origin_vertex system_1_origin = BOTTOM_LEFT,
          origin_vertex system_2_origin = BOTTOM_LEFT);
    //!< constructs a warper given the two reference systems.
    /*!< constructs a warper where the first rectangular system is in
    "system_1", the second system is in "system_2" and the respective origins
    for these systems are in "system_1_origin" and "system_2_origin". */

  ~rectangle_warper();

  point<numeric_type> to_system_1(const point<numeric_type> &in_system_2) const;
    //!< Converts from the second system into the first.
    /*!< This returns a point that is measured in the first frame of reference
    when given a point "in_system_2" that is measured in the second frame of
    reference. */

  point<numeric_type> to_system_2(const point<numeric_type> &in_system_1) const;
    //!< Converts from the first system into the second.
    /*!< This returns a point that is measured in the second frame of reference
    when given a point "in_system_1" that is measured in the first frame of
    reference. */

  rectangle<numeric_type> to_system_1
          (const rectangle<numeric_type> &in_system_2) const;
    //!< flips a rectangle from the second system into the first.
  rectangle<numeric_type> to_system_2
          (const rectangle<numeric_type> &in_system_1) const;
    //!< flips a rectangle from the first system into the second.

  rectangle<numeric_type> system_1() const { return _system_1; }
  rectangle<numeric_type> system_2() const { return _system_2; }
  origin_vertex origin_1() const { return _vert_1; }
  origin_vertex origin_2() const { return _vert_2; }

  void system_1(const rectangle<numeric_type> &to_set,
          origin_vertex origin_corner = BOTTOM_LEFT);
  void system_2(const rectangle<numeric_type> &to_set,
          origin_vertex origin_corner = BOTTOM_LEFT);

  basis::astring text_form() const;
    //!< Prints out the two systems held in the rectangle_warper.

  basis::astring vertex_name(origin_vertex v) const;
    //!< Prints out the name of the vertex location.

  enum vertical_component { RW_BOTTOM, RW_TOP };
  enum horizontal_component { RW_LEFT, RW_RIGHT };

  void separate_vertical(origin_vertex v, vertical_component &to_set) const;
  void separate_horizontal(origin_vertex v, horizontal_component &to_set) const;
    //!< separates out a component of the placement of the vertex.

private:
  rectangle<numeric_type> _system_1;
  rectangle<numeric_type> _system_2;
  origin_vertex _vert_1;
  origin_vertex _vert_2;

  point<numeric_type> scale_point(const rectangle<numeric_type> &source,
          const rectangle<numeric_type> &target,
          origin_vertex v1, origin_vertex v2,
          const point<numeric_type> &old) const;
  rectangle<numeric_type> scale_rectangle(const rectangle<numeric_type> &source,
          const rectangle<numeric_type> &target,
          origin_vertex v1, origin_vertex v2,
          const rectangle<numeric_type> &old) const;
  rectangle<numeric_type> flip_accordingly
          (const rectangle<numeric_type> &to_flip, origin_vertex to_flip_origin,
          origin_vertex target_origin) const;
    //!< Flips the points in "to_flip" to match the "target_origin".
    /*!< swaps the points contained in a rectangle that uses a particular point
    as the vertex ("to_flip_origin") so that the points are arranged
    according to a second choice of vertex ("target_origin"). */
};

//////////////

// implementations for longer methods below...

template <class numeric_type>
rectangle_warper<numeric_type>::rectangle_warper
    (const rectangle<numeric_type> &system_1,
     const rectangle<numeric_type> &system_2,
     origin_vertex v1, origin_vertex v2)
: _system_1(system_1), _system_2(system_2), _vert_1(v1), _vert_2(v2)
{}

template <class numeric_type>
rectangle_warper<numeric_type>::~rectangle_warper() {}

template <class numeric_type>
void rectangle_warper<numeric_type>::system_1
    (const rectangle<numeric_type> &to_set, origin_vertex v)
{ _system_1 = to_set; _vert_1 = v; }

template <class numeric_type>
void rectangle_warper<numeric_type>::system_2
    (const rectangle<numeric_type> &to_set, origin_vertex v)
{ _system_2 = to_set; _vert_2 = v; }

template <class numeric_type>
point<numeric_type> rectangle_warper<numeric_type>::to_system_1
    (const point<numeric_type> &in_system_2) const
{ return scale_point(_system_2, _system_1, _vert_2, _vert_1, in_system_2); }

template <class numeric_type>
point<numeric_type> rectangle_warper<numeric_type>::to_system_2
    (const point<numeric_type> &in_system_1) const
{ return scale_point(_system_1, _system_2, _vert_1, _vert_2, in_system_1); }

template <class numeric_type>
rectangle<numeric_type> rectangle_warper<numeric_type>::to_system_1
    (const rectangle<numeric_type> &in_system_2) const
{ 
  return scale_rectangle(_system_2, _system_1, _vert_2, _vert_1, 
      in_system_2); 
}

template <class numeric_type>
rectangle<numeric_type> rectangle_warper<numeric_type>::to_system_2
    (const rectangle<numeric_type> &in_system_1) const
{
  return scale_rectangle(_system_1, _system_2, _vert_1, _vert_2,
      in_system_1); 
}

template <class numeric_type>
void rectangle_warper<numeric_type>::separate_vertical
    (origin_vertex v, vertical_component &to_set) const
{
  if ( (v == BOTTOM_LEFT) || (v == BOTTOM_RIGHT) ) to_set = RW_BOTTOM;
  to_set = RW_TOP;
}

template <class numeric_type>
void rectangle_warper<numeric_type>::separate_horizontal
    (origin_vertex v, horizontal_component &to_set) const
{
  if ( (v == BOTTOM_LEFT) || (v == TOP_LEFT) ) to_set = RW_LEFT;
  to_set = RW_RIGHT;
}

template <class numeric_type>
rectangle<numeric_type> rectangle_warper<numeric_type>::flip_accordingly
    (const rectangle<numeric_type> &to_flip, origin_vertex flipo,
    origin_vertex targo) const
{
//LOG(basis::astring("flipping ") + to_flip.text_form() + " from " + flipo.text_form() + " to " + targo.text_form());
  if (flipo == targo) return to_flip;
  numeric_type x1(to_flip.vertex_1().x());
  numeric_type y1(to_flip.vertex_1().y());
  numeric_type x2(to_flip.vertex_2().x());
  numeric_type y2(to_flip.vertex_2().y());
  horizontal_component horiz1;
  separate_horizontal(flipo, horiz1);
  horizontal_component horiz2;
  separate_horizontal(targo, horiz2);
  bool flip_x = bool(horiz1 != horiz2);
  vertical_component vert1;
  separate_vertical(flipo, vert1);
  vertical_component vert2;
  separate_vertical(targo, vert2);
  bool flip_y = bool(vert1 != vert2);
  if (flip_x) basis::swap_values(x1, x2);
  if (flip_y) basis::swap_values(y1, y2);
//LOG(basis::astring("it becomes ") + rectangle<numeric_type>(x1, y1, x2, y2).text_form());
  return rectangle<numeric_type>(x1, y1, x2, y2);
}

template <class numeric_type>
rectangle<numeric_type> rectangle_warper<numeric_type>::scale_rectangle
    (const rectangle<numeric_type> &source,
    const rectangle<numeric_type> &target, origin_vertex source_origin,
    origin_vertex target_origin, const rectangle<numeric_type> &old) const
{
  rectangle<numeric_type> s = rectangle<numeric_type>
      (flip_accordingly(source, source_origin, BOTTOM_LEFT));
  numeric_type width_source = s.vertex_2().x() - s.vertex_1().x();
  numeric_type height_source = s.vertex_2().y() - s.vertex_1().y();
  if ( !width_source || !height_source ) {
//    cerr << "degenerate rectangle in rectangle_warper::scaler: " << s
//      << endl << flush;
    return old;
  }
  rectangle<numeric_type> t(flip_accordingly(target, target_origin, BOTTOM_LEFT));
  numeric_type width_target = t.vertex_2().x() - t.vertex_1().x();
  numeric_type height_target = t.vertex_2().y() - t.vertex_1().y();
  numeric_type x_scale = width_target / width_source;
  numeric_type y_scale = height_target / height_source;

//LOG(basis::astring("scaler: source ") + source.text_form() + " with vert " + source_origin.text_form() + " becomes " + s + " target " + target + " with vert " + target_origin + " becomes " + t + ".");

  rectangle<numeric_type> o(flip_accordingly(old, source_origin, BOTTOM_LEFT));

  rectangle<numeric_type> to_return = flip_accordingly(rectangle<numeric_type>
     ((o.vertex_1().x() - s.vertex_1().x()) * x_scale + t.vertex_1().x(),
      (o.vertex_1().y() - s.vertex_1().y()) * y_scale + t.vertex_1().y(),
      (o.vertex_2().x() - s.vertex_1().x()) * x_scale + t.vertex_1().x(),
      (o.vertex_2().y() - s.vertex_1().y()) * y_scale + t.vertex_1().y()),
     BOTTOM_LEFT, target_origin);

//  LOG(basis::astring("old ") + old.text_form() + " with source vert becomes " + o.text_form() + " and then is moved into " + to_return.text_form());

  return to_return;
}

template <class numeric_type>
point<numeric_type> rectangle_warper<numeric_type>::scale_point
  (const rectangle<numeric_type> &source, const rectangle<numeric_type> &target,
   origin_vertex source_origin, origin_vertex target_origin,
   const point<numeric_type> &old) const
{
  // gross but simple.
  return scale_rectangle(source, target, source_origin, target_origin,
      rectangle<numeric_type>(old, old)).vertex_1();
}

template <class numeric_type>
basis::astring rectangle_warper<numeric_type>::vertex_name(origin_vertex v) const
{
  basis::astring name("unknown");
  switch (v) {
    case BOTTOM_LEFT: name = "bottom-left"; break;
    case BOTTOM_RIGHT: name = "bottom-right"; break;
    case TOP_LEFT: name = "top-left"; break;
    case TOP_RIGHT: name = "top-right"; break;
  }
  return name;
}

template <class numeric_type>
basis::astring rectangle_warper<numeric_type>::text_form() const
{
  return basis::astring("<warps from: ") + _system_1.text_form()
      + basis::astring(" with vertex at ") + vertex_name(_vert_1)
      + basis::astring(" into ") + _system_2.text_form()
      + basis::astring(" with vertex at ") + vertex_name(_vert_2)
      + basis::astring(">");
}

} // namespace.

#endif

