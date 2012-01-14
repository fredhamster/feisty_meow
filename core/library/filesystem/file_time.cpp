/*****************************************************************************\
*                                                                             *
*  Name   : file_time                                                         *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "file_time.h"

#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <structures/object_packers.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __UNIX__
  #include <utime.h>
#endif
#ifdef __WIN32__
  #include <sys/utime.h>
#endif

#undef LOG
#define LOG(to_print) printf("%s::%s: %s\n", static_class_name(), func, astring(to_print).s())

using namespace basis;

namespace filesystem {

file_time::file_time() : _when(0) { reset(""); } 

file_time::file_time(FILE *the_FILE) : _when(0) { reset(the_FILE); }

file_time::file_time(const time_t &t) : _when(t) {}

file_time::file_time(const astring &filename)
: _when(0)
{
  FILE *ptr = fopen(filename.s(), "r");
  if (ptr) {
    reset(ptr);
    fclose(ptr);
  }
}

file_time::~file_time() {}

bool file_time::set_time(const basis::astring &filename)
{
  // prepare the access time and modified time to match our stamp.
  utimbuf held_time;
  held_time.actime = raw();
  held_time.modtime = raw();
  // stuff our timestamp on the file.
  return !utime(filename.s(), &held_time);
}

void file_time::reset(const time_t &t) { _when = t; }

void file_time::reset(const astring &filename)
{
  FILE *ptr = fopen(filename.s(), "r");
  if (ptr) {
    reset(ptr);
    fclose(ptr);
  }
}

void file_time::reset(FILE *the_FILE_in)
{
  FUNCDEF("reset");
  _when = 0;
  FILE *the_file = (FILE *)the_FILE_in;
  if (!the_file) {
    return;
  }
  struct stat stat_buffer;
  if (fstat(fileno(the_file), &stat_buffer) < 0) {
    LOG("stat failure on file");
  }
  _when = stat_buffer.st_mtime;
}

int file_time::compare(const file_time &b) const
{
  if (_when > b._when) return 1;
  else if (_when == b._when) return 0;
  else return -1;
}

bool file_time::less_than(const orderable &ft2) const
{
  const file_time *cast = dynamic_cast<const file_time *>(&ft2);
  if (!cast) return false;
  return bool(compare(*cast) < 0);
}

bool file_time::equal_to(const equalizable &ft2) const
{
  const file_time *cast = dynamic_cast<const file_time *>(&ft2);
  if (!cast) return false;
  return bool(compare(*cast) == 0);
}

void file_time::text_form(basis::base_string &time_string) const
{
  time_string.assign(basis::astring(class_name()) + basis::a_sprintf("@%x", _when));
}

void file_time::readable_text_form(basis::base_string &time_string) const
{
  tm *now = localtime(&_when);  // convert to time round hereparts.
  time_string.assign(a_sprintf("%d.%02d.%02d %02d:%02d:%02d",
      now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec));
}

// magic value unfortunately, but this is the length of a packed string with 10 characters.
// we do this because the size of 2^32 in decimal requires that many characters, and we also
// want to just have a fixed length chunk for this.  we are not worried about larger pieces
// than that because we cannot conveniently handle them anyhow.
int file_time::packed_size() const { return 11; }

void file_time::pack(byte_array &packed_form) const
{ a_sprintf("%010d", _when).pack(packed_form); }

bool file_time::unpack(byte_array &packed_form)
{
  astring tempo;
  if (!tempo.unpack(packed_form)) return false;
  _when = tempo.convert(0L);
  return true;
}

} //namespace

