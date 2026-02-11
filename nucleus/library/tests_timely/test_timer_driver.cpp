/*****************************************************************************\
*                                                                             *
*  Name   : t_timer_driver                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Tests the timer driver class from the operating system library.          *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/application_shell.h>
#include <application/event_extensions.h>
#include <application/hoople_main.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <loggers/file_logger.h>
#include <mathematics/chaos.h>
#include <processes/ethread.h>
#include <processes/thread_cabinet.h>
#include <structures/set.h>
#include <structures/unique_id.h>
#include <structures/static_memory_gremlin.h>
#include <timely/time_control.h>
#include <timely/time_stamp.h>
#include <timely/timer_driver.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace processes;
using namespace structures;
using namespace timely;
using namespace unit_test;

#define LOG(s) STAMPED_EMERGENCY_LOG(program_wide_logger::get(), s)

const int DEFAULT_TEST_DURATION = 3 * SECOND_ms;
  // with no arguments to the test, we'll run timers for this very short duration.

const int MAX_THREADS = 120;

//////////////

class timer_driver_tester : virtual public unit_base, virtual public application_shell
{
public:
  timer_driver_tester()
      : application_shell(), _in_progress(false) {}
  DEFINE_CLASS_NAME("timer_driver_tester");
  virtual ~timer_driver_tester() {}

  int execute();

  thread_cabinet &threads() { return _threads; }

  bool in_progress() const { return _in_progress; }
    // returns true if activity is currently occurring on the main thread.
    // we don't expect this activity to be interrupted by timer events.

private:
  bool _in_progress;
    // simple flag to check when a timer hits.  if this is true, then a timer
    // hit while we were doing actual functional operations, rather than
    // just waiting in a sleep.
  thread_cabinet _threads;  // storage for our time_stamp testing threads.
};

//////////////

class timer_test_thread : public ethread
{
public:
  timer_test_thread(application_shell &parent)
      : ethread(parent.randomizer().inclusive(8, 25)), _parent(parent)
  { start(NULL_POINTER); }

  DEFINE_CLASS_NAME("timer_test_thread");
  void perform_activity(void *) {
    FUNCDEF("perform_activity");
    if (time_stamp() < _started)
      deadly_error(class_name(), func, "start time is before current time.");
    if (time_stamp() < _last) 
      deadly_error(class_name(), func, "last check is before current time.");
    _last.reset();  // set the last time to right now.
    time_stamp ted;
    time_stamp jed;
    if (ted > jed)
      deadly_error(class_name(), func, "jed is less than test.");
  }

private:
  application_shell &_parent;
  time_stamp _started;
  time_stamp _last;
};

//////////////

class my_timer_handler : public timeable
{
public:
  my_timer_handler(timer_driver_tester &parent, int id) : _id(id), _parent(parent) {}
  virtual ~my_timer_handler() {}
  DEFINE_CLASS_NAME("my_timer_handler");

  virtual void handle_timer_callback() {
    FUNCDEF("handle_timer_callback");
    if (_parent.in_progress())
      LOG("saw in progress flag set to true!  we interrupted real "
          "ops, not just sleep!");
    LOG(a_sprintf("timer%d hit.", _id));
    timer_test_thread *new_thread = new timer_test_thread(_parent);
    unique_int id = _parent.threads().add_thread(new_thread, false, NULL_POINTER);
      // the test thread auto-starts, so we don't let the cabinet start it.
    if (!id)
      deadly_error(class_name(), func, "failed to start a new thread.");

    if (_parent.threads().threads() > MAX_THREADS) {
      int gone_index = _parent.randomizer().inclusive(0, _parent.threads().threads() - 1);
      unique_int gone_thread = _parent.threads().thread_ids()[gone_index];
      _parent.threads().cancel_thread(gone_thread);
      time_control::sleep_ms(100);  // allow thread to start up.
    }
    _parent.threads().clean_debris();  // toss any dead threads.

    LOG(a_sprintf("%d threads checking time_stamp.", _parent.threads().threads()));
  }

private:
  int _id;
  timer_driver_tester &_parent;
};

//////////////

#define CREATE_TIMER(name, id, dur) \
  my_timer_handler name(*this, id); \
  program_wide_timer().set_timer(dur, &name); \
  LOG(astring("created timer ") + #name + " which hits every " + #dur + " ms")

#define ZAP_TIMER(name) \
  program_wide_timer().zap_timer(&name)

int timer_driver_tester::execute()
{
  int next_id = 1001;  // we start issuing timer IDs at 1001 for this test.

  int runtime_ms = DEFAULT_TEST_DURATION;
  if (application::_global_argc >= 2) {
    astring passed_runtime = application::_global_argv[1];
    runtime_ms = passed_runtime.convert(DEFAULT_TEST_DURATION);
  }

  // some odd timer cycles below to avoid waiting longer than our short default.
  CREATE_TIMER(timer1, unique_int(next_id++).raw_id(), 1 * SECOND_ms);
  CREATE_TIMER(timer2, unique_int(next_id++).raw_id(), 250);
  CREATE_TIMER(timer3, unique_int(next_id++).raw_id(), 3 * SECOND_ms);
  CREATE_TIMER(timer4, unique_int(next_id++).raw_id(), 140);
  CREATE_TIMER(timer5, unique_int(next_id++).raw_id(), 500);

  LOG("pausing for a while...");
  time_stamp when_done(runtime_ms);
  while (time_stamp() < when_done) {
    _in_progress = true;
    // do some various calculations in here and see if we're interrupted
    // during them.  it's one thing to be interrupted in the middle of a
    // sleep, but it's much different to be interrupted in mid operation.
    int scrob = 1;
    for (int i = 1; i < 50; i++) {
      scrob *= i;
    }
    _in_progress = false;    
#ifdef __UNIX__
    time_control::sleep_ms(100);
#else
    bool okay = event_extensions::poll();
    if (!okay) break;
#endif
  }

  ZAP_TIMER(timer1);
  ZAP_TIMER(timer2);
  ZAP_TIMER(timer3);
  ZAP_TIMER(timer4);
  ZAP_TIMER(timer5);

  critical_events::alert_message(astring(class_name()) + ": works for those functions tested.");

  return 0;
}

//////////////

HOOPLE_MAIN(timer_driver_tester, )

