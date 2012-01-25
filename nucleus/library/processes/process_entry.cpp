/*****************************************************************************\
*                                                                             *
*  Name   : process_entry                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "process_entry.h"

#include <basis/array.h>
#include <basis/astring.h>
#include <filesystem/filename.h>

using namespace basis;
using namespace filesystem;

namespace processes {

process_entry::process_entry()
: _process_id(0),
  _references(0),
  _threads(0),
  _parent_process_id(0),
  _module16(0),
  _process_path(new astring)
{}

process_entry::process_entry(const process_entry &to_copy)
: _process_id(0),
  _references(0),
  _threads(0),
  _parent_process_id(0),
  _module16(0),
  _process_path(new astring)
{
  operator =(to_copy);
}

process_entry::~process_entry()
{
  WHACK(_process_path);
}

void process_entry::text_form(basis::base_string &fill) const
{
  fill = text_form();
}

process_entry &process_entry::operator =(const process_entry &to_copy)
{
  if (&to_copy == this) return *this;
  _process_id = to_copy._process_id;
  _references = to_copy._references;
  _threads = to_copy._threads;
  _parent_process_id = to_copy._parent_process_id;
  *_process_path = *to_copy._process_path;
  _module16 = to_copy._module16;
  return *this;
}

const astring &process_entry::path() const { return *_process_path; }

void process_entry::path(const astring &new_path)
{ *_process_path = new_path; }

astring process_entry::text_form() const
{
#ifdef __UNIX__
  filename pat(path());
#else
  filename pat(path().lower());
#endif
  return a_sprintf("%s: pid=%u refs=%u thrd=%u par=%u mod16=%u path=%s",
      pat.basename().raw().s(), _process_id, _references, _threads,
      _parent_process_id, _module16, pat.dirname().raw().s());
}

} //namespace.

