/*****************************************************************************\
*                                                                             *
*  Name   : configlet                                                         *
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

#include "configlet.h"
#include "configurator.h"

#include <basis/astring.h>
#include <basis/functions.h>

using namespace basis;

namespace configuration {

const astring bogus_default = "OOPS: not supposed to ever be seen.  d'oh!";

//////////////

configlet::configlet(const astring &section, const astring &entry)
: _section(new astring(section)),
  _entry(new astring(entry))
{}

configlet::configlet(const configlet &to_copy)
: _section(new astring(*to_copy._section)),
  _entry(new astring(*to_copy._entry))
{}

configlet::~configlet()
{
  WHACK(_section);
  WHACK(_entry);
}

configlet &configlet::operator =(const configlet &to_copy)
{
  if (this == &to_copy) return *this;
  *_section = *to_copy._section;
  *_entry = *to_copy._entry;
  return *this;
}

const astring &configlet::section() const { return *_section; }

const astring &configlet::entry() const { return *_entry; }

void configlet::section(const astring &new_section) const
{ *_section = new_section; }

void configlet::entry(const astring &new_entry) const
{ *_entry = new_entry; }

//////////////

string_configlet::string_configlet(const astring &section, const astring &entry,
    const astring &current_value, const astring &default_value)
: configlet(section, entry),
  _current(new astring(current_value)),
  _default(new astring(default_value))
{}

string_configlet::string_configlet(const string_configlet &to_copy)
: configlet(to_copy.section(), to_copy.entry()),
  _current(new astring(*to_copy._current)),
  _default(new astring(*to_copy._default))
{
}

string_configlet::~string_configlet()
{
  WHACK(_current);
  WHACK(_default);
}

string_configlet &string_configlet::operator =(const string_configlet &to_copy)
{
  if (this == &to_copy) return *this;
  (configlet &)*this = to_copy;
  *_current = *to_copy._current;
  *_default = *to_copy._default;
  return *this;
}

const astring &string_configlet::current_value() const { return *_current; }

const astring &string_configlet::default_value() const { return *_default; }

void string_configlet::current_value(const astring &new_current)
{ *_current = new_current; }

void string_configlet::default_value(const astring &new_default)
{ *_default = new_default; }
  
bool string_configlet::load(configurator &config)
{
  if (config.get(section(), entry(), *_current)) return true;  // success.
  // we failed to read the value...
  *_current = *_default;
  if (config.behavior() == configurator::AUTO_STORE)
    config.put(section(), entry(), *_current);
  return false;  // don't hide that it wasn't there.
}

bool string_configlet::store(configurator &config) const
{ return config.put(section(), entry(), *_current); }

configlet *string_configlet::duplicate() const
{ return new string_configlet(section(), entry(), *_current, *_default); }

//////////////

int_configlet::int_configlet(const astring &section, const astring &entry,
    int current_value, int default_value)
: configlet(section, entry),
  _current(current_value),
  _default(default_value)
{
}

int_configlet::~int_configlet() {}

void int_configlet::current_value(int new_current)
{ _current = new_current; }

bool int_configlet::load(configurator &config)
{
  astring temp;
  bool to_return = config.get(section(), entry(), temp);
    // just check if it was already there.
  int temp_c = config.load(section(), entry(), _default);
  current_value(temp_c);
  return to_return;
}

bool int_configlet::store(configurator &config) const
{ return config.store(section(), entry(), _current); }

configlet *int_configlet::duplicate() const
{ return new int_configlet(*this); }

//////////////

bounded_int_configlet::bounded_int_configlet(const astring &section,
    const astring &entry, int current_value, int default_value,
    int minimum, int maximum)
: int_configlet(section, entry, current_value, default_value),
  _minimum(minimum),
  _maximum(maximum)
{
}

bounded_int_configlet::~bounded_int_configlet() {}

void bounded_int_configlet::current_value(int new_current)
{
  if (new_current < _minimum)
    new_current = default_value();
  if (new_current > _maximum)
    new_current = default_value();
  int_configlet::current_value(new_current);
}

configlet *bounded_int_configlet::duplicate() const
{ return new bounded_int_configlet(*this); }

} //namespace.

