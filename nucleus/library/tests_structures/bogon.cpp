/*****************************************************************************\
*                                                                             *
*  Name   : bogon                                                             *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    A simple test object for amorphs.                                        *
*                                                                             *
*******************************************************************************
* Copyright (c) 1996-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "bogon.h"

#include <string.h>

using namespace basis;
using namespace structures;

bogon::bogon(abyte *to_copy) : my_held(NIL)
{
  if (to_copy) {
    astring t((char *)to_copy);
    if (t.length()) {
      my_held = new abyte[t.length() + 1];
      t.stuff((char *)my_held, t.length() + 1);
    }
  }
}

bogon::bogon(const bogon &to_copy) : my_held(NIL) { operator = (to_copy); }

bogon &bogon::operator = (const bogon &to_copy) {
  if (this == &to_copy) return *this;
  astring t((char *)to_copy.my_held);
  if (my_held) delete [] my_held;
  my_held = new abyte[t.length() + 1];
  t.stuff((char *)my_held, t.length() + 1);
  return *this;
}

bogon::~bogon() { if (my_held) delete [] my_held; }

abyte *bogon::held() const { return my_held; }

int bogon::size() const { return my_held? int(strlen((char *)my_held) + 1) : 0; }

