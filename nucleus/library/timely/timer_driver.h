#ifndef TIMER_DRIVER_CLASS
#define TIMER_DRIVER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : timer_driver                                                      *
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

#include <basis/astring.h>
#include <basis/definitions.h>
#include <basis/mutex.h>

namespace timely {

// forward.
class driven_objects_list;
class signalling_thread;

//////////////

//! timeable is the base for objects that can be hooked into timer events.

class timeable : public virtual basis::root_object
{
public:
////  virtual ~timeable() {}
  virtual void handle_timer_callback() = 0;
    //!< this method is invoked when the timer period elapses for this object.
};

//////////////

//! Provides platform-independent timer support.
/*!
  Multiple objects can be hooked to the timer to be called when their interval
  elapses.  The driver allows new timeables to be added as needed.

  NOTE: Only one of the timer_driver objects is allowed per program.
*/

class timer_driver : public virtual basis::root_object
{
public:
  timer_driver();
  virtual ~timer_driver();

  DEFINE_CLASS_NAME("timer_driver");

  // main methods for controlling timeables.

  bool set_timer(int duration, timeable *to_invoke);
    //!< sets a timer to call "to_invoke" every "duration" milliseconds.
    /*!< if the object "to_invoke" already exists, then its duration is
    changed. */

  bool zap_timer(timeable *to_drop);
    //!< removes the timer that was established for "to_drop".
    /*!< do not zap a timer from its own callback!  that could cause
    synchronization problems. */

  // internal methods.

#ifdef __WIN32__
  basis::un_int *real_timer_id();
    //!< provides the timer id for comparison on windows platforms.
#endif

  void handle_system_timer();
    //!< invoked by the OS timer support and must be called by main thread.

  static timer_driver &global_timer_driver();
    //!< the first time this is invoked, it creates a program-wide timer driver.

private:
  driven_objects_list *_timers;  //!< timer hooked objects.
  basis::mutex *_lock;  //!< protects list of timers.
#ifdef __UNIX__
  signalling_thread *_prompter;  //!< drives our timers.
#endif
#ifdef __WIN32__
  basis::un_int *_real_timer_id;  //!< used for storing window timer handle.
#endif
  bool _in_timer;  //!< true if we're handling the timer right now.

  // these do the low-level system magic required to get a function hooked
  // to a timer.
  void hookup_OS_timer(int duration);
    //!< hooks us into the timer events at the "duration" specified.
  void reset_OS_timer(int next_hit);
    //!< changes the root interval to "next_hit".
    /*!< only that many milliseconds will elapse before the next timer hit. */
  void unhook_OS_timer();
    //!< disconnects us from the timer events.
};

//////////////

#define program_wide_timer() timer_driver::global_timer_driver()
  //!< provides access to the singleton timer_driver.
  /*!< no other timer_driver objects should ever be created, since this
  single one will service all timer needs within the program. */

} //namespace.

#endif

