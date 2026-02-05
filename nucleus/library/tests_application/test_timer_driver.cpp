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

#include <basis/chaos.h>
#include <basis/function.h>
#include <basis/guards.h>
#include <basis/istring.h>
#include <basis/log_base.h>
#include <basis/set.h>
#include <data_struct/unique_id.h>
#include <mechanisms/ithread.h>
#include <mechanisms/thread_cabinet.h>
#include <mechanisms/time_stamp.h>
#include <opsystem/application_shell.h>
#include <opsystem/event_extensions.h>
#include <loggers/file_logger.h>
#include <data_struct/static_memory_gremlin.h>
#include <opsystem/timer_driver.h>

#define LOG(s) STAMPED_EMERGENCY_LOG(program_wide_logger(), s)

const int TEST_DURATION = 3 * MINUTE_ms;

const int MAX_THREADS = 120;

////////////////////////////////////////////////////////////////////////////

class timer_driver_tester : public application_shell
{
public:
  timer_driver_tester()
      : application_shell(static_class_name()), _in_progress(false) {}
  IMPLEMENT_CLASS_NAME("timer_driver_tester");
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

////////////////////////////////////////////////////////////////////////////

class timer_test_thread : public ithread
{
public:
  timer_test_thread(application_shell &parent)
      : ithread(parent.randomizer().inclusive(20, 480)), _parent(parent)
  { start(NIL); }

  IMPLEMENT_CLASS_NAME("timer_test_thread");
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

////////////////////////////////////////////////////////////////////////////

class my_timer_handler : public timed_object
{
public:
  my_timer_handler(timer_driver_tester &parent, int id) : _id(id), _parent(parent) {}
  virtual ~my_timer_handler() {}
  IMPLEMENT_CLASS_NAME("my_timer_handler");

  virtual void handle_timer_callback() {
    FUNCDEF("handle_timer_callback");
    if (_parent.in_progress())
      LOG("saw in progress flag set to true!  we interrupted real "
          "ops, not just sleep!");
    LOG(isprintf("timer%d hit.", _id));
    timer_test_thread *new_thread = new timer_test_thread(_parent);
    unique_int id = _parent.threads().add_thread(new_thread, false, NIL);
      // the test thread auto-starts, so we don't let the cabinet start it.
    if (!id)
      deadly_error(class_name(), func, "failed to start a new thread.");

    if (_parent.threads().threads() > MAX_THREADS) {
      int gone_index = _parent.randomizer().inclusive(0, _parent.threads().threads() - 1);
      unique_int gone_thread = _parent.threads().thread_ids()[gone_index];
      _parent.threads().cancel_thread(gone_thread);
      portable::sleep_ms(100);  // allow thread to start up.
    }
    _parent.threads().clean_debris();  // toss any dead threads.

    LOG(isprintf("%d threads checking time_stamp.", _parent.threads().threads()));
  }

private:
  int _id;
  timer_driver_tester &_parent;
};

////////////////////////////////////////////////////////////////////////////

#define CREATE_TIMER(name, id, dur) \
  my_timer_handler name(*this, id); \
  program_wide_timer().set_timer(dur, &name); \
  LOG(istring("timer ") + #name + " hitting every " \
      + #dur + " ms")

#define ZAP_TIMER(name) \
  program_wide_timer().zap_timer(&name)

int timer_driver_tester::execute()
{
  SET_DEFAULT_COMBO_LOGGER;

  CREATE_TIMER(timer1, 1, 500);
//  CREATE_TIMER(timer1, 1, 10);
  CREATE_TIMER(timer2, 2, SECOND_ms);
  CREATE_TIMER(timer3, 3, 3 * SECOND_ms);
  CREATE_TIMER(timer4, 4, 8 * SECOND_ms);
  CREATE_TIMER(timer5, 5, 12 * SECOND_ms);

  LOG("pausing for a while...");
  time_stamp when_done(TEST_DURATION);
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
    portable::sleep_ms(100);
#else
    bool okay = event_extensions::poll();
    if (!okay) break;
#endif
  }

  guards::alert_message("timer_driver:: works for all functions tested (if messages seem appropriate).");

  ZAP_TIMER(timer1);
  ZAP_TIMER(timer2);
  ZAP_TIMER(timer3);
  ZAP_TIMER(timer4);
  ZAP_TIMER(timer5);

  return 0;
}

////////////////////////////////////////////////////////////////////////////

HOOPLE_MAIN(timer_driver_tester, )

