


/*****************************************************************************\
*                                                                             *
*  Name   : shutdown_alerter                                                  *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2003-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "process_control.h"
#include "process_entry.h"
#include "shutdown_alerter.h"

#include <application/command_line.h>
#include <basis/array.h>
#include <basis/astring.h>
#include <basis/mutex.h>
#include <filesystem/filename.h>
#include <loggers/program_wide_logger.h>
#include <structures/set.h>
#include <structures/static_memory_gremlin.h>
#include <timely/time_control.h>
#include <timely/time_stamp.h>
#include <timely/timer_driver.h>

#include <signal.h>
#include <stdio.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace processes;
using namespace structures;
using namespace timely;

#define DEBUG_SHUTDOWN_ALERTER
  // uncomment for noisy version.

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

//////////////

SAFE_STATIC(astring, shutdown_alerter::_app_name, )

bool &shutdown_alerter::_defunct() { static bool _defu = false; return _defu; }

bool &shutdown_alerter::_saw_interrupt()
{ static bool _saw = false; return _saw; }

int &shutdown_alerter::_timer_period() { static int _tim = 0; return _tim; }

//////////////

static shutdown_alerter *_global_shutdown_alerter = NULL_POINTER;
  // this static object is set by the setup() method.  it should only come
  // into existence once during a program's lifetime.

shutdown_alerter::shutdown_alerter()
{
}

shutdown_alerter::~shutdown_alerter()
{
  set_defunct();
  if (_global_shutdown_alerter) {
    program_wide_timer().zap_timer(_global_shutdown_alerter);
  }
}

void shutdown_alerter::handle_startup() { /* nothing for base class. */ }

void shutdown_alerter::handle_shutdown() { /* nothing for base class. */ }

void shutdown_alerter::handle_timer() { /* nothing for base class. */ }

void shutdown_alerter::handle_timer_callback()
{ 
  // don't invoke the user's timer unless the object is still alive.
  if (!is_defunct()) handle_timer();
}

void shutdown_alerter::close_this_program()
{
  set_defunct();
}

bool shutdown_alerter::close_application(const astring &app_name)
{
  FUNCDEF("close_application");
  process_entry_array procs;
  process_control querier;

  // lookup process ids of apps.
  bool got_list = querier.query_processes(procs);
  if (!got_list) {
    LOG("couldn't get process list.");
    return false;
  }
  int_set pids;
  got_list = querier.find_process_in_list(procs, app_name, pids);
  if (!got_list) {
    LOG("couldn't find process in the list of active ones.");
    return true;
  }

  // zap all of them using our signal.
  for (int i = 0; i < pids.length(); i++) {
//would linux be better served with sigterm also?
#ifdef __UNIX__
    kill(pids[i], SIGHUP);
#endif
#ifdef __WIN32__
//lame--goes to whole program.
    raise(SIGTERM);
#endif
//hmmm: check results...
  }

  return true;
}

void shutdown_alerter::handle_OS_signal(int formal(sig_id))
{
  _saw_interrupt() = true;  // save the status.
  if (_global_shutdown_alerter) {
    _global_shutdown_alerter->close_this_program();
  }
}

void shutdown_alerter::set_defunct()
{
  _defunct() = true;
}

bool shutdown_alerter::setup(const astring &app_name, int timer_period)
{
//hmmm: make sure not already initted.

  // simple initializations first...
  _timer_period() = timer_period;
  _app_name() = app_name;

  _global_shutdown_alerter = this;

  // setup signal handler for HUP signal.  this is the one used to tell us
  // to leave.
#ifdef __UNIX__
  signal(SIGHUP, handle_OS_signal);
#endif

  // setup a handler for interrupt (e.g. ctrl-C) also.
  signal(SIGINT, handle_OS_signal);
#ifdef __WIN32__
  signal(SIGBREAK, handle_OS_signal);
#endif

  return true;
}

bool shutdown_alerter::launch_console(shutdown_alerter &alert,
    const astring &app_name, int timer_period)
{
  FUNCDEF("launch_console");
  if (!alert.setup(app_name, timer_period)) return false;

  alert.handle_startup();  // tell the program it has started up.

  // start a timer if they requested one.
  if (_timer_period()) {
    program_wide_timer().set_timer(_timer_period(), &alert);
  }

#ifdef DEBUG_SHUTDOWN_ALERTER
  const int REPORT_CYCLE = 10 * SECOND_ms;
  time_stamp next_report(REPORT_CYCLE);
#endif

  while (!alert.is_defunct()) {
#ifdef DEBUG_SHUTDOWN_ALERTER
    if (time_stamp() >= next_report) {
      LOG(a_sprintf("%s: shout out from my main thread yo.\n", _global_argv[0]));
      next_report.reset(REPORT_CYCLE);
    }
#endif
    time_control::sleep_ms(42);
  }
  alert.handle_shutdown();
  return true;
}

/*
#ifdef __WIN32__
bool shutdown_alerter::launch_event_loop(shutdown_alerter &alert,
    const astring &app_name, int timer_period)
{
  if (!alert.setup(app_name, timer_period)) return false;
  alert.handle_startup();

  if (timer_period)
    program_wide_timer().set_timer(timer_period, this);

  MSG msg;
  msg.hwnd = 0; msg.message = 0; msg.wParam = 0; msg.lParam = 0;
  while (!alert.is_defunct() && (GetMessage(&msg, NULL_POINTER, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  alert.handle_shutdown();

  return true;
}
#endif
*/





