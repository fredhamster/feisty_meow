/*****************************************************************************\
*                                                                             *
*  Name   : version structures: version, version_record                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1996-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "string_array.h"
#include "version_record.h"

#include <basis/functions.h>

#include <ctype.h>

using namespace basis;

namespace structures {

void *version::__global_module_handle()
{
  static void *__mod_hand = 0;
  return __mod_hand;
}

version::version()
: _components(new string_array)
{
  set_component(MAJOR, "0");
  set_component(MINOR, "0");
  set_component(REVISION, "0");
  set_component(BUILD, "0");
}

version::version(const string_array &version_info)
: _components(new string_array(version_info))
{
  for (int i = 0; i < _components->length(); i++) {
    if (!(*_components)[i]) {
      // this component is blank; replace it with a zero.
      (*_components)[i] = "0";
    }
  }
}

version::version(const astring &formatted_string)
: _components(new string_array)
{
  astring verstring = formatted_string;
  // scan through and replace bogus bits with reasonable bits.
  for (int j = 0; j < verstring.length(); j++) {
    if (verstring[j] == ',') verstring[j] = '.';
      // replace commas with periods.
  }
  // locate the pieces of the version string.
  for (int i = 0; i < verstring.length(); i++) {
    int perindy = verstring.find('.', i);
    if (negative(perindy)) {
      if (verstring.length() - i > 0) {
        // add any extra bits after the last period in the string.
        *_components += verstring.substring(i, verstring.end());
      }
      break;
    }
    // we found a period, so include anything between the current position and
    // that as a component.
    *_components += verstring.substring(i, perindy - 1);
    i = perindy;
      // set i to be at the next period; it will be incremented past that.
  }
}

version::version(int maj, int min, int rev, int build)
: _components(new string_array)
{
  *this = version(a_sprintf("%u.%u.%u.%u", basis::un_int(maj), basis::un_int(min), basis::un_int(rev),
      basis::un_int(build)));
}

version::version(const version &to_copy)
: _components(new string_array)
{ *this = to_copy; }

version::~version() { WHACK(_components); }

version &version::operator =(const version &to_copy)
{
  if (this != &to_copy) *_components = *to_copy._components;
  return *this;
}

astring version::text_form() const { return flex_text_form(); }

int version::components() const { return _components->length(); }

int version::v_major() const
{ return int(get_component(MAJOR).convert(0)); }

int version::v_minor() const
{ return int(get_component(MINOR).convert(0)); }

int version::v_revision() const
{ return int(get_component(REVISION).convert(0)); }

int version::v_build() const
{ return int(get_component(BUILD).convert(0)); }

astring version::get_component(int index) const
{
  bounds_return(index, 0, _components->length() - 1, "0");
  return (*_components)[index];
}

void version::set_component(int index, const astring &to_set)
{
  if (_components->length() <= index)
    _components->resize(index + 1);
  if (to_set.t())
    (*_components)[index] = to_set;
  else
    (*_components)[index] = "0";
}

bool version::equal_to(const equalizable &to_test) const
{
  const version *cast = cast_or_throw(to_test, *this);
  return *_components == *cast->_components;
}

bool version::less_than(const orderable &to_test) const
{
  const version *cast = cast_or_throw(to_test, *this);
  if (v_major() < cast->v_major()) return true;
  else if (v_major() > cast->v_major()) return false;
  if (v_minor() < cast->v_minor()) return true;
  else if (v_minor() > cast->v_minor()) return false;
  if (v_revision() < cast->v_revision()) return true;
  else if (v_revision() > cast->v_revision()) return false;
  if (v_build() < cast->v_build()) return true;
  return false;
}

bool version::compatible(const version &to_test) const
{
//fix to be more general
  return (v_major() == to_test.v_major())
      && (v_minor() == to_test.v_minor())
      && (v_revision() == to_test.v_revision());
}

bool version::bogus() const
{ return !v_major() && !v_minor() && !v_revision() && !v_build(); }

#define SEPARATE \
  if (style != DOTS) to_return += ", "; \
  else to_return += "."

astring version::flex_text_form(version_style style, int where) const
{
  // note: the conversions below are to ensure proper treatment of the
  // size of the int16 types by win32.
  astring to_return;

  if (style == DETAILED) to_return += "major ";
  astring temp = get_component(MAJOR);
  if (!temp) temp = "0";
  to_return += temp;
  if (where == MAJOR) return to_return;
  SEPARATE;

  if (style == DETAILED) to_return += "minor ";
  temp = get_component(MINOR);
  if (!temp) temp = "0";
  to_return += temp;
  if (where == MINOR) return to_return;
  SEPARATE;

  if (style == DETAILED) to_return += "revision ";
  temp = get_component(REVISION);
  if (!temp) temp = "0";
  to_return += temp;
  if (where == REVISION) return to_return;
  SEPARATE;

  if (style == DETAILED) to_return += "build ";
  temp = get_component(BUILD);
  if (!temp) temp = "0";
  to_return += temp;

  // other components don't have handy names.
  if (where > BUILD) {
    for (int i = BUILD + 1; i < where; i++) {
      SEPARATE;
      if (style == DETAILED) to_return += a_sprintf("part_%d ", i + 1);
      temp = get_component(i);
      if (!temp) temp = "0";
      to_return += temp;
    }
  }

  return to_return;
}

version version::from_text(const astring &to_convert)
{ return version(to_convert); }

int version::packed_size() const
{
  return _components->packed_size();
}

void version::pack(byte_array &target) const
{ _components->pack(target); }

bool version::unpack(byte_array &source)
{
  if (!_components->unpack(source)) return false;
  return true;
}

//////////////

#define VR_NEWLINE to_return += "\n"

version_record::~version_record()
{}

astring version_record::text_form() const
{
  astring to_return;
  to_return += "Description: ";
  to_return += description;
  VR_NEWLINE;
  to_return += "File Version: ";
  to_return += file_version.text_form();
  VR_NEWLINE;
  to_return += "Internal Name: ";
  to_return += internal_name;
  VR_NEWLINE;
  to_return += "Original Name: ";
  to_return += original_name;
  VR_NEWLINE;
  to_return += "Product Name: ";
  to_return += product_name;
  VR_NEWLINE;
  to_return += "Product Version: ";
  to_return += product_version.text_form();
  VR_NEWLINE;
  to_return += "Company Name: ";
  to_return += company_name;
  VR_NEWLINE;
  to_return += "Copyright: ";
  to_return += copyright;
  VR_NEWLINE;
  to_return += "Trademarks: ";
  to_return += trademarks;
  VR_NEWLINE;

  return to_return;
}

} //namespace.

