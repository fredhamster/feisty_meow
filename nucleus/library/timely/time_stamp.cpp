/*****************************************************************************\
*                                                                             *
*  Name   : time_stamp                                                        *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1995-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "earth_time.h"
#include "time_stamp.h"

#include <basis/environment.h>
#include <basis/mutex.h>
#include <loggers/program_wide_logger.h>

#include <stdlib.h>
#ifdef __WIN32__
  #define _WINSOCKAPI_  // make windows.h happy about winsock.
  #include <winsock2.h>  // timeval.
#endif

//#define DEBUG_TIME_STAMP

#ifdef DEBUG_TIME_STAMP
  #define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)
  using namespace loggers;
#endif

using namespace basis;

namespace timely {

static mutex &__uptime_synchronizer() {
  static mutex uptiming_syncher;
  return uptiming_syncher;
}

basis::astring time_stamp::notarize(bool add_space)
{
  const time_locus the_time = now();
  astring to_return;
  the_time.text_form_long(to_return, clock_time::MILITARY | clock_time::MILLISECONDS);
  if (add_space) to_return += " ";
  return to_return;
}

time_stamp::time_stamp() : c_stamp(0) { fill_in_time(); }

time_stamp::time_stamp(time_representation offset)
: c_stamp(0) { reset(offset); }

void time_stamp::reset() { fill_in_time(); }

astring time_stamp::text_form(stamp_display_style style) const
{
  time_representation stump = c_stamp;
  bool past = false;
  if (style == STAMP_RELATIVE) {
    // adjust the returned time by subtracting the current time.
    stump -= get_time_now();
    if (negative(stump)) {
      // if we're negative, just note that the stamp is in the past.
      past = true;
      stump = absolute_value(stump);
    }
  }
  time_representation divisor = 3600 * SECOND_ms;
  basis::un_int hours = basis::un_int(stump / divisor);
  stump -= divisor * time_representation(hours);
  divisor /= 60;
  basis::un_int minutes = basis::un_int(stump / divisor);
  stump -= divisor * time_representation(minutes);
  divisor /= 60;
  basis::un_int seconds = basis::un_int(stump / divisor);
  stump -= divisor * time_representation(seconds);
  basis::un_int milliseconds = basis::un_int(stump);
  // make absolutely sure we are between 0 and 999.
  milliseconds %= 1000;

  astring to_return;
  bool did_hours = false;
  if (hours) {
    to_return += astring(astring::SPRINTF, "%uh:", hours);
    did_hours = true;
  }
  if (minutes || did_hours)
    to_return += astring(astring::SPRINTF, "%02um:", minutes);
  to_return += astring(astring::SPRINTF, "%02us.%03u", seconds, milliseconds);
  if (style == STAMP_RELATIVE) {
    if (past) to_return += " ago";
    else to_return += " from now";
  }
  return to_return;
}

void time_stamp::fill_in_time()
{
  time_representation current = get_time_now();
  c_stamp = current;  // reset our own time now.
}

void time_stamp::reset(time_representation offset)
{
  fill_in_time();
  c_stamp += offset;
}

time_stamp::time_representation time_stamp::get_time_now()
{ return rolling_uptime(); }

const double __rollover_point = 2.0 * MAXINT32;
  // this number is our rollover point for 32 bit integers.

double time_stamp::rolling_uptime()
{
  auto_synchronizer l(__uptime_synchronizer());
    // protect our rollover records.

  static basis::un_int __last_ticks = 0;
  static int __rollovers = 0;

  basis::un_int ticks_up = environment::system_uptime();
    // acquire the current uptime as a 32 bit unsigned int.

  if (ticks_up < __last_ticks) {
    // rollover happened.  increment our tracker.
    __rollovers++;
  }
  __last_ticks = ticks_up;

  return double(__rollovers) * __rollover_point + double(ticks_up);
}

void time_stamp::fill_timeval_ms(struct timeval &time_out, int duration)
{
  FUNCDEF("fill_timeval_ms");
  // timeval has tv_sec=seconds, tv_usec=microseconds.
  if (!duration) {
    // duration is immediate for the check; just a quick poll.
    time_out.tv_sec = 0;
    time_out.tv_usec = 0;
#ifdef DEBUG_TIME_STAMP
    LOG("no duration specified");
#endif
  } else {
    // a non-zero duration means we need to compute secs and usecs.
    time_out.tv_sec = duration / 1000;
    // set the number of seconds from the input in milliseconds.
    duration -= time_out.tv_sec * 1000;
    // now take out the chunk we've already recorded as seconds.
    time_out.tv_usec = duration * 1000;
    // set the number of microseconds from the remaining milliseconds.
#ifdef DEBUG_TIME_STAMP
    LOG(a_sprintf("duration of %d ms went to %d sec and %d usec.", duration,
        time_out.tv_sec, time_out.tv_usec));
#endif
  }
}

} //namespace.

