/*****************************************************************************\
*                                                                             *
*  Name   : config_watcher                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2008-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "config_watcher.h"

#include <basis/functions.h>
#include <configuration/table_configurator.h>
#include <structures/set.h>
#include <structures/string_table.h>

using namespace basis;
using namespace structures;

namespace configuration {

config_watcher::config_watcher(configurator &to_watch)
: _watching(to_watch),
  _current_config(new table_configurator),
  _previous_config(new table_configurator)
{
  rescan();  // fill out our lists.
}

config_watcher::~config_watcher()
{
  WHACK(_current_config);
  WHACK(_previous_config);
}

bool config_watcher::rescan()
{
  // copy the current configuration into our previous config tracker.
  *_previous_config = *_current_config;
  // clean out any current items held.
  _current_config->reset();

  // iterate across the sections in the watched config.
  string_set sects;
  _watching.section_set(sects);
  for (int sectindy = 0; sectindy < sects.length(); sectindy++) {
    // every entry in the current section gets added to our current config.
    astring curr_section = sects[sectindy];
    string_table entries;
	_watching.get_section(curr_section, entries);
    _current_config->put_section(curr_section, entries);
  }

  return true;
}

string_set config_watcher::new_sections() const
{
  string_set before;
  _previous_config->section_set(before);
  string_set after;
  _current_config->section_set(before);
  return after - before;
}

string_set config_watcher::deleted_sections() const
{
  string_set before;
  _previous_config->section_set(before);
  string_set after;
  _current_config->section_set(before);
  return before - after;
}

string_set config_watcher::changed_sections() const
{
  string_set before;
  _previous_config->section_set(before);
  string_set after;
  _current_config->section_set(before);
  string_set possible_changes = before.intersection(after);
  string_set definite_changes;
  for (int i = 0; i < possible_changes.elements(); i++) {
    const astring &sect_name = possible_changes[i];
    string_table previous_section;
    _previous_config->get_section(sect_name, previous_section);
    string_table current_section;
    _current_config->get_section(sect_name, current_section);
    if (current_section != previous_section)
      definite_changes += sect_name;
  }
  return definite_changes;
}

string_set config_watcher::deleted_items(const astring &section_name)
{
  string_table previous_section;
  _previous_config->get_section(section_name, previous_section);
  string_set previous_names;
  previous_section.names(previous_names);
  string_table current_section;
  _current_config->get_section(section_name, current_section);
  string_set current_names;
  current_section.names(current_names);
  return previous_names - current_names;
}

string_set config_watcher::new_items(const astring &section_name)
{
  string_table previous_section;
  _previous_config->get_section(section_name, previous_section);
  string_set previous_names;
  previous_section.names(previous_names);
  string_table current_section;
  _current_config->get_section(section_name, current_section);
  string_set current_names;
  current_section.names(current_names);
  return current_names - previous_names;
}

string_set config_watcher::changed_items(const astring &section_name)
{
  string_table previous_section;
  _previous_config->get_section(section_name, previous_section);
  string_set previous_names;
  previous_section.names(previous_names);
  string_table current_section;
  _current_config->get_section(section_name, current_section);
  string_set current_names;
  current_section.names(current_names);

  string_set possible_changes = current_names.intersection(previous_names);
  string_set definite_changes;
  for (int i = 0; i < possible_changes.elements(); i++) {
    const astring &curr_item = possible_changes[i];
    astring prev_value;
    _previous_config->get(section_name, curr_item, prev_value);
    astring curr_value;
    _current_config->get(section_name, curr_item, curr_value);
    if (prev_value != curr_value)
      definite_changes += curr_item;
  }
  return definite_changes;
}

} //namespace.


