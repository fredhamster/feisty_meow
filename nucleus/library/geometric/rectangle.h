#ifndef RECTANGLE_CLASS
#define RECTANGLE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : rectangle                                                         *
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

#include "point.h"

#include <basis/functions.h>

namespace geometric {

//! Represents a geometric rectangle.

template <class numeric_type>
class rectangle : public basis::packable
{
public:
  rectangle(const point<numeric_type> &vertex_1,
          const point<numeric_type> &vertex_2);
  rectangle(numeric_type x_1 = 0, numeric_type y_1 = 0,
          numeric_type x_2 = 0, numeric_type y_2 = 0);

  numeric_type height() const;
  numeric_type width() const;

  rectangle order() const;
    //!< Re-orders the vertices of the line to be increasing.
    /*!< Orients the vertices such that the x and y coordinates of the first
    vertex are respectively closer to the origin than the x and y
    coordinates of the second vertex (or they are equidistant). */

  point<numeric_type> top_left() const;
  point<numeric_type> bottom_left() const;
  point<numeric_type> top_right() const;
  point<numeric_type> bottom_right() const;
    //!< returns the appropriate point as represented by our rectangle.
    /*!< note that these are with respect to a normal cartesian coordinate
    system.  if you want points for a screen based coordinate system (with
    the origin in the top left), then bottom_left and top_right return the
    appropriate bounding points for that rectangle. */

  numeric_type minimum_x() const;
    //!< Return the smallest x from the points in the rectangle.
  numeric_type minimum_y() const;
    //!< Return the smallest y from the points in the rectangle.
  numeric_type maximum_x() const;
    //!< Return the largest x from the points in the rectangle.
  numeric_type maximum_y() const;
    //!< Return the largest y from the points in the rectangle.

  point<numeric_type> center() const;
    //!< Returns the point at the center of the rectangle.

  bool inside(const point<numeric_type> &to_check) const;
    //!< Returns true if `to_check' is inside `this' rectangle.

  bool operator == (const rectangle &to_compare) const;
    //!< Returns true if `to_compare' has vertices equal to `this'.

  rectangle operator + (const point<numeric_type> &to_add) const;
    //!< Returns the rectangle resulting from adding a point to its vertices.
  rectangle operator - (const point<numeric_type> &to_subtract) const;
    //!< Returns the rectangle resulting from subtracting "to_subtract".

  rectangle &operator += (const point<numeric_type> &to_add);
    //!< Adds the point "to_add" to our vertices.
  rectangle &operator -= (const point<numeric_type> &to_subtract);
    //!< Subtracts the point "to_add" to our vertices.

  void encompass(const rectangle &to_adjust_to);
    //!< Finds the largest dimension needed to contain all rectangles passed in.
    /*!< The original dimension of `this' rectangle is compared with
    all subsequent rectangles passed to adjust_dimension, and it is
    modified (joined with `to_adjust_to') if the extent of `to_adjust_to'
    is greater or lesser than the current extent of `this' rectangle. */

  bool intersect(const rectangle &r2) const;
    //!< Returns true if `this' & `r2' cover any common points.

  bool disjoint(const rectangle &r2) const;
    //!< Returns true if `this' & `r2' have mutually exclusive extents.

  bool join_intersecting(const rectangle &r2, rectangle &result);
    //!< Sets "result" to encompass this and "r2" if they intersect.
    /*!< If `this' and `r2' intersect, `result' is adjusted to their dimension
    and true is returned.  If not, false is returned and `result' is
    undefined. */

  bool intersection(const rectangle &r2, rectangle &result);
    //!< Sets "result" to the intersection of this and "r2".
    /*!< If `this' and `r2' intersect, then `result' is set to their
    intersecting extent and true is returned.  If not, then false is returned
    and `result' is undefined. */

  basis::astring text_form() const;
    //!< Prints out the contents of the rectangle.

  bool from_text(const basis::astring &text);
    //!< Returns true if the "text" is parsed into this rectangle.

  point<numeric_type> vertex_1() const;
  point<numeric_type> vertex_2() const;

  void vertex_1(const point<numeric_type> &to_set);
  void vertex_2(const point<numeric_type> &to_set);

