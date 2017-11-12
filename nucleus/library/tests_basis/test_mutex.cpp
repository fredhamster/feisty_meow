/*****************************************************************************\
*                                                                             *
*  Name   : test_mutex                                                        *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1994-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/guards.h>
#include <basis/mutex.h>
#include <configuration/application_configuration.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <mathematics/chaos.h>
#include <processes/ethread.h>
#include <processes/safe_roller.h>
#include <structures/amorph.h>
#include <structures/static_memory_gremlin.h>
#include <timely/time_control.h>
#include <timely/time_stamp.h>
#include <unit_test/unit_base.h>

#ifdef __WIN32__
  #include <process.h>
#endif

using namespace application;
using namespace basis;
using namespace loggers;
using namespace mathematics;
using namespace timely;
using namespace processes;
using namespace structures;
using namespace unit_test;

//#define DEBUG_MUTEX
  // uncomment for a verbose test run.

const int MAX_MUTEX_TIMING_TEST = 2000000;
  // the number of times we'll lock and unlock a mutex.

const int DEFAULT_FISH = 32;
  // the number of threads, by default.

const int DEFAULT_RUN_TIME = 2 * SECOND_ms;
  // the length of time to run the program.

const int THREAD_PAUSE_LOWEST = 0;
const int THREAD_PAUSE_HIGHEST = 48;
  // this is the range of random sleeps that a thread will take after
  // performing it's actions.

const int MIN_SAME_THREAD_LOCKING_TESTS = 100;
const int MAX_SAME_THREAD_LOCKING_TESTS = 1000;
  // the range of times we'll test recursively locking the mutex.

int concurrent_biters = 0;
  // the number of threads that are currently active.

int grab_lock = 0;
  // this is upped whenever a fish obtains access to the mutex.

mutex &guard() { static mutex _muttini; return _muttini; }
  // the guard ensures that the grab lock isn't molested by two fish at
  // once... hopefully.

astring protected_string;
  // this string is protected only by the mutex of guard().

#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger::get(), to_print)
  // our macro for logging with a timestamp.

// expects guardian mutex to already be locked once when coming in.
void test_recursive_locking(chaos &_rando)
{
  int test_attempts = _rando.inclusive(MIN_SAME_THREAD_LOCKING_TESTS,
      MAX_SAME_THREAD_LOCKING_TESTS);
  int locked = 0;
  for (int i = 0; i < test_attempts; i++) {
    bool lock = !!(_rando.inclusive(0, 1));
    if (lock) {
      guard().lock();
      locked++;  // one more lock.
    } else {
      if (locked > 0) {
        // must be sure we are not already locally unlocked completely.
        guard().unlock();
        locked--;
      }
    }
  }
  for (int j = 0; j < locked; j++) {
    // drop any locks we had left during the test.
    guard().unlock();
  }
}

//hmmm: how are these threads different so far?  they seem to do exactly
//      the same thing.  maybe one should eat chars from the string.

#undef UNIT_BASE_THIS_OBJECT
#define UNIT_BASE_THIS_OBJECT c_testing

class piranha : public ethread
{
public:
  chaos _rando;  // our randomizer.
  unit_base &c_testing;  // provides for test recording.

  piranha(unit_base &testing) : ethread(0), c_testing(testing) {
    FUNCDEF("constructor");
    safe_add(concurrent_biters, 1);
    ASSERT_TRUE(concurrent_biters >= 1, "the piranha is very noticeable");
//LOG(astring(astring::SPRINTF, "there are %d biters.", concurrent_biters));
  }

  virtual ~piranha() {
    FUNCDEF("destructor");
    safe_add(concurrent_biters, -1); 
//LOG("reached piranha destructor.");
  }

  DEFINE_CLASS_NAME("piranha");

  void perform_activity(void *formal(data)) {
    FUNCDEF("perform_activity");
    {
      // we grab the lock.
      auto_synchronizer locked(guard());
        // in this case, we make use of auto-synchronizer, handily testing it as well.
      ASSERT_TRUE(&locked != NULL_POINTER, "auto_synchronizer should grab the mutex object's lock");
        // this is not a real test, but indicates that we did actually increase the number of
        // unit tests by one, since we're using auto_synchronizer now.
      safe_add(grab_lock, 1);
      ASSERT_TRUE(grab_lock <= 1, "grab lock should not already be active");
      protected_string += char(_rando.inclusive('a', 'z'));

      test_recursive_locking(_rando);

      safe_add(grab_lock, -1);
    }
    // dropped the lock.  snooze a bit.
    if (!should_stop())
      time_control::sleep_ms(_rando.inclusive(THREAD_PAUSE_LOWEST, THREAD_PAUSE_HIGHEST));
  }

};

class barracuda : public ethread
{
public:
  chaos _rando;  // our randomizer.
  unit_base &c_testing;  // provides for test recording.

  barracuda(unit_base &testing) : ethread(0), c_testing(testing) {
    FUNCDEF("constructor");
    safe_add(concurrent_biters, 1);
    ASSERT_TRUE(concurrent_biters >= 1, "our presence should have been noticed");
//LOG(astring(astring::SPRINTF, "there are %d biters.", concurrent_biters));
  }

  virtual ~barracuda() {
    FUNCDEF("destructor");
    safe_add(concurrent_biters, -1);
//LOG("reached barracuda destructor.");
  }

  DEFINE_CLASS_NAME("barracuda");

  void perform_activity(void *formal(data)) {
    FUNCDEF("perform_activity");
    // we grab the lock.
    guard().lock();
    safe_add(grab_lock, 1);
    ASSERT_TRUE(grab_lock <= 1, "grab lock should not already be active");

    test_recursive_locking(_rando);

    protected_string += char(_rando.inclusive('a', 'z'));
    safe_add(grab_lock, -1);
    guard().unlock();
    // done with the lock.  sleep for a while.
    if (!should_stop())
      time_control::sleep_ms(_rando.inclusive(THREAD_PAUSE_LOWEST, THREAD_PAUSE_HIGHEST));
  }
};

//////////////

#undef UNIT_BASE_THIS_OBJECT
#define UNIT_BASE_THIS_OBJECT (*this)

class test_mutex : virtual public unit_base, virtual public application_shell
{
public:
  chaos _rando;  // our randomizer.

  test_mutex() : application_shell() {}

  DEFINE_CLASS_NAME("test_mutex");

  int execute();
};

int test_mutex::execute()
{
  FUNCDEF("execute");

  // make sure the guard is initialized before the threads run.
  guard().lock();
  guard().unlock();

  {
    // we check how long a lock and unlock of a non-locked mutex will take.
    // this is important to know so that we aren't spending much of our time
    // locking mutexes just due to the mechanism.
    mutex ted;
    time_stamp mutt_in;
    for (int qb = 0; qb < MAX_MUTEX_TIMING_TEST; qb++) {
      ted.lock();
      ted.unlock();
    }
    time_stamp mutt_out;
    double run_count = MAX_MUTEX_TIMING_TEST;
    double full_run_time = (mutt_out.value() - mutt_in.value()) / SECOND_ms;
    double time_per_lock = (mutt_out.value() - mutt_in.value()) / run_count;
    log(a_sprintf("%.0f mutex lock & unlock pairs took %.3f seconds,",
        run_count, full_run_time));
    log(a_sprintf("or %f ms per (lock+unlock).", time_per_lock));
    ASSERT_TRUE(time_per_lock < 1.0, "mutex lock timing should be super fast");
  }

  amorph<ethread> thread_list;

  for (int i = 0; i < DEFAULT_FISH; i++) {
    ethread *t = NULL_POINTER;
    if (i % 2) t = new piranha(*this);
    else t = new barracuda(*this);
    thread_list.append(t);
    ethread *q = thread_list[thread_list.elements() - 1];
    ASSERT_EQUAL(q, t, "amorph pointer equivalence is required");
    // start the thread we added.
    q->start(NULL_POINTER);
  }

  time_stamp when_to_leave(DEFAULT_RUN_TIME);
  while (when_to_leave > time_stamp()) {
    time_control::sleep_ms(100);
  }

//hmmm: try just resetting the amorph;
//      that should work fine.

#ifdef DEBUG_MUTEX
  LOG("now cancelling all threads....");
#endif

  for (int j = 0; j < thread_list.elements(); j++) thread_list[j]->cancel();

#ifdef DEBUG_MUTEX
  LOG("now stopping all threads....");
#endif

  for (int k = 0; k < thread_list.elements(); k++) thread_list[k]->stop();

  int threads_active = 0;
  for (int k = 0; k < thread_list.elements(); k++) {
    if (thread_list[k]->thread_active()) threads_active++;
  }
  ASSERT_EQUAL(threads_active, 0, "threads should actually have stopped by now");

#ifdef DEBUG_MUTEX
  LOG("resetting thread list....");
#endif

  thread_list.reset();  // should whack all threads.

  ASSERT_EQUAL(concurrent_biters, 0, "threads should all be gone by now");

#ifdef DEBUG_MUTEX
  LOG("done exiting from all threads....");

  LOG(astring(astring::SPRINTF, "the accumulated string had %d characters "
      "which means\nthere were %d thread activations from %d threads.",
      protected_string.length(), protected_string.length(),
      DEFAULT_FISH));
#endif

  return final_report();
}

HOOPLE_MAIN(test_mutex, )

