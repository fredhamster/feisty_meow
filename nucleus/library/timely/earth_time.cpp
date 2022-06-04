/*****************************************************************************\
*                                                                             *
*  Name   : earth_time                                                        *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1999-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "earth_time.h"
#include "time_stamp.h"

#include <basis/astring.h>
#include <basis/utf_conversion.h>
#include <textual/parser_bits.h>

#include <time.h>
#include <sys/time.h>
#if defined(__WIN32__) || defined(__UNIX__)
//  #include <sys/timeb.h>
#endif

#include <stdio.h>

// uncomment for noisy code.
//#define DEBUG_EARTH_TIME

using namespace basis;
using namespace structures;
using namespace textual;

namespace timely {

#undef LOG
#ifdef DEBUG_EARTH_TIME
  #define LOG(tpr) printf("%s", (astring("earth_time::") + func + ": " + tpr + parser_bits::platform_eol_to_chars()).s())
#else
  #define LOG(tpr) 
#endif

//////////////

const time_number days_in_month[12]
    = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

const time_number leap_days_in_month[12]
    = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

const time_number julian_days_in_month[12]
    = { 31, 29, 31, 30, 31, 30, 31, 30, 31, 30, 31, 30 };
//hmmm: is this right?

const time_number julian_leap_days_in_month[12]
    = { 31, 30, 31, 30, 31, 30, 31, 30, 31, 30, 31, 30 };

//////////////

void clock_time::pack(byte_array &packed_form) const
{
  attach(packed_form, hour);
  attach(packed_form, minute);
  attach(packed_form, second);
  attach(packed_form, millisecond);
  attach(packed_form, microsecond);
}

bool clock_time::unpack(byte_array &packed_form)
{
  if (!detach(packed_form, hour)) return false;
  if (!detach(packed_form, minute)) return false;
  if (!detach(packed_form, second)) return false;
  if (!detach(packed_form, millisecond)) return false;
  if (!detach(packed_form, microsecond)) return false;
  return true;
}

#define EASY_LT(x, y) \
  if (x < y) return true; \
  if (x > y) return false

bool clock_time::operator < (const clock_time &to_compare) const
{
  EASY_LT(hour, to_compare.hour);
  EASY_LT(minute, to_compare.minute);
  EASY_LT(second, to_compare.second);
  EASY_LT(millisecond, to_compare.millisecond);
  EASY_LT(microsecond, to_compare.microsecond);
  return false;
}

bool clock_time::operator == (const clock_time &to_compare) const
{
  return (hour == to_compare.hour) 
      && (minute == to_compare.minute)
      && (second == to_compare.second) 
      && (millisecond == to_compare.millisecond) 
      && (microsecond == to_compare.microsecond);
}

astring clock_time::text_form(int how) const
{
  astring to_return;
  text_form(to_return, how);
  return to_return;
}

void clock_time::text_form(astring &to_return, int how) const
{
  if (!how) return;  // enforce use of the default.
  if (how & MILITARY)
    to_return += a_sprintf("%02ld:%02ld", hour, minute);
  else {
    time_number uhr = hour;
    if (uhr > 12) uhr -= 12;
    to_return += a_sprintf("%2ld:%02ld", uhr, minute);
  }
  if ( (how & SECONDS) || (how & MILLISECONDS) )
    to_return += a_sprintf(":%02ld", second);
  if (how & MILLISECONDS)
    to_return += a_sprintf(":%03ld", millisecond);
  if (how & MERIDIAN) {
    if (hour >= 12) to_return += "PM";
    else to_return += "AM";
  }
}

// makes sure that "val" is not larger than "max".  if it is, then max is
// used as a divisor and stored in "rolls".
#define limit_value(val, max) \
  if (val < 0) { \
    rolls = val / max; \
    rolls--; /* subtract an extra one since we definitely roll before -max */ \
    val += max * -rolls; \
  } else if (val >= max) { \
    rolls = val / max; \
    val -= max * rolls; \
  } else { rolls = 0; }

time_number clock_time::normalize(clock_time &to_fix)
{
  time_number rolls = 0;  // rollover counter.
  limit_value(to_fix.microsecond, 1000);
  to_fix.millisecond += rolls;
  limit_value(to_fix.millisecond, 1000);
  to_fix.second += rolls;
  limit_value(to_fix.second, 60);
  to_fix.minute += rolls;
  limit_value(to_fix.minute, 60);
  to_fix.hour += rolls;
  limit_value(to_fix.hour, 24);
  return rolls;
}

//////////////

