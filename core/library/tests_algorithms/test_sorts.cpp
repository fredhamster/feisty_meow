/*
*  Name   : test_sorts
*  Author : Chris Koeritz
**
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <algorithms/shell_sort.h>
#include <application/hoople_main.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <loggers/console_logger.h>
#include <mathematics/chaos.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

using namespace algorithms;
using namespace application;
using namespace basis;
using namespace loggers;
using namespace mathematics;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

const int MAX_ELEMENTS = 1200;

const int MAX_VALUE = 28000;

#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger().get(), to_print)

class test_sorts : virtual public unit_base, virtual public application_shell
{
public:
  test_sorts() : application_shell() {}
  DEFINE_CLASS_NAME("test_sorts");
  virtual int execute();
};

int test_sorts::execute()
{
  FUNCDEF("execute");

  int *list = new int[MAX_ELEMENTS];
  for (int i = 0; i < MAX_ELEMENTS; i++)
    list[i] = randomizer().inclusive(0, MAX_VALUE);

//astring ret;
//for (int i = 0; i < MAX_ELEMENTS; i++) ret += a_sprintf("%d ", list[i]);
//LOG(ret);
//LOG("-------------");

  // check a normal sort.
  shell_sort(list, MAX_ELEMENTS);
  int last = -1;
  for (int j = 0; j < MAX_ELEMENTS; j++) {
    ASSERT_FALSE(list[j] < last, "ordering check - list should be ordered at first check");
    last = list[j];
  }

  // re-randomize the list.
  for (int i = 0; i < MAX_ELEMENTS; i++)
    list[i] = randomizer().inclusive(0, MAX_VALUE);

  // check a reversed sort.
  shell_sort(list, MAX_ELEMENTS, true);
  last = MAX_VALUE + 100;  // past the maximum we'll include in the list.
  for (int j = 0; j < MAX_ELEMENTS; j++) {
    ASSERT_FALSE(list[j] > last, "ordering check - list should be ordered at second check");
    last = list[j];
  }

  // clean up now.
  delete [] list;

  return final_report();
}

HOOPLE_MAIN(test_sorts, )

