/*****************************************************************************\
*                                                                             *
*  Name   : machine_uid                                                       *
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

//#include "internet_address.h"
#include "machine_uid.h"

#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/astring.h>
#include <basis/mutex.h>
#include <structures/checksums.h>
#include <structures/object_packers.h>
#include <structures/static_memory_gremlin.h>
#include <textual/byte_formatter.h>

using namespace basis;
using namespace structures;
using namespace textual;

namespace sockets {

machine_uid::machine_uid()
: _contents(new byte_array)
{ *_contents += abyte(INVALID_LOCATION); }

machine_uid::machine_uid(known_location_types type,
    const byte_array &address)
: _contents(new byte_array)
{
  *_contents += abyte(type); 
  *_contents += address;
}

machine_uid::machine_uid(const machine_uid &to_copy)
: packable(),
  _contents(new byte_array)
{ *this = to_copy; }

machine_uid::~machine_uid() { WHACK(_contents); }

void machine_uid::reset(known_location_types type, const byte_array &address)
{
  _contents->reset();
  *_contents += abyte(type); 
  *_contents += address;
}

const byte_array &machine_uid::raw() const { return *_contents; }

const astring &machine_uid::type_name(known_location_types type)
{
  static astring TCPIP_NAME = "TCPIP";
  static astring IPX_NAME = "IPX";
  static astring NETBIOS_NAME = "NETBIOS";
  static astring UNKNOWN_NAME = "INVALID";

  switch (type) {
    case TCPIP_LOCATION: return TCPIP_NAME;
    case IPX_LOCATION: return IPX_NAME;
    case NETBIOS_LOCATION: return NETBIOS_NAME;
    default: return UNKNOWN_NAME;
  }
}

machine_uid &machine_uid::operator = (const machine_uid &to_copy)
{
  if (this == &to_copy) return *this;
  *_contents = *to_copy._contents;
  return *this;
}

astring machine_uid::text_form() const
{
  astring to_return;
  to_return += type_name(type()) + "[";
  for (int i = 1; i < _contents->length(); i++) {
    if (type() == TCPIP_LOCATION)
      to_return += a_sprintf("%d", int(_contents->get(i)));
    else
      to_return += a_sprintf("%02x", int(_contents->get(i)));
    if (i < _contents->length() - 1)
      to_return += ".";
  }
  to_return += "]";
  return to_return;
}

astring machine_uid::compact_form() const
{
  astring to_return;
  byte_formatter::bytes_to_shifted_string(*_contents, to_return);
  return to_return;
}

machine_uid machine_uid::expand(const astring &compacted)
{
  machine_uid to_return;
  to_return._contents->reset();
  byte_formatter::shifted_string_to_bytes(compacted, *to_return._contents);
  return to_return;
}

bool machine_uid::operator == (const machine_uid &to_compare) const
{
  // an empty id is only equal to another empty id.
  if (!_contents->length() || !to_compare._contents->length())
    return !_contents->length() && !to_compare._contents->length();
  if (_contents->length() != to_compare._contents->length()) return false;
  for (int i = 0; i < _contents->length(); i++)
    if (_contents->get(i) != to_compare._contents->get(i)) return false;
  return true;
}

machine_uid::known_location_types machine_uid::type() const
{
  if (!_contents->length()) return INVALID_LOCATION;
  return known_location_types(_contents->get(0));
}

int machine_uid::packed_size() const
{
  return PACKED_SIZE_INT32 + _contents->length();
}

void machine_uid::pack(byte_array &packed_form) const
{
  structures::attach(packed_form, _contents->length());
  packed_form += *_contents;
}

bool machine_uid::unpack(byte_array &packed_form)
{
  int len = 0;
  if (!structures::detach(packed_form, len)) return false;
    // there's no length even?
  if (packed_form.length() < len) return false;
    // not enough size left for the length specified.
  *_contents = packed_form.subarray(0, len - 1);
  packed_form.zap(0, len - 1);
  return true;
}

byte_array machine_uid::native() const
{
  if (_contents->length() <= 1) return byte_array();  // invalid.
  return _contents->subarray(1, _contents->last());
}

//////////////

class internal_machine_uid_array : public array<machine_uid> {};

machine_uid_array::machine_uid_array()
: _uids(new internal_machine_uid_array)
{}

machine_uid_array::machine_uid_array(const machine_uid_array &to_copy)
: root_object(),
  _uids(new internal_machine_uid_array(*to_copy._uids))
{
}

machine_uid_array::~machine_uid_array()
{
  WHACK(_uids);
}

SAFE_STATIC_CONST(machine_uid_array, machine_uid_array::blank_array, )

machine_uid_array &machine_uid_array::operator =
    (const machine_uid_array &to_copy)
{
  if (this != &to_copy) {
    *_uids = *to_copy._uids;
  }
  return *this;
}

bool machine_uid_array::operator += (const machine_uid &to_add)
{
  if (member(to_add)) return false;
  _uids->concatenate(to_add);
  return true;
}

int machine_uid_array::elements() const { return _uids->length(); }

astring machine_uid_array::text_form() const
{
  astring to_return;
  for (int i = 0; i < _uids->length(); i++) {
    to_return += _uids->get(i).text_form() + " ";
  }
  return to_return;
}

machine_uid &machine_uid_array::operator [] (int index)
{ return _uids->use(index); }

const machine_uid &machine_uid_array::operator [] (int index) const
{ return _uids->get(index); }

void machine_uid_array::reset() { _uids->reset(); }

bool machine_uid_array::member(const machine_uid &to_test) const
{
  const int test_len = to_test.raw().length();
  for (int i = 0; i < _uids->length(); i++) {
    const machine_uid &curr = _uids->get(i);
    // test for the length first since that's cheaper.
    if ( (test_len == curr.raw().length()) && (curr == to_test) ) {
      return true;
    } else if (!to_test.valid() && !curr.valid()) return true;
      // both are invalid, so the item to find is present.

/*
      if (!to_test.valid() || !curr.valid()) continue;
        // one is invalid but the other is not; not a match.
all bogus!
//hmmm: weird partial matching being allowed here.
      bool to_return = true;  // assume it's good until told otherwise.
      // if the first parts of the addresses agree, then assume we're
      for (int j = 0; j < minimum(test_len, curr.raw().length()); j++) {
        if (curr.raw().get(j) != to_test.raw().get(j)) {
          to_return = false;
        }
      }
      if (to_return)
        return true;
    }
*/
  }
  return false;
}

//////////////

internet_machine_uid::internet_machine_uid(const astring &hostname,
    const byte_array &ip_address)
{
  byte_array addr(ip_address);
  basis::un_short fletch = checksums::fletcher_checksum((const abyte *)hostname.observe(),
      hostname.length());
  structures::attach(addr, fletch);
  reset(machine_uid::TCPIP_LOCATION, addr);
}

} //namespace.

