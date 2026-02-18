/*****************************************************************************\
*                                                                             *
*  Name   : zing_window                                                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Sends a message to a window.  The window can either be located by its    *
*  unique window handle (if known) or by its exact title.  There is currently *
*  no wildcard matching supported to find other types of titles.  If the      *
*  window is located, then it is sent a windows message of your choice.  You  *
*  can additionally add the wparam and lparam integers that get sent with     *
*  the message.  This is useful for shutting windows down gracefully by       *
*  sending them the WM_CLOSE message.                                         *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/convert_utf.h>
#include <basis/guards.h>
#include <basis/istring.h>
#include <basis/portable.h>
#include <opsystem/command_line.h>
#include <loggers/console_logger.h>
#include <opsystem/filename.h>
#include <data_struct/static_memory_gremlin.h>

#include <stdio.h>

HOOPLE_STARTUP_CODE;

window_handle matching_window = NIL;
istring window_name_sought;

BOOL CALLBACK zingers_enum_proc(window_handle hwnd, LPARAM lParam)
{
  const int MAX_TITLE = 1024;
  flexichar win_name[MAX_TITLE + 8];  // add a little for good luck.
  int chars = GetWindowText(hwnd, win_name, MAX_TITLE - 1);
  if (chars > 0) {
    // see if the current window matches what we're looking for.
    if (istring(from_unicode_temp(win_name)).ifind(window_name_sought) >= 0) {
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
  if ( (cmds.entries() < 2)
      || (cmds.get(0).type() != command_parameter::VALUE) ) {
    out.log(cmds.program_name().basename().raw() + " usage:\n"
        "This takes at least two parameters.  The first parameter is a window\n"
        "handle that is to be contacted.  Alternatively, the first parameter\n"
        "can be the title of the window.  The second parameter is a numerical\n"
        "event that the window is to be zinged with.  Zinging is synonymous\n"
        "here with \"sending a message to the window\".  Depending on the\n"
        "message chosen, the window will behave in various different ways.\n"
        "If there are a third or fourth parameter, then these are taken as\n"
        "the extra data to send in the PostMessage call for zinging.");
    out.log(isprintf("\nExample windows message values:\n\t"
        "WM_CLOSE = %d\n\tWM_PAINT = %d", WM_CLOSE, WM_PAINT));
    return 1;
  }

  istring junk_text = cmds.get(0).text();
  bool saw_hex_code = false;
  if (junk_text.begins("0x")) {
    // we have a hex code.  we will key off of that, which assumes that there
    // is never going to be an important window whose title starts with that.
    saw_hex_code = true;
  }
  int temp_handle = 0;
  if (saw_hex_code) {
    sscanf(junk_text.s(), "%lx", &temp_handle);
  } else {
    sscanf(junk_text.s(), "%ld", &temp_handle);
  }

//hmmm: this code is not so good for 64 bit.
  #pragma warning(disable : 4312)
  window_handle handle = (window_handle)temp_handle;

  // we'll set the flag below if we find anything besides numbers in the
  // window handle string.
  bool has_alpha = false;
  if (!saw_hex_code) {
    // we only check for alphabetical characters if we didn't already
    // decide the parameter had a hex code in front making it a number.
    for (int i = 0; i < junk_text.length(); i++) {
      if ( (junk_text[i] < '0') || (junk_text[i] > '9') ) {
        has_alpha = true;
        break;
      }
    }
  }
  if (has_alpha) {
    // reset our match first.
    matching_window = NIL;
    window_name_sought = junk_text;
//out.log("saw non-numbers in handle, trying as name.");
    // enumerate the windows looking for a match.
    EnumWindows(zingers_enum_proc, 0);
    if (!matching_window) {
      out.log("no matching window could be found.  ignoring request.");
      return 1;
    }
//out.log("found matching window handle...");
    handle = matching_window;
  }

  junk_text = cmds.get(1).text();
  ULONG event = (ULONG)junk_text.convert(0);
  ULONG p1 = 0;
  ULONG p2 = 0;
  if (cmds.entries() >= 3) {
    junk_text = cmds.get(2).text();
    p1 = (ULONG)junk_text.convert(0);
  }
  if (cmds.entries() >= 4) {
    junk_text = cmds.get(3).text();
    p2 = (ULONG)junk_text.convert(0);
  }

  if (!handle || !event) {
    out.log("the window handle or the event was invalid.  ignoring request.");
    return 1;
  }

  PostMessage(handle, event, p1, p2);

  out.log(isprintf("posted at 0x%lx the zing (%u %u %u)", handle, event,
      p1, p2));

  return 0;
}

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
  #include <basis/byte_array.cpp>
  #include <basis/callstack_tracker.cpp>
  #include <basis/convert_utf.cpp>
  #include <basis/definitions.cpp>
  #include <basis/earth_time.cpp>
  #include <basis/guards.cpp>
  #include <basis/istring.cpp>
  #include <basis/log_base.cpp>
  #include <basis/memory_checker.cpp>
  #include <basis/mutex.cpp>
  #include <basis/object_base.h>
  #include <basis/outcome.cpp>
  #include <basis/packable.cpp>
  #include <basis/portable.cpp>
  #include <basis/sequence.h>
  #include <basis/set.h>
  #include <basis/trap_new.addin>
  #include <basis/untrap_new.addin>
  #include <basis/utility.cpp>
  #include <basis/version_record.cpp>
  #include <data_struct/amorph.h>
  #include <data_struct/bit_vector.cpp>
  #include <data_struct/byte_hasher.cpp>
  #include <data_struct/configurator.cpp>
  #include <data_struct/hash_table.h>
  #include <data_struct/pointer_hash.h>
  #include <data_struct/stack.h>
  #include <data_struct/static_memory_gremlin.cpp>
  #include <data_struct/string_hash.h>
  #include <data_struct/string_hasher.cpp>
  #include <data_struct/string_table.cpp>
  #include <data_struct/symbol_table.h>
  #include <data_struct/table_configurator.cpp>
  #include <loggers/console_logger.cpp>
  #include <loggers/file_logger.cpp>
  #include <loggers/locked_logger.cpp>
  #include <loggers/null_logger.cpp>
  #include <loggers/program_wide_logger.cpp>
  #include <opsystem/byte_filer.cpp>
  #include <opsystem/command_line.cpp>
  #include <opsystem/critical_events.cpp>
  #include <opsystem/directory.cpp>
  #include <opsystem/filename.cpp>
  #include <opsystem/ini_config.cpp>
  #include <opsystem/ini_parser.cpp>
  #include <opsystem/path_configuration.cpp>
  #include <opsystem/rendezvous.cpp>
  #include <textual/byte_format.cpp>
  #include <textual/parser_bits.cpp>
  #include <textual/string_manipulation.cpp>
  #include <textual/tokenizer.cpp>
#endif // __BUILD_STATIC_APPLICATION__

