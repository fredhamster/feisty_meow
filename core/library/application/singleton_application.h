#ifndef SINGLETON_APPLICATION_CLASS
#define SINGLETON_APPLICATION_CLASS

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

//! Implements an application lock to ensure only one is running at once.
/*!
  This encapsulates the code used to ensure that only one copy of an
  application is running at a time.  It can either be made specific to a
  user (so that user can run only one at a time) or made global to the entire
  machine.
*/

#include <basis/astring.h>
#include <basis/contracts.h>
#include <processes/rendezvous.h>

namespace application {

class singleton_application
{
public:
  singleton_application(const basis::astring &application_name,
          const basis::astring &user_name = basis::astring::empty_string());
    //!< constructs a singleton guardian for the "application_name".
    /*!< if the "user_name" is non-empty, then it will be used as part of
    the lock name.  this ensures that separate users can still run the
    application at the same time, which is especially important for terminal
    servers.  for a lock that should affect the whole machine and ignore
    users, the "user_name" must be empty. */

  virtual ~singleton_application();

  DEFINE_CLASS_NAME("singleton_application");

  bool already_tried() const;
    //!< returns true if the singleton has already tried to lock.

  bool allow_this_instance();
    //!< the application must check this before starting up.
    /*!< if this returns false, then the application must exit immediately or
    it will be violating the singleton rule. */

  bool already_running();
    //!< returns false if this program is not already running.
    /*!< this is just the opposite of the allow_this_instance() method. */

  void release_lock();
    //!< let's go of the application lock, if we had previously gotten it.
    /*!< this should only be called when the application is about to exit. */

private:
  int c_initial_try;  //!< has the initial locking attempt been made?
    /* if c_initial_try is zero, no attempt made yet.  if it's 1, then tried
    and succeeded.  if it's greater than one, then tried and failed. */
  processes::rendezvous *_app_lock;  //!< used to lock out other instances.
  bool _got_lock;  //!< records whether we successfully got the lock or not.
};

} //namespace.

#endif