void day_in_year::pack(byte_array &packed_form) const
{
  attach(packed_form, day_of_year);
  attach(packed_form, abyte(day_of_week));
  attach(packed_form, abyte(month));
  attach(packed_form, day_in_month);
  attach(packed_form, abyte(1));
    // still packing dst chunk; must for backward compatibility.
}

bool day_in_year::unpack(byte_array &packed_form)
{
  if (!detach(packed_form, day_of_year)) return false;
  abyte temp;
  if (!detach(packed_form, temp)) return false;
  day_of_week = days(temp);
  if (!detach(packed_form, temp)) return false;
  month = months(temp);
  if (!detach(packed_form, day_in_month)) return false;
  if (!detach(packed_form, temp)) return false;  // dst chunk--backward compat.
  return true;
}

bool day_in_year::operator < (const day_in_year &to_compare) const
{
  EASY_LT(month, to_compare.month);
  EASY_LT(day_in_month, to_compare.day_in_month);
  return false;
}

bool day_in_year::operator == (const day_in_year &to_compare) const
{
  return (month == to_compare.month)
      && (day_in_month == to_compare.day_in_month);
}

astring day_in_year::text_form(int how) const
{
  astring to_return;
  text_form(to_return, how);
  return to_return;
}

void day_in_year::text_form(astring &to_stuff, int how) const
{
  if (!how) return;  // enforce use of the default.
  if (how & INCLUDE_DAY) to_stuff += astring(day_name(day_of_week)) + " ";
  const char *monat = short_month_name(month);
  if (how & LONG_MONTH)
    monat = month_name(month);
//hmmm: more formatting, like euro?
  to_stuff += monat;
  to_stuff += a_sprintf(" %02ld", day_in_month);
}

// note: this only works when adjusting across one month, not multiples.
time_number limit_day_of_month(time_number &day, time_number days_in_month, time_number days_in_prev_month)
{
  if (day > days_in_month) {
    day -= days_in_month;
    return 1;  // forward rollover.
  } else if (day < 1) {
    day += days_in_prev_month;
    return -1;
  }
  return 0;  // no rolling.
}

time_number day_in_year::normalize(day_in_year &to_fix, bool leap_year)
{
  time_number rolls = 0;  // rollover counter.
  time_number daysinm = leap_year?
      leap_days_in_month[to_fix.month] : days_in_month[to_fix.month];
  time_number prev_month = to_fix.month - 1;
  if (prev_month < 0) prev_month = 11;
  time_number daysinpm = leap_year?
      leap_days_in_month[prev_month] : days_in_month[prev_month];
  rolls = limit_day_of_month(to_fix.day_in_month, daysinm, daysinpm);
  time_number monat = to_fix.month + rolls;
  limit_value(monat, 12);  // months are zero based.
  to_fix.month = months(monat);
  return rolls;
}

//////////////

void time_locus::pack(byte_array &packed_form) const
{
  attach(packed_form, year);
  clock_time::pack(packed_form);
  day_in_year::pack(packed_form);
}

bool time_locus::unpack(byte_array &packed_form)
{
  if (!detach(packed_form, year)) return false;
  if (!clock_time::unpack(packed_form)) return false;
  if (!day_in_year::unpack(packed_form)) return false;
  return true;
}

astring time_locus::text_form_long(int t, int d, int y) const
{
  astring to_return;
  text_form_long(to_return, t, d, y);
  return to_return;
}

bool time_locus::equal_to(const equalizable &s2) const {
  const time_locus *s2_cast = dynamic_cast<const time_locus *>(&s2);
  if (!s2_cast) throw "error: time_locus::==: unknown type";
  return (year == s2_cast->year)
      && ( (const day_in_year &) *this == *s2_cast)
      && ( (const clock_time &) *this == *s2_cast);
}

bool time_locus::less_than(const orderable &s2) const {
  const time_locus *s2_cast = dynamic_cast<const time_locus *>(&s2);
  if (!s2_cast) throw "error: time_locus::<: unknown type";
  EASY_LT(year, s2_cast->year);
  if (day_in_year::operator < (*s2_cast)) return true;
  if (!(day_in_year::operator == (*s2_cast))) return false;
  if (clock_time::operator < (*s2_cast)) return true;
  return false;
}

void time_locus::text_form_long(astring &to_stuff, int t, int d, int y) const
{
//hmmm: more formatting desired, like european.
  if (!y) {
    text_form_long(to_stuff, t, d);  // enforce use of the default.
    return;
  }
  // add the day.
  day_in_year::text_form(to_stuff, d);
  to_stuff += " ";
  // add the year.
  if (y & SHORT_YEAR)
    to_stuff += a_sprintf("%2ld", year % 100);
  else
    to_stuff += a_sprintf("%4ld", year);
  // add the time.
  to_stuff += " ";
  clock_time::text_form(to_stuff, t);
}

