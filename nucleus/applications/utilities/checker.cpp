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

#include <basis/functions.h>
#include <basis/astring.h>
#include <structures/checksums.h>
#include <structures/static_memory_gremlin.h>
#include <timely/time_stamp.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace basis;
using namespace structures;
using namespace timely;

const int buffer_size = 4096;

//HOOPLE_STARTUP_CODE;

//#define DEBUG_CHECKER
  // uncomment for noisy version.

void print_instructions_and_exit(char *program_name)
{
  printf("\n\
Usage:\n\t%s [-t] filename [filename]\n\n\
This program generates a checksum for each file that is entered on the\n\
command line.  The checksum is (hopefully) an architecture independent\n\
number that is a very compressed representation of the file gestalt.\n\
If one compares two copies of a file, then the checksums should be identical.\n\
This is a useful test of whether a file copy or a program download is\n\
successful in making an identical version of the file.  In particular, if the\n\
file is made slightly bigger or smaller, or if an item in the file is changed,\n\
then the checksums of the two versions should be different numbers.\n\n\
The -b flag is used if the files are to be compared as binary files, and this\n\
is also the default.  The -t flag is used if the files are to be compared as\n\
text files.\n",
  program_name);
  exit(1);
}

#define HIGHEST_CHECK 32714

// do_checksum: takes the specified file name and generates a checksum for it.
// if the file is inaccessible or, at any point, reading it returns an
// error message, then a negative value is returned.
int do_checksum(char *file_name, int open_as_a_text_file)
{
  char file_open_mode[10];
  if (open_as_a_text_file) strcpy(file_open_mode, "rt");
  else strcpy(file_open_mode, "rb");
  FILE *opened_file = fopen(file_name, file_open_mode);
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
int do_fletcher_checksum(char *file_name, int open_as_a_text_file)
{
  char file_open_mode[10];
  if (open_as_a_text_file) strcpy(file_open_mode, "rt");
  else strcpy(file_open_mode, "rb");
  FILE *opened_file = fopen(file_name, file_open_mode);
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
  char name[200];

  // if the file is to be read as a text file, then this is true.
  int open_file_as_text = false;

  if (argc <= 1) print_instructions_and_exit(argv[0]);
  else {
    int current_parameter = 0;
    if (argv[1][0] == '-') {
      if (argv[1][1] == 't') {
        current_parameter++;
        open_file_as_text = true;
      } else if (argv[1][1] == 'b') {
        current_parameter++;
        open_file_as_text = false;
      } else print_instructions_and_exit(argv[0]);
    }
    bool printed_header = false;
    while (++current_parameter < argc) {
      if (!printed_header) {
        printed_header = true;
        printf("%s\n", (astring("[ checker running at ") + time_stamp::notarize(true) + "]").s());
        printf("bizarro  fletcher  filename\n");
        printf("=======  ========  ========\n");
      }
      strcpy(name, argv[current_parameter]);
      int checksum_of_file = do_checksum(name, open_file_as_text);
      int fletcher_chksum = do_fletcher_checksum(name, open_file_as_text);
      if (checksum_of_file >= 0) {
        printf("%s", a_sprintf(" %05d    0x%04x   %s\n", checksum_of_file,
            fletcher_chksum, name).s());
      } else {
        printf("%s", a_sprintf("%s is inaccessible.\n", name).s());
      }
    }
  }
  return 0;
}

