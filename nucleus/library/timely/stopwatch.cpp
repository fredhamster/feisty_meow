/***********************
*                                                                             *
*  Name   : stopwatch
*  Author : Chris Koeritz
*                                                                             *
*******************************************************************************
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "time_stamp.h"
#include "stopwatch.h"

#include <basis/definitions.h>
#include <basis/functions.h>
#include <basis/guards.h>

using namespace basis;

namespace timely {

stopwatch::stopwatch()
: _status(UNSTARTED),
  _start_time(new time_stamp()),
  _stop_time(new time_stamp()),
  _total_so_far(0)
{}

stopwatch::stopwatch(const stopwatch &to_copy)
: _status(UNSTARTED),
  _start_time(new time_stamp()),
  _stop_time(new time_stamp()),
  _total_so_far(0)
{ *this = to_copy; }

stopwatch::~stopwatch()
{
  _status = UNSTARTED;
  WHACK(_start_time);
  WHACK(_stop_time);
}

stopwatch &stopwatch::operator =(const stopwatch &to_copy)
{
  if (this == &to_copy) return *this;
  *_start_time = *to_copy._start_time;
  *_stop_time = *to_copy._stop_time;
  _status = to_copy._status;
  _total_so_far = to_copy._total_so_far;
  return *this;
}

void stopwatch::reset() { _status = UNSTARTED; _total_so_far = 0; }

int stopwatch::milliseconds() { return common_measure(); }

void stopwatch::start()
{
  if (_status == RUNNING) return;
  *_start_time = time_stamp();
  _status = RUNNING;
}

int stopwatch::compute_diff(const time_stamp &t1, const time_stamp &t2)
{ return int(t2.value() - t1.value()); }

void stopwatch::halt()
{
  if (_status == STOPPED) return;
  else if (_status == UNSTARTED) return;

  *_stop_time = time_stamp();
  _total_so_far += compute_diff(*_start_time, *_stop_time);

  _status = STOPPED;
}

int stopwatch::common_measure()
{
  bool restart = false;
  int to_return = 0;
  switch (_status) {
    case UNSTARTED: break;
    case RUNNING:
      // stop stopwatch, restart afterwards.
      halt();
      restart = true;
      // intentional fall through to default, so no break.
    default:
      // set the return value to the accumulated time.
      to_return = _total_so_far;
      break;
  }
  if (restart) start();  // crank the stopwatch back up if we were supposed to.
  return to_return;
}

} //namespace.