  virtual int packed_size() const;
  virtual void pack(basis::byte_array &packed_form) const;
  virtual bool unpack(basis::byte_array &packed_form);

protected:
  point<numeric_type> _vertex_1;
  point<numeric_type> _vertex_2;
};

//////////////

//!< A commonly used rectangle of integers.

typedef rectangle<int> int_rectangle;

//////////////

// implementations below...

template <class numeric_type>
rectangle<numeric_type>::rectangle(const point<numeric_type> &lb, const point<numeric_type> &rt)
: _vertex_1(lb), _vertex_2(rt) {}

template <class numeric_type>
rectangle<numeric_type>::rectangle(numeric_type left, numeric_type bottom, numeric_type right, numeric_type top)
: _vertex_1(point<numeric_type>(left, bottom)),
  _vertex_2(point<numeric_type>(right, top)) {}

template <class numeric_type>
point<numeric_type> rectangle<numeric_type>::vertex_1() const
{ return _vertex_1; }

template <class numeric_type>
point<numeric_type> rectangle<numeric_type>::vertex_2() const
{ return _vertex_2; }

template <class numeric_type>
void rectangle<numeric_type>::vertex_1(const point<numeric_type> &to_set)
{ _vertex_1 = to_set; }

template <class numeric_type>
void rectangle<numeric_type>::vertex_2(const point<numeric_type> &to_set)
{ _vertex_2 = to_set; }

template <class numeric_type>
numeric_type rectangle<numeric_type>::height() const
{ return absolute_value(_vertex_2.y() - _vertex_1.y()); }

template <class numeric_type>
numeric_type rectangle<numeric_type>::width() const
{ return absolute_value(_vertex_2.x() - _vertex_1.x()); }

template <class numeric_type>
numeric_type rectangle<numeric_type>::minimum_x() const
{ return basis::minimum(_vertex_1.x(), _vertex_2.x()); }

template <class numeric_type>
numeric_type rectangle<numeric_type>::minimum_y() const
{ return basis::minimum(_vertex_1.y(), _vertex_2.y()); }

template <class numeric_type>
numeric_type rectangle<numeric_type>::maximum_x() const
{ return basis::maximum(_vertex_1.x(), _vertex_2.x()); }

template <class numeric_type>
numeric_type rectangle<numeric_type>::maximum_y() const
{ return basis::maximum(_vertex_1.y(), _vertex_2.y()); }

template <class numeric_type>
rectangle<numeric_type> rectangle<numeric_type>::order() const
{
  numeric_type x1 = _vertex_1.x();
  numeric_type x2 = _vertex_2.x();
  numeric_type y1 = _vertex_1.y();
  numeric_type y2 = _vertex_2.y();
  basis::flip_increasing(x1, x2);
  basis::flip_increasing(y1, y2);
  return rectangle<numeric_type>(x1, y1, x2, y2);
}

template <class numeric_type>
point<numeric_type> rectangle<numeric_type>::top_left() const
{
  rectangle temp(order());
  return point<numeric_type>(temp.vertex_1().x(), temp.vertex_2().y());
}

template <class numeric_type>
point<numeric_type> rectangle<numeric_type>::bottom_left() const
{
  rectangle temp(order());
  return point<numeric_type>(temp.vertex_1().x(), temp.vertex_1().y());
}

template <class numeric_type>
point<numeric_type> rectangle<numeric_type>::top_right() const
{
  rectangle temp(order());
  return point<numeric_type>(temp.vertex_2().x(), temp.vertex_2().y());
}

template <class numeric_type>
point<numeric_type> rectangle<numeric_type>::bottom_right() const
{
  rectangle temp(order());
  return point<numeric_type>(temp.vertex_2().x(), temp.vertex_1().y());
}

template <class numeric_type>
point<numeric_type> rectangle<numeric_type>::center() const
{
  return point<numeric_type>(numeric_type((_vertex_1.x()
      + _vertex_2.x()) / 2.0), numeric_type((_vertex_1.y()
      + _vertex_2.y()) / 2.0));
}

template <class numeric_type>
bool rectangle<numeric_type>::inside(const point<numeric_type> &to_check) const
{
  rectangle<numeric_type> ordered_me = this->order();
  return bool( (to_check.x() >= ordered_me._vertex_1.x())
    && (to_check.x() <= ordered_me._vertex_2.x())
    && (to_check.y() >= ordered_me._vertex_1.y())
    && (to_check.y() <= ordered_me._vertex_2.y()) );
}

template <class numeric_type>
bool rectangle<numeric_type>::operator == (const rectangle &to_compare) const
{
  point<numeric_type> min1(minimum_x(), minimum_y());
  point<numeric_type> max1(maximum_x(), maximum_y());
  point<numeric_type> min2(to_compare.minimum_x(), to_compare.minimum_y());
  point<numeric_type> max2(to_compare.maximum_x(), to_compare.maximum_y());
  if ( (min1 == min2) && (max1 == max2) ) return true;
  else return false;
}

template <class numeric_type>
rectangle<numeric_type> &rectangle<numeric_type>::operator += (const point<numeric_type> &p)
{ _vertex_1 += p; _vertex_2 += p; return *this; }

template <class numeric_type>
rectangle<numeric_type> &rectangle<numeric_type>::operator -= (const point<numeric_type> &p)
{ _vertex_1 -= p; _vertex_2 -= p; return *this; }

template <class numeric_type>
rectangle<numeric_type> rectangle<numeric_type>::operator + (const point<numeric_type> &p) const
{
  rectangle to_return(*this);
  to_return += p;
  return to_return;
}

template <class contents>
int rectangle<contents>::packed_size() const
{
  basis::byte_array temp;
//hmmm: inefficient!
  pack(temp);
  return temp.length();
}

template <class contents>
void rectangle<contents>::pack(basis::byte_array &packed_form) const
{
  _vertex_1.pack(packed_form);
  _vertex_2.pack(packed_form);
}

template <class contents>
bool rectangle<contents>::unpack(basis::byte_array &packed_form)
{
  if (!_vertex_1.unpack(packed_form)) return false;
  if (!_vertex_2.unpack(packed_form)) return false;
  return true;
}

template <class numeric_type>
rectangle<numeric_type> rectangle<numeric_type>::operator - (const point<numeric_type> &p) const
{
  rectangle to_return(*this);
  to_return -= p;
  return to_return;
}

template <class numeric_type>
void rectangle<numeric_type>::encompass(const rectangle &to_adjust_to)
{
  if (to_adjust_to._vertex_1.x() < _vertex_1.x())
    _vertex_1.set(to_adjust_to._vertex_1.x(), _vertex_1.y());
  if (to_adjust_to._vertex_1.y() < _vertex_1.y())
    _vertex_1.set(_vertex_1.x(), to_adjust_to._vertex_1.y());
  if (to_adjust_to._vertex_2.x() > _vertex_2.x())
    _vertex_2.set(to_adjust_to._vertex_2.x(), _vertex_2.y());
  if (to_adjust_to._vertex_2.y() > _vertex_2.y())
    _vertex_2.set(_vertex_2.x(), to_adjust_to._vertex_2.y());
}

template <class numeric_type>
bool rectangle<numeric_type>::disjoint(const rectangle &r2) const
{
  if ( (maximum_x() < r2.minimum_x())
      || (minimum_x() > r2.maximum_x())
      || (maximum_y() < r2.minimum_y())
      || (minimum_y() > r2.maximum_y()) ) return true;
  else return false;
}

template <class numeric_type>
bool rectangle<numeric_type>::intersect(const rectangle &r2) const
{ return bool(!disjoint(r2)); }

template <class numeric_type>
bool rectangle<numeric_type>::join_intersecting(const rectangle &r2, rectangle &result)
{
  if (disjoint(r2)) return false;
  result = *this;
  result.encompass(r2);
  return true;
}

template <class numeric_type>
bool rectangle<numeric_type>::intersection(const rectangle &r2, rectangle &result)
{
  if (disjoint(r2)) return false;
  result = rectangle<numeric_type>(basis::maximum(minimum_x(), r2.minimum_x()),
      basis::maximum(minimum_y(), r2.minimum_y()),
      basis::minimum(maximum_x(), r2.maximum_x()),
      basis::minimum(maximum_y(), r2.maximum_y()));
  return true;
}

template <class numeric_type>
basis::astring rectangle<numeric_type>::text_form() const
{
  return basis::astring("[") + _vertex_1.text_form() + basis::astring(" ")
      + _vertex_2.text_form() + basis::astring("]");
}

template <class numeric_type>
bool rectangle<numeric_type>::from_text(const basis::astring &_text)
{
  numeric_type nums[4] = { 0, 0, 0, 0 };
  // setup the scanning specifier.
  basis::astring spec(numeric_specifier(nums[0]));
  // scan the string for values.
  basis::astring text(_text);
  for (int i = 0; i < 4; i++) {
    text = crop_non_numeric(text);
    nums[i] = text.convert(nums[i]);
    text = crop_numeric(text);
  }
  vertex_1(point<numeric_type>(nums[0], nums[1]));
  vertex_2(point<numeric_type>(nums[2], nums[3]));
  return true;
}

} // namespace.

#endif

