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

const int MAX_ELEMENTS = 30;
//1200

const int MAX_VALUE = 28000;

#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger().get(), to_print)

class test_sorts : virtual public unit_base, virtual public application_shell
{
public:
	test_sorts()
			: application_shell()
	{
	}
	DEFINE_CLASS_NAME("test_sorts")
	;

	int *populate_random_c_array(int size);
	basis::array<int> populate_random_array(int size);

	void test_shell_sort(int *list, int size);
	void test_heap_sort(int *list, int size);
	void test_merge_sort(basis::array<int> &list);

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

//hmmm: this pattern is very silly.  it's nearly cookie cutter, so why not implement a templated version?
// one diff is the C array versus basis array usage.

void test_sorts::test_shell_sort(int *list, int size)
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
	delete[] list;
}

void test_sorts::test_heap_sort(int *list, int size)
{
	FUNCDEF("test_heap_sort");

	// check a normal sort.
	heap_sort(list, size);

	int last = -1;
	for (int j = 0; j < size; j++) {
		ASSERT_FALSE(list[j] < last, "ordering check - list should be ordered at first check");
		last = list[j];
	}

	// re-randomize the list.
	for (int i = 0; i < size; i++)
		list[i] = randomizer().inclusive(0, MAX_VALUE);

	// check a reversed sort.
	heap_sort(list, size, true);

	last = MAX_VALUE + 100;  // past the maximum we'll include in the list.
	for (int j = 0; j < size; j++) {
		ASSERT_FALSE(list[j] > last, "ordering check - list should be ordered at second check");
		last = list[j];
	}

	// clean up now.
	delete[] list;
}

void test_sorts::test_merge_sort(basis::array<int> &list)
{
	FUNCDEF("test_merge_sort");

	// check a normal sort.
	basis::array<int> ret = merge_sort(list);

//	LOG(a_sprintf("list has %d elems", ret.length()));
//	LOG(astring("list has ") + dump_list(ret.observe(), ret.length()));

	int last = -1;
	for (int j = 0; j < list.length(); j++) {
		ASSERT_FALSE(ret[j] < last, "ordering check - list should be ordered at first check");
		last = ret[j];
	}

	// re-randomize the list.
	for (int i = 0; i < list.length(); i++)
		list[i] = randomizer().inclusive(0, MAX_VALUE);

	// check a reversed sort.
	ret = merge_sort(list, true);

	last = MAX_VALUE + 100;  // past the maximum we'll include in the list.
	for (int j = 0; j < list.length(); j++) {
		ASSERT_FALSE(ret[j] > last, "ordering check - list should be ordered at second check");
		last = ret[j];
	}
}

int test_sorts::execute()
{
	FUNCDEF("execute");

	int size = MAX_ELEMENTS;

	test_shell_sort(populate_random_c_array(size), size);

	test_heap_sort(populate_random_c_array(size), size);

	basis::array<int> testarray = populate_random_array(size);
	test_merge_sort(testarray);

	//  test_quick_sort(populate_random_array(size), size);


	return final_report();
}

HOOPLE_MAIN(test_sorts,)

