/*****************************************************************************\
*
*  Name   : test_callstack_tracker
*  Author : Chris Koeritz
*
*  Purpose:
*
*    Puts the callstack tracking code through its paces, a bit.
*
*******************************************************************************
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

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
};

int test_callstack_tracker::run_filestack_simple()
{
  #ifdef ENABLE_CALLSTACK_TRACKING
//just show this stack.
//return an error if any problem.
  #endif
  return 0;
}

int test_callstack_tracker::run_filestack_middling()
{
  #ifdef ENABLE_CALLSTACK_TRACKING
//show a couple calls in.
//return an error if any problem.
  #endif
  return 0;
}

int test_callstack_tracker::run_filestack_complex()
{
  #ifdef ENABLE_CALLSTACK_TRACKING
//do something recursive and show an elaborate stack
//return an error if any problem.
  #endif
  return 0;
}

int test_callstack_tracker::execute()
{
  FUNCDEF("execute");

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