time_number time_locus::normalize(time_locus &to_fix)
{
  time_number rolls = clock_time::normalize(to_fix);
  to_fix.day_in_month += rolls;

//hmmm: this little gem should be abstracted to a method.
  bool leaping = !(to_fix.year % 4);
  if (!(to_fix.year % 100)) leaping = false;
  if (!(to_fix.year % 400)) leaping = true;

  rolls = day_in_year::normalize(to_fix, leaping);
  to_fix.year += rolls;
  return 0;
    // is that always right?  not for underflow.
//hmmm: resolve the issue of rollovers here.
}

//////////////

time_locus convert(time_number seconds, time_number useconds,
    const tm &cal_values)
{
  FUNCDEF("convert");
  time_locus r;

  r.millisecond = useconds / 1000;
  r.microsecond = useconds % 1000;

  r.hour = cal_values.tm_hour;
  r.minute = cal_values.tm_min;
  r.second = cal_values.tm_sec;
  r.day_in_month = cal_values.tm_mday;
  r.month = months(cal_values.tm_mon);
  r.year = cal_values.tm_year + 1900;
  r.day_of_week = days(cal_values.tm_wday);
  r.day_of_year = cal_values.tm_yday;

  LOG(a_sprintf("convert() returning: %s\n",
      r.text_form_long(clock_time::MILITARY,
      day_in_year::LONG_MONTH | day_in_year::INCLUDE_DAY,
      time_locus::LONG_YEAR).s()));

  return r;
}

time_locus now()
{
  FUNCDEF("now")
  timeval currtime;
  int okay = gettimeofday(&currtime, NULL_POINTER);
  if (okay != 0) {
    LOG("failed to gettimeofday!?");
  }
  time_t currtime_secs = currtime.tv_sec;
  struct tm result;
  tm *tz_ptr = localtime_r(&currtime_secs, &result);
  if (tz_ptr != &result) {
    LOG("failed to get time for local area with localtime_r");
  }
  return convert(currtime.tv_sec, currtime.tv_usec, result);
}

time_locus greenwich_now()
{
  FUNCDEF("greenwich_now")
  timeval currtime;
  int okay = gettimeofday(&currtime, NULL_POINTER);
  if (okay != 0) {
    LOG("failed to gettimeofday!?");
  }
  time_t currtime_secs = currtime.tv_sec;
  tm result;
  tm *tz_ptr = gmtime_r(&currtime_secs, &result);
  if (tz_ptr != &result) {
    LOG("failed to get time for local area with gmtime_r");
  }
  return convert(currtime.tv_sec, currtime.tv_usec, result);
}

clock_time time_now() { return now(); }

days day_now() { return now().day_of_week; }

months month_now() { return now().month; }

time_number year_now() { return now().year; }

day_in_year date_now() { return now(); }

const char *day_name(days to_name)
{
  switch (to_name) {
    case SUNDAY: return "Sunday";
    case MONDAY: return "Monday";
    case TUESDAY: return "Tuesday";
    case WEDNESDAY: return "Wednesday";
    case THURSDAY: return "Thursday";
    case FRIDAY: return "Friday";
    case SATURDAY: return "Saturday";
    default: return "Not_a_day";
  }
}

const char *month_name(months to_name)
{
  switch (to_name) {
    case JANUARY: return "January";
    case FEBRUARY: return "February";
    case MARCH: return "March";
    case APRIL: return "April";
    case MAY: return "May";
    case JUNE: return "June";
    case JULY: return "July";
    case AUGUST: return "August";
    case SEPTEMBER: return "September";
    case OCTOBER: return "October";
    case NOVEMBER: return "November";
    case DECEMBER: return "December";
    default: return "Not_a_month";
  }
}

const char *short_month_name(months to_name)
{
  switch (to_name) {
    case JANUARY: return "Jan";
    case FEBRUARY: return "Feb";
    case MARCH: return "Mar";
    case APRIL: return "Apr";
    case MAY: return "May";
    case JUNE: return "Jun";
    case JULY: return "Jul";
    case AUGUST: return "Aug";
    case SEPTEMBER: return "Sep";
    case OCTOBER: return "Oct";
    case NOVEMBER: return "Nov";
    case DECEMBER: return "Dec";
    default: return "Not";
  }
}

} // namespace.

