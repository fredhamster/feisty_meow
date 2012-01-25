#ifndef ROLLER_CLASS
#define ROLLER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : roller                                                            *
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

#include <basis/definitions.h>

namespace structures {

//! Maintains a pseudo-unique identifier number and issues a new one on demand.
/*!
  The unique id is templated on the type of identifier, but the type used must
  support:
    1) assigment to a value,
    2) a greater than or equal operator (>=), and
    3) the increment operator (++).
  Zero is often treated as the invalid / empty / inactive identifier, but
  roller does not prevent its use since some ranges might need to bridge
  between negative and positive numbers.
*/

template <class contents>
class roller
{
public:
  roller(contents start_of_range, contents end_of_range);
    //!< constructs a roller between the start and end ranges.
    /*!< this constructs a roller that runs from the value "start_of_range"
    and will stay smaller than "end_of_range" (unless the initial parameters
    are screwed up; this class doesn't validate the start and end of range).
    when the roller hits the end of range, then its value is reset to the
    "start_of_range" again. */

  ~roller();

  // these report the constructor parameters.
  contents minimum() { return _start_of_range; }
    //!< the smallest value that the roller can have.
  contents maximum() { return _end_of_range; }
    //!< the outer limit of the roller; it should never reach this.

  contents next_id();
    //!< returns a unique (per instance of this type) id.

  contents current() const;
    //!< returns the current id to be used; be careful!
    /*!< this value will be the next one returned, so only look at the current
    id and don't use it unwisely.  this function is useful if you want to
    assign an id provisionally but might not want to complete the issuance of
    it. */

  void set_current(contents new_current);
    //!< allows the current id to be manipulated.
    /*!< this must be done with care lest existing ids be re-used. */

private:
  contents _current_id;  //!< the next id to bring forth.
  contents _start_of_range;  //!< first possible value.
  contents _end_of_range;  //!< one more than last possible value.
};

//////////////

//! A roller that's based on integers.  This is the most common type so far.

class int_roller : public roller<int>
{
public:
  int_roller(int start_of_range, int end_of_range)
          : roller<int>(start_of_range, end_of_range) {}
};

//////////////

// implementations below...

template <class contents>
roller<contents>::roller(contents start, contents end)
: _current_id(start), _start_of_range(start), _end_of_range(end) {}

template <class contents>
void roller<contents>::set_current(contents new_current)
{
  _current_id = new_current;
  if (_current_id >= _end_of_range) _current_id = _start_of_range;
}

template <class contents> roller<contents>::~roller() {}

template <class contents> contents roller<contents>::current() const
{ return _current_id; }

template <class contents> contents roller<contents>::next_id()
{
  contents to_return = _current_id;
  if (to_return == _end_of_range) {
    // somehow the id to return is at the end of the range.  this probably
    // means the end of range condition wasn't detected last time due to an
    // error in the parameters or the operation of == or ++ in the templated
    // class.
    _current_id = _start_of_range;
    to_return = _current_id;
  }
  _current_id++;  // next id.
  if (_current_id == _end_of_range) _current_id = _start_of_range;
    // reset the current position when hits the end of the range.
  return to_return;
}

} //namespace.

#endif

