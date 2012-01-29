/*****************************************************************************\
*                                                                             *
*  Name   : table_configurator                                                *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2001-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "table_configurator.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <structures/string_array.h>
#include <structures/string_table.h>

using namespace basis;
using namespace structures;

namespace configuration {

#undef LOG
#define LOG(to_print) printf("%s::%s: %s\n", static_class_name(), func, astring(to_print).s())

class table_o_string_tables : public symbol_table<string_table>
{
public:
};

//////////////

table_configurator::table_configurator(treatment_of_defaults behavior)
: configurator(behavior),
  _real_table(new table_o_string_tables)
{}

table_configurator::table_configurator(const table_configurator &to_copy)
: configurator(to_copy.behavior()),
  _real_table(new table_o_string_tables)
{ *this = to_copy; }

table_configurator::~table_configurator()
{
  WHACK(_real_table);
}

table_configurator &table_configurator::operator =
    (const table_configurator &to_copy)
{
  if (this == &to_copy) return *this;
  reset();
  string_array sects;
  const_cast<table_configurator &>(to_copy).sections(sects);
  for (int sectindy = 0; sectindy < sects.length(); sectindy++) {
    // every entry in the current section gets added to our current config.
    astring curr_section = sects[sectindy];
    string_table entries;
	const_cast<table_configurator &>(to_copy).get_section(curr_section, entries);
    put_section(curr_section, entries);
  }

  return *this;
}

void table_configurator::reset() { _real_table->reset(); }

bool table_configurator::section_exists(const astring &section)
{ return !!_real_table->find(section); }

void table_configurator::sections(string_array &to_fill)
{
  to_fill.reset();
  for (int i = 0; i < _real_table->symbols(); i++)
    to_fill += _real_table->name(i);
}

bool table_configurator::delete_section(const astring &section)
{ return _real_table->whack(section) == common::OKAY; }

bool table_configurator::delete_entry(const astring &section,
    const astring &ent)
{
  string_table *sect = _real_table->find(section);
  if (!sect) return false;
  return sect->whack(ent) == common::OKAY;
}

bool table_configurator::put(const astring &section,
    const astring &entry, const astring &to_store)
{
  if (!to_store.length()) return delete_entry(section, entry);
  else if (!entry.length()) return delete_section(section);
  string_table *sect = _real_table->find(section);
  if (!sect) {
    // none exists yet, so add one.
    _real_table->add(section, string_table());
    sect = _real_table->find(section);
  }
  sect->add(entry, to_store);
  return true;
}

bool table_configurator::get(const astring &section,
    const astring &entry, astring &found)
{
  found = "";
  string_table *sect = _real_table->find(section);
  if (!sect) return false;
  astring *looked = sect->find(entry);
  if (!looked) return false;
  found = *looked;
  return true;
}

/*
scavenge?
bool is_comment(char to_check, const char *comment_list)
{
  int len = int(strlen(comment_list));
  for (int i = 0; i < len; i++) {
    if (to_check == comment_list[i])
      return true;
  }
  return false;
}
*/

/* scavenge?
//hmmm: could we move the commented and clean_comments methods into
//      parser bits?
//      yes!  we should move those; they are no longer used here!

bool table_configurator::commented(const astring &to_check,
    const char *comment_list)
{
  for (int i = 0; i < to_check.length(); i++) {
    if (white_space(to_check[i]))
      continue;  // skip spaces.
    if (is_comment(to_check[i], comment_list))
      return true;  // started with a comment.
    return false;  // we had our chance for a comment, but that wasn't it.
  }
  return false;
}

astring table_configurator::clean_comments(const astring &to_clean,
    const char *comment_list)
{
  FUNCDEF("clean_comments");
//LOG(astring("clean in with: ") + to_clean);
  astring to_return(' ', to_clean.length());  // make a long enough string.
  to_return.reset();  // keep allocated buffer size, but throw out contents.
  for (int i = 0; i < to_clean.length(); i++) {
    if (is_comment(to_clean[i], comment_list)) {
      // here we go; the rest is commented out.
      break;
    }
    to_return += to_clean[i];
  }
//LOG(astring("clean out with: ") + to_return);
  return to_return;
}
*/

bool table_configurator::get_section(const astring &section,
    string_table &info)
{
///  FUNCDEF("get_section");
  info.reset();
  string_table *sect = _real_table->find(section);
  if (!sect) return false;
  for (int i = 0; i < sect->symbols(); i++)
    info.add(sect->name(i), (*sect)[i]);
  return true;
}

bool table_configurator::put_section(const astring &section,
    const string_table &info)
{
///  FUNCDEF("put_section");
  string_table *sect = _real_table->find(section);
  if (!sect) {
    // none exists yet, so add one.
    _real_table->add(section, string_table());
    sect = _real_table->find(section);
  }
  *sect = info;
  return true;
}

} //namespace.

