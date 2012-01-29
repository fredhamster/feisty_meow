#ifndef STOPWATCH_CLASS
#define STOPWATCH_CLASS

/***
*
*  Name   : stopwatch
*  Author : Chris Koeritz
*******************************************************************************
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "time_stamp.h"

namespace timely {

//! A class for measuring event durations in real time.
/*!
  Once the stopwatch is constructed, it can then be repeatedly started and
  halted, and then started again.  The number of milliseconds or
  microseconds elapsed can be requested while the stopwatch is running, but
  that can disrupt fine-grained measurements.
*/

class stopwatch : public virtual basis::root_object
{
public:
  stopwatch();
  stopwatch(const stopwatch &to_copy);

  virtual ~stopwatch();

  stopwatch &operator =(const stopwatch &to_copy);

  void start();
    //!< Begins the timing.
    /*!< If the stopwatch is already timing, then "start" does nothing. */

  void halt();
    //!< Stops the timing.
    /*!< start() may be called again to resume timing after the halt.  If the
    stopwatch is already stopped, or never was started, then halt does nothing. */
  void stop() { halt(); }
    //!< a synonym for halt().

  void reset();
    //!< Stops the stopwatch and clears it to zero time elapsed.

  int milliseconds();
    //!< Returns the elapsed number of milliseconds on the stopwatch, overall.
  int elapsed() { return milliseconds(); }
    //!< a synonym for milliseconds().

private:
  enum stopwatch_kinds { UNSTARTED, RUNNING, STOPPED };  //!< states for the stopwatch.
  stopwatch_kinds _status;  //!< our current state.
  time_stamp *_start_time;  //!< last time we got started.
  time_stamp *_stop_time;  //!< last time we got stopped.
  int _total_so_far;  //!< total amount of time run for so far.

  int common_measure();
    //!< returns the current time used to this point, in milliseconds.

  int compute_diff(const time_stamp &t1, const time_stamp &t2);
    //!< the difference in milliseconds between the times "t1" and "t2".
};

//////////////

//! Logs a warning when an operation takes longer than expected.
/*!
  Place TIME_CHECK_BEGIN before the code that you want to check, then place
  TIME_CHECK_END afterwards.  The two calls must be in the same scope.
  "logger" should be a reference to a log_base object.   [ by Brit Minor ]
*/
#define TIME_CHECK_BEGIN \
  stopwatch t; \
  t.start();
#define TIME_CHECK_END(logger, who, msec_limit, what, filter) { \
  t.halt(); \
  if (t.milliseconds() > msec_limit) { \
    (logger).log( a_sprintf("TIME_CHECK: %s: %d ms wait for %s.", \
        (who), t.milliseconds(), (what)), filter); \
  } \
}

} //namespace.

#endif

