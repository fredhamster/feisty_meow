/*
*  Name   : test_ellipse                                                      *
*  Author : Chris Koeritz                                                     *
*  Purpose:                                                                   *
*    Tests the ellipse class.                                                 *
**
* Copyright (c) 1993-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/guards.h>
#include <geometric/cartesian_objects.h>
#include <geometric/ellipse.h>
#include <geometric/point.h>
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

typedef cartesian_point e_point; 

class test_ellipse : virtual public unit_base, virtual public application_shell
{
public:
  test_ellipse() : application_shell() {}
  DEFINE_CLASS_NAME("test_ellipse");
  int execute();
  point<double> supposed_good_value(const angle<double> &rotation);
};

point<double> test_ellipse::supposed_good_value(const angle<double> &rotation)
{
  double_plus rot(rotation.get(DEGREES));
//log(a_sprintf("rotation coming in is %f", rot.value()));
  if (rot == double_plus(0.0)) return point<double>(25.000000, 20.0000);
  if (rot == double_plus(35.3)) return point<double>(24.7134, 23.3372);
  if (rot == double_plus(70.6)) return point<double>(22.8791, 28.1757);
  if (rot == double_plus(105.9)) return point<double>(17.5249, 11.3112);
  if (rot == double_plus(141.2)) return point<double>(15.3608, 16.2700);
  if (rot == double_plus(176.5)) return point<double>(15.0023, 19.6943);
  if (rot == double_plus(211.8)) return point<double>(15.2242, 22.9611);
  if (rot == double_plus(247.1)) return point<double>(16.7732, 27.6388);
  if (rot == double_plus(282.4)) return point<double>(22.0127, 10.8459);
  if (rot == double_plus(317.7)) return point<double>(24.5511, 15.8588);
  if (rot == double_plus(353.0)) return point<double>(24.9906, 19.3872);
  return point<double>(0, 0);  // unknown angle.
}

int test_ellipse::execute()
{
  FUNCDEF("execute");
  ellipse fred(e_point(20, 20), 5, 10);
  for (double i = 0; i < 360.0; i += 35.3) {
    e_point where(fred.location(double_angle(i, DEGREES)));
    a_sprintf test_name("%.2f", double_angle(i, DEGREES).get(DEGREES));
//    log(astring(astring::SPRINTF, "at angle %f ellipse is at ", i) + where.text_form());
    point<double> compare = supposed_good_value(double_angle(i, DEGREES));
    // right now point is not orderable, so we compare x and y but use the same test name.
    ASSERT_EQUAL(double_plus(where.x()), double_plus(compare.x()),
        test_name + " rotation should have proper position");
    ASSERT_EQUAL(double_plus(where.y()), double_plus(compare.y()),
        test_name + " rotation should have proper position");
  }
  return final_report();
}

//////////////

HOOPLE_MAIN(test_ellipse, )

