#ifndef ANGLE_CLASS
#define ANGLE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : angle                                                             *
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

#include <basis/byte_array.h>
#include <basis/common_outcomes.h>
#include <basis/contracts.h>
#include <structures/object_packers.h>

#include <math.h>

namespace geometric {

//! Represents a geometric angle.

//! angles can be measured in degrees or radians in this class.
enum angular_units { DEGREES, RADIANS };

template <class contents>
class angle : public basis::packable
{
public:
  DEFINE_CLASS_NAME("angle");

  angle(contents inital_rotation = 0, angular_units unit = RADIANS);
    //!< constructs a new angle with "initial_rotation" in the "unit".

  void set(contents a, angular_units unit);
    //!< sets the angle to a new rotation "a" in the "unit".
  contents get(angular_units unit) const;
    //!< retrieves the current angular measure.

  angle operator - (void) const;
    //!< returns the negation of this angle.

  angle operator + (const angle &to_add) const;
  angle operator - (const angle &to_subtract) const;
  angle operator * (contents to_multiply) const;
  angle operator / (contents to_divide) const;
  angle &operator += (const angle &to_add);
  angle &operator -= (const angle &to_subtract);
  angle &operator *= (contents to_multiply);
  angle &operator /= (contents to_divide);

  contents sine() const;
    //!< returns the sin function of this angle.
  contents cosine() const;
    //!< returns the cos function of this angle.
  contents tangent() const;
    //!< returns the tan function of this angle.

  static angle arctangent(contents opposite, contents adjacent,
          basis::outcome &retval);
    //!< returns the atan of the angle.
    /*!< the outcome will be set to OKAY if the function operated successfully.
    otherwise it will be set to BAD_INPUT. */
  static angle arccosine(contents adjacent, contents hypotenuse,
          basis::outcome &retval);
    //!< returns the acos of the angle.
  static angle arcsine(contents opposite, contents hypotenuse,
          basis::outcome &retval);
    //!< returns the asin of the angle.

  virtual int packed_size() const;
  virtual void pack(basis::byte_array &packed_form) const;
    //!< packs the angle for shipping in bytes.
  virtual bool unpack(basis::byte_array &packed_form);
    //!< unpacks the angle from the "packed_form".

private:
  contents _theta;  //!< the held angular measure.

  contents to_internal(contents initial, angular_units unit) const;
    //!< converts the angle into the units we use inside the class.
  contents from_internal(contents initial, angular_units unit) const;
    //!< converts the angle from our internal measure into "unit" measure.
};

//////////////

//! double_angle provides a non-templated class for forward declarations.

class double_angle : public angle<double>
{
public:
  double_angle(double init = 0, angular_units unit = RADIANS)
      : angle<double>(init, unit) {}
  double_angle(const angle<double> &to_copy) : angle<double>(to_copy) {}
};

//////////////

// implementation of larger methods below.

template <class contents>
angle<contents>::angle(contents a, angular_units unit) { set(a, unit); }

template <class contents>
angle<contents> angle<contents>::operator - (void) const
{ angle<contents> to_return(*this); to_return *= -1; return to_return; }

template <class contents>
angle<contents> angle<contents>::operator + (const angle<contents> &a) const
{ angle<contents> to_return(*this); to_return += a; return to_return; }

template <class contents>
angle<contents> angle<contents>::operator - (const angle<contents> &a) const
{ angle<contents> to_return(*this); to_return -= a; return to_return; }

template <class contents>
angle<contents> angle<contents>::operator * (contents to_multiply) const
{
  angle<contents> to_return(*this);
  to_return *= to_multiply;
  return to_return;
}

template <class contents>
angle<contents> angle<contents>::operator / (contents to_divide) const
{ angle<contents> to_return(*this); to_return /= to_divide; return to_return; }

template <class contents>
angle<contents> &angle<contents>::operator += (const angle<contents> &a)
{ _theta += a._theta; return *this; }

template <class contents>
angle<contents> &angle<contents>::operator -= (const angle<contents> &a)
{ _theta -= a._theta; return *this; }

template <class contents>
angle<contents> &angle<contents>::operator *= (contents f)
{ _theta *= f; return *this; }

template <class contents>
angle<contents> &angle<contents>::operator /= (contents f)
{ _theta /= f; return *this; }

template <class contents>
contents angle<contents>::sine() const { return sin(_theta); }

template <class contents>
contents angle<contents>::cosine() const { return cos(_theta); }

template <class contents>
contents angle<contents>::tangent() const { return tan(_theta); }

template <class contents>
int angle<contents>::packed_size() const
{
  basis::byte_array temp;
//hmmm: inefficient!
  pack(temp);
  return temp.length();
}

template <class contents>
void angle<contents>::pack(basis::byte_array &packed_form) const
{ structures::attach(packed_form, _theta); }

template <class contents>
bool angle<contents>::unpack(basis::byte_array &packed_form)
{ return structures::detach(packed_form, _theta); }

template <class contents>
contents angle<contents>::to_internal(contents a, angular_units unit) const
{
  switch(unit) {
    case RADIANS: return a;
    case DEGREES: return a * PI_APPROX / 180.0;
    default: return 0;
  }
}

template <class contents>
contents angle<contents>::from_internal(contents a, angular_units unit) const
{
  switch(unit) {
    case RADIANS: return a;
    case DEGREES: return a * 180.0 / PI_APPROX;
    default: return 0;
  }
}

template <class contents>
void angle<contents>::set(contents a, angular_units unit)
{ _theta = to_internal(a, unit); }

template <class contents>
contents angle<contents>::get(angular_units unit) const
{ return from_internal(_theta, unit); }

template <class contents>
angle<contents> angle<contents>::arccosine(contents adjacent,
    contents hypotenuse, basis::outcome &retval)
{
  contents d = adjacent / hypotenuse;
  retval = basis::common::BAD_INPUT;
  bounds_return(d, -1.0, 1.0, angle<contents>());
  retval = basis::common::OKAY;
  return angle<contents>(acos(d), RADIANS);
}

template <class contents>
angle<contents> angle<contents>::arcsine(contents opposite, contents hypotenuse,
    basis::outcome &retval)
{
  contents d = opposite / hypotenuse;
  retval = basis::common::BAD_INPUT;
  bounds_return(d, -1.0, 1.0, angle<contents>());
  retval = basis::common::OKAY;
  return angle<contents>(asin(d), RADIANS);
}

template <class contents>
angle<contents> angle<contents>::arctangent(contents opposite, contents adjacent,
    basis::outcome &retval)
{
  retval = basis::common::BAD_INPUT;
  if ( (adjacent == 0.0) && (opposite == 0.0) ) return angle<contents>();
  retval = basis::common::OKAY;
  return angle<contents>(atan2(opposite, adjacent), RADIANS);
}

} // namespace.

#endif

