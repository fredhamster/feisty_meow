


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

#include "throughput_counter.h"

#include <basis/functions.h>

using namespace basis;
using namespace timely;

namespace sockets {

throughput_counter::throughput_counter()
: _running(false),
  _start(new time_stamp),
  _end(new time_stamp),
  _time_overall(0),
  _byte_count(0),
  _send_count(0)
{}

throughput_counter::throughput_counter(const throughput_counter &to_copy)
: _start(new time_stamp),
  _end(new time_stamp)
{
  *this = to_copy;
}

throughput_counter::~throughput_counter()
{
  _running = false;
  WHACK(_start);
  WHACK(_end);
}

throughput_counter &throughput_counter::operator =
    (const throughput_counter &to_copy)
{
  if (this == &to_copy) return *this;  // bail on copying to self.
  _running = to_copy._running;
  *_start = *to_copy._start;
  *_end = *to_copy._end;
  _time_overall = to_copy._time_overall;
  _byte_count = to_copy._byte_count;
  _send_count = to_copy._send_count;
  return *this;
}

void throughput_counter::combine(const throughput_counter &to_blend)
{
  if (this == &to_blend) return;  // no, we don't like that.
  _time_overall += to_blend._time_overall;
  _byte_count += to_blend._byte_count;
  _send_count += to_blend._send_count;
}

void throughput_counter::start()
{
  if (running()) return;  // can't start if already started.
  *_start = time_stamp();
  *_end = time_stamp();  // just to clear.
  _running = true;
}

void throughput_counter::stop()
{
  if (!running()) return;  // better have been started before stopping.
  *_end = time_stamp();
  _time_overall += _end->value() - _start->value();
  _running = false;
}

void throughput_counter::reset()
{
  _running = false;
  _start->reset();
  _end->reset();
  _time_overall = 0;
  _byte_count = 0;
  _send_count = 0;
}

void throughput_counter::send(double size_of_send)
{
  if (!running()) return;  // can't add if we're not in a run.
  _send_count++;
  _byte_count += size_of_send;
}

void throughput_counter::add_run(double size_of_send, double time_of_send,
    double number_of_runs)
{
  _send_count += number_of_runs;
  _byte_count += size_of_send;
  _time_overall += time_of_send;
}

time_stamp throughput_counter::start_time() const { return *_start; }

time_stamp throughput_counter::stop_time() const { return *_end; }

double throughput_counter::total_time() const
{
  double extra_time = running()? time_stamp().value() - _start->value() : 0;
  return _time_overall + extra_time;
}

double throughput_counter::bytes_per_second() const
{
  double total = total_time() / SECOND_ms;
  return double(bytes_sent()) / total;
}

double throughput_counter::kilobytes_per_second() const
{ return bytes_per_second() / double(KILOBYTE); }

double throughput_counter::megabytes_per_second() const
{ return kilobytes_per_second() / double(KILOBYTE); }

} //namespace.

