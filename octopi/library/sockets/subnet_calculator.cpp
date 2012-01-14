


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

//hmmm: this class only handles 32 bit (4 byte) internet addresses.  it should
//      use an arbitrary length integer object to overcome differences in the
//      sizes of internet addresses.
//      really we should just start using the machine uid underneath...

#include "subnet_calculator.h"

#include <basis/functions.h>
#include <basis/astring.h>

using namespace basis;

namespace sockets {

subnet_calculator::subnet_calculator(const astring &mask, const astring &samp)
: _valid(false),
  _subnet_mask(new astring(mask)),
  _ip_address(new astring(samp)),
  _low_end(new astring("")),
  _high_end(new astring(""))
{ calculate(); }

subnet_calculator::~subnet_calculator()
{
  WHACK(_subnet_mask);
  WHACK(_ip_address);
  WHACK(_low_end);
  WHACK(_high_end);
  _valid = false;
}

const astring &subnet_calculator::subnet_mask() const { return *_subnet_mask; }

const astring &subnet_calculator::ip_address() const { return *_ip_address; }

void subnet_calculator::subnet_mask(const astring &new_mask)
{
  _valid = false;
  *_subnet_mask = new_mask;
}

void subnet_calculator::ip_address(const astring &new_address)
{
  _valid = false;
  *_ip_address = new_address;
}

const astring &subnet_calculator::low_end()
{
  calculate();
  return *_low_end;
}

const astring &subnet_calculator::high_end()
{
  calculate();
  return *_high_end;
}

astring subnet_calculator::convert(basis::un_int num_format)
{
  astring to_return;
  basis::un_int temp_num = num_format;
  for (int i = 0; i < 4; i++) {
    if (to_return.t())
      to_return = astring(".") + to_return;
    // shave a byte off for inclusion in the string.
    basis::un_int new_byte = temp_num % 256;
    temp_num >>= 8;
    to_return = astring(astring::SPRINTF, "%d", new_byte) + to_return;
  }
  return to_return;
}

un_int subnet_calculator::convert(const astring &ip_format)
{
  basis::un_int to_return = 0;
  astring ip_temp = ip_format;
  for (int i = 0; i < 3; i++) {
    int indy = ip_temp.find(".");
    if (indy < 0) return to_return;
    astring this_piece = ip_temp.substring(0, indy - 1);
    to_return <<= 8;
    to_return += this_piece.convert(0);
    ip_temp.zap(0, indy);
  }

  if (ip_temp.length()) {
    to_return <<= 8;
    to_return += ip_temp.convert(0);
  }

  return to_return;
}

void subnet_calculator::calculate()
{
  if (valid()) return;  // already valid.

  basis::un_int ip = convert(*_ip_address);
  basis::un_int sub = convert(*_subnet_mask);

  basis::un_int low_end = sub & ip;
    // strips off the host part of the ip address and leaves just the network
    // component that comes from the ip address.

  basis::un_int temp_sub = sub;
  int bits_to_add = 0;
  // we shift the subnet mask until we find it's first lowest order "1" bit.
  // this tells us how many bits are in the host portion (unless the mask is
  // zero, in which case the host is all of the bits).
  while (temp_sub && !(temp_sub % 2)) {
    temp_sub >>= 1;  // shift off a bit.
    bits_to_add++;  // record that the place we were at had a zero bit.
  }
  if (!sub) bits_to_add = 32;
    // account for a blank subnet mask, meaning there is no network and all
    // bits are used for host addresses.
  basis::un_int add_in_for_bcast = 0;
    // the part added in to make a broadcast address.  this is all ones.
  for (int i = 0; i < bits_to_add; i++) {
    add_in_for_bcast <<= 1;  // shift the existing add_in to the left.
    add_in_for_bcast++;  // make a new one bit in the ones place.
  }

  basis::un_int high_end = low_end + add_in_for_bcast;

  *_low_end = convert(low_end);
  *_high_end = convert(high_end);
  _valid = true;
}

} //namespace.


