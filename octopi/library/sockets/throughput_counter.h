#ifndef THROUGHPUT_COUNTER_CLASS
#define THROUGHPUT_COUNTER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : throughput_counter                                                *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <timely/time_stamp.h>

namespace sockets {

//! Reports on average bandwidth of the transfers being measured.
/*!
  Tracks the amount of data sent over a period of time and provides
  statistics about the transfer rate.
*/

class throughput_counter
{
public:
  throughput_counter();
  throughput_counter(const throughput_counter &to_copy);
  ~throughput_counter();

  throughput_counter &operator =(const throughput_counter &to_copy);

  void start();
    //!< begins timing a run.
    /*!< the current time is recorded and any data sent will be tracked until
    stop() is invoked.  results from previous runs will be merged with the
    current run. */

  void stop();
    //!< ends the current run.
    /*!< the report functions provide information about the speed achieved
    over this and previous runs. */

  void reset();
    //!< clears all statistics and starts over.

  void combine(const throughput_counter &to_blend);
    //!< incorporates the statistics from "to_blend" into this counter.
    /*!< the stats in "to_blend" then no longer need to be considered,
    since this object records its own plus the blended statistics.  note
    that makes the most sense if both this and "to_blend" are not currently
    running a simulation, although combining running counters is not
    prohibited.  if either counter is running, those current runs are
    ignored and only accumulated stats are combined. */

  void send(double size_of_send);
    //!< records a sending of "size_of_send" bytes.
    /*!< this should only be called while a test run is being made; the send
    will be ignored if a run is not occurring. */

  void add_run(double size_of_send, double time_of_send,
          double number_of_runs = 1.0);
    //!< records a run without changing the state of the current run.
    /*!< this supports adding a timed run to the counter without requiring that
    start and stop be used.  this will work whether a run is currently
    being timed or not. */

  bool running() const { return _running; }
    //!< returns whether a test run is being worked on or not.

  timely::time_stamp start_time() const;
    //!< reports the time when this run started.
    /*!< this and stop_time() report the timing information for the current
    run, and so are only really relevant when a run is occurring. */
  timely::time_stamp stop_time() const;
    //!< reports the time when this run was stopped.

  double bytes_sent() const { return _byte_count; }
    //!< returns the number of bytes sent so far.
    /*!< bytes_sent() and number_of_sends() work at any point during a test
    run to provide an interim measurement.  however after a test run, they
    report the statistics for the entire history of testing. */
  double number_of_sends() const { return _send_count; }
    //!< returns the number of sends that have occurred.

  double bytes_per_second() const;
    //!< returns the number of bytes that transfers are getting per second.
  double kilobytes_per_second() const;
    //!< returns the number of kilobytes that transfers are getting per second.
  double megabytes_per_second() const;
    //!< returns the number of megabytes that transfers are getting per second.

  double total_time() const;
    //!< the run time so far, in milliseconds.
    /*!< this also counts the time in the current run, if one is occurring. */

private:
  bool _running;  //!< true if we're currently testing.
  timely::time_stamp *_start;  //!< when the current run was started.
  timely::time_stamp *_end;  //!< when the run was stopped.
  double _time_overall;  //!< how much time has been accumulated.
  double _byte_count;  //!< the amount of data sent so far.
  double _send_count;  //!< the number of times data has been sent.
};

} //namespace.

#endif

