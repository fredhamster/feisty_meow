/*****************************************************************************\
*                                                                             *
*  Name   : string_table                                                      *
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

#include "object_packers.h"
#include "string_table.h"
#include "symbol_table.h"

#include <basis/astring.h>
#include <basis/functions.h>

using namespace basis;

namespace structures {

string_table::string_table(const string_table &to_copy)
: symbol_table<astring>(to_copy.estimated_elements()),
  _add_spaces(false)
{
  *this = to_copy;
}

string_table::~string_table() {}

bool string_table::is_comment(const astring &to_check)
{ return to_check.begins(STRTAB_COMMENT_PREFIX); }

string_table &string_table::operator = (const string_table &to_copy)
{
  if (this == &to_copy) return *this;
  (symbol_table<astring> &)*this = (const symbol_table<astring> &)to_copy;
  _add_spaces = to_copy._add_spaces;
  return *this;
}

astring string_table::text_form() const
{
  astring output;
  const char *space_char = "";
  if (_add_spaces) space_char = " ";
  for (int i = 0; i < symbols(); i++) {
    if (is_comment(name(i)))
      output += a_sprintf("%s\n", operator[](i).s());
    else
      output += a_sprintf("%s%s=%s%s\n", name(i).s(), space_char,
          space_char, operator[](i).s());
  }
  return output;
}

bool string_table::operator ==(const string_table &to_compare) const
{
  if (to_compare.symbols() != symbols()) return false;
  for (int i = 0; i < symbols(); i++) {
    const astring &key = name(i);
    astring *str1 = find(key);
    astring *str2 = to_compare.find(key);
    if (!str2) return false;
    if (*str1 != *str2) return false;
  }
  return true;
}

int string_table::packed_size() const
{
  int size = sizeof(int);
  for (int i = 0; i < symbols(); i++) {
    size += name(i).length();
    size += operator[](i).length();
  }
  return size;
}

void string_table::pack(byte_array &packed_form) const
{
  structures::attach(packed_form, symbols());
  for (int i = 0; i < symbols(); i++) {
    name(i).pack(packed_form);
    operator[](i).pack(packed_form);
  }
}

bool string_table::unpack(byte_array &packed_form)
{
  reset();
  int syms;
  if (!structures::detach(packed_form, syms)) return false;
  for (int i = 0; i < syms; i++) {
    astring name, content;
    if (!name.unpack(packed_form)) return false;
    if (!content.unpack(packed_form)) return false;
    outcome ret = add(name, content);
    if (ret != common::IS_NEW) return false;
  }
  return true;
}

} //namespace.


