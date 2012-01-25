#ifndef FILTER_SET_CLASS
#define FILTER_SET_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : filter_set
*  Author : Chris Koeritz
*                                                                             *
*******************************************************************************
* Copyright (c) 1996-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <basis/definitions.h>
#include <basis/mutex.h>
#include <structures/set.h>

namespace loggers {

//! A simple object that wraps a templated set of ints.
class filter_set : public structures::set<int>, public virtual basis::root_object
{
public:
  filter_set() {}
    //!< Constructs an empty set of filters.

  virtual ~filter_set() {}

  filter_set(const structures::set<int> &to_copy) : structures::set<int>(to_copy) {}
    //!< Constructs a copy of the "to_copy" array.

  DEFINE_CLASS_NAME("filter_set");

  //! Adds a member to the filter set.
  /*! The filter set is used to check all extended filter values passed to
  log and print.  if the special filters of ALWAYS_PRINT or NEVER_PRINT are
  added, then either everything will be logged or nothing will be. */
  virtual void add_filter(int new_filter) {
    basis::auto_synchronizer l(c_lock);
    add(new_filter);
  }

  //! Removes a member from the filter set.
  virtual void remove_filter(int old_filter) {
    basis::auto_synchronizer l(c_lock);
    remove(old_filter);
  }

  //! Returns true if the "filter_to_check" is a member of the filter set.
  /*! If "filter_to_check" is ALWAYS_PRINT, this always returns true.  If
  the value is NEVER_PRINT, false is always returned. */
  virtual bool member(int filter_to_check) {
     if (filter_to_check == basis::ALWAYS_PRINT) return true;
     if (filter_to_check == basis::NEVER_PRINT) return false;
     return structures::set<int>::member(filter_to_check);
  }

  //! Resets the filter set to be empty.
  virtual void clear_filters() {
    basis::auto_synchronizer l(c_lock);
    clear();
  }

private:
  basis::mutex c_lock;
};

}  // namespace.

#endif

