/*****************************************************************************\
*                                                                             *
*  Name   : list_manager                                                      *
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

#include "bundle_list.h"
#include "list_manager.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/mutex.h>
#include <structures/string_array.h>

using namespace basis;
using namespace octopi;
using namespace structures;
using namespace timely;

namespace synchronic {

//#define DEBUG_LIST_MANAGER
  // uncomment for noisier version.

#undef GRAB_LOCK
#define GRAB_LOCK \
  auto_synchronizer l(*_locking)

#undef LOG
#define LOG(to_print) \
  CLASS_EMERGENCY_LOG(program_wide_logger::get(), to_print)

list_manager::list_manager(const string_array &list_name, bool backgrounded)
: tentacle(list_name, backgrounded),
  _entries(new bundle_list),
  _locking(new mutex)
{
}

list_manager::~list_manager()
{
  WHACK(_entries);
  WHACK(_locking);
}

const string_array &list_manager::list_name() const { return group(); }

int list_manager::entries() const
{
  GRAB_LOCK;
  return _entries->elements();
}

void list_manager::reset()
{
  GRAB_LOCK;
  _entries->zap(0, _entries->elements() - 1);
}

bool list_manager::is_listed(const string_array &classifier)
{
  GRAB_LOCK;
  int indy = locked_find(classifier);
  return !negative(indy);
}

bool list_manager::update(const string_array &classifier, int offset)
{
  GRAB_LOCK;
  int indy = locked_find(classifier);
  if (negative(indy)) return false;  // not found.
  _entries->borrow(indy)->_updated = time_stamp(offset);
  return true;
}

void list_manager::clean(int older_than)
{
  GRAB_LOCK;
  for (int i = 0; i < _entries->elements(); i++) {
    synchronizable *curr = _entries->borrow(i);
    if (curr->_updated < time_stamp(-older_than)) {
      // this one is too old to keep around.
      _entries->zap(i, i);
      i--;  // skip back before deleted item.
    }
  }
}

bool list_manager::zap(const string_array &classifier)
{
  GRAB_LOCK;
  int indy = locked_find(classifier);
  if (negative(indy)) return false;  // not found.
  _entries->zap(indy, indy);
  return true;  // did find and whack it.
}

int list_manager::locked_find(const string_array &classifier)
{
  for (int i = 0; i < _entries->elements(); i++) {
    // check that the classifier lengths are equal; otherwise no match.
    if (_entries->get(i)->classifier().length() != classifier.length())
      continue;
    // starting from the end of most significance, we compare the strings.
    // we don't want to bother comparing the end that's most likely to be
    // the same for items in the list (the front, that is).
    bool problems = false;
    for (int j = classifier.length() - 1; j >= 0; j--) {
      if (_entries->get(i)->classifier()[j] != classifier[j]) {
        problems = true;
        break;  // get out now since we're hosed.
      }
    }
    if (problems) continue;  // nope, there was a mismatch.
    // success; this guy matches.
    return i;
  }
  return common::NOT_FOUND;  // not found.
}

synchronizable *list_manager::clone_object(const string_array &classifier)
{
  GRAB_LOCK;
  int indy = locked_find(classifier);
  if (negative(indy)) return NIL;
  return dynamic_cast<synchronizable *>(_entries->get(indy)->clone());
}

void list_manager::retrieve(bundle_list &to_fill) const
{
  to_fill.reset();
  GRAB_LOCK;
  for (int i = 0; i < _entries->elements(); i++)
    to_fill += dynamic_cast<synchronizable *>(_entries->get(i)->clone());
}

outcome list_manager::consume(infoton &to_chow,
    const octopus_request_id &formal(item_id), byte_array &transformed)
{
#ifdef DEBUG_LIST_MANAGER
  FUNCDEF("consume");
#endif
  transformed.reset();
  synchronizable *bun = dynamic_cast<synchronizable *>(&to_chow);
  if (!bun) return BAD_INPUT;

  GRAB_LOCK;

  // now perform an appropriate action depending on the type of update.
  switch (bun->_mod) {
    case synchronizable::ADDED:
    case synchronizable::CHANGED: {
      // see if the item already exists; if it does, overwrite it.
      int indy = locked_find(bun->classifier());
      if (negative(indy)) {
        // the item is new, so just drop it in the list.
        *_entries += dynamic_cast<synchronizable *>(bun->clone());
      } else {
        // not a new item, so merge with the existing contents.
        _entries->borrow(indy)->merge(*bun);
        _entries->borrow(indy)->_updated = time_stamp();
      }
      return OKAY;
    }
    case synchronizable::DELETED: {
      int indy = locked_find(bun->classifier());
      if (non_negative(indy)) {
        // found it, so whack the entry as needed by calling merge.
        outcome ret = _entries->borrow(indy)->merge(*bun);
        _entries->borrow(indy)->_updated = time_stamp();
        if (ret == synchronizable::EMPTY) {
          // they have told us that this must go now.
#ifdef DEBUG_LIST_MANAGER
          LOG(astring("removing entry now due to merge outcome: ")
              + _entries->borrow(indy)->text_form());
#endif
          _entries->zap(indy, indy);
        }
        return OKAY;
      } else {
        // that item was not listed.
#ifdef DEBUG_LIST_MANAGER
        LOG(astring("could not find entry for ") + bun->text_form());
#endif
        return NOT_FOUND;
      }
      break;
    }
    default: return NO_HANDLER;
  }
  return OKAY;
}

void list_manager::expunge(const octopus_entity &formal(to_remove))
{
  FUNCDEF("expunge");
}

} //namespace.

