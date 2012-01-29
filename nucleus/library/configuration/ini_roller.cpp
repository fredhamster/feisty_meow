/*****************************************************************************\
*                                                                             *
*  Name   : ini_roller                                                        *
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

#include "ini_roller.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/mutex.h>
///#include <structures/configurator.h>

using namespace basis;
using namespace structures;

namespace configuration {

//#define DEBUG_ID_GRANTING
  // uncomment if you want verbose granting of unique ids.

const int ID_FACTOR = 28;
  // this many ids are grabbed at once for eventual issuance.

ini_roller::ini_roller(configurator &config, const astring &section,
    const astring &entry, int min, int max)
: _ini(config),
  _ids(new int_roller(min, max)),
  _section(new astring(section)),
  _entry(new astring(entry)),
  _lock(new mutex)
{
  int current = _ini.load(section, entry, min);
  _ids->set_current(current);
  // make our first requisition of ids.  we start here rather than playing
  // games with the next_id function.
  _ini.store(section, entry, _ids->current() + ID_FACTOR);
}

ini_roller::~ini_roller()
{
  // force the id to be past what we've allocated, but not too far past.
  _ini.store(*_section, *_entry, _ids->current() + 1);
  WHACK(_ids);
  WHACK(_section);
  WHACK(_entry);
  WHACK(_lock);
}

int ini_roller::current_id() const
{
  auto_synchronizer l(*_lock);
  return _ids->current();
}

int ini_roller::next_id()
{
#ifdef DEBUG_ID_GRANTING
  FUNCDEF("next_id");
#endif
  auto_synchronizer l(*_lock);
  int to_return = _ids->current();

  // this uses a relaxed id issuance policy; the id that's in the INI
  // file is only updated when we run out of the range that we allocate for it.
  // the roller's current value is used whenever issuing an id, but next_id()
  // is always called before that id is actually issued.

  if ( (_ids->current() < _ids->maximum() - 2) 
      && (_ids->current() % ID_FACTOR) ) {
    // no id range grabbing needed yet and no rollover.
    _ids->next_id();
#ifdef DEBUG_ID_GRANTING
    LOG(astring(astring::SPRINTF, "standard id issue: %d.", to_return));
#endif
    return to_return;
  }

  // now we need to allocate a new range of ids...  and store in ini.
  int new_range = to_return + ID_FACTOR;
#ifdef DEBUG_ID_GRANTING
  LOG(astring(astring::SPRINTF, "finding next range, new start in ini "
      "is: %d.", new_range));
#endif
  // if the id wraps around, reset it.
  if ( (new_range < 0) || (new_range >= _ids->maximum()) )
    new_range = ID_FACTOR;
#ifdef DEBUG_ID_GRANTING
  LOG(astring(astring::SPRINTF, "after check, new ini id is: %d.",
      new_range));
#endif
  _ini.store(*_section, *_entry, new_range);
    // set the next stored id to the block above where we're using.
  _ids->next_id();  // jump to the next one in the range.
#ifdef DEBUG_ID_GRANTING
  LOG(astring(astring::SPRINTF, "after store, id is: %d.", to_return));
#endif
  return to_return;
}

} //namespace.

