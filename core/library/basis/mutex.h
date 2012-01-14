#ifndef MUTEX_CLASS
#define MUTEX_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : mutex                                                             *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1996-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "contracts.h"

//! A simple primitive class that encapsulates OS support for mutual exclusion.
/*!
  The word "mutex" is an abbreviation for "mutual exclusion".  The mutex
  provides a simple synchronization object that supports the programming of
  critical sections.  It is guaranteed to be safe for threads, but it is only
  useful within one application rather than between multiple applications.
  The mutex_base is hardly ever used directly; instead the mutex class should
  be used.
*/

namespace basis {

class mutex : public virtual base_synchronizer
{
public:
  mutex();  //!< Constructs a new mutex.

  virtual ~mutex();
    //!< Destroys the mutex.  It should not be locked upon destruction.

  //! Constructor for use with malloc/free instead of new/delete.
  void construct();

  //! Destructor for use with malloc/free instead of new/delete.
  void destruct();

  void lock();
    //!< Clamps down on the mutex, if possible.
    /*!< Otherwise the current thread is blocked until the mutex is unlocked. */

  void unlock();
    //!< Gives up the possession of the mutex.

  virtual void establish_lock();
    //!< Satisfies base class requirements for locking.
  virtual void repeal_lock();
    //!< Satisfies base class requirements for unlocking.

private:
  void *c_os_mutex;  //!< OS version of the mutex.

  void defang();
    //!< Removes the underlying OS synchronization primitive.
    /*!< This method renders this mutex object inoperable.  This is useful
    when the reason for the lock has vanished, but the mutex object cannot be
    deleted yet.  Sometimes it may still be referred to, but there is no
    longer any critical section to be protected. */

  mutex(const mutex &);  //!< not allowed.
  mutex &operator =(const mutex &);  //!< not allowed.
};

//////////////

//! auto_synchronizer simplifies concurrent code by automatically unlocking.
/*!
  This relies on the base_synchronizer for the form of the objects that
  will provide synchronization.  The synchronization object is locked when the
  auto_synchronizer is created and it is unlocked when the auto_synchronizer
  is destroyed.  This is most useful when the auto_synchronizer is an automatic
  object; the synchronization lock is grabbed at the point of creation (even
  in the middle of a function) and it is released when the function or block
  scope exits, thus freeing us of the responsibility of always unlocking the
  object before exiting from the critical section.

  More Detail:
  The auto_synchronizer provides an easy way to provide nearly idiot-proof
  synchronization of functions that share the same locking object.  By giving
  the synchronizer a working object that's derived from synchronization_base,
  its mere construction establishes the lock and its destruction releases the
  lock.  Thus you can protect a critical section in a function by creating
  the auto_synchronizer at the top of the function as an automatic object, or
  wherever in the function body is appropriate.  When the function exits, the
  auto_synchronizer will be destroyed as part of the cleanup and the lock will
  be released.  If there are multiple synchronization objects in a function,
  then be very careful.  One must order them appropriately to avoid a deadlock.
 
  for example: @code
    mutex my_lock;  // the real synchronization primitive.
    ...  // lots of program in between.
    int calculate_average() {
      // our function that must control thread concurrency.
      auto_synchronizer syncho(my_lock);  // establishes the lock.
      ...  // lots of stuff done in the function in safety from other threads.
    } // end of the function. @endcode
 
  Note that there was no unlock of the mutex above.  Remembering to unlock
  synchronization primitives is one of the most troublesome requirements of
  programming with multiple threads; the auto_synchronizer can be used in
  many situations to automate the release of the lock.
*/

class auto_synchronizer
{
public:
  auto_synchronizer(base_synchronizer &locker) : _locker(locker)
          { _locker.establish_lock(); }
    //!< Construction locks the "locker" object for the current program scope.
    /*!< This automatically locks a synchronization object until the current
    scope (such as a function or even just a block) is exited, which implements
    synchronization without needing multiple unlock calls before every return
    statement. */

  ~auto_synchronizer() { _locker.repeal_lock(); }
    //!< Releases the lock as this object goes out of scope.

private:
  base_synchronizer &_locker;  //!< the locking object.

  // disallowed.
  auto_synchronizer(const auto_synchronizer &locker);
  auto_synchronizer &operator =(const auto_synchronizer &locker);
};

} //namespace.

#endif

