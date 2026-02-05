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

#include <basis/function.h>
#include <basis/guards.h>
#include <basis/istring.h>
#include <mechanisms/time_stamp.h>
#include <opsystem/application_shell.h>
#include <loggers/console_logger.h>
#include <opsystem/filename.h>
#include <data_struct/static_memory_gremlin.h>

#include <signal.h>
#include <stdio.h>

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger(), s)

static bool _leave_now = false;

class test_break_signal : public application_shell
{
public:
  test_break_signal() : application_shell(class_name()) {}
  IMPLEMENT_CLASS_NAME("test_break_signal");
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
  signal(SIGINT, handle_break);
  LOG("starting loop--hit ctrl-C to exit or wait for timeout.");
  time_stamp leave_time(20 * SECOND_ms);
  while (!_leave_now && (time_stamp() < leave_time) ) {
    portable::sleep_ms(20);
  }

  // we jump to here when catching the signal.
  istring to_print("break_signal:: works for those functions tested.");
  guards::alert_message(to_print.s());
  fflush(NIL);
  return 0;
}

HOOPLE_MAIN(test_break_signal, )

#undef static_class_name

