#ifndef UNIQUE_ID_CLASS
#define UNIQUE_ID_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : unique_id                                                         *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1999-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/contracts.h>

namespace structures {

//! Provides an abstraction for the responsibilities of a unique identifier.
/*!
  These are generally used as a way of masking the underlying ID object while
  providing some equality comparisons.  It is especially useful when the
  underlying object is not itself an object, but just a simple type.
*/

template <class uniquifier>
class unique_id : public virtual basis::equalizable
{
public:
  unique_id(uniquifier initial_value) : _id(initial_value) {}
    //!< Constructs a unique id from the "initial_value".

  unique_id(const unique_id<uniquifier> &to_copy) { *this = to_copy; }
    //!< Constructs a unique id as a copy of the "to_copy" object.

  ~unique_id() {}

  virtual bool equal_to(const equalizable &to_compare) const {
    const unique_id<uniquifier> *cast = dynamic_cast<const unique_id<uniquifier> *>(&to_compare);
    if (!cast) throw "error: unique_id::==: unknown type";
    return cast->_id == _id; 
  }

  //! Returns true if the held id is the same as "to_compare".
  /*! The templated uniquifying type absolutely must provide an equality
  operator (==) and an assignment operator (=). */
  bool operator == (const unique_id<uniquifier> &to_compare) const
          { return _id == to_compare._id; }

  //! Sets this id to be the same as "to_copy".
  unique_id & operator = (const unique_id<uniquifier> &to_copy)
          { if (this != &to_copy) _id = to_copy._id; return *this; }

  uniquifier raw_id() const { return _id; }
    //!< Returns the held identifier in its native form.

  void set_raw_id(uniquifier new_value) { _id = new_value; }
    //!< Sets the held identifier to "new_value".

private:
  uniquifier _id;  //!< the held object in its native form.
};

//////////////

//! A unique identifier class that supports sorting.
/*!
  The orderable version can be compared for magnitude and permits sorting
  the ids based on the underlying type.  The underlying type must implement
  at least the less than operator.
*/

template <class uniquifier>
class orderable_unique_id : public unique_id<uniquifier>
{
public:
  orderable_unique_id(const uniquifier &initial_value)
          : unique_id<uniquifier>(initial_value) {}
  orderable_unique_id(const unique_id<uniquifier> &initial_value)
          : unique_id<uniquifier>(initial_value) {}
  ~orderable_unique_id() {}

  /*! the "uniquifier" type absolutely must provide a less than operator (<)
  and it must meet the requirements of the "unique_id" template. */
  bool operator < (const unique_id<uniquifier> &to_compare) const
          { return this->raw_id() < to_compare.raw_id(); }
};

//////////////

//! A unique identifier based on integers.

class unique_int : public unique_id<int>
{
public:
  unique_int(int initial = 0) : unique_id<int>(initial) {}
    //!< implicit default for "initial" of zero indicates bogus id.

  bool operator ! () const { return raw_id() == 0; }
    //!< provides a way to test whether an id is valid.
    /*!< This uses the implicit assumption that a zero id is invalid or
    unassigned. */
};

} //namespace.

#endif

