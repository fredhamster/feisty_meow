/*****************************************************************************\
*                                                                             *
*  Name   : filename_list                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "filename_list.h"

#include <structures/string_array.h>
#include <textual/parser_bits.h>

using namespace basis;
using namespace structures;
using namespace textual;

namespace filesystem {

filename_list::filename_list() : amorph<file_info>() {}

filename_list &filename_list::operator =(const filename_list &to_copy)
{
  if (this == &to_copy) return *this;
  reset();
  for (int i = 0; i < to_copy.elements(); i++) {
    append(new file_info(*to_copy.get(i)));
  }
  return *this;
}

int filename_list::total_files() const { return elements(); }

int filename_list::packed_size() const
{ return amorph_packed_size(*this); }

void filename_list::pack(byte_array &packed_form) const
{ amorph_pack(packed_form, *this); }

bool filename_list::unpack(byte_array &packed_form)
{ return amorph_unpack(packed_form, *this); }

double filename_list::total_size() const
{
  double to_return = 0;
  for (int i = 0; i < elements(); i++)
    to_return += get(i)->_file_size;
  return to_return;
}

bool filename_list::calculate_progress(const filename &file,
    double current_offset, int &current_file, double &current_size)
{
  current_file = 0;
  current_size = 0;
  int posn = locate(file);
  if (negative(posn)) {
    if (file.raw().t()) return false;  // not a member.
    // they're at the start of the run.
    current_file = 1;
    return true;
  }
  current_file = posn + 1;  // position zero in array means file number 1.
  double size_finished = 0;
  // iterate on all files before the current one.
  for (int i = 0; i < posn; i++) {
    size_finished += get(i)->_file_size;
  }
  current_size = size_finished + current_offset;
  return true;
}

filename_list &filename_list::operator = (const string_array &to_copy)
{
  reset();
  for (int i = 0; i < to_copy.length(); i++) {
    append(new file_info(to_copy[i], 0));
  }
  return *this;
}

void filename_list::fill(string_array &to_fill)
{
  to_fill.reset();
  for (int i = 0; i < elements(); i++) {
    to_fill += get(i)->raw();
  }
}

const file_info *filename_list::find(const filename &to_check) const
{
  for (int i = 0; i < elements(); i++) {
#if defined(__WIN32__) || defined(__VMS__)
    if (to_check.raw().iequals(get(i)->raw())) return get(i);
#else
    if (to_check.raw() == get(i)->raw()) return get(i);
#endif
  }
  return NIL;
}

int filename_list::locate(const filename &to_find) const
{
  for (int i = 0; i < elements(); i++) {
#if defined(__WIN32__) || defined(__VMS__)
    if (to_find.raw().iequals(get(i)->raw())) return i;
#else
    if (to_find.raw() == get(i)->raw()) return i;
#endif
  }
  return common::NOT_FOUND;
}

bool filename_list::member(const filename &to_check) const
{
  for (int i = 0; i < elements(); i++) {
#if defined(__WIN32__) || defined(__VMS__)
    if (to_check.raw().iequals(get(i)->raw())) return true;
#else
    if (to_check.raw() == get(i)->raw()) return true;
#endif
  }
  return false;
}

bool filename_list::member_with_state(const file_info &to_check, file_info::file_similarity how)
{
  for (int i = 0; i < elements(); i++) {
#if defined(__WIN32__) || defined(__VMS__)
    if (to_check.raw().iequals(get(i)->raw())) {
#else
    if (to_check.raw() == get(i)->raw()) {
#endif
      // once we have matched a name, the other checks will cause us to
      // reject any other potential matches after this one if the requested
      // check fails.
      if ((how & file_info::EQUAL_FILESIZE) && (to_check._file_size != get(i)->_file_size) )
        return false;
      if ((how & file_info::EQUAL_TIMESTAMP) && (to_check._time != get(i)->_time) )
        return false;
      if ((how & file_info::EQUAL_CHECKSUM) && (to_check._checksum != get(i)->_checksum) )
        return false;
      return true;
    }
  }
  return false;
}

astring filename_list::text_form() const
{
  astring to_return;
//hmmm: a length limit might be nice?
  for (int i = 0; i < elements(); i++) {
    to_return += a_sprintf("%d. ", i + 1);
    if (get(i))
      to_return += get(i)->text_form();
    if (i != elements() - 1)
      to_return += parser_bits::platform_eol_to_chars();
  }
  return to_return;
}

} //namespace.

