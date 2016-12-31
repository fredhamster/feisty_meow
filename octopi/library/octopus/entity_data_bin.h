#ifndef ENTITY_DATA_BIN_CLASS
#define ENTITY_DATA_BIN_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : entity_data_bin                                                   *
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

#include <basis/contracts.h>
#include <basis/definitions.h>
#include <basis/mutex.h>
#include <structures/set.h>

namespace octopi {

// forward.
class basketcase;
class entity_basket;
class entity_item_hash;
class infoton;
class infoton_list;
class octopus_entity;
class octopus_request_id;

//! Stores a set of infotons grouped by the entity that owns them.

class entity_data_bin
{
public:
  entity_data_bin(int max_bytes_per_entity);
    //!< allows each entity in the bin to have "max_bytes_per_entity" bytes stored.
    /*!<  any storage attempts that would go beyond that limit are rejected. */

  virtual ~entity_data_bin();

  DEFINE_CLASS_NAME("entity_data_bin");

  int max_bytes_per_entity() const { return _max_per_ent; }
    // reports the maximum size allowed per entity for storage.
  void max_bytes_per_entity(int max_bytes_per) { _max_per_ent = max_bytes_per; }
    // allows resetting of the storage size allowed per entity.  note that if
    // this value is made smaller and the bin is already holding more than
    // the new limit, then no additional stores will be allowed until some of
    // the data is removed.

  int entities() const;
    // returns the number of entities that currently possess storage bins.
    // this is a very costly call.

  int items_held() const { return _items_held; }
    // returns the number of items held here, if any.  this is a very
    // inexpensive call that should be used prior to checking for data.
    // it's safe to check this at any time, since it's just an int.  there's
    // every likelihood that the number might change by the time one acquires
    // the lock on the bin, but if it's zero then that's a good reason to
    // avoid looking for data yet.

  bool get_sizes(const octopus_entity &id, int &items, int &bytes);
    // finds the storage for "id".  if there is any there, true is returned
    // and "items" is set to the number of pending items and "bytes" is set
    // to the number of bytes for those items.

  bool add_item(infoton *to_add, const octopus_request_id &id);
    // stores an item "to_add" for an entity listed in "id".  if the item
    // cannot be stored due to space constraints, false is returned and
    // "to_add" is deleted.

  infoton *acquire_for_identifier(const octopus_request_id &id);
    // locates an item for the specific "id".  this will generally be a
    // response to a previous request.  if no object can be found that matches
    // the "id", then NULL_POINTER is returned.

  infoton *acquire_for_entity(const octopus_entity &requester,
          octopus_request_id &id);
    // this returns an infoton for the "requester", if any are available.  call
    // this function repeatedly to ensure that all available items have
    // been provided.  the "original_id" is a copy of the "item_id" that was
    // originally passed to evaluate_request().  the returned object must
    // eventually be destroyed if non-null.

  int acquire_for_entity(const octopus_entity &requester,
          infoton_list &items, int maximum_size);
    // retrieves up to "maximum_size" in bytes of pending items for the
    // "requester" into "items".  the number of items found is returned.

  infoton *acquire_for_any(octopus_request_id &id);
    // acquires an infoton for any random entity.  if no items are ready at
    // all, then NULL_POINTER is returned.

  basis::astring text_form() const;
    // returns a textual list of what's held here.

  void clean_out_deadwood(int decay_interval = 4 * basis::MINUTE_ms);
    // gets rid of any items that haven't been picked up in a timely manner.
    // note that this should be called periodically by the controlling object.
    // it will not be called automatically.

private:
  entity_item_hash *_table;  // our main storage object.
  basis::mutex *_ent_lock;  // protects our structures.
  int _action_count;
    // used for debugging; tracks how many acquires have occurred since the
    // last dump of item count.
  int _max_per_ent;  // the maximum size allowed per entity.
  int _items_held;  // the number of items in residence.

  friend class monk_the_detective;  // eerie supernatural powers, for testing.

  int scramble_counter();  // counts the number of items used.

  // not available.
  entity_data_bin(const entity_data_bin &);
  entity_data_bin &operator =(const entity_data_bin &);
};

} //namespace.

#endif

