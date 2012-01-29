/*
*  Name   : await_app_exit
*  Author : Chris Koeritz
*  Purpose:                                                                   *
*    This program waits for a particular application to exit before this app  *
*  itself exits.  This allows a pause while another possibly slow process is  *
*  leaving.                                                                   *
* Copyright (c) 2003-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/functions.h>
#include <basis/astring.h>
#include <structures/set.h>
#include <timely/time_control.h>
#include <timely/time_stamp.h>
#include <application/hoople_main.h>
#include <filesystem/byte_filer.h>
#include <loggers/console_logger.h>
#include <loggers/file_logger.h>
#include <filesystem/filename.h>
#include <configuration/ini_configurator.h>
#include <configuration/application_configuration.h>
#include <structures/static_memory_gremlin.h>
#include <processes/process_control.h>
#include <processes/process_entry.h>

using namespace application;
using namespace basis;
using namespace loggers;
using namespace filesystem;
using namespace processes;
using namespace structures;
using namespace textual;
using namespace timely;

#undef BASE_LOG
#define BASE_LOG(to_print) EMERGENCY_LOG(program_wide_logger::get(), astring(to_print))
#undef LOG
#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger::get(), astring(to_print))

class await_app_exit : public application_shell
{
public:
  await_app_exit() : application_shell() {}
  DEFINE_CLASS_NAME("await_app_exit");
  int execute();
};

int await_app_exit::execute()
{
  FUNCDEF("execute");
  SETUP_COMBO_LOGGER;
  if (_global_argc < 3) {
    BASE_LOG("This program needs two parameters on the command line.  The first is an");
    BASE_LOG("application name (e.g. 'blofeld.exe' is a valid example--no path should be");
    BASE_LOG("included but the .exe suffix must be included) to seek out in the process");
    BASE_LOG("list and the second parameter is the time to wait for it to exit (in seconds).");
    BASE_LOG("This program will not exit until the specified application is no longer");
    BASE_LOG("running or the timeout elapses.  If the timeout elapses, then a failure exit");
    BASE_LOG("will occur from this program so that it is known that the target application");
    BASE_LOG("never exited.");
    return 2;
  }

  astring app_name = _global_argv[1];  // get the app's name.
  astring duration = _global_argv[2];  // get the time to wait.
  int timeout = duration.convert(0) * 1000;
  if (timeout < 0) {
    LOG(astring("The timeout specified is invalid: ") + duration);
    return 3;
  }

  // now see if that app is even running.
  process_control querier;
  process_entry_array processes;
  querier.query_processes(processes);
  int_set pids;
  time_stamp when_to_leave(timeout);  // when we should stop checking.

  // wait for the app to go away.
  while (querier.find_process_in_list(processes, app_name, pids)) {
    // the program of interest is still running.
    time_control::sleep_ms(100);
    querier.query_processes(processes);
    if (time_stamp() > when_to_leave) {
      LOG(astring("The timeout elapsed and ") + app_name + " is still running.");
      return 4;
    }
  }
  LOG(astring("The ") + app_name + " process has exited.");
  return 0;
}

HOOPLE_MAIN(await_app_exit, )

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
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
  #include <basis/trap_new.addin>
  #include <basis/untrap_new.addin>
  #include <basis/utility.cpp>
  #include <basis/version_record.cpp>
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
  #include <timely/time_stamp.cpp>
  #include <application/base_application.cpp>
  #include <application/application_shell.cpp>
  #include <filesystem/byte_filer.cpp>
  #include <application/command_line.cpp>
  #include <opsystem/critical_events.cpp>
  #include <filesystem/directory.cpp>
  #include <filesystem/filename.cpp>
  #include <configuration/ini_configurator.cpp>
  #include <opsystem/ini_parser.cpp>
  #include <configuration/application_configuration.cpp>
  #include <application/rendezvous.cpp>
  #include <processes/process_control.cpp>
  #include <processes/process_entry.cpp>
  #include <textual/byte_formatter.cpp>
  #include <textual/parser_bits.cpp>
  #include <textual/string_manipulation.cpp>
  #include <configuration/variable_tokenizer.cpp>
#endif // __BUILD_STATIC_APPLICATION__

