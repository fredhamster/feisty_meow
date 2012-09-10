/*****************************************************************************\
*                                                                             *
*  Name   : nechung console application                                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*****************************************************************************/

//! @file nechung.cpp The application base for the Nechung Oracle Program (NOP).

#include "nechung_oracle.h"

#include <basis/astring.h>
#include <basis/guards.h>
#include <basis/environment.h>
#include <filesystem/filename.h>
#include <loggers/console_logger.h>
#include <loggers/program_wide_logger.h>

//using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;

///HOOPLE_STARTUP_CODE;
//hmmm: missing.

#undef LOG
#define LOG(s) program_wide_logger::get().log((s), 0)
//hmmm: need ALWAYS_PRINT back in there!

#define DEFAULT_FORTUNE_FILE "fortunes.dat"

int main(int argc, char *argv[])
{
  SETUP_CONSOLE_LOGGER;

  astring name;
  astring index;
  if (argc > 1) {
    // use the first command line argument.
    name = argv[1];
  } else {
    // if nothing on the command line, then use our defaults.
    name = environment::get("NECHUNG");
      // first try the environment variable.
    if (!name) name = DEFAULT_FORTUNE_FILE;
      // next, use the hardwired default.
  }

  if (name.length() < 5) {
    LOG(astring("nechung:: file name is too short (") + name + ").");
    return 1;
  }
  filename index_file_name(name);
  astring extension(index_file_name.extension());
  int end = index_file_name.raw().end();
#ifdef DEBUG_NECHUNG
  LOG(astring("fortune filename is ") + name);
  LOG(astring("extension is ") + extension);
#endif
  astring tmp = index_file_name;
  tmp.zap( (end + 1) - extension.length(), end);
  tmp += "idx";
  astring base_part = filename(tmp).basename();
  index_file_name = environment::TMP() + "/" + base_part;
#ifdef DEBUG_NECHUNG
  LOG(astring("index file is ") + index_file_name);
#endif
  index = index_file_name;

  nechung_oracle some_fortunes(name, index);
  some_fortunes.display_random();
  return 0;
}

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
  #include <application/application_shell.cpp>
  #include <basis/astring.cpp>
  #include <basis/common_outcomes.cpp>
  #include <basis/environment.cpp>
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
  #include <filesystem/file_time.cpp>
  #include <loggers/combo_logger.cpp>
  #include <loggers/console_logger.cpp>
  #include <loggers/critical_events.cpp>
  #include <loggers/file_logger.cpp>
  #include <loggers/program_wide_logger.cpp>
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

