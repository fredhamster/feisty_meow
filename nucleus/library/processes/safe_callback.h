#ifndef SAFE_CALLBACK_CLASS
#define SAFE_CALLBACK_CLASS

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

#include <basis/astring.h>
#include <basis/mutex.h>
#include <basis/contracts.h>

namespace processes {

// forward.
class callback_data_block;
class global_live_objects;

//! A reasonably easy way to make callbacks safe from shutdown ordering issues.
/*!
  If an object implementing an unsafe, standard callback is destroyed, then
  all of the pending callbacks on other threads are jeopardized.  Objects with
  unsafe objects will not fare well in a multi-threaded program at all.  This
  class ensures that no callback can ever occur on a dead object, but this
  promise is subject to the caveats below.

  Caveat One:

  if you have synchronization control over objects that you own, please
  DO NOT have them locked while your callback base class is being destroyed
  NOR when you call the end_availability() method.  allowing them to be locked
  at those times can result in a program deadlock.  ensuring this is a simple
  matter of having a properly formed destructor, as in caveat two.

  Caveat Two:

  an object that implements safe_callback MUST invoke the end_availability
  method as the FIRST thing in its destructor.  otherwise, if some portions
  of the object are shutdown before the safe_callback is stopped, then the
  active callback invocation can have the rug pulled out from under it and
  suddenly be working with bad objects.  here is an example of a safe
  destructor implementation: @code

    my_safe_callback::~my_safe_callback() {
      // safely revoke this object's listing before other destructions.
      end_availability();
      // destroy other objects now...
      WHACK(first_thing);  //...etc....
    } @endcode

  note also: if your object owns or is derived from more than one
  safe_callback, then ALL of those must have their end_availability methods
  invoked before ANY of the other objects owned can be destroyed.  we
  recommend against deriving from more than one safe_callback just for
  simplicity's sake.
*/

class safe_callback
{
public:
  safe_callback();
    //!< construction signifies that the callback is now in operation.

  void end_availability();
    //!< prepares to shut down this object.
    /*!< This removes the object from the list of available callbacks.  this
    MUST be called in a derived destructor BEFORE any other objects owned by
    the derived class are destroyed. */

  virtual ~safe_callback();
    //!< actually destroys the object.
    /*!< don't allow this to be called without having invoked
    end_availability() at the top of your derived class's destructor.
    otherwise, you have broken your code by failing caveat two, above. */

  DEFINE_CLASS_NAME("safe_callback");

  bool decoupled() const { return _decoupled; }
    //!< if true, then end_availability() was already invoked on the object.
    /*!< if this is returning true, then one can trust that the object is
    properly deregistered and safely decoupled from the callback.  a return of
    false means that the object still might be hooked into callbacks and it is
    not yet safe to destroy the object. */

  bool invoke_callback(callback_data_block &new_data);
    //!< this function is invoked by a user of the safe_callback derived object.
    /*!< this performs some safety checks before invoking the real_callback()
    method.  given the caveats are adhered to, we automatically ensure that the
    object to be called actually exists in a healthy state.  we also enforce
    that the called object cannot be destroyed until after any active
    callback invocation is finished, and that any pending callbacks are
    rejected once the object is invalid.
    true is returned if the safe callback was actually completed.  a false
    return indicates that the object had already shut down.  perhaps that's
    a sign that the caller should now remove the object if it hasn't already
    been removed externally...  certainly if the call is rejected more than
    once, there's no reason to keep invoking the callback.
    each safe_callback implements its own locking to ensure that the
    object is still alive.  thus, derived objects don't need to provide some
    separate proof of their health.  this also allows the derived object
    to mostly ignore callback-related locking in its own synchronization
    code (mostly--see caveats above). */

protected:
  virtual void real_callback(callback_data_block &new_data) = 0;
    //!< derived classes implement this to provide their callback functionality.
    /*!< this call will be prevented from ever occurring on an invalid "this"
    pointer (given the caveats above are adhered to). */

private:
  bool _decoupled;  //!< true if we have ended our availability.
  basis::mutex *_callback_lock;  //!< synchronizes access to this object.
  void begin_availability();
    //!< allows the safe_callback derived object to be called.
    /*!< if this was never invoked, then attempts to callback are rejected. */

public:
  global_live_objects &_invocables();
    //!< provides access to the program-wide list of healthy callback objects.
    /*!< this should not be used by anyone external to the safe_callback
    implementation. */
};

//////////////

//! a simple place-holder that anonymizes the type passed to the callback.
/*!
  the virtual destructor above ensures that RTTI can be used if necessary;
  objects of different class types can be differentiated when passed to the
  callback.
*/
class callback_data_block
{
public:
  virtual ~callback_data_block();
};
 
//////////////

} //namespace.

#endif

