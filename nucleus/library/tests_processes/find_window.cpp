/*****************************************************************************\
*                                                                             *
*  Name   : find_window                                                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Locates a window by the title.  If it's found, the window handle is      *
*  displayed.                                                                 *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/windoze_helper.h>
#include <basis/astring.h>
#include <basis/utf_conversion.h>
#include <basis/guards.h>
#include <filesystem/filename.h>
#include <loggers/console_logger.h>
#include <structures/static_memory_gremlin.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace structures;
//using namespace timely;

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

#ifdef __WIN32__

window_handle matching_window = NULL_POINTER;
astring window_name_sought;

BOOL CALLBACK zingers_enum_proc(window_handle hwnd, LPARAM lParam)
{
  const int MAX_TITLE = 1024;
  flexichar win_name[MAX_TITLE + 8];  // add a little for good luck.
  int chars = GetWindowText(hwnd, win_name, MAX_TITLE - 1);
  if (chars > 0) {
    // see if the current window matches what we're looking for.
    if (astring(from_unicode_temp(win_name)).ifind(window_name_sought) >= 0) {
      matching_window = hwnd;
      return false;  // don't keep going.
    }
  }
  return true;  // haven't found it yet.
}

int main(int argc, char *argv[])
{
  console_logger out;
  command_line cmds(argc, argv);
  if ( (cmds.entries() < 1)
      || (cmds.get(0).type() != command_parameter::VALUE) ) {
    out.log(cmds.program_name().basename().raw() + " usage:\n"
        "this takes a single parameter, which is the name of a window\n"
        "that is expected to be present in the current winstation. if the\n"
        "window is found, its window handle is displayed.");
    return 1;
  }
  astring title = cmds.get(0).text();
  matching_window = NULL_POINTER;  // reset our match first.
  window_name_sought = title;
  // enumerate the windows looking for a match.
  EnumWindows(zingers_enum_proc, 0);
  if (!matching_window) {
    out.log("no matching window could be found.  ignoring request.");
    return 1;
  }
//out.log("found matching window handle...");

  if (matching_window)
    out.log(a_sprintf("window handle is 0x%lx (or %ld) for ", matching_window,
        matching_window) + title + ".");
  else
    out.log(astring("no window found for ") + title + ".");
  return 0;
}

#else
// non-windows implementation is a no-op.
int main(int argc, char *argv[])
{
  return 0;
}
#endif

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
  #include <algorithms/sorts.cpp>
  #include <application/callstack_tracker.cpp>
  #include <application/windoze_helper.cpp>
  #include <basis/astring.cpp>
  #include <basis/common_outcomes.cpp>
  #include <basis/environment.cpp>
  #include <basis/guards.cpp>
  #include <basis/mutex.cpp>
  #include <basis/utf_conversion.cpp>
  #include <configuration/application_configuration.cpp>
  #include <configuration/configurator.cpp>
  #include <configuration/ini_configurator.cpp>
  #include <configuration/ini_parser.cpp>
  #include <configuration/table_configurator.cpp>
  #include <configuration/variable_tokenizer.cpp>
  #include <filesystem/byte_filer.cpp>
  #include <filesystem/directory.cpp>
  #include <filesystem/filename.cpp>
  #include <loggers/console_logger.cpp>
  #include <loggers/program_wide_logger.cpp>
  #include <structures/checksums.cpp>
  #include <structures/object_packers.cpp>
  #include <structures/static_memory_gremlin.cpp>
  #include <structures/string_hasher.cpp>
  #include <structures/string_table.cpp>
  #include <structures/version_record.cpp>
  #include <textual/parser_bits.cpp>
  #include <timely/earth_time.cpp>
  #include <timely/time_stamp.cpp>
#endif // __BUILD_STATIC_APPLICATION__

