#ifndef TIME_CONTROL_CLASS
#define TIME_CONTROL_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : time_control
*  Author : Chris Koeritz
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

///#include <basis/astring.h>
///#include <basis/contracts.h>
///#include <basis/definitions.h>

namespace timely {

//! Provides some functions that affect time, or ones perception of time.

class time_control : public virtual basis::nameable
{
public:
  ~time_control() {}

  static void sleep_ms(basis::un_int msec);
    //!< a system independent name for a forced snooze measured in milliseconds.
    /*!< the application will do nothing on the current thread for the amount of
    time specified.  on some systems, if "msec" is zero, then the sleep will
    yield the current thread's remaining timeslice back to other threads. */

  bool set_time(const time_locus &new_time);
    //!< makes the current time equal to "new_time".
    /*!< This will only work if the operation is supported on this OS and if
    the current user has the proper privileges. */
};

} //namespace.

#endif

