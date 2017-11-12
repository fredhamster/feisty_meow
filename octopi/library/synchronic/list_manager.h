#ifndef LIST_MANAGER_CLASS
#define LIST_MANAGER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : list_manager                                                      *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <octopus/tentacle.h>

namespace synchronic {

// forward.
class synchronizable;
class bundle_list;

//! Supports distributed management of a list of object states.
/*!
  An object may have a collection of attributes which are important to keep
  up to date.  The list_manager provides a means to keep that information
  relevant given periodic updates to the state information by the entity in
  charge of the actual object.                                          
*/

class list_manager : public octopi::tentacle
{
public:
  list_manager(const structures::string_array &list_name, bool backgrounded);
    // given the root of the "list_name", this will administrate the list.
    // all entries in the list must have "list_name" as their prefix.
    // if "backgrounded" is true, then this list will consume its requests
    // on a thread.  otherwise it deals with them immediately.

  virtual ~list_manager();

  int entries() const;  // returns the number of items held here.

  const structures::string_array &list_name() const;
    // returns the list name this was constructed with (which is the same
    // as the group for this object).

  bool is_listed(const structures::string_array &classifier);
    // returns true if the object with "classifier" exists in the list.

  bool update(const structures::string_array &classifier, int offset = 0);
    // returns true if the object with "classifier" could be found and its
    // last update timestamp set to the current time if "offset" is zero.
    // if "offset" is negative, then the time will be updated to some time
    // before now.  if positive, then it's updated to a time after now.

  void clean(int older_than);
    // flushes out any items that haven't been updated in "older_than"
    // microseconds.

  synchronizable *clone_object(const structures::string_array &classifier);
    // returns a clone of the object listed for "classifier" or NULL_POINTER if there
    // are none matching.  the returned object must be destroyed if non-null.

  bool zap(const structures::string_array &classifier);
    // returns true if we were able to find and remove the item held under
    // the "classifier".

  void retrieve(bundle_list &to_fill) const;
    // loads "to_fill" with a copy of the current set of attribute bundles.

  void reset();
    // wipes out all objects that used to be listed.

  virtual basis::outcome consume(octopi::infoton &to_chow,
          const octopi::octopus_request_id &item_id,
          basis::byte_array &transformed);
    // processes a request encapsulated in "to_chow".  the "item_id" is
    // currently unused.

  virtual basis::outcome reconstitute(const structures::string_array &classifier,
          basis::byte_array &packed_form, octopi::infoton * &reformed) = 0;
    // recovers the original form "reformed" from a "packed_form" of the
    // object.  this must be provided by derived list_manager objects.

  virtual void expunge(const octopi::octopus_entity &to_remove);
    // cleans out any items held for the entity "to_remove".

private:
  bundle_list *_entries;  // the set of elements that make up our list.
  basis::mutex *_locking;  // protects our contents.

  int locked_find(const structures::string_array &classifier);
    // locates the item with the "classifier" in this list.  if it's present,
    // a non-negative index number is returned.
};

} //namespace.

#endif

