/*****************************************************************************\
*                                                                             *
*  Name   : safe_roller                                                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1998-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "safe_roller.h"

#include <basis/functions.h>
#include <basis/mutex.h>
#include <structures/roller.h>
#include <structures/static_memory_gremlin.h>

using namespace basis;
using namespace structures;

namespace processes {

SAFE_STATIC(mutex, __roller_synch, )

void safe_add(int &to_change, int addition)
{
  auto_synchronizer l(__roller_synch());
  to_change += addition;
}

//////////////

safe_roller::safe_roller(int start_of_range, int end_of_range)
: _rolling(new int_roller(start_of_range, end_of_range)),
  _lock(new mutex)
{
}

safe_roller::~safe_roller()
{
  WHACK(_rolling);
  WHACK(_lock);
}

int safe_roller::next_id()
{
  _lock->lock();
  int to_return = _rolling->next_id();
  _lock->unlock();
  return to_return;
}

int safe_roller::current() const
{
  _lock->lock();
  int to_return = _rolling->current();
  _lock->unlock();
  return to_return;
}

void safe_roller::set_current(int new_current)
{
  _lock->lock();
  _rolling->set_current(new_current);
  _lock->unlock();
}

} //namespace.


