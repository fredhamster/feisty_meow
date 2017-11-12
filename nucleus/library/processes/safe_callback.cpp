


/*****************************************************************************\
*                                                                             *
*  Name   : safe_callback                                                     *
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

#include "safe_callback.h"

#include <basis/guards.h>
#include <basis/astring.h>
#include <basis/mutex.h>
#include <loggers/critical_events.h>
#include <structures/byte_hasher.h>
#include <structures/hash_table.h>
#include <structures/static_memory_gremlin.h>

using namespace basis;
using namespace loggers;
using namespace structures;

namespace processes {

//////////////

callback_data_block::~callback_data_block() {}

//////////////

class live_object_info
{
public:
  int _references;  // the number of times it's been added to the list.

  live_object_info() : _references(1) {}
};

//////////////

static bool _live_objects_are_gone = false;
  // flags when the global_live_objects singleton winks out of existence.  this
  // should prevent ordering conflicts during the static destructions.

class global_live_objects : public virtual root_object
{
public:
  global_live_objects() : _objects(rotating_byte_hasher(), 12) {}
    // note that we have about a 2 billion callback object limit currently.

  ~global_live_objects() { _live_objects_are_gone = true; }

  DEFINE_CLASS_NAME("global_live_objects");

  // returns true if the "object" is listed as valid.
  bool listed(void *object) {
    auto_synchronizer l(_lock);
    live_object_info *loi = NULL_POINTER;
    return _objects.find(object, loi);
  }

  // adds the "object" to the list, or if it's already there, ups the refcount.
  void add(void *object) {
    auto_synchronizer l(_lock);
    live_object_info *loi = NULL_POINTER;
    if (!_objects.find(object, loi)) {
      // this is a new item.
      _objects.add(object, new live_object_info);
      return;
    }
    // this item already exists.
    loi->_references++;
  }

  // reduces the refcount on the "object" and removes it if there are zero
  // references.
  void remove(void *object) {
    auto_synchronizer l(_lock);
    live_object_info *loi = NULL_POINTER;
    if (!_objects.find(object, loi)) {
      // this item doesn't exist???  bad usage has occurred..
      return;
    }
    // patch its reference count.
    loi->_references--;
    if (!loi->_references) {
      // ooh, it croaked.  get rid of it now.
      _objects.zap(object);
    }
  }

private:
  mutex _lock;  // protects our list.
  hash_table<void *, live_object_info> _objects;
    // the valid objects are listed here.
};

//////////////

safe_callback::safe_callback()
: _decoupled(false),
  _callback_lock(new mutex)
{ begin_availability(); }

safe_callback::~safe_callback()
{
  if (!_decoupled)
    non_continuable_error(class_name(), "destructor",
        "the derived safe_callback has not called end_availability() yet.\r\n"
        "this violates caveat two of safe_callback (see header).");
  WHACK(_callback_lock);
}

SAFE_STATIC(global_live_objects, safe_callback::_invocables, )

void safe_callback::begin_availability()
{
  // we don't lock the mutex here because there'd better not already be
  // a call to the callback function that happens while the safe_callback
  // object is still being constructed...!

  _decoupled = false;  // set to be sure we know we ARE hooked in.
  if (_live_objects_are_gone) return;  // hosed.
  _invocables().add(this);  // list this object as valid.
}

void safe_callback::end_availability()
{
  if (_decoupled) return;  // never unhook any one safe_callback object twice.
  if (_live_objects_are_gone) {
    // nothing to unlist from.
    _decoupled = true;
    return;
  }
  _callback_lock->lock();  // protect access to this object.
  _invocables().remove(this);  // unlist this object.
  _decoupled = true;  // we are now out of the action.
  _callback_lock->unlock();  // release lock again.
  // we shoot the lock here so that people hear about it immediately.  they
  // will then be released from their pending but failed callback invocations.
  WHACK(_callback_lock);
}

bool safe_callback::invoke_callback(callback_data_block &new_data)
{
  auto_synchronizer l(*_callback_lock);
    // we now have the lock.
  if (!_invocables().listed(this)) return false;
    // this object is no longer valid, so we must not touch it.
  if (_decoupled) return false;
    // object is partially decoupled already somehow.  perhaps this instance
    // has already been connected but another hook-up exists at some place
    // else in the derivation hierarchy.  they'd better be careful to shut
    // down all the safe_callbacks before proceeding to destroy...
    // if they do that, they're fine, since the shutdown of any owned data
    // members would be postponed until after all the callbacks had been
    // removed.
  real_callback(new_data);
  return true;
}

} //namespace.



