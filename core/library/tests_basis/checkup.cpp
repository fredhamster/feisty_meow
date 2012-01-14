/*
*  Name   : checkup
*  Author : Chris Koeritz
**
* Copyright (c) 1990-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include "checkup.h"

#include <basis/astring.h>
#include <loggers/critical_events.h>

using namespace basis;
using namespace loggers;
using namespace unit_test;

namespace system_checkup {

#undef UNIT_BASE_THIS_OBJECT 
#define UNIT_BASE_THIS_OBJECT testing
#undef static_class_name
#define static_class_name() astring("system_checkup")

bool check_system_characteristics(unit_base &testing)
{
  FUNCDEF("check_system_characteristics")
  // a big assumption is that the size of an unsigned character is just
  // one byte.  if this is not true, probably many things would break...
  int byte_size = sizeof(abyte);
  ASSERT_EQUAL(byte_size, 1, "byte size should be 1 byte");
  int int16_size = sizeof(int16);
  ASSERT_FALSE( (sizeof(uint16) != int16_size) || (int16_size != 2),
      "uint16 size should be 2 bytes");
  int uint_size = sizeof(un_int);
//hmmm: the checks are actually redundant...
  ASSERT_FALSE( (uint_size != sizeof(int)) || (uint_size != 4),
      "un_int size should be 2 bytes");
  // all tests successfully passed.
  return true;
}

#undef UNIT_BASE_THIS_OBJECT 
#define UNIT_BASE_THIS_OBJECT (*this)
#undef static_class_name

} // namespace

