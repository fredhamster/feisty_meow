#ifndef PROGRAM_WIDE_LOGGER_FACILITY
#define PROGRAM_WIDE_LOGGER_FACILITY

/*****************************************************************************\
*                                                                             *
*  Name   : program_wide_logger facility                                      *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "logging_macros.h"
  // it seems reasonable to provide the logging macros to any file that is also using
  // the program-wide logger.

#include <basis/contracts.h>
#include <basis/definitions.h>
#include <loggers/standard_log_base.h>

namespace loggers {

//! A class that provides logging facilities vertically to all of hoople.
/*!
  Provides a per-program logging subsystem that can be used from almost
  anywhere in the source code.
  The program wide logger feature is a globally defined object that
  can be switched out to perform different types of logging.
  Note: this facility is not thread-safe.  The coder must guarantee
  that the appropriate logger is set up before cranking up a bunch of
  threads that use logging.
*/

class program_wide_logger
{
public:
  static loggers::standard_log_base &get();
    //!< Provided by the startup code within each application for logging.
    /*!< This can be used like any base_logger object, but the diagnostic items
    will be stored into one log file (or other logging mechanism) per
    program. */

  static loggers::standard_log_base *set(loggers::standard_log_base *new_log);
    //!< replaces the current program-wide logger with the "new_log".
    /*! The "new_log" must be a valid base_logger object.  The responsibility
    for maintaining "new_log" is taken over by the program-wide logger.
    In return, the old program-wide logger is handed back to you and it is
    now your responsibility.  Be very careful with it if it's owned by other
    code and not totally managed by you. */

private:
  static loggers::standard_log_base *c_the_wide_log;  //!< the actual logger.
};

//////////////

//! a trash can for logging; throws away all entries.
/*!
  Provides a logger that throws away all of the strings that are logged to it.
  This is useful when an interface requires a base_logger but one does not
  wish to generate an output file.  This object is similar to /dev/null in Unix
  (or Linux) and to nul: in Win32.
*/

class null_logger : public virtual standard_log_base
{
public:
  virtual ~null_logger() {}
  DEFINE_CLASS_NAME("null_logger");
  virtual basis::outcome log(const basis::base_string &formal(info), int formal(filter)) {
    /* if (filter || !(&info)) {} */
    return basis::common::OKAY;
  }
};

} //namespace.

#endif // outer guard.

