/*****************************************************************************\
*                                                                             *
*  Name   : system_values                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2004-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "ini_configurator.h"
#include "system_values.h"

#include <algorithms/shell_sort.h>
#include <structures/int_hash.h>
#include <structures/string_table.h>
#include <textual/list_parsing.h>
#include <textual/parser_bits.h>

using namespace algorithms;
using namespace basis;
using namespace structures;
using namespace textual;

namespace configuration {

const int MAX_VALUE_BITS = 8;  // we provide 2^n slots in hash.

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

//////////////

class value_record
{
public:
  astring _name;  // the name of the value.
  astring _descrip;  // the description of the value.
  astring _location;  // the file defining the value.

  value_record(const astring &name = astring::empty_string(),
      const astring &description = astring::empty_string(),
      const astring &location = astring::empty_string())
  : _name(name), _descrip(description), _location(location) {}
};

//////////////

class system_values_lookup_list : public int_hash<value_record>
{
public:
  system_values_lookup_list() : int_hash<value_record>(MAX_VALUE_BITS) {}

  // finds the symbolic "name" in the table, which is not as efficient as
  // lookin up integers.
  value_record *text_find(const astring &name, int &value) {
    // scoot across all of the ids.
    const int_set &cids = ids();
    for (int i = 0; i < cids.elements(); i++) {
      int current_id = cids[i];
      value_record *curr = find(current_id);
      if (!curr) {
//serious error.
        continue;
      }
      if (curr->_name == name) {
        // this is a match to the name they were seeking.
        value = current_id;
        return curr;
      }
    }
    return NIL;
  }
};

//////////////

system_values::system_values(const astring &section_tag)
: _tag(new astring(section_tag)),
  _list(new system_values_lookup_list),
  _file(new astring(DEFAULT_MANIFEST))
{
  FUNCDEF("constructor");
  open_values();
}
  
system_values::~system_values()
{
  WHACK(_list);
  WHACK(_tag);
  WHACK(_file);
}

const char *system_values::DEFAULT_MANIFEST = "manifest.txt";
  // this is the default manifest and it is expected to live right in
  // the folder where the applications are.

bool system_values::use_other_manifest(const astring &manifest_file)
{
  *_file = manifest_file;
  return open_values();
}

const char *system_values::OUTCOME_VALUES() { return "DEFINE_OUTCOME"; }

const char *system_values::FILTER_VALUES() { return "DEFINE_FILTER"; }

const char *system_values::EVENT_VALUES() { return "DEFINE_EVENT"; }

bool system_values::open_values()
{
  FUNCDEF("open_values");
  ini_configurator ini(*_file, ini_configurator::RETURN_ONLY,
      ini_configurator::APPLICATION_DIRECTORY);

  string_table full_section;
  bool got_section = ini.get_section(*_tag, full_section);
  if (!got_section) return false;  // nothing there to look up.
  for (int i = 0; i < full_section.symbols(); i++) {

    string_array items;
    list_parsing::parse_csv_line(full_section.name(i), items);
    if (items.length() < 4) {
      continue;
    }

    value_record *entry = new value_record(items[0], items[2], items[3]);
    int value = items[1].convert(0);
    _list->add(value, entry);
  }

  return true;
}

#define SV_EOL parser_bits::platform_eol_to_chars()

//hmmm: it might be nice to have an alternate version sorted by name...

astring system_values::text_form() const
{
  int_set cids = _list->ids();

  if (!_tag->equal_to("DEFINE_OUTCOME")) {
    // sort the list in identifier order.
    shell_sort(cids.access(), cids.elements());
  } else {
    // sort the list in reverse identifier order, since zero is first
    // for outcomes and then they go negative.
    shell_sort(cids.access(), cids.elements(), true);
  }

  astring to_return("values for ");
  to_return += *_tag;
  to_return += SV_EOL;
  for (int i = 0; i < cids.elements(); i++) {
    int current_id = cids[i];
    value_record *curr = _list->find(current_id);
    if (!curr) {
//serious error.
      continue;
    }
    to_return += a_sprintf("%d: ", current_id);
    to_return += curr->_name + " \"" + curr->_descrip + "\" from "
        + curr->_location;
    to_return += SV_EOL;
  }
  return to_return;
}

bool system_values::lookup(int value, astring &symbolic_name,
    astring &description, astring &file_location)
{
  value_record *found = _list->find(value);
  if (!found) return false;
  symbolic_name = found->_name;
  description = found->_descrip;
  file_location = found->_location;
  return true;
}

bool system_values::lookup(const astring &symbolic_name, int &value,
    astring &description, astring &file_location)
{
  value_record *found = _list->text_find(symbolic_name, value);
  if (!found) return false;
  description = found->_descrip;
  file_location = found->_location;
  return true;
}

int system_values::elements() const { return _list->ids().elements(); }

bool system_values::get(int index, astring &symbolic_name, int &value,
    astring &description, astring &file_location)
{
  bounds_return(index, 0, _list->ids().elements() - 1, false);  // bad index.
  value = _list->ids()[index];
  return lookup(value, symbolic_name, description, file_location);
}

} //namespace.


