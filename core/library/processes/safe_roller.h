#ifndef SAFE_ROLLER_CLASS
#define SAFE_ROLLER_CLASS

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

#include <basis/mutex.h>
#include <structures/roller.h>

namespace processes {

//! Implements a thread-safe roller object.
/*!
  Integers can be generated by this object without concern for corruption by
  multiple threads.
*/

class safe_roller
{
public:
  safe_roller(int start_of_range = 0, int end_of_range = MAXINT32);
    //!< Provides numbers between the start and end in a thread-safe way.
    /*!< constructs a roller that runs from the value "start_of_range" and
    will stay smaller than "end_of_range" (unless the initial parameters are
    screwed up; this class doesn't validate the start and end of range).
    when the roller hits the end of range, then its value is reset to the
    "start_of_range" again. */

  ~safe_roller();

  int next_id();
    //!< returns a unique (per instance of this type) id.

  int current() const;
    //!< returns the current id to be used; be careful!
    /*!< this value will be the next one returned, so only look at the
    current id and don't use it unwisely.  this function is useful if you want
    to assign an id provisionally but might not want to complete the
    issuance of it. */

  void set_current(int new_current);
    //!< allows the current id to be manipulated.
    /*!< this must be done with care lest existing ids be re-used. */

private:
  structures::int_roller *_rolling;  //!< the id generator.
  basis::mutex *_lock;  //!< thread synchronization.

  // forbidden...
  safe_roller(const safe_roller &);
  safe_roller &operator =(const safe_roller &);
};

//////////////

void safe_add(int &to_change, int addition);
  //!< thread-safe integer addition.
  /*!< the number passed in "addition" is atomically added to the number
  referenced in "to_change".  this allows thread-safe manipulation of
  a simple number. */

} //namespace.

#endif

