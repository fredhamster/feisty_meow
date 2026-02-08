/*****************************************************************************\
*                                                                             *
*  Name   : test_break_signal                                                 *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2004-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <timely/time_control.h>
#include <timely/time_stamp.h>
#include <application/application_shell.h>
#include <application/hoople_main.h>
#include <loggers/console_logger.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <filesystem/filename.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

#include <signal.h>
//hmmm: much better if we had signal handling wrapped in a class instead of using bare calls to the OS signal library.
#include <stdio.h>
//hmmm: also a flush mechanism for i/o being inside feisty code would be better than this call to stdio.

using namespace application;
using namespace basis;
using namespace loggers;
using namespace timely;
using namespace unit_test;

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

static bool _leave_now = false;

const int DEFAULT_PAUSE_TIME = 20;  // how long we'll wait, unless told a different time.

class test_break_signal : virtual public unit_base, virtual public application_shell
{
public:
  test_break_signal() : application_shell() {}
  DEFINE_CLASS_NAME("test_break_signal");
  virtual int execute();
};

void handle_break(int formal(signal))
{
  #undef static_class_name
  #define static_class_name() "test_break_signal"
  FUNCDEF("handle_break");
  LOG("we have hit the signal handler for break!");
  _leave_now = true;
  #undef static_class_name
}

int test_break_signal::execute()
{
  FUNCDEF("execute");

  int pause_time = DEFAULT_PAUSE_TIME;
  if (application::_global_argc >= 2) {
    astring passed_pause = application::_global_argv[1];
    pause_time = passed_pause.convert(DEFAULT_PAUSE_TIME);
  }


  signal(SIGINT, handle_break);
  LOG("starting loop--hit ctrl-C to exit or wait for timeout.");
  time_stamp leave_time(pause_time * SECOND_ms);
  while (!_leave_now && (time_stamp() < leave_time) ) {
    time_control::sleep_ms(20);
  }

  // we jump to here when catching the signal.
  astring to_print("break_signal:: works for those functions tested.");
  critical_events::alert_message(to_print.s());
  fflush(NULL_POINTER);
  return 0;
}

HOOPLE_MAIN(test_break_signal, )

#undef static_class_name

