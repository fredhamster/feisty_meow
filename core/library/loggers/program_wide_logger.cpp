/*****************************************************************************\
*                                                                             *
*  Name   : program_wide_logger                                               *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1994-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "program_wide_logger.h"

using namespace basis;
using namespace loggers;

namespace loggers {

standard_log_base *program_wide_logger::c_the_wide_log = new null_logger;

standard_log_base &program_wide_logger::get() { return *c_the_wide_log; }

standard_log_base *program_wide_logger::set(standard_log_base *new_log)
{
  if (!new_log) return NIL;  // can't fool me that easily.
  standard_log_base *old_log = c_the_wide_log;
  c_the_wide_log = new_log;
  return old_log;
}

} //namespace.

