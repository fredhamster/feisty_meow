/*****************************************************************************\
*                                                                             *
*  Name   : configurator                                                      *
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

#include "configurator.h"

#include <basis/astring.h>
#include <structures/string_table.h>
#include <structures/set.h>

using namespace basis;
using namespace structures;

namespace configuration {

configurator::~configurator() {}

astring configurator::load(const astring &section, const astring &entry,
        const astring &default_string)
{
  astring to_return;
  if (!get(section, entry, to_return)) {
    to_return = default_string;
    if (_behavior == AUTO_STORE) put(section, entry, to_return);
      // save the entry back if we're in autostore mode.
  }
  return to_return;
}

bool configurator::store(const astring &section, const astring &entry,
    const astring &to_store)
{ return put(section, entry, to_store); }

bool configurator::store(const astring &section, const astring &entry,
    int value)
{
  return store(section, entry, astring(astring::SPRINTF, "%d", value));
}

void configurator::sections(string_array &list)
{
  // default implementation does nothing.
  list = string_array();
}

void configurator::section_set(string_set &list)
{
  string_array temp;
  sections(temp);
  list = temp;
}

int configurator::load(const astring &section, const astring &entry,
    int def_value)
{
  astring value_string;
  if (!get(section, entry, value_string)) {
    if (_behavior == AUTO_STORE) store(section, entry, def_value);
    return def_value;
  }
  return value_string.convert(def_value);
}

bool configurator::section_exists(const astring &section)
{
  string_table infos;
  // heavy-weight call here...
  return get_section(section, infos);
}

} //namespace.

