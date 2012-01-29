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

#include "combo_logger.h"

#include <filesystem/directory.h>
#include <filesystem/filename.h>
#include <mathematics/chaos.h>
#include <structures/static_memory_gremlin.h>
#include <textual/parser_bits.h>

#ifdef __WIN32__
  #include <io.h>
#endif
#include <stdio.h>
#ifdef __UNIX__
  #include <unistd.h>
#endif

using namespace basis;
using namespace filesystem;
using namespace structures;
using namespace textual;

namespace loggers {

//const int REDUCE_FACTOR = 5;
  // we whack this portion of the file every time we truncate.  if it's set
  // to 14, for example, then a 14th of the file is whacked every time whacking
  // is needed.

//const int MAXIMUM_BUFFER_SIZE = 140000;
  // the maximum allowed chunk that can be copied from the old logfile
  // to the current one.

//int static_chaos() { return chaos().inclusive(0, 1280004); }

combo_logger::combo_logger(const astring &filename, int limit, stream_choices target)
: file_logger(filename, limit),
  console_logger(target)
{
}

void combo_logger::add_filter(int new_filter)
{
  file_logger::add_filter(new_filter);
  console_logger::add_filter(new_filter);
}

void combo_logger::remove_filter(int old_filter)
{
  file_logger::remove_filter(old_filter);
  console_logger::remove_filter(old_filter);
}

void combo_logger::clear_filters()
{
  file_logger::clear_filters();
  console_logger::clear_filters();
}

void combo_logger::eol(textual::parser_bits::line_ending to_set)
{
  file_logger::eol(to_set);
  console_logger::eol(to_set);
}

outcome combo_logger::log(const base_string &info, int filter)
{
  console_logger::log(info, filter);
  return file_logger::log(info, filter);
}

} //namespace.

