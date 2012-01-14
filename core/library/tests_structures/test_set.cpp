/*****************************************************************************\
*                                                                             *
*  Name   : test_set                                                          *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1995-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/hoople_main.h>
#include <basis/guards.h>
#include <loggers/console_logger.h>
#include <loggers/critical_events.h>
#include <structures/set.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace loggers;
using namespace mathematics;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

class test_set : virtual public unit_base, virtual public application_shell 
{
public:
  test_set() {}
  DEFINE_CLASS_NAME("test_set");
  virtual int execute();
};

int test_set::execute()
{
  FUNCDEF("execute");
  int_set fred;
  ASSERT_TRUE(fred.empty(), "first empty check should work");
  ASSERT_TRUE(fred.add(23), "fred 1st add should go in");
  ASSERT_TRUE(fred.add(90123), "fred 2nd add should be okay");
  ASSERT_FALSE(fred.add(23), "fred 3rd add works fine");
  ASSERT_FALSE(fred.add(90123), "fred 4th add is good");
  ASSERT_FALSE(fred.empty(), "second empty check should work");
  ASSERT_TRUE(fred.non_empty(), "non_empty check should be right");

  int_set gen;
  ASSERT_TRUE(gen.add(13), "gen 1st add is okay");
  ASSERT_TRUE(gen.add(23), "gen 2nd add should be fine");
  ASSERT_TRUE(gen.add(8012), "gen 3rd add was good");

  int_set intersect(gen.intersection(fred));
  ASSERT_EQUAL(intersect.elements(), 1, "intersection elements should be one");
  ASSERT_TRUE(intersect.member(23), "element should be present as 23");

  int_set uni(gen.set_union(fred));
  ASSERT_EQUAL(uni.elements(), 4, "union elements should be correct");
  ASSERT_TRUE(uni.member(23), "first element we seek should be present");
  ASSERT_TRUE(uni.member(90123), "second element we seek should be present");
  ASSERT_TRUE(uni.member(13), "third element we seek should be present");
  ASSERT_TRUE(uni.member(8012), "fourth element we seek should be present");

  return final_report();
}

//////////////

HOOPLE_MAIN(test_set, )

