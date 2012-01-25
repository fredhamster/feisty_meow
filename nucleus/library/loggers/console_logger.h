#ifndef CONSOLE_LOGGER_CLASS
#define CONSOLE_LOGGER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : console_logger                                                    *
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

//! A logger that sends to the console screen using the standard output device.
/*!
  The object can optionally be used to log to the standard error device
  instead.
*/

#include "standard_log_base.h"

#include <basis/astring.h>
#include <basis/contracts.h>
#include <basis/enhance_cpp.h>

namespace loggers {

class console_logger : public virtual standard_log_base
{
public:
  enum stream_choices { TO_STDOUT, TO_STDERR };  //!< where to send the logging.

  console_logger(stream_choices log_target = TO_STDOUT);
    //!< if "standard_error" is true, than stderr is used instead of stdout.

  virtual ~console_logger();

  DEFINE_CLASS_NAME("console_logger");

  bool to_standard_output() const { return c_target == TO_STDOUT; }
    //!< reports if the logger goes to standard output (default).

  bool to_standard_error() const { return c_target == TO_STDERR; }
    //!< reports if the logger goes to standard error instead.

  void set_output(stream_choices target) { c_target = target; }
    //!< enables the target of logging to be changed after construction.

  virtual basis::outcome log(const basis::base_string &info, int filter);
    //!< sends the string "info" to the standard output device.
    /*!< if the "filter" is not in the current filter set, then the
    information will be dropped.  otherwise the information is displayed,
    where appropriate.  for some environments, this will essentially throw
    it away.  for example, in ms-windows, a windowed program will only save
    standard out if one redirects standard output to a file, whereas a
    console mode program will output the text to its parent prompt. */

private:
  stream_choices c_target;   //!< records whether stdout or stderr is used.
};

//////////////

//!< a macro that retasks the program-wide logger as a console_logger.
#define SETUP_CONSOLE_LOGGER { \
  loggers::standard_log_base *old_log = loggers::program_wide_logger::set \
      (new loggers::console_logger); \
  /* assumes we are good to entirely remove the old logger. */ \
  WHACK(old_log); \
}

} // namespace.

#endif

