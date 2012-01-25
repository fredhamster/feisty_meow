/*
*  Name   : test_version
*  Author : Chris Koeritz
**
* Copyright (c) 2009-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <loggers/console_logger.h>
#include <loggers/critical_events.h>
#include <structures/static_memory_gremlin.h>
#include <structures/version_record.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace mathematics;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger().get(), astring(to_print))

class test_version : public virtual unit_base, virtual public application_shell
{
public:
  test_version() : application_shell() {}
  DEFINE_CLASS_NAME("test_version");
  virtual int execute();
};

int test_version::execute()
{
  FUNCDEF("execute");

  version v1(5, 6, 138);
  version v2(5, 7, 108);
  ASSERT_TRUE((v1 < v2), "compare v1 < v2 should work properly");
  ASSERT_FALSE(v1 > v2, "compare v1 > v2 should work properly");
  ASSERT_FALSE(v1 == v2, "compare v1 == v2 should work properly");
  ASSERT_TRUE((v1 != v2), "compare v1 != v2 should work properly");

  version v3(4, 6, 180);
  ASSERT_TRUE((v3 < v1), "compare v3 < v1 should work properly");
  ASSERT_FALSE(v3 > v1, "compare v3 > v1 should work properly");
  ASSERT_FALSE(v3 == v1, "compare v3 == v1 should work properly");
  ASSERT_TRUE((v3 != v1), "compare v3 != v1 should work properly");
  ASSERT_TRUE((v3 < v2), "compare v3 < v2 should work properly");
  ASSERT_FALSE(v3 > v2, "compare v3 > v2 should work properly");
  ASSERT_FALSE(v3 == v2, "compare v3 == v2 should work properly");
  ASSERT_TRUE((v3 != v2), "compare v3 != v2 should work properly");
  ASSERT_FALSE(v1 < v3, "compare v1 < v3 should work properly");
  ASSERT_TRUE((v1 > v3), "compare v1 > v3 should work properly");
  ASSERT_FALSE(v1 == v3, "compare v1 == v3 should work properly");
  ASSERT_TRUE((v1 != v3), "compare v1 != v3 should work properly");
  ASSERT_FALSE(v2 < v3, "compare v2 < v3 should work properly");
  ASSERT_TRUE((v2 > v3), "compare v2 > v3 should work properly");
  ASSERT_FALSE(v2 == v3, "compare v2 == v3 should work properly");
  ASSERT_TRUE((v2 != v3), "compare v2 != v3 should work properly");

  version v4(4, 6, 180);
  ASSERT_FALSE(v3 < v4, "compare v3 < v4 should work properly");
  ASSERT_FALSE(v3 > v4, "compare v3 > v4 should work properly");
  ASSERT_TRUE((v3 == v4), "compare v3 == v4 should work properly");
  ASSERT_FALSE(v3 != v4, "compare v3 != v4 should work properly");
  ASSERT_FALSE(v4 < v3, "compare v4 < v3 should work properly");
  ASSERT_FALSE(v4 > v3, "compare v4 > v3 should work properly");
  ASSERT_TRUE((v4 == v3), "compare v4 == v3 should work properly");
  ASSERT_FALSE(v4 != v3, "compare v4 != v3 should work properly");

  return final_report();
}

HOOPLE_MAIN(test_version, )

