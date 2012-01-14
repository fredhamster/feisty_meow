/*****************************************************************************\
*                                                                             *
*  Name   : test_point                                                        *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Tests out the point class.                                               *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/guards.h>
#include <geometric/point.h>
#include <loggers/console_logger.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace geometric;
using namespace loggers;
using namespace structures;
using namespace unit_test;

class test_point : virtual public unit_base, virtual public application_shell
{
public:
  test_point() : application_shell() {}
  DEFINE_CLASS_NAME("test_point");
  virtual int execute();
};

int test_point::execute()
{
  FUNCDEF("execute");
  {
    // first test just instantiates some things.
    point<double> fred(23, angle<double>(4));
    point<double> bob(399, angle<double>(2.3));
    double dist = bob.distance(fred);
//LOG(astring("fred is ") + fred + " and bob is " + bob);
//LOG(a_sprintf("distance between is ", dist));
    point<double> borg(fred - bob);
//LOG(astring("borg is fred-bob, which is ") + borg);
    ASSERT_FALSE(borg.magnitude() - dist > 0.001,
        "difference must be small between distance and magnitude");
  }

  {
    astring pt1 = "12,38";
    point<double> to_scan;
    to_scan.from_text(pt1);
    ASSERT_FALSE( (to_scan.x() != 12) || (to_scan.y() != 38),
        "second test: first from_text should work");
    astring pt2 = "{14.3,16.2989}";
    to_scan.from_text(pt2);
    ASSERT_FALSE( (to_scan.x() != 14.3) || (to_scan.y() != 16.2989),
        "second test: second from_text should work too");
  }

  return final_report();
}

HOOPLE_MAIN(test_point, )

