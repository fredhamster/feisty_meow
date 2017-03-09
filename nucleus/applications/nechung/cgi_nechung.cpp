/*****************************************************************************\
*                                                                             *
*  Name   : CGI nechung                                                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1997-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

//! @file cgi_nechung.cpp Spits out a CGI appropriate chunk of text with a fortune in it.

#include "nechung_oracle.h"

#include <basis/astring.h>
#include <basis/environment.h>
#include <basis/guards.h>
#include <filesystem/filename.h>
#include <loggers/console_logger.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>

#include <stdio.h>

//using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;

#undef LOG
#define LOG(s) program_wide_logger::get().log(s, 0)

////HOOPLE_STARTUP_CODE;
//hmmm: need this back when things are ready!

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
  // send the header for html text.
  printf("content-type: text/html\n\n");
  // send the preliminary gunk.
  printf("<body text=\"#00ee00\" bgcolor=\"#000000\" link=\"#66ff99\" "
      "vlink=\"#cc33cc\" alink=\"#ff9900\">\n");
//old text color #33ccff
  printf("<tt style=\"font-weight: bold;\">\n");
  printf("<font size=\"+1\">\n");

  astring to_show = some_fortunes.pick_random();
  int line_posn = 0;
  for (int i = 0; i < to_show.length(); i++) {
    if (to_show[i] == ' ') {
      // spaces get translated to one non-breaking space.
      printf("&nbsp;");
      line_posn++;
    } else if (to_show[i] == '\t') {
      // tabs get translated to positioning at tab stops based on eight.
      int to_add = 8 - line_posn % 8;
      for (int j = 0; j < to_add; j++) printf("&nbsp;");
      line_posn += to_add;
    } else if (to_show[i] == '\r')
      continue;
    else if (to_show[i] == '\n') {
      printf("<br>%c", to_show[i]);
      line_posn = 0;
    } else {
      printf("%c", to_show[i]);
      line_posn++;
    }
  }
  printf("\n");
  printf("</font>\n");
  printf("</tt>\n");
  printf("</body>\n");
  printf("</html>\n");
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

