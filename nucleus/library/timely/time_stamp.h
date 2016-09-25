#ifndef TIME_STAMP_CLASS
#define TIME_STAMP_CLASS

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

#include <basis/astring.h>
#include <basis/contracts.h>
#include <basis/definitions.h>

// forward.
class rollover_record;
struct timeval;

namespace timely {

//! Represents a point in time relative to the operating system startup time.
/*!
  This duration is measured in milliseconds.  This class provides a handy way
  of measuring relative durations at the millisecond time scale.
  Unfortunately, operating systems that reckon their millisecond uptime in
  32 bit integers will suffer from a rollover problem every 49 days or so,
  but this class corrects this issue.
*/

class time_stamp : public virtual basis::orderable
{
public:
  DEFINE_CLASS_NAME("time_stamp");

  typedef double time_representation;
    //!< the representation of time for this universe, measured in milliseconds.

  time_stamp();
    //!< creates a time_stamp containing the current time.

  time_stamp(time_representation offset);
    //!< creates a stamp after the current time by "offset" milliseconds.
    /*!< negative offsets are allowed, in which case the stamp will be
    prior to the current time. */

  static double rolling_uptime();
    //!< give the OS uptime in a more durable form that handles rollovers.

  void reset();
    //!< sets the stamp time back to now.
  void reset(time_representation offset);
    //!< sets the stamp forward by "offset" similar to the second constructor.

  time_representation value() const { return c_stamp; }
    //!< returns the time_stamp in terms of the lower level type.

  enum stamp_display_style { STAMP_RELATIVE, STAMP_ABSOLUTE };

  basis::astring text_form(stamp_display_style style = STAMP_RELATIVE) const;
    //!< returns a simple textual representation of the time_stamp.
    /*!< if the "style" is ABSOLUTE, then the stamp is shown in H:M:S.ms
    form based on the system uptime.  if the "style" is RELATIVE, then the
    stamp is shown as an offset from now. */

  static basis::astring notarize(bool add_space = true);
    //!< a useful method for getting a textual version of the time "right now".
    /*!< this was formerly known as a very useful method called 'utility::timestamp'.
    that naming was fairly abusive to keep straight from the time_stamp class, so the
    functionality has been absorbed. */

  // standard operators: keep in mind that time is represented by an ever
  // increasing number.  so, if a value A is less than a value B, that means
  // that A is older than B, since it occurred at an earlier time.
  virtual bool less_than(const basis::orderable &that) const {
    const time_stamp *cast = dynamic_cast<const time_stamp *>(&that);
    if (!cast) return false;
    return c_stamp < cast->c_stamp;
  }

  virtual bool equal_to(const basis::equalizable &that) const {
    const time_stamp *cast = dynamic_cast<const time_stamp *>(&that);
    if (!cast) return false;
    return c_stamp == cast->c_stamp;
  }

  // helper functions for using the OS's time support.

  static void fill_timeval_ms(timeval &time_point, int milliseconds);
    //!< returns a timeval system object that represents the "milliseconds".
    /*!< if "milliseconds" is zero, then the returned timeval will
    represent zero time passing (rather than infinite duration as some
    functions assume). */

private:
  time_representation c_stamp;  //!< the low-level time stamp is held here.

  void fill_in_time();  //!< stuffs the current time into the held value.

  static time_representation get_time_now();
    //!< platform specific function that gets uptime.

  static rollover_record &rollover_rover();
};

} //namespace.

#endif

