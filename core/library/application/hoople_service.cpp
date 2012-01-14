//////////////
// Name   : hoople_service
// Author : Chris Koeritz
//////////////
// Copyright (c) 2000-$now By Author.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation:
//     http://www.gnu.org/licenses/gpl.html
// or under the terms of the GNU Library license:
//     http://www.gnu.org/licenses/lgpl.html
// at your preference.  Those licenses describe your legal rights to this
// software, and no other rights or warranties apply.
// Please send updates for this code to: fred@gruntose.com -- Thanks, fred.
//////////////

#include "hoople_service.h"

#include <basis/array.h>
#include <basis/mutex.h>
#include <filesystem/filename.h>
#include <loggers/program_wide_logger.h>
#include <loggers/critical_events.h>
#include <processes/process_control.h>
#include <processes/process_entry.h>
#include <structures/set.h>
#include <structures/static_memory_gremlin.h>
#include <timely/time_control.h>
#include <timely/time_stamp.h>
#include <timely/timer_driver.h>

#include <signal.h>

using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace processes;
using namespace structures;
using namespace timely;

//#define DEBUG_HOOPLE_SERVICE
  // uncomment for noisy version.

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

namespace application {

//////////////

SAFE_STATIC(astring, hoople_service::_app_name, )

bool &hoople_service::_defunct() { static bool _defu = false; return _defu; }

bool &hoople_service::_saw_interrupt()
{ static bool _saw = false; return _saw; }

int &hoople_service::_timer_period() { static int _tim = 0; return _tim; }

//////////////

static hoople_service *_global_hoople_service = NIL;
  // this static object is set by the setup() method.  it should only come
  // into existence once during a program's lifetime.

hoople_service::hoople_service()
{
}

hoople_service::~hoople_service()
{
  make_defunct();
  if (_global_hoople_service) {
    program_wide_timer().zap_timer(_global_hoople_service);
  }
}

void hoople_service::handle_startup() { /* nothing for base class. */ }

void hoople_service::handle_shutdown() { /* nothing for base class. */ }

void hoople_service::handle_timer() { /* nothing for base class. */ }

void hoople_service::handle_timer_callback()
{ 
  // don't invoke the user's timer unless the object is still alive.
  if (!is_defunct()) handle_timer();
}

void hoople_service::close_this_program()
{
  make_defunct();
}

bool hoople_service::close_application(const astring &app_name)
{
  FUNCDEF("close_application");
  process_entry_array procs;
  process_control querier;

  // lookup process ids of apps.
  bool got_list = querier.query_processes(procs);
  if (!got_list) {
    LOG(astring("couldn't get process list."));
    return false;
  }
  int_set pids;
  got_list = querier.find_process_in_list(procs, app_name, pids);
  if (!got_list) {
    LOG(astring("couldn't find process in the list of active ones."));
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

void hoople_service::handle_OS_signal(int formal(sig_id))
{
  _saw_interrupt() = true;  // save the status.
  if (_global_hoople_service) {
    _global_hoople_service->close_this_program();
  }
}

void hoople_service::make_defunct()
{
  _defunct() = true;
}

bool hoople_service::setup(const astring &app_name, int timer_period)
{
//hmmm: make sure not already initted.

  // simple initializations first...
  _timer_period() = timer_period;
  _app_name() = app_name;

  _global_hoople_service = this;

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

bool hoople_service::launch_console(hoople_service &alert,
    const astring &app_name, int timer_period)
{
#ifdef DEBUG_HOOPLE_SERVICE
  FUNCDEF("launch_console");
#endif
  if (!alert.setup(app_name, timer_period)) return false;

  alert.handle_startup();  // tell the program it has started up.

  // start a timer if they requested one.
  if (_timer_period()) {
    program_wide_timer().set_timer(_timer_period(), &alert);
  }

#ifdef DEBUG_HOOPLE_SERVICE
  time_stamp next_report(10 * SECOND_ms);
#endif

  while (!alert.is_defunct()) {
#ifdef DEBUG_HOOPLE_SERVICE
    if (time_stamp() >= next_report) {
      printf("%s: shout out from my main thread yo.\n", _global_argv[0]);
      next_report.reset(10 * SECOND_ms);
    }
#endif
    time_control::sleep_ms(42);
  }
  alert.handle_shutdown();
  return true;
}

/*
#ifdef __WIN32__
bool hoople_service::launch_event_loop(hoople_service &alert,
    const astring &app_name, int timer_period)
{
  if (!alert.setup(app_name, timer_period)) return false;
  alert.handle_startup();

  if (timer_period)
    program_wide_timer().set_timer(timer_period, this);

  MSG msg;
  msg.hwnd = 0; msg.message = 0; msg.wParam = 0; msg.lParam = 0;
  while (!alert.is_defunct() && (GetMessage(&msg, NIL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  alert.handle_shutdown();

  return true;
}
#endif
*/

} //namespace.

