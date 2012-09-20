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

#include "entity_data_bin.h"
#include "entity_defs.h"
#include "infoton.h"
#include "tentacle.h"

#include <basis/astring.h>

#include <basis/mutex.h>
#include <loggers/program_wide_logger.h>
#include <structures/set.h>
#include <structures/string_array.h>
#include <structures/amorph.h>
#include <structures/string_hash.h>
#include <textual/parser_bits.h>
#include <timely/time_stamp.h>

using namespace basis;
using namespace loggers;
using namespace structures;
using namespace textual;
using namespace timely;

namespace octopi {

//#define DEBUG_ENTITY_DATA_BIN
  // uncomment for more debugging information.

#undef GRAB_LOCK
#define GRAB_LOCK \
  auto_synchronizer l(*_ent_lock)

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

const int OCTOPUS_TABLE_BITS = 6;
  // the hash table for items will have 2^N entries.

//hmmm: parameterize in class interface?
////const int DATA_DECAY_INTERVAL = 4 * MINUTE_ms;
  // if we haven't gotten a data item out to its entity in this long, then
  // we assume the entity has croaked or doesn't want its data.

//////////////

class infoton_holder
{
public:
  infoton *_item;      // the data making up the production.
  octopus_request_id _id;  // the id, if any, of the original request.
  time_stamp _when_added;  // when the data became available.

  infoton_holder(const octopus_request_id &id = octopus_request_id(),
      infoton *item = NIL)
  : _item(item), _id(id), _when_added() {}

  ~infoton_holder() { WHACK(_item); }

  astring text_form() const {
    return astring("id=") + _id.text_form() + ", added="
        + _when_added.text_form() + ", item="
        + _item->classifier().text_form() + ", data="
        + _item->text_form();
  }
};

//////////////

class entity_basket : public amorph<infoton_holder>
{
public:
  time_stamp _last_active;

  astring text_form() const {
    astring to_return;
    for (int i = 0; i < elements(); i++)
      to_return += get(i)->text_form() + parser_bits::platform_eol_to_chars();
    return to_return;
  }
};

//////////////

class entity_hasher : public hashing_algorithm
{
public:
  virtual hashing_algorithm *clone() const { return new entity_hasher; }

  virtual basis::un_int hash(const void *key_data, int formal(key_length)) const {
    octopus_entity *key = (octopus_entity *)key_data;
    // jiggle the pieces of the id into a number.
    return basis::un_int(
        key->process_id()
        + (key->add_in() << 10)
        + (key->sequencer() << 14)
        + (key->hostname()[0] << 20)
        + (key->hostname()[1] << 24) );
  }
};

//////////////

class entity_item_hash
: public hash_table<octopus_entity, entity_basket>
{
public:
  entity_item_hash(const entity_hasher &hash)
  : hash_table<octopus_entity, entity_basket>(hash, OCTOPUS_TABLE_BITS)
  {}
};

//////////////

class basketcase : public structures::set<octopus_entity>
{
public:
};

//////////////

// used for our apply methods for communicating back to the caller.
struct apply_struct
{
  basketcase *_empty_baskets;
  entity_basket *_any_item;
  int &_items_held;  // hooks to parent's item count.
  int _decay_interval;  // how long are items allowed to live?

