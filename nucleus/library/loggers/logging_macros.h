#ifndef LOGGING_MACROS_GROUP
#define LOGGING_MACROS_GROUP

/*****************************************************************************\
*                                                                             *
*  Name   : logging macros                                                    *
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

/*! @file logging_macros.h
  these macros can assist in logging.  they rely on the base_logger class and the program-wide
  logger class for logging services.  note that it is often convenient to customize one or more
  of these to yield a simpler macro name per project, such as PRINT or LOG.
*/

#include <basis/enhance_cpp.h>
#include <loggers/logging_filters.h>
#include <timely/time_stamp.h>

///hmmm: very temporary until the call stack tracking is available again.
#define update_current_stack_frame_line_number(x)

//! Logs a string "to_log" on "the_logger" using the "filter".
/*! The filter is checked before the string is allowed to come into
existence, which saves allocations when the item would never be printed
out anyway. */
#define FILTER_LOG(the_logger, to_log, filter) { \
  if (the_logger.member(filter)) { \
    the_logger.log(to_log, filter); \
  } \
}

//! Logs a string at the emergency level, meaning that it always gets logged.
#define EMERGENCY_LOG(the_logger, to_log) \
  FILTER_LOG(the_logger, to_log, basis::ALWAYS_PRINT)

//! Corresponding functions for including the time and date in the log entry.
#define STAMPED_FILTER_LOG(the_logger, to_log, filter) { \
  if (the_logger.member(filter)) { \
    astring temp_log = to_log; \
    if (temp_log.length()) \
      temp_log.insert(0, timely::time_stamp::notarize(true)); \
    the_logger.log(temp_log, filter); \
  } \
}
//! Time-stamped logging that will always be printed.
#define STAMPED_EMERGENCY_LOG(the_logger, to_log) \
  STAMPED_FILTER_LOG(the_logger, to_log, basis::ALWAYS_PRINT)

//! Class specific logging method that uses a filter.
/* These add a class and function name to the log entry. */
#define CLASS_FILTER_LOG(the_logger, to_log, filter) { \
  update_current_stack_frame_line_number(__LINE__); \
  if (the_logger.member(filter)) { \
    astring temp_log = to_log; \
    if (temp_log.length()) { \
      temp_log.insert(0, timely::time_stamp::notarize(true)); \
      BASE_FUNCTION(func); \
      temp_log += " ["; \
      temp_log += function_name; \
      temp_log += "]"; \
    } \
    the_logger.log(temp_log, filter); \
  } \
  update_current_stack_frame_line_number(__LINE__); \
}
//! Class specific logging method that always prints.
#define CLASS_EMERGENCY_LOG(the_logger, to_log) \
  CLASS_FILTER_LOG(the_logger, to_log, basis::ALWAYS_PRINT)

//! Logs information that includes specific class instance information.
/*! This use the instance name of the object, which can include more
information than the simple class name. */
#define INSTANCE_FILTER_LOG(the_logger, to_log, filter) { \
  update_current_stack_frame_line_number(__LINE__); \
  if (the_logger.member(filter)) { \
    astring temp_log = to_log; \
    if (temp_log.length()) { \
      temp_log.insert(0, timely::time_stamp::notarize(true)); \
      BASE_INSTANCE_FUNCTION(func); \
      temp_log += " ["; \
      temp_log += function_name; \
      temp_log += "]"; \
    } \
    the_logger.log(temp_log, filter); \
    update_current_stack_frame_line_number(__LINE__); \
  } \
}
//! Logs with class instance info, but this always prints.
#define INSTANCE_EMERGENCY_LOG(the_logger, to_log) \
  INSTANCE_FILTER_LOG(the_logger, to_log, basis::ALWAYS_PRINT)

#endif

