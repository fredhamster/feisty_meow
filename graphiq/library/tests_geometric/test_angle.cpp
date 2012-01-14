/*
*  Name   : test_angle                                                        *
*  Author : Chris Koeritz                                                     *
*  Purpose:                                                                   *
*    Tests the angle class.                                                   *
**
* Copyright (c) 2001-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <geometric/angle.h>
#include <loggers/program_wide_logger.h>
#include <mathematics/double_plus.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace geometric;
using namespace loggers;
using namespace mathematics;
using namespace structures;
using namespace unit_test;

typedef double_plus floot;

class test_angle : public virtual unit_base, public virtual application_shell
{
public:
  test_angle() : application_shell() {}
  DEFINE_CLASS_NAME("test_angle");
  virtual int execute();
};

int test_angle::execute()
{
  FUNCDEF("execute");
  {
    // first test group: double angle inverse trigonometrics.
    angle<double> a(30.3, DEGREES);
    ASSERT_EQUAL(floot(a.get(RADIANS)), floot(.528835), "radian conversion should be right");
    
    outcome retval;
    angle<double> at = angle<double>::arctangent(28.3, 29.5, retval);
    ASSERT_EQUAL(floot(at.get(DEGREES)), floot(43.8106), "atan should be what we expect");
    angle<double> as = angle<double>::arcsine(17.6, 82.3, retval);
    ASSERT_EQUAL(floot(as.get(DEGREES)), floot(12.3482), "asin should be what we expect");
    angle<double> ac = angle<double>::arccosine(17.2, 42.0, retval);
    ASSERT_EQUAL(floot(ac.get(DEGREES)), floot(65.8251), "acos should be what we expect");
  }
  {
    // second test: packing an angle.
    angle<double> q(128, DEGREES);
    byte_array pacd;
    int siz = q.packed_size();
    q.pack(pacd);
    ASSERT_EQUAL(siz, pacd.length(), "packed size should report proper length");
    angle<double> x;
    x.unpack(pacd);
    ASSERT_EQUAL(floot(q.get(RADIANS)), floot(x.get(RADIANS)),
        "unpacking should return original value");
    ASSERT_FALSE(pacd.length(), "unpacking should consume entire array");
  }

  return final_report();
}

//////////////

HOOPLE_MAIN(test_angle, )

