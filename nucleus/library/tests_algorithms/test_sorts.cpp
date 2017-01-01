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
	test_sorts()
			: application_shell()
	{
	}

	DEFINE_CLASS_NAME("test_sorts")

	int *populate_random_c_array(int size);
	basis::array<int> populate_random_array(int size);
//	void rerandomize(int list[], int size);
	bool verify_ascending(const int *list, int size);
	bool verify_descending(const int *list, int size);

	void test_shell_sort(int size);
	void test_heap_sort(int size);
	void test_merge_sort(int size);
	void test_quick_sort(int size);

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

//void test_sorts::rerandomize(int list[], int size)
//{
//	for (int i = 0; i < size; i++)
//		list[i] = randomizer().inclusive(0, MAX_VALUE);
//}

bool test_sorts::verify_ascending(const int *list, int size)
{
	FUNCDEF("verify_ascending")
	int last = list[0];
	for (int j = 1; j < size; j++) {
		if (list[j] < last) return false;
		last = list[j];
	}
	return true;
}

bool test_sorts::verify_descending(const int *list, int size)
{
	FUNCDEF("verify_descending")
	int last = list[0];
	for (int j = 1; j < size; j++) {
		if (list[j] > last) return false;
		last = list[j];
	}
	return true;
}

void test_sorts::test_shell_sort(int size)
{
	FUNCDEF("test_shell_sort");

	int *list = populate_random_c_array(size);

	// check a normal sort.
	shell_sort(list, size);
	ASSERT_TRUE(verify_ascending(list, size),
	    "ordering check - list should be ordered at first check");

	randomize_list(list, size);

	// check a reversed sort.
	shell_sort(list, size, true);
	ASSERT_TRUE(verify_descending(list, size),
	    "ordering check - list should be ordered at second check");

	// clean up now.
	delete[] list;
}

void test_sorts::test_heap_sort(int size)
{
	FUNCDEF("test_heap_sort");

	int *list = populate_random_c_array(size);

	// check a normal sort.
	heap_sort(list, size);
	ASSERT_TRUE(verify_ascending(list, size),
	    "ordering check - list should be ordered at first check");

	randomize_list(list, size);

	// check a reversed sort.
	heap_sort(list, size, true);
	ASSERT_TRUE(verify_descending(list, size),
	    "ordering check - list should be ordered at second check");

	// clean up now.
	delete[] list;
}

void test_sorts::test_merge_sort(int size)
{
	FUNCDEF("test_merge_sort");

	basis::array<int> list = populate_random_array(size);

	// check a normal sort.
	basis::array<int> ret = merge_sort(list);

//	LOG(astring("list has ") + dump_list(ret.observe(), ret.length()));

	ASSERT_TRUE(verify_ascending(ret.access(), size),
	    "ordering check - list should be ordered at first check");

	randomize_list(list.access(), size);

	// check a reversed sort.
	ret = merge_sort(list, true);
	ASSERT_TRUE(verify_descending(ret.access(), size),
	    "ordering check - list should be ordered at second check");
}

void test_sorts::test_quick_sort(int size)
{
	FUNCDEF("test_quick_sort");

	int *list = populate_random_c_array(size);

	// check a normal sort.
	quick_sort(list, size);
	ASSERT_TRUE(verify_ascending(list, size),
	    "ordering check - list should be ordered at first check");

//	LOG(a_sprintf("after quick sort: %s", dump_list(list, size).s()));

	randomize_list(list, size);

	// check a reversed sort.
	quick_sort(list, size, true);
	ASSERT_TRUE(verify_descending(list, size),
	    "ordering check - list should be ordered at second check");

	// clean up now.
	delete[] list;
}


int test_sorts::execute()
{
	FUNCDEF("execute");

	int size = MAX_ELEMENTS;

	test_shell_sort(size);

	test_heap_sort(size);

	test_merge_sort(size);

	test_quick_sort(size);

	return final_report();
}

HOOPLE_MAIN(test_sorts,)

