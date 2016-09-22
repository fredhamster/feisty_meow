/*****************************************************************************\
*                                                                             *
*  Name   : ethread                                                           *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1998-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "ethread.h"

#include <application/windoze_helper.h>
#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <structures/static_memory_gremlin.h>
#include <timely/time_control.h>

#ifdef _MSC_VER
  #include <process.h>
#elif defined(__UNIX__) || defined(__GNU_WINDOWS__)
  #include <pthread.h>
#else
  #error unknown OS for thread support.
#endif

using namespace basis;
using namespace loggers;
using namespace structures;
using namespace timely;

//#define COUNT_THREADS
  // if this is enabled, then threads will be counted when they are created
  // or destroyed.

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

namespace processes {

const int MAXIMUM_CREATE_ATTEMPTS = 20;
  // the number of retries we allow to try creating a thread, if the first
  // attempt fails.

const int MINIMUM_SLEEP_PERIOD = 10;
  // this is the smallest time we'll sleep for if we're slack.

const int MAXIMUM_SLEEP_PERIOD = 200;
  // the number of milliseconds we use for breaking up longer sleep periods.

const int SNOOZE_FOR_RETRY = 100;
  // how long to sleep when a thread creation fails.

#ifdef COUNT_THREADS
  // singleton thread counter code.
  class thread_counter : public virtual root_object {
  public:
    thread_counter() : _count(0) {}
    DEFINE_CLASS_NAME("thread_counter");
    void increment() { 
      auto_synchronizer l(_lock);
      _count++;
    }
    void decrement() {
      auto_synchronizer l(_lock);
      _count--;
    }
  private:
    int _count;
    mutex _lock;
  };

  SAFE_STATIC(thread_counter, _current_threads, )

//hmmm: this seems to not be used anywhere yet.  it needs to be accessible
//      externally if it's going to serve any useful purpose.

#endif

ethread::ethread()
: _thread_ready(false),
  _thread_active(false),
  _stop_thread(false),
  _data(NIL),
#ifdef _MSC_VER
  _handle(0),
#else
  _handle(new pthread_t),
#endif
  _sleep_time(0),
  _periodic(false),
  _next_activation(new time_stamp),
  _how(TIGHT_INTERVAL)  // unused.
{
  FUNCDEF("constructor [one-shot]");
}

ethread::ethread(int sleep_timer, timed_thread_types how)
: _thread_ready(false),
  _thread_active(false),
  _stop_thread(false),
  _data(NIL),
#ifdef _MSC_VER
  _handle(0),
#else
  _handle(new pthread_t),
#endif
  _sleep_time(sleep_timer),
  _periodic(true),
  _next_activation(new time_stamp),
  _how(how)
{
  FUNCDEF("constructor [periodic]");
  if (sleep_timer < MINIMUM_SLEEP_PERIOD) {
    _sleep_time = MINIMUM_SLEEP_PERIOD;
  }
}

ethread::~ethread()
{
  stop();
  WHACK(_next_activation);
#ifndef _MSC_VER
  WHACK(_handle);
#endif
}

///void ethread::pre_thread() {}

///void ethread::post_thread() {}

// the reschedule operation assumes that assignment to a time stamp
// object (based on a real numbers) happens indivisibly.
void ethread::reschedule(int delay)
{
  *_next_activation = time_stamp(delay);  // start after the delay.
}

bool ethread::start(void *thread_data)
{
  FUNCDEF("start");
  if (!thread_finished()) return false;  // already running.
  _data = thread_data;  // store the thread's data pointer.
  _stop_thread = false;  // don't stop now.
  _thread_ready = true;  // we're starting it now.
  _next_activation->reset();  // make "now" the next time to activate.
  bool success = false;
  int error = 0;
  int attempts = 0;
  while (attempts++ < MAXIMUM_CREATE_ATTEMPTS) {
#ifndef _MSC_VER
    pthread_attr_t attribs;  // special flags for creation of thread.
    int aret = pthread_attr_init(&attribs);
    if (aret) LOG("failed to init attribs.");
    aret = pthread_attr_setdetachstate(&attribs, PTHREAD_CREATE_DETACHED);
    if (aret) LOG("failed to set detach state.");
    int ret = -1;
    if (_periodic)
      ret = pthread_create(_handle, &attribs, periodic_thread_driver,
          (void *)this);
    else
      ret = pthread_create(_handle, &attribs, one_shot_thread_driver,
          (void *)this);
    if (!ret) success = true;
    else error = ret;
#else
    if (_periodic)
      _handle = _beginthread(periodic_thread_driver, 0, (void *)this);
    else
      _handle = _beginthread(one_shot_thread_driver, 0, (void *)this);
    if (_handle != -1) success = true;
    else error = critical_events::system_error();
#endif
    if (success) break;  // got it created.
    LOG("failed to create thread; trying again...");
    time_control::sleep_ms(SNOOZE_FOR_RETRY);
  }
  if (!success) {
    // couldn't start it, so reset our state.
    LOG(astring("failed to create thread, error is ")
        + critical_events::system_error_text(error));
    exempt_stop();
    return false;
  }
  return true;
}

void ethread::stop()
{
  cancel();  // tell thread to leave.
  if (!thread_started()) return;  // not running.
  while (!thread_finished()) {
#ifdef _MSC_VER
    int result = 0;
    if (!GetExitCodeThread((HANDLE)_handle, (LPDWORD)&result)
        || (result != STILL_ACTIVE)) {
      exempt_stop();
      break;
    }
#endif
    time_control::sleep_ms(10);  // wait for thread to leave.
  }
}

void ethread::exempt_stop()
{
  _thread_active = false;
  _thread_ready = false;
#ifdef _MSC_VER
  _handle = 0;
#endif
}

#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
void *ethread::one_shot_thread_driver(void *hidden_pointer)
#elif defined(_MSC_VER)
void ethread::one_shot_thread_driver(void *hidden_pointer)
#else
#error unknown thread signature.
#endif
{
  FUNCDEF("one_shot_thread_driver");
  ethread *manager = (ethread *)hidden_pointer;
#ifndef _MSC_VER
  if (!manager) return NIL;
#else
  if (!manager) return;
#endif
#ifdef COUNT_THREADS
  _current_threads().increment();
#endif
///  manager->pre_thread();
  manager->_thread_active = true;
  manager->perform_activity(manager->_data);
///  manager->post_thread();
  manager->exempt_stop();
#ifdef COUNT_THREADS
  _current_threads().decrement();
#endif
#ifndef _MSC_VER
  pthread_exit(NIL);
  return NIL;
#else
  _endthread();
#endif
}

#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
void *ethread::periodic_thread_driver(void *hidden_pointer)
#elif defined(_MSC_VER)
void ethread::periodic_thread_driver(void *hidden_pointer)
#else
#error unknown thread signature.
#endif
{
  FUNCDEF("periodic_thread_driver");
  ethread *manager = (ethread *)hidden_pointer;
#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
  if (!manager) return NIL;
#elif defined(_MSC_VER)
  if (!manager) return;
#endif
#ifdef COUNT_THREADS
  _current_threads().increment();
#endif
///  manager->pre_thread();

  while (!manager->_stop_thread) {
    // for TIGHT_INTERVAL, we reset the next active time here.  this is safe
    // relative to the reschedule() method, since we're about to do
    // perform_activity() right now anyway.  this brings about a pretty hard
    // interval; if perform_activity() takes N milliseconds, then there will
    // only be sleep_time - N (min zero) ms before the next invocation.
    if (manager->_how == TIGHT_INTERVAL)
      *manager->_next_activation = time_stamp(manager->_sleep_time);

    manager->_thread_active = true;
    manager->perform_activity(manager->_data);
    manager->_thread_active = false;

    // SLACK_INTERVAL means between activations.  we reset the next activation
    // here to ensure we wait the period specified for sleep time, including
    // whatever time was taken for the activity itself.
    if (manager->_how == SLACK_INTERVAL)
      *manager->_next_activation = time_stamp(manager->_sleep_time);

    // we do the sleep timing in chunks so that there's not such a huge wait
    // when the user stops the thread before the sleep interval elapses.
    // snooze until time for the next activation.
    while (!manager->_stop_thread) {
      int time_diff = int(manager->_next_activation->value()
          - time_stamp().value());
      if (time_diff < 0) time_diff = 0;  // time keeps on slipping.
      // make sure we take our time if we're slack intervalled.
      if (manager->_how == SLACK_INTERVAL) {
        if (time_diff < MINIMUM_SLEEP_PERIOD)
          time_diff = MINIMUM_SLEEP_PERIOD;
      }
      if (time_diff > MAXIMUM_SLEEP_PERIOD)
        time_diff = MAXIMUM_SLEEP_PERIOD;
      if (!manager->_stop_thread)
        time_control::sleep_ms(time_diff);
      if (time_stamp() >= *manager->_next_activation)
        break;
    }
  }
///  manager->post_thread();
  manager->exempt_stop();
#ifdef COUNT_THREADS
  _current_threads().decrement();
#endif
#ifndef _MSC_VER
  pthread_exit(NIL);
  return NIL;
#else
  _endthread();
#endif
}

} //namespace.

