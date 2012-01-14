#ifndef SUBNET_CALCULATOR_CLASS
#define SUBNET_CALCULATOR_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : subnet_calculator                                                 *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1997-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>

namespace sockets {

//! Provides an easy way to determine the range of a subnet given the subnet mask and a sample IP address.

class subnet_calculator : public basis::root_object
{
public:
  subnet_calculator(const basis::astring &subnet_mask, const basis::astring &ip_address);
  ~subnet_calculator();

  basis::astring convert(basis::un_int num_format);
  basis::un_int convert(const basis::astring &ip_format);
    // converts between the numerical and string forms of the ip address.

  // these two functions return the computed low / high range of the subnet.
  // they ensure that the subnet calculator is valid by calling the calculate
  // method if it hasn't already been called.
  const basis::astring &low_end();
  const basis::astring &high_end();

  // these allow observation and modification of the parameters that are needed
  // to calculate the subnet range.
  const basis::astring &subnet_mask() const;
  void subnet_mask(const basis::astring &new_mask);
  const basis::astring &ip_address() const;
  void ip_address(const basis::astring &new_address);

  bool valid() const { return _valid; }
    // returns whether the object has recalculated its mask information yet
    // or not.  the object should always be valid until the mask or address
    // are changed.  once the two range methods are called, the object should 
    // be valid afterwards.

private:
  bool _valid;  // is this object valid yet (has calculate been called)?
  basis::astring *_subnet_mask;  // the mask used for the subnets in question.
  basis::astring *_ip_address;  // internet address of an example host on the subnet.
  basis::astring *_low_end;  // lower bound and
  basis::astring *_high_end;   // upper bound of the subnet.

  void calculate();
    // performs the main action of determining the lower and upper bounds
    // of the subnet's range.
};

} //namespace.

#endif

