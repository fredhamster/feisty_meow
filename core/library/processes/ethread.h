#ifndef ETHREAD_CLASS
#define ETHREAD_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : ethread (easy thread)                                             *
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

#include <basis/contracts.h>
#include <timely/time_stamp.h>

#include <signal.h>

#ifndef __APPLE__
#ifdef __UNIX__
//  typedef long unsigned int pthread_t;
#endif
#endif

namespace processes {

//! Provides a platform-independent object for adding threads to a program.
/*!
  This greatly simplifies creating and managing threads by hiding all the
  operating system details.  The user just needs to override one virtual
  function in their derived object to perform the main activity of their
  thread.  The thread can be a one time invocation or it can run periodically.
  Control over the thread remains in the hands of the program that started
  it.
*/

class ethread : public virtual basis::root_object
{
public:
  ethread();
    //!< creates a single-shot thread object.
    /*!< the OS-level thread is not started until the start() method is
    invoked.  this constructor creates a thread that will only execute
    once; when start() is called, the thread starts up and performs its
    activity.  it will then stop.  to run it again, start() must be invoked
    again.  however, if the perform_activity() method just keeps running,
    then the single-shot thread can live as long as needed.  it is important
    for such a thread to periodically check should_exit() to avoid having
    the program hang-up when it's supposed to be shutting down. */

  enum timed_thread_types { TIGHT_INTERVAL, SLACK_INTERVAL };

  ethread(int sleep_timer, timed_thread_types how = SLACK_INTERVAL);
    //!< creates a managed thread object that runs on a periodic interval.
    /*!< the thread will activate every "sleep_timer" milliseconds.  when
    start() is invoked, the thread's action (via the perform_activity()
    method) will be performed at regular intervals (using the specified value
    for "sleep_timer").  the thread will continue activating until the stop()
    method is called.  a faster interval is used internally during sleep
    periods such that calling stop() will not consume the whole "sleep_timer"
    period.  if the "how" is TIGHT_INTERVAL, then the thread will activate
    every "sleep_timer" milliseconds, as accurately as possible.  if the "how"
    is SLACK_INTERVAL, then the thread will activate after a delay of
    "sleep_timer" milliseconds from its last activation.  the latter mode
    allows the thread to consume its entire intended operation time knowing
    that there will still be slack time between when it is active.  the
    former mode requires the thread to only run for some amount of time less
    than its "sleep_timer"; otherwise it will hog a lot of the CPU. */

  virtual ~ethread();

  DEFINE_CLASS_NAME("ethread");

  bool start(void *thread_data);
    //!< causes the thread to start, if it has not already been started.
    /*!< if the thread has terminated previously, then this will restart the
    thread.  true is returned if the thread could be started.  false is
    returned if the thread could not be started or if it is already running. */

  void stop();
    //!< tells the thread to shutdown and waits for the shutdown to occur.
    /*!< this will cause the OS thread to terminate once the current (if any)
    perform_activity() invocation completes.  the thread may be restarted
    with start(). */

  void cancel() { _stop_thread = true; }
    //!< stops the thread but does not wait until it has terminated.
    /*!< this is appropriate for use within the perform_activity() method. */

//  virtual void pre_thread();
    //!< invoked just after after start(), when the OS thread is created.
    /*!< the call comes in _from_ the thread itself, so the derived method
    must be thread-safe. */
//  virtual void post_thread();
    //!< this is invoked just before the thread is to be terminated.
    /*!< the call also comes in from the thread itself, so the implementation
    must be thread-safe. */

  virtual void perform_activity(void *thread_data) = 0;
    //!< carries out the main activity of the thread.
    /*!< this is called repeatedly by the main thread management function and
    so should return as soon as possible.  if it does not return fairly
    regularly, then the thread shutdown process will not occur until the
    function exits on its own. */

  void exempt_stop();
    //!< this special form of stop() does not wait for the thread to exit.
    /*!< it is required in certain weird OS situations where the thread does
    not exit properly and stop() would cause an infinite wait.  don't use it
    unless you are SURE that this is the case. */

  void reschedule(int delay = 0);
    //!< causes a periodic thread to activate after "delay" milliseconds from now.
    /*!< this resets the normal activation period, but after the next
    activation occurs, the normal activation interval takes over again. */

  int sleep_time() const { return _sleep_time; }
    //!< returns the current periodic thread interval.
    /*!< this is only meaningful for periodic threads. */

  void sleep_time(int new_sleep) { _sleep_time = new_sleep; }
    //!< adjusts the period for the thread to the "new_sleep" interval.
    /*!< this is only meaningful for periodic threads. */

  // these functions report on the thread state.

  bool thread_started() const { return _thread_ready; }
    //!< returns true if the thread has been started.
    /*!< this does not mean it is necessarily active. */

  bool thread_finished() const { return !_thread_ready; }
    //!< returns true if the thread has exited.
    /*!< This can happen either by the thread responding to the stop() or
    cancel() methods or when the thread stops of its own accord.  if this
    returns true, it means that the thread will not start up again unless
    the user invokes start(). */

  bool thread_active() const { return _thread_active; }
    //!< returns true if the thread is currently performing its activity.
    /*!< this information is not necessarily relevant even at the point it is
    returned (because of the nature of multethreading), so don't believe this
    information for anything important. */

  bool should_stop() const { return _stop_thread; }
    //!< reports whether the thread should stop right now.
    /*!< this returns true due to an invocation of stop() or cancel(). */

private:
  bool _thread_ready;  //!< is the thread ready to run (or running)?
  bool _thread_active;  //!< is the thread currently performing?
  bool _stop_thread;  //!< true if the thread should stop now.
  void *_data;  //!< holds the thread's link back to whatever.
#ifdef __UNIX__
  pthread_t *_handle;  //!< thread structure for our thread.
#elif defined(__WIN32__)
  uintptr_t _handle;  //!< thread handle for the active thread, or zero.
#endif
  int _sleep_time;  //!< threads perform at roughly this interval.
  bool _periodic;  //!< true if this thread should run repeatedly.
  timely::time_stamp *_next_activation;  //!< the next time perform_activity is called.
  timed_thread_types _how;  //!< how is the period evaluated?

  // the OS level thread functions.
#ifdef __UNIX__
  static void *periodic_thread_driver(void *hidden_pointer);
  static void *one_shot_thread_driver(void *hidden_pointer);
#elif defined(__WIN32__)
  static void periodic_thread_driver(void *hidden_pointer);
  static void one_shot_thread_driver(void *hidden_pointer);
#endif

  // forbidden.
  ethread(const ethread &);
  ethread &operator =(const ethread &);
};

} //namespace.

#endif

