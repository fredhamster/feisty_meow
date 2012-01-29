/*****************************************************************************\
*                                                                             *
*  Name   : t_unique_id                                                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/guards.h>
#include <loggers/program_wide_logger.h>
#include <mathematics/chaos.h>
#include <structures/unique_id.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

#ifdef DEBUG_STACK
  #define LOG(to_print) EMERGENCY_LOG(program_wide_logger::get(), to_print)
#endif

using namespace application;
using namespace basis;
///using namespace configuration;
using namespace mathematics;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#include <stdlib.h>
#include <string.h>

//////////////

class test_unique_id : public virtual unit_base, public virtual application_shell
{
public:
  test_unique_id() {}
  DEFINE_CLASS_NAME("test_unique_id");
  int execute();
};

HOOPLE_MAIN(test_unique_id, );

//////////////

int test_unique_id::execute()
{
  FUNCDEF("execute");
  unique_int ted(25);
  unique_int jed;

  ASSERT_TRUE(ted.raw_id(), "testing non-zero should not claim was zero");
  ASSERT_TRUE(!jed, "testing zero should claim was zero");
  ASSERT_TRUE(!!ted, "testing non-zero doubled should not claim was zero");
  ASSERT_TRUE(!!!jed, "testing zero doubled should claim was zero");

  unique_int med(25);
  unique_int fed(0);

  ASSERT_EQUAL(med.raw_id(), ted.raw_id(), "testing equality 1 should have correct result");
  ASSERT_EQUAL(fed.raw_id(), jed.raw_id(), "testing equality 2 should have correct result");
  ASSERT_INEQUAL(med.raw_id(), jed.raw_id(), "testing equality 3 should have correct result");
  ASSERT_INEQUAL(fed.raw_id(), ted.raw_id(), "testing equality 4 should have correct result");

  ASSERT_FALSE(med != ted, "equality operator 1 should have correct result");
  ASSERT_FALSE(fed != jed, "equality operator 2 should have correct result");
  ASSERT_FALSE(med == jed, "equality operator 3 should have correct result");
  ASSERT_FALSE(fed == ted, "equality operator 4 should have correct result");

  return final_report();
}

