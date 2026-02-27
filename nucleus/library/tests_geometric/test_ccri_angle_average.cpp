
/*
 *  Name   : test_ccri_angle_average
 *  Author : Chris Koeritz
 *  Purpose:
 *    Tests the angle averaging method (for angles in degrees)
 *
 * Copyright (c) 2001-$now By Author.  This program is free software; you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either version 2 of
 * the License or (at your option) any later version.  This is online at:
 *     http://www.fsf.org/copyleft/gpl.html
 * Please send any updates to: fred@gruntose.com
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

class test_ccri_angle_average : public virtual unit_base, public virtual application_shell
{
public:
  test_ccri_angle_average() : application_shell() {}
  DEFINE_CLASS_NAME("test_ccri_angle_average");
  virtual int execute();

  // returns average of angles a1 and a2, in degrees.
  double angleAverage(double a1, double a2) {
    a1 = fmod(a1, 360.0);
    a2 = fmod(a2, 360.0);
    if (absolute_value(a1 - a2) > 180.0) {
      if (a1 < 180.0) a1 += 360.0;
      else a2 += 360.0;
    }
    return fmod( (a1 + a2) / 2.0, 360.0);
  }

};

int test_ccri_angle_average::execute()
{
  FUNCDEF("execute");
  
  outcome retval;
  double a1, a2, avg;

  // there could be two right answers for angles 180 degrees apart, but we only test for one.
  a1 = 23; a2 = 203;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(113), a_sprintf("%f and %f 180 degrees apart", a1, a2));
  a1 = 359; a2 = 179;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(269), a_sprintf("%f and %f 180 degrees apart", a1, a2));
  a1 = 90; a2 = 270;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(180), a_sprintf("%f and %f 180 degrees apart", a1, a2));

  // more cases.
  a1 = 89; a2 = 274;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(1.5), a_sprintf("%f and %f", a1, a2));
  a1 = 89.9; a2 = 270.1;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(0), a_sprintf("%f and %f", a1, a2));
  a1 = 0; a2 = 0;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(0), a_sprintf("%f and %f", a1, a2));
  a1 = 0; a2 = 359;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(359.5), a_sprintf("%f and %f", a1, a2));
  a1 = 358; a2 = 359;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(358.5), a_sprintf("%f and %f", a1, a2));
  a1 = 1; a2 = 357;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(359), a_sprintf("%f and %f", a1, a2));
  a1 = 23; a2 = 160;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(91.5), a_sprintf("%f and %f", a1, a2));
  a1 = 47; a2 = 221;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(134), a_sprintf("%f and %f", a1, a2));
  a1 = 113; a2 = 114;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(113.5), a_sprintf("%f and %f", a1, a2));
  a1 = 113; a2 = 270;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(191.5), a_sprintf("%f and %f", a1, a2));
  a1 = 190; a2 = 230;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(210), a_sprintf("%f and %f", a1, a2));
  a1 = 12; a2 = 273;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(322.5), a_sprintf("%f and %f", a1, a2));
  a1 = 181; a2 = 179;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(180), a_sprintf("%f and %f", a1, a2));
  a1 = 89; a2 = 271;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(0), a_sprintf("%f and %f", a1, a2));

  a1 = 359; a2 = 120;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(59.5), a_sprintf("%f and %f", a1, a2));
  a1 = 220; a2 = 359;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(289.5), a_sprintf("%f and %f", a1, a2));
  a1 = 3; a2 = 189;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(276), a_sprintf("%f and %f", a1, a2));
  a1 = 93; a2 = 275;
  avg = angleAverage(a1, a2);
  ASSERT_EQUAL(floot(avg), floot(4), a_sprintf("%f and %f", a1, a2));

  return final_report();
}

//////////////

HOOPLE_MAIN(test_ccri_angle_average, )

