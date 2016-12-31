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

#include <application/hoople_main.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <loggers/console_logger.h>
#include <mathematics/chaos.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>
#include <algorithms/sorts.h>

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

  int *populate_random_c_array(int size);
  basis::array<int> populate_random_array(int size);
  int test_shell_sort(int *list, int size);

  virtual int execute();
};

int *test_sorts::populate_random_c_array(int size)
{
	int *list = new int[size];
	for (int i = 0; i < size; i++)
		list[i] = randomizer().inclusive(0, MAX_VALUE);
	return list;
}

basis::array<int> test_sorts::populate_random_array(int size)
{
	basis::array<int> to_return(size);
	for (int i = 0; i < size; i++)
		to_return[i] = randomizer().inclusive(0, MAX_VALUE);
	return to_return;
}

int test_sorts::test_shell_sort(int *list, int size)
{
  FUNCDEF("test_shell_sort");

  // check a normal sort.
  shell_sort(list, size);
  int last = -1;
  for (int j = 0; j < size; j++) {
    ASSERT_FALSE(list[j] < last, "ordering check - list should be ordered at first check");
    last = list[j];
  }

  // re-randomize the list.
  for (int i = 0; i < size; i++)
    list[i] = randomizer().inclusive(0, MAX_VALUE);

  // check a reversed sort.
  shell_sort(list, size, true);
  last = MAX_VALUE + 100;  // past the maximum we'll include in the list.
  for (int j = 0; j < size; j++) {
    ASSERT_FALSE(list[j] > last, "ordering check - list should be ordered at second check");
    last = list[j];
  }

  // clean up now.
  delete [] list;

  //hmmm: wait, what if it fails above?
  return true;
}

int test_sorts::execute()
{
  FUNCDEF("execute");

  int size = MAX_ELEMENTS;

//astring ret;
//for (int i = 0; i < size; i++) ret += a_sprintf("%d ", list[i]);
//LOG(ret);
//LOG("-------------");

  test_shell_sort(populate_random_c_array(size), size);

//  test_quick_sort(populate_random_list(size), size);

//  test_merge_sort(populate_random_array(size));

  return final_report();
}

HOOPLE_MAIN(test_sorts, )

