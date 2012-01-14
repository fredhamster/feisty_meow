


/*****************************************************************************\
*                                                                             *
*  Name   : section_manager                                                   *
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

#include "section_manager.h"

#include <basis/functions.h>
#include <basis/astring.h>
#include <structures/string_array.h>
#include <structures/string_table.h>

using namespace basis;
using namespace configuration;
using namespace structures;

namespace configuration {

section_manager::section_manager(configurator &config,
    const astring &toc_title, const astring &header_prefix)
: _config(config),
  _toc_title(new astring(toc_title)),
  _section_prefix(new astring(header_prefix))
{
}

section_manager::~section_manager()
{
  WHACK(_toc_title);
  WHACK(_section_prefix);
}

bool section_manager::get_toc(string_table &toc)
{ return _config.get_section(*_toc_title, toc); }

bool section_manager::get_section_names(string_array &sections)
{
  sections.reset();  // clean up the array they gave us.
  string_table toc;
  if (!get_toc(toc)) return false;
  for (int i = 0; i < toc.symbols(); i++) sections += toc.name(i);
  return true;
}

bool section_manager::section_exists(const astring &section_name)
{
  if (!section_name) return false;  // empty names are invalid.
  return _config.load(*_toc_title, section_name, "").t();
}

astring section_manager::make_section_heading(const astring &section)
{ return *_section_prefix + section; }

bool section_manager::add_section(const astring &section_name,
    const string_table &to_add)
{
  if (!section_name) return false;  // empty names are invalid.
  if (section_exists(section_name)) return false;
  // write the name into the table of contents.
  astring section_value = "t";  // place-holder for section entries.
  if (!to_add.symbols())
    section_value = "";  // nothing to write; delete the section entry.
  if (!_config.put(*_toc_title, section_name, section_value))
    return false;
  // create the appropriate header for the new section.
  astring header = make_section_heading(section_name);
  // if there aren't any elements, the put_section should just wipe out
  // the entire section.
  return _config.put_section(header, to_add);
}

bool section_manager::replace_section(const astring &section_name,
    const string_table &replacement)
{
  if (!section_name) return false;  // empty names are invalid.
  if (!section_exists(section_name)) return false;
  if (!zap_section(section_name)) return false;
  if (!replacement.symbols()) return true;  // nothing to write.
  return add_section(section_name, replacement);
}

bool section_manager::zap_section(const astring &section_name)
{
  if (!section_name) return false;  // empty names are invalid.
  astring header = make_section_heading(section_name);
  if (!_config.delete_section(header)) return false;
  return _config.delete_entry(*_toc_title, section_name);
}

bool section_manager::find_section(const astring &section_name,
    string_table &found)
{
  if (!section_name) return false;  // empty names are invalid.
  if (!section_exists(section_name)) return false;
  astring header = make_section_heading(section_name);
  return _config.get_section(header, found);
}

} //namespace.


