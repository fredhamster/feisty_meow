/*
*  Name   : test_earth_time                                                   *
*  Author : Chris Koeritz                                                     *
**
* Copyright (c) 2007-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#define DEBUG_EARTH_TIME
  // set this to enable debugging features of the string class.

#include <application/hoople_main.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <loggers/file_logger.h>
#include <mathematics/chaos.h>
#include <structures/static_memory_gremlin.h>
#include <timely/earth_time.h>
#include <timely/time_stamp.h>
#include <unit_test/unit_base.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace application;
using namespace basis;
using namespace mathematics;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)
#undef BASE_LOG
#define BASE_LOG(s) STAMPED_EMERGENCY_LOG(program_wide_logger::get(), s)

const int TIME_FORMAT = clock_time::MERIDIAN | clock_time::SECONDS
    | clock_time::MILLISECONDS;
  // the way we like to see our seconds get printed out.

//////////////

class test_earth_time : virtual public unit_base, virtual public application_shell
{
public:
  test_earth_time() {}
  DEFINE_CLASS_NAME("test_earth_time");

  virtual int execute();

  void run_test_01();
  void run_test_02();
};

//////////////

void test_earth_time::run_test_01()
{
  FUNCDEF("run_test_01");
  // this test makes sure that clock_time's normalize is working as expected.

  time_locus checker_1(clock_time(12, 0, 60), day_in_year(), 2007);
  clock_time::normalize(checker_1);
  time_locus compare_1(clock_time(12, 1, 0), day_in_year(), 2007);
//BASE_LOG(astring("a=") + checker_1.text_form(TIME_FORMAT));
//BASE_LOG(astring("b=") + compare_1.text_form(TIME_FORMAT));
  ASSERT_EQUAL(checker_1, compare_1, "normalize should not fail test 1");

  time_locus checker_2(clock_time(12, 0, -1), day_in_year(), 2007);
  clock_time::normalize(checker_2);
  time_locus compare_2(clock_time(11, 59, 59), day_in_year(), 2007);
  ASSERT_EQUAL(checker_2, compare_2, "normalize should not fail test 2");

  time_locus checker_3(clock_time(11, 59, 61), day_in_year(), 2007);
  clock_time::normalize(checker_3);
  time_locus compare_3(clock_time(12, 00, 01), day_in_year(), 2007);
  ASSERT_EQUAL(checker_3, compare_3, "normalize should not fail test 3");

  time_locus checker_4(clock_time(12, 54, -61), day_in_year(), 2007);
  clock_time::normalize(checker_4);
  time_locus compare_4(clock_time(12, 52, 59), day_in_year(), 2007);
  ASSERT_EQUAL(checker_4, compare_4, "normalize should not fail test 4");

  time_locus checker_5(clock_time(12, -32, -62), day_in_year(), 2007);
  clock_time::normalize(checker_5);
  time_locus compare_5(clock_time(11, 26, 58), day_in_year(), 2007);
  ASSERT_EQUAL(checker_5, compare_5, "normalize should not fail test 5");
}

void test_earth_time::run_test_02()
{
  FUNCDEF("run_test_02");
  // this test makes sure that day_in_year's normalize is working as expected.

  time_locus checker_1(clock_time(0, 0, -1), day_in_year(JANUARY, 1), 2007);
  time_locus::normalize(checker_1);
  time_locus compare_1(clock_time(23, 59, 59), day_in_year(DECEMBER, 31), 2006);
//BASE_LOG(astring("a=") + checker_1.text_form(TIME_FORMAT));
//BASE_LOG(astring("b=") + compare_1.text_form(TIME_FORMAT));
  ASSERT_EQUAL(checker_1, compare_1, "normalize should not fail test 1");

  time_locus checker_2(clock_time(23, 59, 60), day_in_year(DECEMBER, 31), 2007);
  time_locus::normalize(checker_2);
  time_locus compare_2(clock_time(0, 0, 0), day_in_year(JANUARY, 1), 2008);
  ASSERT_EQUAL(checker_2, compare_2, "normalize should not fail test 2");


//add more cases!
//  test leap years
//  test lotso things.

}

int test_earth_time::execute()
{
  FUNCDEF("execute");

  run_test_01();
  run_test_02(); 

  return final_report();
}

//////////////

HOOPLE_MAIN(test_earth_time, )

