/*****************************************************************************\
*                                                                             *
*  Name   : zap_process                                                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Whacks a process named on the command line, if possible.                 *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/command_line.h>
#include <application/windoze_helper.h>
#include <basis/astring.h>
#include <basis/guards.h>
#include <filesystem/filename.h>
#include <loggers/console_logger.h>
#include <loggers/critical_events.h>
#include <processes/process_control.h>
#include <processes/process_entry.h>
#include <structures/static_memory_gremlin.h>

//HOOPLE_STARTUP_CODE;
//hmmm: missing

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace processes;
using namespace structures;

int main(int argc, char *argv[])
{
  console_logger out;
  command_line cmds(argc, argv);
  if ( (cmds.entries() < 1)
      || (cmds.get(0).type() != command_parameter::VALUE) ) {
    out.log(cmds.program_name().basename().raw() + " usage:\n"
        "this takes a single parameter, which is the name of a program\n"
        "to hunt down and eradicate.  this will zap the program without\n"
        "any warning or any chance for it to save its state.", ALWAYS_PRINT);
    return 1;
  }
  astring program_name = cmds.get(0).text();
  program_name.to_lower();
  process_entry_array processes;
  process_control proc_con;
  if (!proc_con.query_processes(processes))
    non_continuable_error("procto", "main", "failed to query processes!");
  bool found = false;
  bool success = true;
  for (int i = 0; i < processes.length(); i++) {
    filename path = processes[i].path();
//    out.log(astring("got process path: ") + path.raw(), ALWAYS_PRINT);
    astring base = path.basename().raw().lower();
    if (base == program_name) {
      found = true;
//      out.log("would whack this entry:");
//      out.log(processes[i].text_form());
      bool ret = proc_con.zap_process(processes[i]._process_id);
      if (ret)
        out.log(a_sprintf("Killed process %d [", processes[i]._process_id)
            + program_name + astring("]"), ALWAYS_PRINT);
      else {
        out.log(astring(astring::SPRINTF, "Failed to zap process %d [",
            processes[i]._process_id) + program_name + astring("]"), ALWAYS_PRINT);
        success = false;
      }
    }
  }
  if (!found)
    out.log(astring("Could not find the program named ") + program_name, ALWAYS_PRINT);
  if (!success) return 123;
  else return 0;
}

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
  #include <application/application_shell.cpp>
  #include <application/command_line.cpp>
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
  #include <loggers/combo_logger.cpp>
  #include <loggers/console_logger.cpp>
  #include <loggers/critical_events.cpp>
  #include <loggers/file_logger.cpp>
  #include <loggers/program_wide_logger.cpp>
  #include <processes/process_control.cpp>
  #include <processes/process_entry.cpp>
  #include <structures/bit_vector.cpp>
  #include <structures/checksums.cpp>
  #include <structures/object_packers.cpp>
  #include <structures/static_memory_gremlin.cpp>
  #include <structures/string_hasher.cpp>
  #include <structures/string_table.cpp>
  #include <structures/version_record.cpp>
  #include <textual/byte_formatter.cpp>
  #include <textual/parser_bits.cpp>
  #include <textual/string_manipulation.cpp>
  #include <timely/earth_time.cpp>
  #include <timely/time_stamp.cpp>
#endif // __BUILD_STATIC_APPLICATION__

