/*****************************************************************************\
*                                                                             *
*  Name   : short_path                                                        *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    This program converts a pathname to its 8.3 name.  Only for windows.     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2007-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/windoze_helper.h>
#include <basis/astring.h>
#include <structures/static_memory_gremlin.h>

#include <stdio.h>
#include <string.h>
#ifdef __WIN32__
  #include <windows.h>
#endif

using namespace basis;
using namespace structures;

///HOOPLE_STARTUP_CODE;

int main(int argc, char *argv[])
{
  astring shorty('\0', 2048);
  if (argc < 2) {
    printf("This program needs a path to convert to its short form.\n");
    return 23;
  }
#ifdef __WIN32__
  GetShortPathNameA(argv[1], shorty.s(), 2045);
#else
  strcpy(shorty.s(), argv[1]);
#endif
  shorty.replace_all('\\', '/');
  printf("%s", shorty.s());
  return 0;
}

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
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

