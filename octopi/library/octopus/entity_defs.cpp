/*****************************************************************************\
*                                                                             *
*  Name   : entity definitions for octopus                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "entity_defs.h"

#include <configuration/application_configuration.h>
#include <mathematics/chaos.h>
#include <structures/amorph.h>
#include <structures/static_memory_gremlin.h>
#include <textual/byte_formatter.h>
#include <textual/parser_bits.h>
#include <textual/string_manipulation.h>

#include <stdio.h>  // for sscanf.

using namespace basis;
using namespace configuration;
using namespace mathematics;
using namespace textual;
using namespace structures;

namespace octopi {

octopus_entity::octopus_entity()
: _hostname(new astring),
  _pid(0),
  _sequencer(0),
  _add_in(0)
{}

octopus_entity::octopus_entity(const astring &hostname,
    int process_id, int sequencer, int add_in)
: _hostname(new astring(hostname)),
  _pid(process_id),
  _sequencer(sequencer),
  _add_in(add_in)
{}

octopus_entity::octopus_entity(const octopus_entity &to_copy)
: packable(),
  _hostname(new astring)
{ operator = (to_copy); }

octopus_entity::~octopus_entity() { WHACK(_hostname); }

octopus_entity octopus_entity::from_text(const astring &to_convert)
{
  astring host;
  int process_id;
  int sequencer;
  int add_in;
  breakout(to_convert, host, process_id, sequencer, add_in);

  byte_array temp_form;
  byte_formatter::string_to_bytes(host, temp_form);
  host = astring((char *)temp_form.observe());

  return octopus_entity(host, process_id, sequencer, add_in);
}

octopus_entity &octopus_entity::operator =(const octopus_entity &to_copy)
{
  if (this == &to_copy) return *this;
  *_hostname = *to_copy._hostname;
  _pid = to_copy._pid;
  _sequencer = to_copy._sequencer;
  _add_in = to_copy._add_in;
  return *this;
}

const astring &octopus_entity::hostname() const { return *_hostname; }

int octopus_entity::process_id() const { return _pid; }

int octopus_entity::sequencer() const { return _sequencer; }

int octopus_entity::add_in() const { return _add_in; }

void octopus_entity::process_id(int id) { _pid = id; }

void octopus_entity::sequencer(int seq) { _sequencer = seq; }

void octopus_entity::add_in(int add) { _add_in = add; }

void octopus_entity::hostname(const astring &new_host)
{ *_hostname = new_host; }

bool octopus_entity::blank() const
{ return !_sequencer && !_add_in && !_pid && _hostname->empty(); }

int octopus_entity::packed_size() const
{ return sizeof(int) * 3 + _hostname->length() + 1; }

#define REPLACEMENT_CHARACTER '#'
  // used to replace unprintable characters in the entity text_form().

astring octopus_entity::text_form() const
{
  astring chewed_host = *_hostname;
  // make sure the name we're going to show is totally printable.  some
  // users of entities have odd notions of hostname strings.  there are no
  // rules requiring these to be readable normal strings.
  for (int i = 0; i < chewed_host.length(); i++) {
    if (!parser_bits::is_printable_ascii(chewed_host[i]))
      chewed_host[i] = REPLACEMENT_CHARACTER;
  }
  return a_sprintf("%d.%d.%d.%s", _add_in, _sequencer, _pid, chewed_host.s());
}

astring octopus_entity::mangled_form() const
{
  astring hostdump;
  byte_array temp_form(_hostname->length() + 1, (abyte *)_hostname->observe());
  byte_formatter::bytes_to_string(temp_form, hostdump, false);
  return a_sprintf("%d.%d.%d.%s", _add_in, _sequencer, _pid, hostdump.s());
}

void octopus_entity::breakout(const astring &mangled_form, astring &hostname,
    int &process_id, int &sequencer, int &add_in)
{
  // there is pretty much no error checking here.
  astring dupe = mangled_form;  // allows us to destroy the id.
  sscanf(dupe.s(), "%d", &add_in);
  dupe.zap(0, dupe.find('.'));
  sscanf(dupe.s(), "%d", &sequencer);
  dupe.zap(0, dupe.find('.'));
  sscanf(dupe.s(), "%d", &process_id);
  dupe.zap(0, dupe.find('.'));
  hostname = dupe;
}

bool octopus_entity::operator == (const octopus_entity &that) const
{
  return (_add_in == that._add_in)
      && (_pid == that._pid)
      && (_sequencer == that._sequencer)
      && (*_hostname == *that._hostname);
}

void octopus_entity::pack(byte_array &packed_form) const
{
  _hostname->pack(packed_form);
  attach(packed_form, _pid);
  attach(packed_form, _sequencer);
  attach(packed_form, _add_in);
}
       
bool octopus_entity::unpack(byte_array &packed_form)
{
  if (!_hostname->unpack(packed_form)) return false;
  if (!detach(packed_form, _pid)) return false;
  if (!detach(packed_form, _sequencer)) return false;
  if (!detach(packed_form, _add_in)) return false;
  return true;
}

//////////////

int octopus_request_id::packed_size() const
{ return _entity.packed_size() + sizeof(int); }

astring octopus_request_id::mangled_form() const
{ return _entity.mangled_form() + a_sprintf("/%d", _request_num); }

astring octopus_request_id::text_form() const
{ return _entity.text_form() + a_sprintf("/%d", _request_num); }

bool octopus_request_id::blank() const
{ return _entity.blank() || !_request_num; }

octopus_request_id octopus_request_id::randomized_id()
{
  chaos randona;
  return octopus_request_id(
      octopus_entity(string_manipulation::make_random_name(),
          application_configuration::process_id(), randona.inclusive(0, MAXINT32 / 2),
          randona.inclusive(0, MAXINT32 / 2)),
      randona.inclusive(0, MAXINT32 / 2));
}

octopus_request_id octopus_request_id::from_text(const astring &to_convert)
{
  // find the slash, starting at the end.
  int indy = to_convert.find('/', to_convert.end(), true);
  if (negative(indy)) return octopus_request_id();  // bad format.
  astring partial = to_convert.substring(0, indy - 1);
  int req_id = to_convert.substring(indy + 1, to_convert.end()).convert(0);
  return octopus_request_id(octopus_entity::from_text(partial), req_id);
}

void octopus_request_id::pack(byte_array &packed_form) const
{
  _entity.pack(packed_form);
  attach(packed_form, _request_num);
}

bool octopus_request_id::unpack(byte_array &packed_form)
{
  if (!_entity.unpack(packed_form)) return false;
  if (!detach(packed_form, _request_num)) return false;
  return true;
}

} // namespace.

