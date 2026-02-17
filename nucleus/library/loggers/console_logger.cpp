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

#include "console_logger.h"
#include "logging_filters.h"

#include <textual/parser_bits.h>

#include <stdio.h>

using namespace basis;
using namespace textual;

namespace loggers {

console_logger::console_logger(stream_choices target) : c_target(target)
{}

console_logger::~console_logger() {}

outcome console_logger::log(const base_string &info, int filter)
{

if (filter) {} //temp ignored

  FILE *log_to = stdout;
  if (c_target == TO_STDERR) log_to = stderr;
  if (member(filter)) {
    // format the output with %s to ensure we get all characters, rather
    // than having some get interpreted if we used info as the format spec.
    fprintf(log_to, "%s", (char *)info.observe());
    // send the EOL char if the style is appropriate for that.
    if (eol() != parser_bits::NO_ENDING) fprintf(log_to, "%s", get_ending().s());
    // write immediately to avoid lost output on crash.
    fflush(log_to);
  }
  return common::OKAY;
}

} //namespace.


