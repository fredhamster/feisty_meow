#ifndef RENDEZVOUS_CLASS
#define RENDEZVOUS_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : rendezvous                                                        *
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

#include <basis/astring.h>
#include <basis/contracts.h>

namespace processes {

//! An inter-process synchronization primitive.
/*!
  A lock can be created that only one process owns at a time; those that do
  not acquire the lock can either return immediately or wait until the current
  lock owner releases the rendezvous.  This is unlike the mutex object in
  basis, since mutexes only synchronize within the same application.  The
  rendezvous can instead allow synchronization of resources between
  applications, but this comes at a higher cost per usage.
*/

class rendezvous : public virtual basis::base_synchronizer
{
public:
  rendezvous(const basis::astring &root_name);
    //!< the healthy() method should be checked to ensure creation succeeded.

  virtual ~rendezvous();
    //!< any lock held is released and the lower level structures freed.

  DEFINE_CLASS_NAME("rendezvous");

  bool healthy() const;
    //!< returns true if the rendezvous object is operable.
    /*!< there are cases where creation of the rendezvous might fail; they
    can be trapped here. */

  //! different ways that the lock() attempt can be made.
  enum locking_methods { NO_LOCKING, ENDLESS_WAIT, IMMEDIATE_RETURN };

  bool lock(locking_methods how = ENDLESS_WAIT);
    //!< grab the lock, if possible.
    /*!< if this is not the first time locking the same rendezvous, that's
    fine as long as the number of unlocks matches the number of locks. */

  void unlock();
    //!< releases a previously acquired lock.

  // these two methods implement the synchronizer_base interface.
  virtual void establish_lock();
  virtual void repeal_lock();

  const basis::astring &root_name() const;
    //!< returns the root name passed in the constructor.

private:
  void *_handle;  //!< the real OS version of the inter-app lock.
  bool _locked;  //!< true if the lock was successful and still locked.
  basis::astring *_root_name;  //!< the identifier for this rendezvous.
};

} //namespace.

#endif

