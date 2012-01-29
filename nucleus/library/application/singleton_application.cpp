/*****************************************************************************\
*                                                                             *
*  Name   : singleton_application                                             *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2006-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "singleton_application.h"

#include <basis/functions.h>
#include <filesystem/filename.h>

using namespace basis;
using namespace filesystem;
using namespace processes;

namespace application {

singleton_application::singleton_application(const astring &application_name,
    const astring &user_name)
: c_initial_try(0),
  _app_lock(new rendezvous(filename(application_name).basename().raw()
      + "__" + user_name)),
  _got_lock(false)
{
}

singleton_application::~singleton_application()
{
  release_lock();
  WHACK(_app_lock);
}

bool singleton_application::already_running()
{ return !allow_this_instance(); }

bool singleton_application::already_tried() const
{ return c_initial_try > 0; }

void singleton_application::release_lock()
{
  if (_got_lock) {
    _app_lock->unlock();
    _got_lock = false;
  }
}

bool singleton_application::allow_this_instance()
{
  if (_got_lock) return true;  // already grabbed it.

  if (!already_tried()) {
    _got_lock = _app_lock->lock(rendezvous::IMMEDIATE_RETURN);
    if (_got_lock) c_initial_try = 1;  // succeeded in locking.
    else c_initial_try = 2;  // failure.
  }

  return _got_lock;
}

} //namespace.

