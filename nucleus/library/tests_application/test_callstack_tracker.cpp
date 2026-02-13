/*
*
*  Name   : test_callstack_tracker
*  Author : Chris Koeritz
*
*  Purpose:
*
*    Puts the callstack tracking code through its paces, a bit.
*
****
* Copyright (c) 1992-$now By Author.  This program is free software; you can
* redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either version 2 of
* the License or (at your option) any later version.  This is online at:
*     http://www.fsf.org/copyleft/gpl.html
* Please send any updates to: fred@gruntose.com
*/

#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/astring.h>

#include <application/application_shell.h>
#include <application/callstack_tracker.h>
#include <application/hoople_main.h>
#include <loggers/console_logger.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <filesystem/filename.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_array.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace unit_test;

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

//////////////

class test_callstack_tracker : virtual public unit_base, virtual public application_shell
{
public:
  test_callstack_tracker() : application_shell() {}
  DEFINE_CLASS_NAME("test_callstack_tracker");
  // our test suite...
  int run_filestack_simple();
  int run_filestack_middling();
  int run_filestack_complex();
  int execute();

private:
  int sub_call_1();
  int sub_call_2();
  int sub_call_3();
  int sub_call_4();

  int recursive_factorial(int num);
};

//////////////

int test_callstack_tracker::run_filestack_simple()
{
  FUNCDEF("run_filestack_simple")
  #ifdef ENABLE_CALLSTACK_TRACKING
    // just shows this method's own stack.
    GET_AND_TEST_STACK_TRACE("trace of simple stack:", 1);
  #endif
  return 0;
}

//////////////

int test_callstack_tracker::sub_call_1()
{
  FUNCDEF("sub_call_1")
  return sub_call_2();
}

int test_callstack_tracker::sub_call_2()
{
  FUNCDEF("sub_call_2");
  return sub_call_3();
}

int test_callstack_tracker::sub_call_3()
{
  FUNCDEF("sub_call_3")
  return sub_call_4();
}

int test_callstack_tracker::sub_call_4()
{
  FUNCDEF("sub_call_4");
  #ifdef ENABLE_CALLSTACK_TRACKING
    // just shows this method's own stack.
    GET_AND_TEST_STACK_TRACE("trace of middling stack:", 1);
  #endif
  return 0;
}

int test_callstack_tracker::run_filestack_middling()
{
  FUNCDEF("run_filestack_middling");
  int retval = 0;
  #ifdef ENABLE_CALLSTACK_TRACKING
    // show a couple calls in using our sub_calls.
    retval = sub_call_1();
  #endif
  return retval;
}

int test_callstack_tracker::run_filestack_complex()
{
  FUNCDEF("run_filestack_complex")
  int fact_sought = 12;
  int factotum = recursive_factorial(12);
  LOG(a_sprintf("factorial of %d was computed as %d", fact_sought, factotum));
  if (factotum < 0) {
    // uh-oh, there was an actual failure of some sort.
    return 1;
  }
  return 0;
}

//////////////

int test_callstack_tracker::recursive_factorial(int num)
{
  FUNCDEF("recursive_factorial")
  if (num < 0) {
    return -1;  // signifies that things have gone badly.
  }
  if (num <= 1) {
    #ifdef ENABLE_CALLSTACK_TRACKING
      // now show the callstack, since we should be at the deepest level of nesting for this factorial implementation.
      GET_AND_TEST_STACK_TRACE("trace of recursive_factorial:", -1);
    #endif
    return 1;
  } else {
    return num * recursive_factorial(num - 1);
  }
}

//////////////

int test_callstack_tracker::execute()
{
  FUNCDEF("execute");

//hmmm: do we really want the test to fail out for any subtest failure?  maybe so?
  int ret = run_filestack_simple();
  if (ret) return ret;
  ret = run_filestack_middling();
  if (ret) return ret;
  ret = run_filestack_complex();
  if (ret) return ret;

  critical_events::alert_message(astring(class_name()) + ": works for those functions tested.");

  return final_report();
}

HOOPLE_MAIN(test_callstack_tracker, )

