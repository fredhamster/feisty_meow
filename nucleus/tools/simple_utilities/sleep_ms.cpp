/*****************************************************************************\
*                                                                             *
*  Name   : sleep_ms                                                          *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Takes a number from the command line and sleeps for that many milli-     *
*  seconds.                                                                   *
*                                                                             *
*******************************************************************************
* Copyright (c) 2001-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/command_line.h>
#include <application/windoze_helper.h>
#include <basis/definitions.h>
#include <timely/time_control.h>

#include <stdio.h>

using namespace application;
using namespace basis;
using namespace timely;

DEFINE_ARGC_AND_ARGV;
///DEFINE_INSTANCE_HANDLE;

int main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("%s usage:\nThe first parameter is taken as the number of "
        "milliseconds to sleep.\n", argv[0]);
    return 1;
  }
  
  int snooze_ms;
  sscanf(argv[1], "%d", &snooze_ms);
  time_control::sleep_ms(snooze_ms);
  return 0;
}

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
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
  #include <timely/time_control.cpp>
  #include <timely/time_stamp.cpp>
#endif // __BUILD_STATIC_APPLICATION__

