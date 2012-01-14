/*****************************************************************************\
*                                                                             *
*  Name   : dump_bytes                                                        *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Prints the files specified out in terms of their hex bytes.              *
*                                                                             *
*******************************************************************************
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <filesystem/byte_filer.h>
#include <loggers/console_logger.h>
#include <structures/static_memory_gremlin.h>
#include <textual/byte_formatter.h>

using namespace application;
using namespace basis;
using namespace loggers;
using namespace filesystem;
using namespace structures;
using namespace textual;

///HOOPLE_STARTUP_CODE;

const int MAXIMUM_BYTEDUMP_BUFFER_SIZE = 32768;
  // this is the size of the chunk we read from the files at a time.  it is
  // important to make this a multiple of 16, since that's the size of the line
  // we use in the byte dumping.

#define console program_wide_logger::get()

int print_instructions_and_exit(char *program_name)
{
  console.log(astring(astring::SPRINTF, "\n\
Usage:\n\t%s filename [filename]\n\n\
Prints out (on standard output) a abyte dump of the files specified.\n\n",
      program_name), ALWAYS_PRINT);
  return 12;
}

int main(int argc, char *argv[])
{
  SETUP_CONSOLE_LOGGER;

  if (argc <= 1) return print_instructions_and_exit(argv[0]);
  else {
    int current_parameter = 0;
/*
    if (argv[1][0] == '-') {
      if (argv[1][1] == 't') {
        current_parameter++;
        open_file_as_text = true;
      } else if (argv[1][1] == 'b') {
        current_parameter++;
        open_file_as_text = false;
      } else print_instructions_and_exit(argv[0]);
    }
*/
    bool past_first_file = false;
    while (++current_parameter < argc) {
      if (past_first_file) {
        // we're into the second file so start using some white space.
        console.log(astring::empty_string(), ALWAYS_PRINT);
        console.log(astring::empty_string(), ALWAYS_PRINT);
      }
      past_first_file = true;  // set condition for next time.
      astring name = argv[current_parameter];
      byte_filer current(name, "rb");
      if (!current.good()) {
        console.log(astring("Cannot find the file named \"") + name + astring("\"."),
            ALWAYS_PRINT);
        continue;
      }
      abyte buff[MAXIMUM_BYTEDUMP_BUFFER_SIZE + 10];
        // buffer plus some extra room.
      bool printed_header = false;
      int current_label = 0;
      while (true) {
        int bytes_read = current.read(buff, MAXIMUM_BYTEDUMP_BUFFER_SIZE);
        if (bytes_read <= 0) break;  // no contents.
//console.log(astring(astring::SPRINTF, "read %d bytes", bytes_read));
        if (!printed_header) {
          console.log(name + ":", ALWAYS_PRINT);
          console.log(astring::empty_string(), ALWAYS_PRINT);  // blank line.
          printed_header = true;
        }
        astring to_log = byte_formatter::text_dump(buff, bytes_read,
            current_label);
        if (to_log[to_log.end()] == '\n')
          to_log.zap(to_log.end(), to_log.end());
        console.log(to_log, ALWAYS_PRINT);
        current_label += bytes_read;
      }
    }
  }
  return 0;
}

