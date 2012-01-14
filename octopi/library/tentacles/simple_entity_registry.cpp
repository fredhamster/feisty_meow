/*****************************************************************************\
*                                                                             *
*  Name   : simple_entity_registry                                            *
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

#include "simple_entity_registry.h"

#include <basis/mutex.h>
#include <structures/string_hash.h>
#include <octopus/entity_defs.h>

using namespace basis;
using namespace octopi;
using namespace structures;
using namespace timely;

namespace octopi {

#undef GRAB_LOCK
#define GRAB_LOCK \
  auto_synchronizer l(*_secure_lock)

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s);

const int ENTITY_HASH_BITS = 8;
  // the hash table for entities will be 2^N wide.

// this record is stored for each verified entity.  if such a record exists,
// then the entity has passed through whatever security system is installed
// in the octopus.

//////////////

class recognized_entity
{
public:
  octopus_entity _entity;    // the identifier for this entity.
  time_stamp _last_active;   // when was this entity last active?
  byte_array _verification;  // verification information sent by entity.
};

//////////////

class recognized_entity_list : public string_hash<recognized_entity>
{
public:
  recognized_entity_list() : string_hash<recognized_entity>(ENTITY_HASH_BITS) {}
};

//////////////

simple_entity_registry::simple_entity_registry()
: _secure_lock(new mutex),
  _entities(new recognized_entity_list)
{
}

simple_entity_registry::~simple_entity_registry()
{
  WHACK(_entities);
  WHACK(_secure_lock);
}

bool simple_entity_registry::authorized(const octopus_entity &entity)
{
  GRAB_LOCK;
  recognized_entity *found = _entities->find(entity.mangled_form());
  return !!found;
}

bool simple_entity_registry::refresh_entity(const octopus_entity &entity)
{
  GRAB_LOCK;
  recognized_entity *found = _entities->find(entity.mangled_form());
  if (!found) return false;
  // we found their record so update that time stamp.
  found->_last_active = time_stamp();
  return true;
}

bool simple_entity_registry::add_entity(const octopus_entity &entity,
    const byte_array &verification)
{
  GRAB_LOCK;
  recognized_entity *found = _entities->find(entity.mangled_form());
  if (found) {
    // already had a record for this guy so update the time stamp.
    found->_last_active = time_stamp();
    // if there's a verification token, make sure we keep their most recent
    // version of it.
    if (verification.length())
      found->_verification = verification;
  } else {
    // this is a new entity, so add a new entity record for it.
    recognized_entity *new_one = new recognized_entity;
    new_one->_entity = entity;
    new_one->_verification = verification;
    _entities->add(entity.mangled_form(), new_one);
  }
  return true;
}

bool simple_entity_registry::zap_entity(const octopus_entity &entity)
{
  GRAB_LOCK;
  return _entities->zap(entity.mangled_form());
}

bool simple_entity_registry::locate_entity(const octopus_entity &entity,
    time_stamp &last_active, byte_array &verification)
{
  GRAB_LOCK;
  recognized_entity *found = _entities->find(entity.mangled_form());
  if (!found) return false;
  last_active = found->_last_active;
  verification = found->_verification;
  return true;
}

bool text_form_applier(const astring &formal(key), recognized_entity &info,
    void *datalink)
{
  astring *accum = (astring *)datalink;
  *accum += astring("ent=") + info._entity.mangled_form() + ", active="
      + info._last_active.text_form()
      + a_sprintf(", %d veribytes", info._verification.length());
  return true;
}

astring simple_entity_registry::text_form()
{
  astring to_return;
  GRAB_LOCK;
  _entities->apply(text_form_applier, &to_return);
  return to_return;
}

} //namespace.

