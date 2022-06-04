/*
*  Name   : timer_driver
*  Author : Chris Koeritz

* Copyright (c) 2003-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "timer_driver.h"

#include <application/windoze_helper.h>
#include <basis/functions.h>
#include <basis/mutex.h>
#include <processes/ethread.h>
#include <structures/amorph.h>
#include <structures/static_memory_gremlin.h>
#include <timely/time_control.h>
#include <timely/time_stamp.h>

#include <signal.h>
#include <stdio.h>
#ifdef __UNIX__
  #include <sys/time.h>
#endif

using namespace basis;
using namespace processes;
using namespace structures;
using namespace timely;

#define DEBUG_TIMER_DRIVER
  // uncomment for noisy code.

#undef LOG
#define LOG(tpr) printf("%s", (time_stamp::notarize() + "timer_driver::" + func + tpr).s() )

namespace timely {

const int INITIAL_TIMER_GRANULARITY = 14;
  // the timer will support durations of this length or greater initially.
  // later durations will be computed based on the timers waiting.

const int MAX_TIMER_PREDICTION = 140;
  // this is the maximum predictive delay before we wake up again to see if
  // any new timed items have arrived.  this helps us to not wait too long
  // when something's scheduled in between snoozes.

const int PAUSE_TIME = 200;
  // we will pause this many milliseconds if the timer is already occurring
  // when we're trying to get the lock on our list.

const int LONG_TIME = 1 * HOUR_ms;
  // the hook can be postponed a really long time with this when necessary.

//////////////

SAFE_STATIC(timer_driver, timer_driver::global_timer_driver, )

//////////////

#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
const int OUR_SIGNAL = SIGUSR2;

class signalling_thread : public ethread
{
public:
  signalling_thread(int initial_interval) : ethread(initial_interval) {}
  
  void perform_activity(void *formal(ptr)) {
    raise(OUR_SIGNAL);
  }

private:
};
#endif

#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
void timer_driver_private_handler(int signal_seen)
#elif defined(__WIN32__)
void __stdcall timer_driver_private_handler(window_handle hwnd, basis::un_int msg,
    UINT_PTR id, un_long time)
#else
  #error No timer method known for this OS.
#endif
{
#ifdef DEBUG_TIMER_DRIVER
  #undef static_class_name
  #define static_class_name() "timer_driver"
  FUNCDEF("timer_driver_private_handler");
#endif
#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
  int seen = signal_seen;
  if (seen != OUR_SIGNAL) {
#elif defined(__WIN32__)
  basis::un_int *seen = (basis::un_int *)id;
  if (seen != program_wide_timer().real_timer_id()) {
#else
  if (true) {  // unknown OS.
#endif
#ifdef DEBUG_TIMER_DRIVER
    LOG(a_sprintf("unknown signal/message %d caught.", seen));
#endif
    return;
  }
  program_wide_timer().handle_system_timer();
  #undef static_class_name
}

//////////////

class driven_object_record
{
public:
  int _duration;  // the interval for timer hits on this object.
  timeable *_to_invoke;  // the object that will be called back.
  time_stamp _next_hit;  // next time the timer should hit for this object.
  bool _okay_to_invoke;  // true if this object is okay to call timers on.
  bool _handling_timer;  // true if we're handling this object right now.

  driven_object_record(int duration, timeable *to_invoke)
  : _duration(duration), _to_invoke(to_invoke), _next_hit(duration),
    _okay_to_invoke(true), _handling_timer(false) {}
};

class driven_objects_list
: public amorph<driven_object_record>,
  public virtual root_object
{
public:
  DEFINE_CLASS_NAME("driven_objects_list");

  int find_obj(timeable *obj) {
    for (int i = 0; i < elements(); i++) {
      if (borrow(i) && (borrow(i)->_to_invoke == obj))
        return i;
    }
    return common::NOT_FOUND;
  }
};

//////////////

timer_driver::timer_driver()
: _timers(new driven_objects_list),
  _lock(new mutex),
#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
  _prompter(new signalling_thread(INITIAL_TIMER_GRANULARITY)),
#else
  _real_timer_id(NULL_POINTER),
#endif
  _in_timer(false)
{
  hookup_OS_timer(INITIAL_TIMER_GRANULARITY);

#ifdef __UNIX__
  // register for the our personal signal.
  signal(OUR_SIGNAL, &timer_driver_private_handler);
  _prompter->start(NULL_POINTER);
#endif
}

timer_driver::~timer_driver()
{
#ifdef DEBUG_TIMER_DRIVER
  FUNCDEF("destructor");
#endif
#ifdef __UNIX__
  _prompter->stop();

  struct sigaction action;
  action.sa_handler = SIG_DFL;
  action.sa_sigaction = NULL_POINTER;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
#ifndef __APPLE__
  action.sa_restorer = NULL_POINTER;
#endif
  int ret = sigaction(OUR_SIGNAL, &action, NULL_POINTER);
  if (ret) {
///uhhh
  }
#endif
  unhook_OS_timer();

  // make sure we aren't still in a timer handler when we reset our list.
  while (true) {
    _lock->lock();
    if (_in_timer) {
      _lock->unlock();
#ifdef DEBUG_TIMER_DRIVER
      LOG("waiting to acquire timer_driver lock.");
#endif
      time_control::sleep_ms(PAUSE_TIME);
    } else {
      break;
    }
  }

  _timers->reset();  // clear out the registered functions.
  _lock->unlock();

  WHACK(_timers);
  WHACK(_lock);
#ifdef __UNIX__
  WHACK(_prompter);
#endif

#ifdef DEBUG_TIMER_DRIVER
  LOG("timer_driver is closing down.");
#endif
}

#ifdef _MSC_VER
basis::un_int *timer_driver::real_timer_id() { return _real_timer_id; }
#endif

bool timer_driver::zap_timer(timeable *to_remove)
{
#ifdef DEBUG_TIMER_DRIVER
  FUNCDEF("zap_timer");
#endif
#ifdef DEBUG_TIMER_DRIVER
  if (_in_timer) {
    LOG("hmmm: zapping timer while handling previous timer...!");
  }
#endif
  auto_synchronizer l(*_lock);
  int indy = _timers->find_obj(to_remove);
  if (negative(indy)) return false;  // unknown.
#ifdef DEBUG_TIMER_DRIVER
  LOG(a_sprintf("zapping timer %x.", to_remove));
#endif
  driven_object_record *reco = _timers->borrow(indy);
  reco->_okay_to_invoke = false;
  if (reco->_handling_timer) {
    // results are not guaranteed if we see this situation.
#ifdef DEBUG_TIMER_DRIVER
    LOG(a_sprintf("Logic Error: timer %x being zapped WHILE BEING HANDLED!",
        to_remove));
#endif
  }
  return true;
}

bool timer_driver::set_timer(int duration, timeable *to_invoke)
{
#ifdef DEBUG_TIMER_DRIVER
  FUNCDEF("set_timer");
  if (_in_timer) {
    LOG("hmmm: setting timer while handling previous timer...!");
  }
#endif
#ifdef DEBUG_TIMER_DRIVER
  LOG(a_sprintf("setting timer %x to %d ms.", to_invoke, duration));
#endif
  auto_synchronizer l(*_lock);
  // find any existing record.
  int indy = _timers->find_obj(to_invoke);
  if (negative(indy)) {
    // add a new record to list.
    _timers->append(new driven_object_record(duration, to_invoke));
  } else {
    // change the existing record.
    driven_object_record *reco = _timers->borrow(indy);
    reco->_duration = duration;
    reco->_okay_to_invoke = true;  // just in case.
  }
  return true;
}

void timer_driver::handle_system_timer()
{
#ifdef DEBUG_TIMER_DRIVER
  FUNCDEF("handle_system_timer");
#endif
  if (_in_timer) {
#ifdef DEBUG_TIMER_DRIVER
    LOG("terrible error: invoked system timer while handling previous timer.");
#endif
    return;
  }
  unhook_OS_timer();

#ifdef DEBUG_TIMER_DRIVER
  LOG("into handling OS timer...");
#endif

  array<driven_object_record *> to_invoke_now;

  {
    // lock the list for a short time, just to put in a stake for the timer
    // flag; no one is allowed to change the list while this is set to true.
    auto_synchronizer l(*_lock);
    _in_timer = true;

    // zip across our list and find out which of the timer functions should be
    // invoked.
    for (int i = 0; i < _timers->elements(); i++) {
      driven_object_record *funky = _timers->borrow(i);
      if (!funky) {
        const char *msg = "error: timer_driver's timer list logic is broken.";
#ifdef DEBUG_TIMER_DRIVER
        LOG(msg);
#endif
#ifdef CATCH_ERRORS
        throw msg;
#endif
        _timers->zap(i, i);
        i--;  // skip back over dud record.
        continue;
      }
      if (funky->_next_hit <= time_stamp()) {
        // this one needs to be jangled.
        to_invoke_now += funky;
      }
    }
  }

#ifdef DEBUG_TIMER_DRIVER
  astring pointer_dump;
  for (int i = 0; i < to_invoke_now.length(); i++) {
    driven_object_record *funky = to_invoke_now[i];
    pointer_dump += a_sprintf("%x ", funky->_to_invoke);
  }
  if (pointer_dump.t())
    LOG(astring("activating ") + pointer_dump);
#endif

  // now that we have a list of timer functions, let's call on them.
  for (int i = 0; i < to_invoke_now.length(); i++) {
    driven_object_record *funky = to_invoke_now[i];
    {
      auto_synchronizer l(*_lock);
      if (!funky->_okay_to_invoke) continue;  // skip this guy.
      funky->_handling_timer = true;
    }
    // call the timer function.
    funky->_to_invoke->handle_timer_callback();
    {
      auto_synchronizer l(*_lock);
      funky->_handling_timer = false;
    }
    // reset the time for the next hit.
    funky->_next_hit.reset(funky->_duration);
  }

  // compute the smallest duration before the next guy should fire.
  int next_timer_duration = MAX_TIMER_PREDICTION;
  time_stamp now;  // pick a point in time as reference for all timers.
  for (int i = 0; i < _timers->elements(); i++) {
    driven_object_record *funky = _timers->borrow(i);
    int funky_time = int(funky->_next_hit.value() - now.value());
    // we limit the granularity of timing since we don't want to be raging
    // on the CPU with too small a duration.
    if (funky_time < INITIAL_TIMER_GRANULARITY)
      funky_time = INITIAL_TIMER_GRANULARITY;
    if (funky_time < next_timer_duration)
      next_timer_duration = funky_time;
  }

  {
    // release the timer flag again and do any cleanups that are necessary.
    auto_synchronizer l(*_lock);
    _in_timer = false;
    for (int i = 0; i < _timers->elements(); i++) {
      driven_object_record *funky = _timers->borrow(i);
      if (!funky->_okay_to_invoke) {
        // clean up something that was unhooked.
        _timers->zap(i, i);
        i--;
      }
    }
  }

#ifdef DEBUG_TIMER_DRIVER
  LOG("done handling OS timer.");
#endif

  // set the next expiration time to the smallest next guy.
  reset_OS_timer(next_timer_duration);
}

// the following OS_timer methods do not need to lock the mutex, since they
// are not actually touching the list of timers.

void timer_driver::hookup_OS_timer(int duration)
{
  FUNCDEF("hookup_OS_timer");
  if (negative(duration)) {
#ifdef DEBUG_TIMER_DRIVER
    LOG("seeing negative duration for timer!");
#endif
    duration = 1;
  } else if (!duration) {
#ifdef DEBUG_TIMER_DRIVER
    LOG("patching zero duration for timer.");
#endif
    duration = 1;
  }
#ifdef DEBUG_TIMER_DRIVER
  LOG(a_sprintf("hooking next OS timer in %d ms.", duration));
#endif
#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
  // just make our thread hit after the duration specified.
  _prompter->reschedule(duration);
#elif defined(_MSC_VER)
  int max_tries_left = 100;
  while (max_tries_left-- >= 0) {
    _real_timer_id = (basis::un_int *)SetTimer(NULL_POINTER, 0, duration,
        timer_driver_private_handler);
    if (!_real_timer_id) {
      // failure to set the timer.
      LOG("could not set the interval timer.");
      time_control::sleep_ms(50);  // snooze for a bit to see if we can get right.
      continue;
    } else
      break;  // success hooking timer.
  }
#endif
}

void timer_driver::unhook_OS_timer()
{
#ifdef DEBUG_TIMER_DRIVER
  FUNCDEF("unhook_OS_timer");
#endif
#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
  // postpone the thread for quite a while so we can take care of business.
  _prompter->reschedule(LONG_TIME);
#elif defined(_MSC_VER)
  if (_real_timer_id) KillTimer(NULL_POINTER, (UINT_PTR)_real_timer_id);
#endif
#ifdef DEBUG_TIMER_DRIVER
  LOG("unhooked OS timer.");
#endif
}

void timer_driver::reset_OS_timer(int next_hit)
{
#ifdef DEBUG_TIMER_DRIVER
  FUNCDEF("reset_OS_timer");
#endif
  unhook_OS_timer();  // stop the timer from running.
  hookup_OS_timer(next_hit);  // restart the timer with the new interval.
}

} //namespace.

