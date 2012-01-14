/*****************************************************************************\
*                                                                             *
*  Name   : test_rectangle_warper                                             *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Tests the rectangle warper class.                                        *
*                                                                             *
*******************************************************************************
* Copyright (c) 1993-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <geometric/warper.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace geometric;
using namespace loggers;
using namespace structures;
using namespace unit_test;
using namespace geometric;

class test_rectangle_warper : public virtual unit_base, virtual public application_shell
{
public:
  test_rectangle_warper() {}
  DEFINE_CLASS_NAME("test_rectangle_warper");
  virtual int execute();
};

int test_rectangle_warper::execute()
{
  FUNCDEF("execute");
  rectangle<double> inner(-20, 0, -60, 30);
  rectangle<double> outer(20, 30, 0, 0);
  rectangle_warper<double> ito(inner, outer, rectangle_warper<double>::TOP_LEFT,
      rectangle_warper<double>::BOTTOM_RIGHT);
//LOG(astring("inner to outer warper is: " + ito.text_form()));

  rectangle<double> warped_inner(ito.to_system_2(inner));
//LOG(astring("warped inner becomes ") + warped_inner.text_form());
  rectangle<double> warped_outer(ito.to_system_1(outer));
//LOG(astring(" and outer becomes ") + warped_outer.text_form());
  ASSERT_FALSE( (warped_inner != outer) || (warped_outer != inner),
      "systems should warp to each other correctly");

  point<double> in_center(inner.center());
  point<double> warp_in_center(ito.to_system_2(in_center));
  point<double> out_center(outer.center());
  point<double> warp_out_center(ito.to_system_1(out_center));
  ASSERT_FALSE( (warp_out_center != inner.center()) || (warp_in_center != outer.center()),
      "centers should warp to each other");
  return final_report();
}

HOOPLE_MAIN(test_rectangle_warper, );

