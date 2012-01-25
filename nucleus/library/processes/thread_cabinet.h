#ifndef THREAD_CABINET_CLASS
#define THREAD_CABINET_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : thread_cabinet                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

// forward.
#include "ethread.h"

#include <basis/mutex.h>
#include <structures/roller.h>
#include <structures/set.h>
#include <structures/unique_id.h>

namespace processes {

class thread_amorph;

//! Manages a collection of threads.
/*!
  The thread cabinet allows one to corral a bunch of threads in one place
  and treat them as a group if necessary.
*/

class thread_cabinet
{
public:
  thread_cabinet();
  virtual ~thread_cabinet();

  DEFINE_CLASS_NAME("thread_cabinet");

  structures::unique_int add_thread(ethread *to_add, bool start_it, void *parm);
    //!< adds a thread to be managed by the thread_cabinet.
    /*!< the thread cabinet takes over responsibility for the thread "to_add".
    if "start_it" is true, then the thread is started.  otherwise it is
    left in whatever state it was in.  the "parm" is passed to the thread's
    start() method. */

  bool zap_thread(const structures::unique_int &to_whack);
    //!< removes the thread with the id "to_whack".
    /*!< if it's found and stopped and removed, true is returned.  note that
    if the thread is found, then this will wait until the thread exits before
    whacking it. */

  bool cancel_thread(const structures::unique_int &to_cancel);
    //!< shuts down the thread "to_cancel" as quickly as possible.
    /*!< this calls the cancel() method on the thread "to_cancel", which tells
    the thread to stop as soon as possible.  once it has stopped, the
    clean_debris() method will throw it and other stopped threads out. */

  int threads() const;  //!< number of threads being managed here.

  structures::int_set thread_ids() const;
    //!< returns the identifiers of all threads managed by this object.

  bool any_running() const;
    //!< returns true if any threads are currently running.

  void start_all(void *pointer);
    //!< cranks up any threads that are not already running.
    /*!< the "pointer" will be provided to any threads that are started. */

  void cancel_all();
    //!< signals to all threads that they should exit as soon as possible.
    /*!< this does not actually wait for them to exit. */

  void stop_all();
    //!< makes all of the threads quit.
    /*!< they are cleaned up after they have stopped running also.  any
    attempts to add threads while this method is operating will be rejected. */

  ethread *get_thread(int index);
    //!< this returns the thread at "index" in our list.
    /*!< note that this is not safe to use if other threads could be removing
    threads from the cabinet or calling clean_debris(). */

  void clean_debris();
    //!< clean out threads that have finished.
    /*!< note that if threads were added to the list without starting them,
    then these get cleaned out also. */

private:
  basis::mutex *_lock;  //!< synchronizes our thread list.
  thread_amorph *_threads;  //!< the list of threads we're managing.
  structures::int_roller *_next_id;  //!< the id to be assigned to the next thread.
  int _no_additions;  //!< true if we're not allowed to add threads currently.
};

} //namespace.

#endif

