/*
*  Name   : test_double_plus
*  Author : Chris Koeritz
*  Purpose: Tests the double_plus class out.
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

class test_double_plus : public virtual unit_base, public virtual application_shell
{
public:
  test_double_plus() : application_shell() {}
  DEFINE_CLASS_NAME("test_double_plus");
  virtual int execute();
};

int test_double_plus::execute()
{
  FUNCDEF("execute");
  floot x1 = 43.8106392325;
  floot x2 = 43.8106;
  ASSERT_EQUAL(x1, x2, "these doubles should be close enough");

  floot y1 = 16.78;
  floot y2 = 16.798273773;
  ASSERT_INEQUAL(y1, y2, "these doubles shouldn't be close enough");

  floot z1(16.8, 0.1);
  floot z2(16.798273773, 0.1);
  ASSERT_EQUAL(a_sprintf("%.3f", z2.truncate()), astring("16.800"),
      "truncate should calculate proper string");
  ASSERT_EQUAL(z1, z2, "these doubles should be close enough with short delta");

  floot q1(16.75, 0.01);
  floot q2(16.749273773, 0.01);
  ASSERT_EQUAL(a_sprintf("%.3f", q2.truncate()), astring("16.750"),
      "wider truncate should calculate proper string");
  ASSERT_EQUAL(q1, q2, "next couple doubles should be close enough with small delta");

  return final_report();
}

//////////////

HOOPLE_MAIN(test_double_plus, )