  apply_struct(int &items_held)
      : _empty_baskets(NIL), _any_item(NIL), _items_held(items_held),
        _decay_interval(0) {}
};

//////////////

entity_data_bin::entity_data_bin(int max_size_per_entity)
: _table(new entity_item_hash(entity_hasher())),
  _ent_lock(new mutex),
  _action_count(0),
  _max_per_ent(max_size_per_entity),
  _items_held(0)
{}

entity_data_bin::~entity_data_bin()
{
  WHACK(_table);
  WHACK(_ent_lock);
}

int entity_data_bin::entities() const
{
  GRAB_LOCK;
  return _table->elements();
}

struct text_form_accumulator { astring _accum; };

bool text_form_applier(const octopus_entity &formal(key), entity_basket &bask,
    void *data_link)
{
  text_form_accumulator *shuttle = (text_form_accumulator *)data_link;
  shuttle->_accum += bask.text_form();
  return true;
}

astring entity_data_bin::text_form() const
{
  GRAB_LOCK;
  text_form_accumulator shuttle;
  _table->apply(text_form_applier, &shuttle);
  return shuttle._accum;
}

bool scramble_applier(const octopus_entity &formal(key), entity_basket &bask,
    void *data_link)
{
  #undef static_class_name
  #define static_class_name() "entity_data_bin"
  FUNCDEF("scramble_applier");
  int *county = (int *)data_link;
  *county += bask.elements();
  return true;
  #undef static_class_name
}

// this could be extended to do more interesting checks also; currently it's
// just like the entities() method really.
int entity_data_bin::scramble_counter()
{
  GRAB_LOCK;
  int count = 0;
  _table->apply(scramble_applier, &count);
  return count;
}

#ifdef DEBUG_ENTITY_DATA_BIN
  #define DUMP_STATE \
    if ( !(_action_count++ % 100) ) { \
      int items = scramble_counter(); \
      LOG(a_sprintf("-> %d items counted.", items)); \
    }
#else
  #define DUMP_STATE
#endif

bool entity_data_bin::add_item(infoton *to_add,
    const octopus_request_id &orig_id)
{
  FUNCDEF("add_item");
  GRAB_LOCK;
  // create a record to add to the appropriate bin.
  infoton_holder *holder = new infoton_holder(orig_id, to_add);

  // see if a basket already exists for the entity.
  entity_basket *bask = _table->find(orig_id._entity);
  if (!bask) {
    // this entity doesn't have a basket so add one.
    bask = new entity_basket;
    _table->add(orig_id._entity, bask);
  }

  bask->_last_active = time_stamp();  // reset activity time.

  // count up the current amount of data in use.
  int current_size = 0;
  for (int i = 0; i < bask->elements(); i++)
    current_size += bask->borrow(i)->_item->packed_size();

  if (current_size + to_add->packed_size() > _max_per_ent) {
    WHACK(holder);
    return false;
  }
  
  // append the latest production to the list.
  bask->append(holder);
  _items_held++;
  return true;
}

bool any_item_applier(const octopus_entity &formal(key), entity_basket &bask,
    void *data_link)
{
  #define static_class_name() "entity_data_bin"
  FUNCDEF("any_item_applier");
  apply_struct *apple = (apply_struct *)data_link;
  // check the basket to see if it has any items.
  if (!bask.elements()) {
//#ifdef DEBUG_ENTITY_DATA_BIN
//    LOG(astring("saw empty basket ") + key.mangled_form());
//#endif
    return true;  // continue iterating.
  }
  apple->_any_item = &bask;
  return false;  // stop iteration.
  #undef static_class_name
}

infoton *entity_data_bin::acquire_for_any(octopus_request_id &id)
{
  FUNCDEF("acquire_for_any");
  GRAB_LOCK;
  apply_struct apple(_items_held);
  _table->apply(any_item_applier, &apple);
  if (!apple._any_item) return NIL;
  DUMP_STATE;
  // retrieve the information from our basket that was provided.
  infoton_holder *found = apple._any_item->acquire(0);
  apple._any_item->zap(0, 0);
  if (!apple._any_item->elements()) {
    // toss this empty basket.
#ifdef DEBUG_ENTITY_DATA_BIN
    LOG(astring("tossing empty basket ") + found->_id._entity.mangled_form());
#endif
    _table->zap(found->_id._entity);
  }
  apple._any_item = NIL;
  infoton *to_return = found->_item;
  id = found->_id;
  found->_item = NIL;  // clear so it won't be whacked.
  WHACK(found);
  _items_held--;
//#ifdef DEBUG_ENTITY_DATA_BIN
  if (_items_held < 0)
    LOG("logic error: number of items went below zero.");
//#endif
  return to_return;
}

int entity_data_bin::acquire_for_entity(const octopus_entity &requester,
    infoton_list &items, int maximum_size)
{
  FUNCDEF("acquire_for_entity [multiple]");
  // this method does not grab the lock because it simply composes other
  // class methods without interacting with class data members.
  items.reset();
  if (maximum_size <= 0) maximum_size = 20 * KILOBYTE;
    // pick a reasonable default.
  octopus_request_id id;
  int items_found = 0;
  while (maximum_size > 0) {
    infoton *inf = acquire_for_entity(requester, id);
    if (!inf)
      break;  // none left.
    items.append(new infoton_id_pair(inf, id));    
    maximum_size -= inf->packed_size();
    items_found++;
  }
  return items_found;
}

infoton *entity_data_bin::acquire_for_entity(const octopus_entity &requester,
    octopus_request_id &id)
{
  FUNCDEF("acquire_for_entity [single]");
  id = octopus_request_id();  // reset it.
  GRAB_LOCK;
  infoton *to_return = NIL;
  entity_basket *bask = _table->find(requester);
  if (!bask) {
    return NIL;
  }
  if (!bask->elements()) {
#ifdef DEBUG_ENTITY_DATA_BIN
    LOG(astring("tossing empty basket ") + requester.mangled_form());
#endif
    _table->zap(requester);
    return NIL;
  }
  DUMP_STATE;
  id = bask->get(0)->_id;
  to_return = bask->borrow(0)->_item;
  bask->borrow(0)->_item = NIL;
  bask->zap(0, 0);
  if (!bask->elements()) {
#ifdef DEBUG_ENTITY_DATA_BIN
    LOG(astring("tossing empty basket ") + requester.mangled_form());
#endif
    _table->zap(requester);
  }
  _items_held--;
//#ifdef DEBUG_ENTITY_DATA_BIN
  if (_items_held < 0)
    LOG("logic error: number of items went below zero.");
//#endif
  return to_return;
}

infoton *entity_data_bin::acquire_for_identifier(const octopus_request_id &id)
{
  FUNCDEF("acquire_for_identifier");
  infoton *to_return = NIL;
  GRAB_LOCK;
  entity_basket *bask = _table->find(id._entity);
  if (!bask) return NIL;
  if (!bask->elements()) {
#ifdef DEBUG_ENTITY_DATA_BIN
    LOG(astring("tossing empty basket ") + id._entity.mangled_form());
#endif
    _table->zap(id._entity);
    return NIL;
  }
  for (int i = 0; i < bask->elements(); i++) {
    if (bask->get(i)->_id == id) {
      to_return = bask->borrow(i)->_item;  // snag the item.
      bask->borrow(i)->_item = NIL;  // clear the list's version out.
      bask->zap(i, i);  // whack the sanitized element.
      DUMP_STATE;
      if (!bask->elements()) {
#ifdef DEBUG_ENTITY_DATA_BIN
        LOG(astring("tossing empty basket ") + id._entity.mangled_form());
#endif
        _table->zap(id._entity);
      }
      _items_held--;
//#ifdef DEBUG_ENTITY_DATA_BIN
      if (_items_held < 0)
        LOG("logic error: number of items went below zero.");
//#endif
      return to_return;
    }
  }
  return NIL;
}

bool cleaning_applier(const octopus_entity &key, entity_basket &bask,
    void *data_link)
{
  #define static_class_name() "entity_data_bin"
  FUNCDEF("cleaning_applier");
  apply_struct *apple = (apply_struct *)data_link;
  time_stamp expiration_time(-apple->_decay_interval);

  int whack_count = 0;
  for (int i = 0; i < bask.elements(); i++) {
    infoton_holder &rec = *bask.borrow(i);
    if (rec._when_added <= expiration_time) {
      // if a requester hasn't picked this up in N seconds, then drop it.
#ifdef DEBUG_ENTITY_DATA_BIN
      LOG(astring("whacking old item ") + rec._id.text_form());
#endif
      whack_count++;
      apple->_items_held--;
//#ifdef DEBUG_ENTITY_DATA_BIN
      if (apple->_items_held < 0)
        LOG("logic error: number of items went below zero.");
//#endif
      bask.zap(i, i);
      i--;  // skip back before the delete.
    } else {
      // NOTE: this break is based on an assumption about the storage of
      // items; if it's ever the case in the future that items can be
      // disordered on time of arrival in the queue, then the break should
      // be removed.
      break;
    }
  }
#ifdef DEBUG_ENTITY_DATA_BIN
  if (whack_count)
    LOG(a_sprintf("==> whacked %d old items.", whack_count));
#endif
  if (!bask.elements()) {
    // if the basket has nothing left in it then we signal the parent that
    // it can be deleted.
//LOG("adding to empty basket list.");
    *apple->_empty_baskets += key;
//LOG("added to empty basket list.");
  }

  // keep iterating on items unless we know it's time to go.
  return true;
  #undef static_class_name
}

void entity_data_bin::clean_out_deadwood(int decay_interval)
{
#ifdef DEBUG_ENTITY_DATA_BIN
  FUNCDEF("clean_out_deadwood");
#endif
  GRAB_LOCK;
  // check that no items have timed out.
  apply_struct apple(_items_held);
  basketcase empty_baskets;
  apple._empty_baskets = &empty_baskets;
  apple._decay_interval = decay_interval;
  _table->apply(cleaning_applier, &apple);

  // clean up any entities whose baskets are empty.
  for (int i = empty_baskets.length() - 1; i >= 0; i--) {
#ifdef DEBUG_ENTITY_DATA_BIN
     LOG(astring("removing basket ") + empty_baskets.get(i).mangled_form());
#endif
    _table->zap(empty_baskets.get(i));
    empty_baskets.zap(i, i);
    // we don't skip back since we're scanning the array from its end.
  }
}

bool entity_data_bin::get_sizes(const octopus_entity &id, int &items,
    int &bytes)
{
  FUNCDEF("get_sizes");
  items = 0;
  bytes = 0;
  GRAB_LOCK;
  entity_basket *bask = _table->find(id);
  if (!bask || !bask->elements()) return false;
  items = bask->elements();
  for (int i = 0; i < bask->elements(); i++)
    bytes += bask->borrow(i)->_item->packed_size();
  return true;
}

} //namespace.

