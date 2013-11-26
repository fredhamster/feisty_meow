/*****************************************************************************\
*                                                                             *
*  Name   : checker                                                           *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Generates checksums for a set of files.                                  *
*                                                                             *
*******************************************************************************
* Copyright (c) 1990-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/command_line.h>
#include <basis/functions.h>
#include <basis/astring.h>
#include <loggers/program_wide_logger.h>
#include <structures/checksums.h>
#include <structures/static_memory_gremlin.h>
#include <timely/time_stamp.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace application;
using namespace basis;
using namespace loggers;
using namespace structures;
using namespace timely;

const int buffer_size = 4096;

//#define DEBUG_CHECKER
  // uncomment for noisy version.

#undef LOG
#define LOG(to_print) program_wide_logger::get().log(to_print, ALWAYS_PRINT)

int print_instructions(bool good, const astring &program_name)
{
  printf("\n\
Usage:\n\t%s [-q] [-t|-b] filename [filename]\n\n\
This program generates a checksum for each file that is entered on the\n\
command line.  The checksum is (hopefully) an architecture independent\n\
number that is a very compressed representation of the file gestalt.\n\
If one compares two copies of a file, then the checksums should be identical.\n\
This is a useful test of whether a file copy or a program download is\n\
successful in making an identical version of the file.  In particular, if the\n\
file is made slightly bigger or smaller, or if an item in the file is changed,\n\
then the checksums of the two versions should be different numbers.\n\n\
The -q flag specifies a quieter print-out, without any headers.\n\
The -b flag is used if the files are to be compared as binary files, and this\n\
is also the default.  The -t flag is used if the files are to be compared as\n\
text files.\n",
  program_name.s());
  return !good;  // zero is successful exit.
}

#define HIGHEST_CHECK 32714

// do_checksum: takes the specified file name and generates a checksum for it.
// if the file is inaccessible or, at any point, reading it returns an
// error message, then a negative value is returned.
int do_checksum(const astring &file_name, int open_as_a_text_file)
{
  char file_open_mode[10];
  if (open_as_a_text_file) strcpy(file_open_mode, "rt");
  else strcpy(file_open_mode, "rb");
  FILE *opened_file = fopen(file_name.s(), file_open_mode);
#ifdef DEBUG_CHECKER
  LOG(astring("opened ") + file_name);
#endif
  if (!opened_file) return common::NOT_FOUND;
  int characters_read = 0;
  int current_checksum_value = 0;
  char buffer_chunk[buffer_size];
  while (!feof(opened_file)) {
    characters_read = int(fread(buffer_chunk, sizeof(char), buffer_size,
        opened_file));
    // if result is 0 or negative, stop messing with the file.
#ifdef DEBUG_CHECKER
    LOG(a_sprintf("char read = %d", characters_read));
#endif
    if (characters_read <= 0) {
      if (characters_read < 0) current_checksum_value = -1;
      else if (current_checksum_value == 0) current_checksum_value = -1;
      break;
    }
    current_checksum_value = (current_checksum_value
            + checksums::bizarre_checksum((abyte *)buffer_chunk, characters_read))
        % HIGHEST_CHECK;
#ifdef DEBUG_CHECKER
    LOG(a_sprintf("current checksum=%d", current_checksum_value));
#endif
  }
  fclose(opened_file);
  return int(current_checksum_value);
}

// do_fletcher_checksum: takes the specified file name and generates a fletcher
// checksum for it.  if the file is inaccessible or, at any point,
// reading it returns an error message, then a negative value is returned.
int do_fletcher_checksum(const astring &file_name, int open_as_a_text_file)
{
  char file_open_mode[10];
  if (open_as_a_text_file) strcpy(file_open_mode, "rt");
  else strcpy(file_open_mode, "rb");
  FILE *opened_file = fopen(file_name.s(), file_open_mode);
#ifdef DEBUG_CHECKER
  LOG(astring("opened ") + file_name);
#endif
  if (!opened_file) return common::NOT_FOUND;
  int characters_read = 0;
  int current_checksum_value = 0;
  char buffer_chunk[buffer_size];
  while (!feof(opened_file)) {
    characters_read = int(fread(buffer_chunk, sizeof(char), buffer_size,
        opened_file));
    // if result is 0 or negative, stop messing with the file.
#ifdef DEBUG_CHECKER
    LOG(a_sprintf("char read = %d", characters_read));
#endif
    if (characters_read <= 0) {
      if (characters_read < 0) current_checksum_value = -1;
      else if (current_checksum_value == 0) current_checksum_value = -1;
      break;
    }
    current_checksum_value = checksums::rolling_fletcher_checksum
        ((uint16)current_checksum_value, (abyte *)buffer_chunk,
        characters_read);
#ifdef DEBUG_CHECKER
    LOG(a_sprintf("current checksum=%d", current_checksum_value));
#endif
  }
  fclose(opened_file);
  return current_checksum_value;
}

int main(int argc, char *argv[])
{
  // if the file is to be read as a text file, then this is true.
  bool open_file_as_text = false;
  // if we are to show our normal header info, this will be true.
  bool show_header = true;

  if (argc <= 1) return print_instructions(false, argv[0]);

  command_line cmds(argc, argv);
  int index = 0;
  if (cmds.find('b', index)) open_file_as_text = false;
  index = 0;
  if (cmds.find('t', index)) open_file_as_text = true;
  index = 0;
  if (cmds.find('q', index)) show_header = false;
  index = 0;
  if (cmds.find('?', index)) return print_instructions(true, argv[0]);
  index = 0;
  if (cmds.find("help", index)) return print_instructions(true, argv[0]);
  bool printed_header = false;

  for (int entry = 0; entry < cmds.entries(); entry++) {
    command_parameter c = cmds.get(entry);
    if (c.type() != command_parameter::VALUE) continue;
    if (!printed_header) {
      printed_header = true;
      if (show_header) {
        printf("%s\n", (astring("[ checker running at ") + time_stamp::notarize(true) + "]").s());
        printf("bizarro  fletcher  filename\n");
        printf("=======  ========  ========\n");
      }
    }
    astring name = c.text();
    int checksum_of_file = do_checksum(name, open_file_as_text);
    int fletcher_chksum = do_fletcher_checksum(name, open_file_as_text);
    if (checksum_of_file >= 0) {
      printf("%s", a_sprintf(" %05d    0x%04x   %s\n", checksum_of_file,
          fletcher_chksum, name.s()).s());
    } else {
      printf("%s", a_sprintf("%s is inaccessible.\n", name.s()).s());
    }
  }
  return 0;
}

