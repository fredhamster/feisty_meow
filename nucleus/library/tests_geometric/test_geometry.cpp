/*
*  Name   : test_geometry                                                     *
*  Author : Chris Koeritz                                                     *
*  Purpose:                                                                   *
*    Exercises some of the classes in the geometry library.                   *
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
#include <basis/guards.h>
#include <geometric/cartesian_objects.h>
#include <geometric/circle.h>
#include <geometric/ellipse.h>
#include <geometric/line.h>
#include <geometric/screen_rectangle.h>
#include <geometric/rectangle.h>
#include <geometric/warper.h>
#include <geometric/triangle.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace geometric;
using namespace loggers;
using namespace structures;
using namespace unit_test;
using namespace geometric;

class test_geometric : public virtual unit_base, public virtual application_shell
{
public:
  test_geometric() {}
  DEFINE_CLASS_NAME("test_geometric");
  virtual int execute();
};

int test_geometric::execute()
{
  FUNCDEF("execute");
  // test constructors
  circle fred;
  ellipse tobias;
  line<double> slugmart;
  rectangle<double> burger;
  rectangle_warper<double> space_warp(burger, burger);
  triangle euclid;

  burger = cartesian_rectangle(23, 19, 82, 745);
  ASSERT_TRUE(burger.from_text(astring("84.0 290.0 10.0 912.0")),
      "cartesian from_text test should not return failure");
  ASSERT_FALSE(burger != cartesian_rectangle(84.0, 290.0, 10.0, 912.0),
      "cartesian from_text test should compare with expected value");

  screen_rectangle xingu(23, 19, 82, 745);
  ASSERT_TRUE(xingu == screen_rectangle(screen_point(23, 19), screen_point(82, 745)),
      "xingu screen test construction should agree with expectations");
  ASSERT_TRUE(xingu.from_text(astring("84 290 10 912")),
      "xingu screen from_text test should not return failure");
  ASSERT_TRUE(xingu == screen_rectangle(84, 290, 10, 912),
      "xingu screen from_text test should compare with expected value");

  screen_rectangle guinness(-223, 19, 82, -745);
  ASSERT_TRUE(guinness == screen_rectangle(screen_point(-223, 19), screen_point(82, -745)),
      "guinness screen test construction should agree with expectations");
  ASSERT_TRUE(guinness.from_text(astring("-84 290 -10 912")),
      "guinness screen from_text test should not return failure");
  ASSERT_TRUE(guinness == screen_rectangle(-84, 290, -10, 912),
      "screen from_text test should compare with expected value");

//log(astring(astring::SPRINTF, "the string form is %s.", guinness.text_form().s()));

  // test non-mainstream constructors
  // test non-mainstream constructors

  // test operators

  // test conversions

  // test class specific functions

  // test other things?

  return final_report();
}

HOOPLE_MAIN(test_geometric, );

