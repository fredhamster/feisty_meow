/*****************************************************************************\
*                                                                             *
*  Name   : test_math_ops                                                     *
*  Author : Chris Koeritz                                                     *
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
#include <basis/guards.h>
#include <basis/astring.h>
#include <mathematics/math_ops.h>
#include <loggers/console_logger.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
//using namespace filesystem;
using namespace loggers;
using namespace mathematics;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger().get(), astring(to_print))

class test_math_ops : virtual public unit_base, virtual public application_shell
{
public:
  test_math_ops() {}
  DEFINE_CLASS_NAME("test_math_ops");
  virtual int execute();
};

//////////////

int test_math_ops::execute()
{
  FUNCDEF("execute");
  // test one: make sure factorial is working.
  basis::un_int fact3 = math_ops::factorial(3);
  ASSERT_EQUAL(fact3, 6, "3! did not equal 6");
  basis::un_int fact8 = math_ops::factorial(8);
  ASSERT_EQUAL(fact8, 40320, "8! did not equal 40320");
  basis::un_int fact10 = math_ops::factorial(10);
  ASSERT_EQUAL(fact10, 3628800, "10! did not equal 3628800");

  return final_report();
}

HOOPLE_MAIN(test_math_ops, )

