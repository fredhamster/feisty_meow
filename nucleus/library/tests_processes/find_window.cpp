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

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
  #include <basis/array.h>
  #include <basis/byte_array.cpp>
  #include <basis/callstack_tracker.cpp>
  #include <basis/utf_conversion.cpp>
  #include <basis/definitions.cpp>
  #include <basis/earth_time.cpp>
  #include <basis/guards.cpp>
  #include <basis/astring.cpp>
  #include <basis/log_base.cpp>
  #include <basis/memory_checker.cpp>
  #include <basis/mutex.cpp>
  #include <basis/contracts.h>
  #include <basis/outcome.cpp>
  #include <basis/packable.cpp>
  #include <basis/portable.cpp>
  #include <basis/sequence.h>
  #include <structures/set.h>
  #include <basis/trap_new.addin>
  #include <basis/untrap_new.addin>
  #include <basis/utility.cpp>
  #include <basis/version_record.cpp>
  #include <structures/amorph.h>
  #include <structures/bit_vector.cpp>
  #include <structures/byte_hasher.cpp>
  #include <structures/configurator.cpp>
  #include <structures/hash_table.h>
  #include <structures/pointer_hash.h>
  #include <structures/stack.h>
  #include <structures/static_memory_gremlin.cpp>
  #include <structures/string_hash.h>
  #include <structures/string_hasher.cpp>
  #include <structures/string_table.cpp>
  #include <structures/symbol_table.h>
  #include <structures/table_configurator.cpp>
  #include <loggers/console_logger.cpp>
  #include <loggers/file_logger.cpp>
  #include <loggers/locked_logger.cpp>
  #include <loggers/null_logger.cpp>
  #include <loggers/program_wide_logger.cpp>
  #include <filesystem/byte_filer.cpp>
  #include <application/command_line.cpp>
  #include <opsystem/critical_events.cpp>
  #include <filesystem/directory.cpp>
  #include <filesystem/filename.cpp>
  #include <configuration/ini_configurator.cpp>
  #include <opsystem/ini_parser.cpp>
  #include <configuration/application_configuration.cpp>
  #include <processes/rendezvous.cpp>
  #include <textual/byte_formatter.cpp>
  #include <textual/parser_bits.cpp>
  #include <textual/string_manipulation.cpp>
  #include <configuration/variable_tokenizer.cpp>
#endif // __BUILD_STATIC_APPLICATION__

