#ifndef COMBO_LOGGER_CLASS
#define COMBO_LOGGER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : combo_logger
*  Author : Chris Koeritz
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "file_logger.h"

namespace loggers {

//! combines a file_logger with a console logger, behaving like the 'tee' command.
/*! this will output the diagnostic info to both a file and to the console.  this is useful
when one would like to see the output as its happening but also have a record for later.  */

class combo_logger : public virtual file_logger, public virtual console_logger
{
public:
  combo_logger(const basis::astring &filename,
      int limit = DEFAULT_LOG_FILE_SIZE,
      stream_choices log_target = TO_STDOUT);

  virtual ~combo_logger() {}

  DEFINE_CLASS_NAME("combo_logger");

  virtual basis::outcome log(const basis::base_string &info, int filter = basis::ALWAYS_PRINT);

  // overrides that enforce properties for both loggers.
  virtual void add_filter(int new_filter);
  virtual void remove_filter(int old_filter);
  virtual void clear_filters();
  virtual void eol(textual::parser_bits::line_ending to_set);
};

//////////////

//! a macro that retasks the program-wide logger as a combo_logger.
#define SETUP_COMBO_LOGGER { \
  basis::base_logger *old_log = program_wide_logger::set \
      (new loggers::combo_logger \
          (loggers::file_logger::log_file_for_app_name())); \
  WHACK(old_log); \
}

} //namespace.

#endif

