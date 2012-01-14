#ifndef SYNCHRONIZABLE_CLASS
#define SYNCHRONIZABLE_CLASS

/*
*  Name   : synchronizable
*  Author : Chris Koeritz
***
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <octopus/infoton.h>
#include <timely/time_stamp.h>

namespace synchronic {

//! Encapsulates all of the attributes known for an object.
/*!
  This relies on the naming scheme for infotons, so objects are unique
  only so far as their classifiers are different.  For example, if the objects
  are to be differentiated by the computer that they run on, then some
  unique form of that computer's name should be used as one of the components
  of the classifier.  Each object can hold a variety of information which
  is not defined here.  Instead, we require the merge() method that
  performs object specific reconciliation when an update arrives.
*/

class synchronizable : public octopi::infoton
{
public:
  enum modifications {
    ADDED,     // the object is new.
    CHANGED,   // the object has been modified.
    DELETED    // the object got removed.
  };

  modifications _mod;
    // the type of change that has happened for this object.  the derived
    // class must pack this information in the derived pack() method and
    // retrieve the modification info in unpack().  there are helper functions
    // pack_mod() and unpack_mod() to automate that responsibility.

  timely::time_stamp _updated;
    // when this information was last updated.  this should not be packed,
    // since it is only locally relevant.

  synchronizable(const structures::string_array &object_id) : infoton(object_id) {}
    // constructs the base portion of an attribute bundle for an object with
    // the "object_id".  the "object_id" must follow the rules for infoton
    // classifiers.  the last string in the object id is the list-unique
    // identifier for this set of attributes.

  enum outcomes {
    OKAY = basis::common::OKAY,  // the operation completed successfully.
    BAD_TYPE = basis::common::BAD_TYPE,  // provided object had an incompatible type.
    EMPTY = basis::common::IS_EMPTY  // the merge resulted in clearing this entry.
  };

  // helper functions for packing the modification information.
  void pack_mod(basis::byte_array &packed_form) const;
  bool unpack_mod(basis::byte_array &packed_form);
  int packed_mod_size() const { return sizeof(int); }
    //!< returns the size of the packed modifier.

  virtual basis::outcome merge(const synchronizable &to_merge) = 0;
    // overwrites any attributes in "this" bundle with the contents found
    // in "to_merge".  this can fail if the object types are different.

  virtual basis::astring text_form() const = 0;
    // provides a visual form of the data held in this bundle.

  // promote requirements of the infoton to derived objects.
  virtual void pack(basis::byte_array &packed_form) const = 0;
  virtual bool unpack(basis::byte_array &packed_form) = 0;
  virtual clonable *clone() const = 0;
  virtual int packed_size() const = 0;
};

//////////////

// implementations.

void synchronizable::pack_mod(basis::byte_array &packed_form) const
{ structures::attach(packed_form, int(_mod)); }

bool synchronizable::unpack_mod(basis::byte_array &packed_form)
{
  int temp;
  if (!structures::detach(packed_form, temp)) return false;
  _mod = (modifications)temp;
  return true;
}

} //namespace.

#endif

