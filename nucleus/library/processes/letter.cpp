/*****************************************************************************\
*                                                                             *
*  Name   : letter                                                            *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1998-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "letter.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <timely/time_stamp.h>

using namespace basis;
using namespace timely;

namespace processes {

letter::letter(int type, int start_after)
: _type(type),
  _ready_time(new time_stamp(start_after))
{}

letter::letter(const letter &to_copy)
: _type(to_copy._type),
  _ready_time(new time_stamp(*to_copy._ready_time))
{
}

letter::~letter()
{
  _type = 0;
  WHACK(_ready_time);
}

bool letter::ready_to_send() { return time_stamp() >= *_ready_time; }

void letter::set_ready_time(int start_after)
{ *_ready_time = time_stamp(start_after); }

letter &letter::operator =(const letter &to_copy)
{
  if (this == &to_copy) return *this;
  _type = to_copy._type;
  *_ready_time = *to_copy._ready_time;
  return *this;
}

} //namespace.

