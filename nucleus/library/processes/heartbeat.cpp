


/*****************************************************************************\
*                                                                             *
*  Name   : heartbeat                                                         *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1996-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "heartbeat.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <timely/time_stamp.h>

using namespace basis;
using namespace timely;

namespace processes {

heartbeat::heartbeat(int misses_allowed, int check_interval)
: _next_heartbeat(new time_stamp()),
  _check_interval(0),
  _misses_allowed(0),
  _misses(0)
{ reset(misses_allowed, check_interval); }

heartbeat::heartbeat(const heartbeat &to_copy)
: root_object(),
  _next_heartbeat(new time_stamp()),
  _check_interval(0),
  _misses_allowed(0),
  _misses(0)
{ *this = to_copy; }

heartbeat::~heartbeat() { WHACK(_next_heartbeat); }

time_stamp heartbeat::heartbeat_time() const { return *_next_heartbeat; }

bool heartbeat::due() const { return time_left() <= 0; }

void heartbeat::made_request() { _misses++; reset_next_beat(); }

void heartbeat::kabump() { _misses = 0; reset_next_beat(); }

void heartbeat::reset_next_beat()
{ *_next_heartbeat = time_stamp(_check_interval); }

int heartbeat::time_left() const
{ return int(_next_heartbeat->value() - time_stamp().value()); }

bool heartbeat::dead() const
{
  // two cases mean the timer's dead; (1) if the misses are already too high,
  // or (2) if the heartbeat is due and the misses are as many as allowed.
  return (_misses > _misses_allowed)
      || (due() && (_misses >= _misses_allowed));
}

void heartbeat::reset(int misses_allowed, int check_interval)
{
  _misses_allowed = misses_allowed;
  _misses = 0;
  _check_interval = check_interval;
  reset_next_beat();
}

astring heartbeat::text_form(bool detailed) const
{
  astring to_return = (dead()? astring("expired, ") : astring("alive, "));
  to_return += (!dead() && due() ? astring("due now, ")
      : astring::empty_string());
  to_return += a_sprintf("beats left=%d", misses_left());
  if (detailed) {
    to_return += a_sprintf(", missed=%d, interval=%d, ",
        missed_so_far(), checking_interval());
    to_return += astring("next=") + heartbeat_time().text_form();
  }
  return to_return;
}

heartbeat &heartbeat::operator =(const heartbeat &to_copy)
{
  if (this == &to_copy) return *this;
  _check_interval = to_copy._check_interval;
  _misses_allowed = to_copy._misses_allowed;
  _misses = to_copy._misses;
  *_next_heartbeat = *to_copy._next_heartbeat;
  return *this;
}

} //namespace.


