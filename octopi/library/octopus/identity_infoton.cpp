/*****************************************************************************\
*                                                                             *
*  Name   : identity_infoton                                                  *
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

#include "identity_infoton.h"
#include "tentacle.h"

#include <basis/byte_array.h>
#include <basis/mutex.h>
#include <structures/string_array.h>
#include <structures/static_memory_gremlin.h>

using namespace basis;
using namespace structures;

namespace octopi {

identity_infoton::identity_infoton()
: infoton(identity_classifier()),
  _new_name()
{}

identity_infoton::identity_infoton(const octopus_entity &uid)
: infoton(identity_classifier()),
  _new_name(uid)
{}

identity_infoton::identity_infoton(const identity_infoton &to_copy)
: root_object(),
  infoton(to_copy),
  _new_name(to_copy._new_name)
{}

identity_infoton::~identity_infoton() {} 

void identity_infoton::text_form(base_string &fill) const
{
  astring ent_info;
  _new_name.text_form(ent_info);
  fill.assign(astring("entity=") + ent_info);
}

identity_infoton &identity_infoton::operator =(const identity_infoton &to_copy)
{
  if (this == &to_copy) return *this;
  set_classifier(to_copy.classifier());
  _new_name = to_copy._new_name;
  return *this;
}

const astring identity_classifier_strings[] = { "#octide" };

SAFE_STATIC_CONST(string_array, identity_infoton::identity_classifier,
    (1, identity_classifier_strings))

int identity_infoton::packed_size() const { return _new_name.packed_size(); }

clonable *identity_infoton::clone() const
{ return cloner<identity_infoton>(*this); }

void identity_infoton::pack(byte_array &packed_form) const
{ _new_name.pack(packed_form); }

bool identity_infoton::unpack(byte_array &packed_form)
{
  if (!_new_name.unpack(packed_form)) return false;
  return true;
}

} //namespace.


