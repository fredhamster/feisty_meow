/*****************************************************************************\
*                                                                             *
*  Name   : nechung_oracle                                                    *
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

#include "nechung_oracle.h"

#include <basis/astring.h>
#include <filesystem/byte_filer.h>
#include <filesystem/file_time.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>

#include <stdio.h>
#include <string.h>

//using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;

#undef LOG
#define LOG(s) program_wide_logger::get().log(s, 0)
///hmmm: fix filter value to be ALWAYS_PRINT!

const int MAX_LINE_LENGTH = 2048;

nechung_oracle::nechung_oracle(const astring &nechung_filename,
    const astring &index_filename)
: c_randomizer(),
  c_filename_held(nechung_filename),
  c_index_held(index_filename),
  c_number_of_fortunes(0)
{ parse_file(); }

nechung_oracle::~nechung_oracle() {}

void nechung_oracle::parse_file()
{
  FUNCDEF("parse_file");
  // below is code for comparing dates on the fortune file and the index file.
  byte_filer fortune_file(c_filename_held.s(), "rb");
#ifdef DEBUG_NECHUNG
  LOG(astring("filename=") + c_filename_held + " idx file=" + c_index_held);
#endif
  if (!fortune_file.good())
    non_continuable_error(class_name(), func, "Cannot open fortune file.");

  byte_array buffer(MAX_LINE_LENGTH + 1);
    // used throughout parsing for line storage.

  byte_filer index_file(c_index_held.observe(), "r");
  if (index_file.good()) {
#ifdef DEBUG_NECHUNG
    LOG("index file exists");
#endif
    file_time index_time((FILE *)index_file.file_handle());
    file_time fortune_time((FILE *)fortune_file.file_handle());
    if (index_time >= fortune_time) {
      // need to read in the list of indices
      index_file.getline(buffer, MAX_LINE_LENGTH);
      sscanf((char *)buffer.access(), "%d", &c_number_of_fortunes);
#ifdef DEBUG_NECHUNG
      LOG(astring(astring::SPRINTF, "%d entries in index", 
          c_number_of_fortunes));
#endif
      return;
    }
  }
  index_file.close();

  // below is code for creating the list.
  enum fortune_states {
    chowing_separators,  // looking for the breaks between fortunes.
    adding_fortunes,     // saw the separator so get ready for a new fortune.
    chowing_fortunes,    // currently in a fortune accumulating lines.
    done_parsing         // finished parsing the fortune file.
  };

  c_number_of_fortunes = 0;
  fortune_states state = chowing_separators;

  int posn;
  int_array fortune_posns;  // our list of fortunes.
  while (state != done_parsing) {
#ifdef DEBUG_NECHUNG
    LOG(astring(astring::SPRINTF, "#%d", c_number_of_fortunes));
#endif
    if (fortune_file.eof()) {
      // exit from the loop now...
      state = done_parsing;
      continue;
    }
    switch (state) {
      case chowing_separators: {
#ifdef DEBUG_NECHUNG
        LOG("chowseps, ");
#endif
        posn = int(fortune_file.tell());
        if (posn < 0)
          non_continuable_error(class_name(), func, "Cannot get file position.");
        fortune_file.getline(buffer, MAX_LINE_LENGTH);
#ifdef DEBUG_NECHUNG
        LOG(astring("got a line: ") + buffer);
#endif
        if (buffer[0] != NECHUNG_SEPARATION_CHARACTER) state = adding_fortunes;
        else {
          // special casing is for when we see a separator on the line
          // by itself versus when it is the beginning of a line.  if the
          // beginning of a line, we currently take that to mean the rest
          // of the line is the fortune.
          if (strlen((char *)buffer.access()) == 2) posn += 2;
          else posn++;
          state = adding_fortunes;
        }
        break;
      }
      case adding_fortunes: {
#ifdef DEBUG_NECHUNG
        LOG("add forts, ");
#endif
        fortune_posns += posn;
        c_number_of_fortunes++;
        state = chowing_fortunes;
        break;
      }
      case chowing_fortunes: {
#ifdef DEBUG_NECHUNG
        LOG("chow forts, ");
#endif
        posn = int(fortune_file.tell());
        if (posn < 0)
          non_continuable_error(class_name(), func, "Cannot get file size.");
        fortune_file.getline(buffer, MAX_LINE_LENGTH);
#ifdef DEBUG_NECHUNG
        LOG(astring(astring::SPRINTF, "got a line: %s", buffer.access()));
        LOG(astring(astring::SPRINTF, "len is %d", strlen((char *)buffer.access())));
#endif
        if ( (buffer[0] == NECHUNG_SEPARATION_CHARACTER)
            && (strlen((char *)buffer.access()) == 2) )
          state = chowing_separators;
        else if (buffer[0] == NECHUNG_SEPARATION_CHARACTER) {
          posn++;
          state = adding_fortunes;
        }
        break;
      }
      case done_parsing: {
        non_continuable_error(class_name(), func, "Illegal state reached.");
      }
    }
  }
  fortune_file.close();

  // make a new index file.
  index_file.open(c_index_held.observe(), "w");
  if (!index_file.good())
    non_continuable_error(class_name(), func, astring("Cannot open index file: ") + c_index_held);
  astring to_write(astring::SPRINTF, "%d\n", c_number_of_fortunes);
  index_file.write((abyte *)to_write.s(), to_write.length());
  for (int j = 0; j < c_number_of_fortunes; j++) {
    to_write.sprintf("%d\n", fortune_posns[j]);
    index_file.write((abyte *)to_write.s(), to_write.length());
  }
  index_file.close();
}

astring nechung_oracle::pick_random()
{
  FUNCDEF("pick_random");
#ifdef DEBUG_NECHUNG
  LOG(astring("got to ") + func);
#endif

  byte_filer fortune_file(c_filename_held.s(), "rb");

///printf("num forts = %d\n", c_number_of_fortunes );

  if (!fortune_file.good())
    non_continuable_error(class_name(), func, "Cannot open data file.");
  int to_display = c_randomizer.inclusive(0, c_number_of_fortunes - 1);

///printf("rand chose= %d\n", to_display);

/////
///hmmm: this bit could be more efficient by just jumping to the Nth line
///      instead of reading through up to the Nth line.
/////
  byte_filer index_file(c_index_held.observe(), "r");
  int chosen_posn = 0;  // which position to read the chosen line at.
  if (index_file.good()) {
    astring accumulated_text;
    byte_array buffer(MAX_LINE_LENGTH + 1);
    for (int i = 0; i <= to_display; i++) {
#ifdef DEBUG_NECHUNG
      accumulated_text += astring(astring::SPRINTF, "#%d: ", i);
#endif
      index_file.getline(buffer, MAX_LINE_LENGTH);
      sscanf((char *)buffer.access(), "%d", &chosen_posn);
#ifdef DEBUG_NECHUNG
      accumulated_text += astring(astring::SPRINTF, "%d, ", chosen_posn);
      if ((i + 1) % 5 == 0) accumulated_text += "\n";
#endif
    }
#ifdef DEBUG_NECHUNG
    LOG(accumulated_text);
#endif
    
  } else {
    non_continuable_error(class_name(), func, \
        astring("Could not open the index file \"") + c_index_held + "\"");
  }
  index_file.close();
#ifdef DEBUG_NECHUNG
  LOG(astring(astring::SPRINTF, "about to seek @ num %d and "
      "index %d", to_display, chosen_posn));
#endif
  if (!fortune_file.seek(chosen_posn, byte_filer::FROM_START))
    non_continuable_error(class_name(), func, "Cannot seek to indexed position.");
#ifdef DEBUG_NECHUNG
  LOG("after seek");
#endif

  astring to_return;
  byte_array temp(MAX_LINE_LENGTH + 1);
  while (!fortune_file.eof()) {
    int chars_read = fortune_file.getline(temp, MAX_LINE_LENGTH);
    if (!chars_read) {
      if (!fortune_file.eof()) {
        non_continuable_error(class_name(), func, "Error while reading fortune.");
      } else break;
    }
    if (temp[0] == NECHUNG_SEPARATION_CHARACTER) break;
    else to_return += astring((char *)temp.access());
  }
  return to_return;
}

//hmmm: stolen from parser bits.  reconnect when available.
bool is_eol(char to_check)
{ return (to_check == '\n') || (to_check == '\r'); }

void nechung_oracle::display_random()
{
  astring to_show = pick_random();
  while (is_eol(to_show[to_show.end()]))
    to_show.zap(to_show.end(), to_show.end());
  LOG(to_show);
}

