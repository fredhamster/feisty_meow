/*****************************************************************************\
*                                                                             *
*  Name   : splitter                                                          *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Takes text as input and splits the lines so that they will fit on a      *
*  standard 80 column terminal.                                               *
*                                                                             *
*******************************************************************************
* Copyright (c) 1993-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <filesystem/byte_filer.h>
#include <filesystem/filename.h>
#include <loggers/console_logger.h>
#include <loggers/file_logger.h>
#include <structures/static_memory_gremlin.h>
#include <structures/set.h>
#include <textual/string_manipulation.h>

#include <stdio.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;

const int MAX_BUFFER = 1024;

class splitter_app : public application_shell
{
public:
  splitter_app() : application_shell() {}

  DEFINE_CLASS_NAME("splitter_app");

  virtual int execute();

  int print_instructions();

private:
};

//////////////

int splitter_app::print_instructions()
{
  astring name = filename(_global_argv[0]).basename().raw();
  log(a_sprintf("%s usage:", name.s()));
  log(astring::empty_string());
  log(a_sprintf("\
This program splits long lines in input files into a more reasonable size.\n\
Any filenames on the command line are split and sent to standard output.\n\
The following options change how the splitting is performed:\n\
   --help or -?\tShow this help information.\n\
   --mincol N\tMinimum column to use for output.\n\
   --maxcol N\tMaximum column to use for output.\n\
"));
  return -3;
}

int splitter_app::execute()
{
  command_line cmds(_global_argc, _global_argv);  // parse the command line up.

  // retrieve any specific flags first.
  astring temp;
  int min_col = 0;
  int min_indy = -1;
//hmmm: this whole thing is annoying.  we need a better way to have a list of parms.
  if (cmds.find("mincol", min_indy)) {
    cmds.get_value("mincol", temp);
    min_col = temp.convert(min_col);
  }
  int max_col = 78;
  int max_indy = -1;
  if (cmds.find("maxcol", max_indy)) {
    cmds.get_value("maxcol", temp);
    max_col = temp.convert(max_col);
  }
//printf("got max_col=%d\n", max_col);
  // look for help command.
  int junk_index = 0;
  if (cmds.find("help", junk_index, false)
      || cmds.find('h', junk_index, false)
      || cmds.find("?", junk_index, false)
      || cmds.find('?', junk_index, false) ) {
    print_instructions();
    return 0;
  }

  // see if we found any flags that would make us skip some of the parameters.
//hmmm: automate this!
  int skip_index = 0;
  if ( (min_indy >= 0) || (max_indy >= 0) ) {
    skip_index = basis::maximum(min_indy, max_indy);
    skip_index += 2;
  }
//printf("got a skip index of %d\n", skip_index);

  // gather extra input files.
  string_set input_files;
  for (int i = skip_index; i < cmds.entries(); i++) {
    const command_parameter &curr = cmds.get(i);
    if (curr.type() == command_parameter::VALUE) {
//log(astring("adding input file:") + curr.text());
      input_files += curr.text();
    }
  }

  astring accumulator;
  for (int q = 0; q < input_files.length(); q++) {
    byte_filer current(input_files[q], "r");
    if (!current.good()) continue;
    while (!current.eof()) {
      astring line_read;
      int num_chars = current.getline(line_read, MAX_BUFFER);
      if (!num_chars) continue;
//printf("line len=%d, cont=%s\n", line_read.length(), line_read.s());
      accumulator += line_read;
    }
  }

  // now get from standard input if there weren't any files specified.
  if (!input_files.length()) {
    char input_line[MAX_BUFFER + 2];
    while (!feof(stdin)) {
      char *got = fgets(input_line, MAX_BUFFER, stdin);
      if (!got) break;
//printf("line=%s\n", got);
      accumulator += got;
    }
  }
//printf("splitting accum with %d chars...\n", accumulator.length());
  astring chewed;
  string_manipulation::split_lines(accumulator, chewed, min_col, max_col);
//printf("chewed string now has %d chars...\n", chewed.length());
  printf("%s", chewed.s());
  return 0;
}

//////////////

HOOPLE_MAIN(splitter_app, )

