/*****************************************************************************\
*                                                                             *
*  Name   : tentacle                                                          *
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
#include <processes/ethread.h>
#include <structures/amorph.h>

using namespace basis;
using namespace loggers;
using namespace processes;
using namespace structures;

namespace octopi {

//#define DEBUG_TENTACLE
  // uncomment for noisier version.

#undef GRAB_CONSUMER_LOCK
#define GRAB_CONSUMER_LOCK auto_synchronizer l(*_input_guard)

#undef LOG
#define LOG(t) CLASS_EMERGENCY_LOG(program_wide_logger::get(), t)

//////////////

struct infoton_record {
  infoton *_product;
  octopus_request_id _id;

  infoton_record(infoton *product, octopus_request_id id)
      : _product(product), _id(id) {}

  ~infoton_record() { WHACK(_product); }
};

class queueton : public amorph<infoton_record> {};

//////////////

class pod_motivator : public ethread
{
public:
  pod_motivator(tentacle &parent, int motivational_rate)
  : ethread(motivational_rate, ethread::SLACK_INTERVAL),
    _parent(parent) {}

  void perform_activity(void *formal(ptr)) { _parent.propel_arm(); }

private:
  tentacle &_parent;
};

//////////////

tentacle::tentacle(const string_array &group_name, bool backgrounded,
    int motivational_rate)
: _group(new string_array(group_name)),
  _pending(new queueton),
  _input_guard(new mutex),
  _action(NIL),
  _products(NIL),
  _backgrounded(backgrounded)
{
  // we only start the thread if they've said they'll support backgrounding.
  if (backgrounded)
    _action = new pod_motivator(*this, motivational_rate);
}

tentacle::~tentacle()
{
  if (_action) _action->stop();
  WHACK(_action);
  WHACK(_group);
  WHACK(_pending);
  WHACK(_input_guard);
}

const string_array &tentacle::group() const { return *_group; }

const char *tentacle::outcome_name(const outcome &to_name)
{ return common::outcome_name(to_name); }

int tentacle::motivational_rate() const
{ if (_action) return _action->sleep_time(); else return 0; }

entity_data_bin *tentacle::get_storage() { return _products; }

void tentacle::attach_storage(entity_data_bin &storage)
{
  _products = &storage;
  if (_action) _action->start(NIL);
}

void tentacle::detach_storage()
{
  if (_action) _action->stop();
  _products = NIL;
}

bool tentacle::store_product(infoton *product,
    const octopus_request_id &original_id)
{
  FUNCDEF("store_product");
  if (!_products) {
//#ifdef DEBUG_TENTACLE
    LOG("storage bunker has not been established!");
//#endif
    return false;
  }
  return _products->add_item(product, original_id);
}

outcome tentacle::enqueue(infoton *to_chow, const octopus_request_id &item_id)
{
  GRAB_CONSUMER_LOCK;
  int max_size = 0;
  // this may be a bad assumption, but here goes: we assume that the limit
  // on per entity storage in the bin is pretty much the same as a reasonable
  // limit here on the queue of pending items.  we need to limit it and would
  // rather not add another numerical parameter to the constructor.
  if (_products)
    max_size = _products->max_bytes_per_entity();
  int curr_size = 0;
  if (max_size) {
    // check that the pending queue is also constrained.
    for (int i = 0; i < _pending->elements(); i++) {
      curr_size += _pending->borrow(i)->_product->packed_size();
    }
    if (curr_size + to_chow->packed_size() > max_size) {
      WHACK(to_chow);
      return NO_SPACE;
    }
  }
  *_pending += new infoton_record(to_chow, item_id);
//is there ever a failure outcome?
//yes, when space is tight!
  return OKAY;
}

infoton *tentacle::next_request(octopus_request_id &item_id)
{
  GRAB_CONSUMER_LOCK;
  if (!_pending->elements()) return NIL;  // nothing to return.
  infoton *to_return = (*_pending)[0]->_product;
  (*_pending)[0]->_product = NIL;
    // clean out so destructor doesn't delete the object.
  item_id = (*_pending)[0]->_id;
  _pending->zap(0, 0);
  return to_return;
}

void tentacle::propel_arm()
{
  FUNCDEF("propel_arm");
  infoton *next_item = NIL;
  do {
    octopus_request_id id;
    next_item = next_request(id);
    if (!next_item) break;
    byte_array ignored;
    outcome ret = consume(*next_item, id, ignored);
    if (ret != OKAY) {
#ifdef DEBUG_TENTACLE
      LOG(astring("failed to act on ") + next_item->classifier().text_form());
#endif
    }
    WHACK(next_item);  // fulfill responsibility for cleanup.
  } while (next_item);
}

} //namespace.

