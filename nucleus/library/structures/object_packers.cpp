/*****************************************************************************\
*                                                                             *
*  Name   : object_packers                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1995-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "object_packers.h"

#include <math.h>

using namespace basis;

namespace structures {

// rotate_in and snag_out do most of the real "work", if any.

void rotate_in(byte_array &attach_into, int to_attach, int size_in_bytes)
{
  basis::un_int temp = basis::un_int(to_attach);
  for (int i = 0; i < size_in_bytes; i++) {
    attach_into += abyte(temp % 0x100);
    temp >>= 8;
  }
}

void snag_out(byte_array &eat_from, basis::un_int &accumulator, int size_in_bytes)
{
  accumulator = 0;
  for (int i = 0; i < size_in_bytes; i++) {
    accumulator <<= 8;
    accumulator += eat_from[size_in_bytes - i - 1];
  }
  eat_from.zap(0, size_in_bytes - 1);
}

//////////////

int packed_size(const byte_array &packed_form)
{ return 2 * sizeof(int) + packed_form.length(); }

void attach(byte_array &packed_form, const byte_array &to_attach)
{
  obscure_attach(packed_form, to_attach.length());
  packed_form += to_attach;
}

bool detach(byte_array &packed_form, byte_array &to_detach)
{
  un_int len = 0;
  if (!obscure_detach(packed_form, len)) return false;
  if (packed_form.length() < (int)len) return false;
  to_detach = packed_form.subarray(0, len - 1);
  packed_form.zap(0, len - 1);
  return true;
}

//////////////

// these are the only "real" attach/detach functions on number types.  the
// others are all faking it by calling these.

void attach(byte_array &packed_form, basis::un_int to_attach)
{ rotate_in(packed_form, to_attach, 4); }

bool detach(byte_array &packed_form, basis::un_int &to_detach)
{
  if (packed_form.length() < 4) return false;
  basis::un_int temp;
  snag_out(packed_form, temp, 4);
  to_detach = basis::un_int(temp);
  return true;
}

void attach(byte_array &packed_form, basis::un_short to_attach)
{ rotate_in(packed_form, to_attach, 2); }

bool detach(byte_array &packed_form, basis::un_short &to_detach)
{
  if (packed_form.length() < 2) return false;
  basis::un_int temp;
  snag_out(packed_form, temp, 2);
  to_detach = basis::un_short(temp);
  return true;
}

void attach(byte_array &packed_form, abyte to_attach)
{ packed_form += to_attach; }

bool detach(byte_array &packed_form, abyte &to_detach)
{
  if (packed_form.length() < 1) return false;
  to_detach = packed_form[0];
  packed_form.zap(0, 0);
  return true;
}

//////////////

void attach(byte_array &packed_form, int to_attach)
{ attach(packed_form, basis::un_int(to_attach)); }

bool detach(byte_array &packed_form, int &to_detach)
{ return detach(packed_form, (basis::un_int &)to_detach); }

//void attach(byte_array &packed_form, basis::un_long to_attach)
//{ attach(packed_form, basis::un_int(to_attach)); }

//bool detach(byte_array &packed_form, basis::un_long &to_detach)
//{ return detach(packed_form, (basis::un_int &)to_detach); }

//void attach(byte_array &packed_form, long to_attach)
//{ attach(packed_form, basis::un_int(to_attach)); }

//bool detach(byte_array &packed_form, long &to_detach)
//{ return detach(packed_form, (basis::un_int &)to_detach); }

void attach(byte_array &packed_form, short to_attach)
{ attach(packed_form, basis::un_short(to_attach)); }

bool detach(byte_array &packed_form, short &to_detach)
{ return detach(packed_form, (basis::un_short &)to_detach); }

void attach(byte_array &packed_form, char to_attach)
{ attach(packed_form, abyte(to_attach)); }

bool detach(byte_array &packed_form, char &to_detach)
{ return detach(packed_form, (abyte &)to_detach); }

void attach(byte_array &packed_form, bool to_attach)
{ attach(packed_form, abyte(to_attach)); }

//////////////

// can't assume that bool is same size as byte, although it should fit
// into a byte just fine.
bool detach(byte_array &packed_form, bool &to_detach)
{
  abyte chomp;
  if (!detach(packed_form, chomp)) return false;
  to_detach = !!chomp;
  return true;
}

// operates on a number less than 1.0 that we need to snag the next digit
// to the right of the decimal point from.
double break_off_digit(double &input) {
//printf(astring(astring::SPRINTF, "break input=%f\n", input).s());
  input *= 10.0;
//printf(astring(astring::SPRINTF, "after mult=%f\n", input).s());
  double mod_part = fmod(input, 1.0);
//printf(astring(astring::SPRINTF, "modded=%f\n", mod_part).s());
  double to_return = input - mod_part;
//printf(astring(astring::SPRINTF, "to ret=%f\n", to_return).s());
  input -= to_return;
  return to_return;
}

//hmmm: not very efficient!  it's just packing and wasting bytes doing it...
int packed_size(double to_pack)
{
  byte_array packed;
  attach(packed, to_pack);
  return packed.length();
}

void attach(byte_array &packed_form, double to_pack)
{
  int exponent = 0;
  double mantissa = frexp(to_pack, &exponent);
  abyte pos = mantissa < 0.0? false : true;
  mantissa = fabs(mantissa);
//printf("mant=%10.10f pos=%d expon=%d\n", mantissa, int(pos), exponent);
  packed_form += pos;
  attach(packed_form, exponent);
  byte_array mantis;
  // even if the double has 52 bits for mantissa (where ms docs say 44),
  // a 16 digit bcd encoded number should handle the size (based on size of
  // 2^52 in digits).
  for (int i = 0; i < 9; i++) {
    double dig1 = break_off_digit(mantissa);
//printf(astring(astring::SPRINTF, "break digit=%d\n", int(dig1)).s());
    double dig2 = break_off_digit(mantissa);
//printf(astring(astring::SPRINTF, "break digit=%d\n", int(dig2)).s());
    mantis += abyte(dig1 * 16 + dig2);
  }
  attach(packed_form, mantis);
//printf("attach exit\n");
}

bool detach(byte_array &packed_form, double &to_unpack)
{
//printf("detach entry\n");
  if (packed_form.length() < 1) return false;  // no sign byte.
  abyte pos = packed_form[0];
//printf(astring(astring::SPRINTF, "pos=%d\n", int(pos)).s());
  packed_form.zap(0, 0);
  int exponent;
  if (!detach(packed_form, exponent)) return false;
//printf(astring(astring::SPRINTF, "expon=%d\n", exponent).s());
  byte_array mantis;
  if (!detach(packed_form, mantis)) return false;
  double mantissa = 0;
  for (int i = mantis.last(); i >= 0; i--) {
    abyte chop = mantis[i];
    double dig1 = chop / 16;
//printf(astring(astring::SPRINTF, "break digit=%d\n", int(dig1)).s());
    double dig2 = chop % 16;
//printf(astring(astring::SPRINTF, "break digit=%d\n", int(dig2)).s());
    mantissa += dig2;
    mantissa /= 10;
    mantissa += dig1;
    mantissa /= 10;
  }
//printf(astring(astring::SPRINTF, "mant=%10.10f\n", mantissa).s());
  to_unpack = ldexp(mantissa, exponent);
  if (!pos) to_unpack = -1.0 * to_unpack;
//printf("pos=%d\n", int(pos));
//printf(astring(astring::SPRINTF, "to_unpack=%f\n", to_unpack).s());
//printf("detach exit\n");
  return true;
}

void attach(byte_array &packed_form, float to_pack)
{ attach(packed_form, double(to_pack)); }

bool detach(byte_array &packed_form, float &to_unpack)
{
  double real_unpack;
  bool to_return = detach(packed_form, real_unpack);
  to_unpack = (float)real_unpack;
  return to_return;
}

//////////////

void obscure_attach(byte_array &packed_form, un_int to_attach)
{
//printf("initial value=%x\n", to_attach);
  basis::un_int first_part = 0xfade0000;
//printf("first part curr=%x\n", first_part);
  basis::un_int second_part = 0x0000ce0f;
//printf("second part curr=%x\n", second_part);
  first_part = first_part | (to_attach & 0x0000ffff);
//printf("first part now=%x\n", first_part);
  second_part = second_part | (to_attach & 0xffff0000);
//printf("second part now=%x\n", second_part);
  attach(packed_form, first_part);
  attach(packed_form, second_part);
}

bool obscure_detach(byte_array &packed_form, un_int &to_detach)
{
  basis::un_int first_part;
  basis::un_int second_part;
  if (!detach(packed_form, first_part)) return false;
  if (!detach(packed_form, second_part)) return false;
//printf("first part after unpack=%x\n", first_part);
//printf("second part after unpack=%x\n", second_part);
  if (basis::un_int(first_part & 0xffff0000) != basis::un_int(0xfade0000)) return false;
//printf("first part with and=%x\n", first_part & 0xffff0000);
  if (basis::un_int(second_part & 0x0000ffff) != basis::un_int(0x0000ce0f)) return false;
//printf("second part with and=%x\n", second_part & 0x0000ffff);
  to_detach = int( (second_part & 0xffff0000) + (first_part & 0x0000ffff) );
//printf("final result=%x\n", to_detach);
  return true;
}

//////////////

} // namespace

