/*****************************************************************************\
*                                                                             *
*  Name   : octopus                                                           *
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
#include "identity_tentacle.h"
#include "infoton.h"
#include "octopus.h"
#include "tentacle.h"
#include "unhandled_request.h"

#include <basis/astring.h>
#include <basis/mutex.h>
#include <configuration/application_configuration.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <mathematics/chaos.h>
#include <structures/amorph.h>
#include <structures/string_hash.h>
#include <timely/time_control.h>
#include <timely/time_stamp.h>

using namespace basis;
using namespace configuration;
using namespace loggers;
using namespace mathematics;
using namespace processes;
using namespace structures;
using namespace timely;

namespace octopi {

//#define DEBUG_OCTOPUS
  // uncomment for debugging noise.
//#define DEBUG_OCTOPUS_FILTERS
  // uncomment for noisy filter processing.

#undef GRAB_LOCK
#define GRAB_LOCK \
  auto_synchronizer l(*_molock)

// this macro returns a result and deletes the request due to a failure.  it
// stores a response for the request, in case they were expecting one, since
// otherwise they will wait a long time for a response that isn't coming.  if
// those responses are never picked up, they will eventually be cleaned out.
#define WHACK_RETURN(to_ret, to_whack) { \
  unhandled_request *bad_response = new unhandled_request(id, \
      request->classifier(), to_ret); \
  _responses->add_item(bad_response, id); \
  WHACK(to_whack); \
  return to_ret; \
}

const int MAXIMUM_TRASH_SIZE = 128 * KILOBYTE;
  // this is how much we'll toss out on closing an entity.

#undef LOG
#define LOG(t) CLASS_EMERGENCY_LOG(program_wide_logger::get(), t)

const int OCTOPUS_CHECKING_INTERVAL = 4 * MINUTE_ms;
  // the frequency in milliseconds of cleaning on the response bin.  this
  // doesn't need to happen very often; it only tosses data that has been
  // abandoned in the response bin.

//////////////

class filter_list : public array<tentacle *>
{
public:
  bool remove(tentacle *to_remove) {
    for (int i = 0; i < length(); i++) {
      if (get(i) == to_remove) {
        zap(i, i);
        return true;
      }
    }
    return false;
  }
};

//////////////

class tentacle_record 
{
public:
  tentacle *_limb;
  bool _filter;

  tentacle_record(tentacle *limb, bool filter)
      : _limb(limb), _filter(filter) {}

  ~tentacle_record() { WHACK(_limb); }
};

//////////////

class modula_oblongata : public amorph<tentacle_record>
{
public:
  modula_oblongata() : amorph<tentacle_record>() {}

  int find_index(const string_array &group) {
    for (int i = 0; i < elements(); i++) {
      if (borrow(i)->_limb->group().prefix_compare(group))
        return i;
    }
    return common::NOT_FOUND;
  }

  tentacle *find(const string_array &group) {
    int indy = find_index(group);
    if (negative(indy)) return NIL;
    return borrow(indy)->_limb;
  }

  bool zap(int a, int b) {
    outcome ret = amorph<tentacle_record>::zap(a, b);
    return ret == common::OKAY;
  }

  bool zap(const string_array &group) {
    int indy = find_index(group);
    if (negative(indy)) return false;
    amorph<tentacle_record>::zap(indy, indy);
    return true;
  }
};

//////////////

octopus::octopus(const astring &name, int max_per_ent)
: _name(new astring(name)),
  _tentacles(new modula_oblongata),
  _molock(new mutex),
  _responses(new entity_data_bin(max_per_ent)),
  _disallow_removals(0),
  _next_cleaning(new time_stamp(OCTOPUS_CHECKING_INTERVAL)),
  _clean_lock(new mutex),
  _filters(new filter_list),
  _sequencer(new safe_roller(1, MAXINT32 / 2)),
  _rando(new chaos)
{
  add_tentacle(new identity_tentacle(*this), true);
    // register a way to issue identities.  this is a filter.
  add_tentacle(new unhandled_request_tentacle(false), false);
    // provide a way to unpack the unhandled_request object.
}

octopus::~octopus()
{
//  FUNCDEF("destructor");
  WHACK(_filters);
  WHACK(_tentacles);
  WHACK(_responses);
  WHACK(_next_cleaning);
  WHACK(_clean_lock);
  WHACK(_name);
  WHACK(_molock);
  WHACK(_rando);
  WHACK(_sequencer);
}

void octopus::lock_tentacles() { _molock->lock(); }

void octopus::unlock_tentacles() { _molock->unlock(); }

entity_data_bin &octopus::responses() { return *_responses; }

int octopus::locked_tentacle_count() { return _tentacles->elements(); }

const astring &octopus::name() const { return *_name; }

tentacle *octopus::locked_get_tentacle(int indy)
{ return _tentacles->borrow(indy)->_limb; }

infoton *octopus::acquire_specific_result(const octopus_request_id &id)
{ return _responses->acquire_for_identifier(id); }

infoton *octopus::acquire_result(const octopus_entity &requester,
    octopus_request_id &id)
{ return _responses->acquire_for_entity(requester, id); }

void octopus::unlock_tentacle(tentacle *to_unlock)
{
  to_unlock = NIL;
  _molock->unlock();
}

void octopus::expunge(const octopus_entity &to_remove)
{
  FUNCDEF("expunge");
  {
    // temporary lock so we can keep tentacles from evaporating.
    GRAB_LOCK;
    _disallow_removals++;
  }

  // we've now ensured that no tentacles will be removed, so at most the
  // list would get longer.  we'll settle on its current length.
  int len = _tentacles->elements();
  for (int i = 0; i < len; i++) {
    tentacle_record *curr = _tentacles->borrow(i);
    if (!curr || !curr->_limb) {
//complain... logic error.
      continue;
    }
    // activate the expunge method on the current tentacle.
    curr->_limb->expunge(to_remove);
  }

  {
    // re-enable tentacle removals.
    GRAB_LOCK;
    _disallow_removals--;
  }

  // throw out any data that was waiting for that guy.
  int items_found = 1;
  infoton_list junk;
  while (items_found) {
    // grab a chunk of items to be trashed.
    items_found = responses().acquire_for_entity(to_remove, junk,
        MAXIMUM_TRASH_SIZE);
    junk.reset();
//#ifdef DEBUG_OCTOPUS
    if (items_found)
      LOG(a_sprintf("cleaned %d items for expunged entity ", items_found)
          + to_remove.mangled_form());
//#endif
  }

}

outcome octopus::zap_tentacle(const string_array &tentacle_name)
{
  tentacle *found = NIL;
  outcome ret = remove_tentacle(tentacle_name, found);
  WHACK(found);
  return ret;
}

outcome octopus::add_tentacle(tentacle *to_add, bool filter)
{
  FUNCDEF("add_tentacle");
  if (!to_add) return tentacle::BAD_INPUT;
  if (!to_add->group().length()) return tentacle::BAD_INPUT;
  outcome zapped_it = zap_tentacle(to_add->group());
  if (zapped_it == tentacle::OKAY) {
//#ifdef DEBUG_OCTOPUS
    LOG(astring("removed existing tentacle: ") + to_add->group().text_form());
//#endif
  }
  GRAB_LOCK;
  tentacle *found = _tentacles->find(to_add->group());
  // if found is non-NIL, then that would be a serious logic error since
  // we just zapped it above.
  if (found) return tentacle::ALREADY_EXISTS;
  to_add->attach_storage(*_responses);
  tentacle_record *new_record = new tentacle_record(to_add, filter);
  _tentacles->append(new_record);
  if (filter) *_filters += to_add;
#ifdef DEBUG_OCTOPUS
  LOG(astring("added tentacle on ") + to_add->group().text_form());
#endif
  return tentacle::OKAY;
}

outcome octopus::remove_tentacle(const string_array &group_name,
    tentacle * &free_me)
{
  FUNCDEF("remove_tentacle");
  free_me = NIL;
  if (!group_name.length()) return tentacle::BAD_INPUT;
  while (true) {
    // repeatedly grab the lock and make sure we're allowed to remove.  if
    // we're told we can't remove yet, then we drop the lock again and pause.
    _molock->lock();
    if (!_disallow_removals) {
      // we ARE allowed to remove it right now.  we leave the loop in
      // possession of the lock.
      break;
    }
    if (_disallow_removals < 0) {
      continuable_error(class_name(), func, "logic error in removal "
          "reference counter.");
    }
    _molock->unlock();
    time_control::sleep_ms(0);  // yield thread's execution to another thread.
  }
  int indy = _tentacles->find_index(group_name);
  if (negative(indy)) {
    // nope, no match.
    _molock->unlock();
    return tentacle::NOT_FOUND;
  }
  // found the match.
  tentacle_record *freeing = _tentacles->acquire(indy);
  _tentacles->zap(indy, indy);
  free_me = freeing->_limb;
  _filters->remove(free_me);
  _molock->unlock();
  freeing->_limb = NIL;
  WHACK(freeing);
  return tentacle::OKAY;
}

outcome octopus::restore(const string_array &classifier,
    byte_array &packed_form, infoton * &reformed)
{
#ifdef DEBUG_OCTOPUS
  FUNCDEF("restore");
#endif
  periodic_cleaning();  // freshen up if it's that time.

  reformed = NIL;
  if (!classifier.length()) return tentacle::BAD_INPUT;
  if (!packed_form.length()) return tentacle::BAD_INPUT;
  if (!classifier.length()) return tentacle::BAD_INPUT;
  {
    // keep anyone from being removed until we're done.
    GRAB_LOCK;
    _disallow_removals++;
  }
  tentacle *found = _tentacles->find(classifier);
  outcome to_return;
  if (!found) {
#ifdef DEBUG_OCTOPUS
    LOG(astring("tentacle not found for: ") + classifier.text_form());
#endif
    to_return = tentacle::NOT_FOUND;
  } else {
    to_return = found->reconstitute(classifier, packed_form, reformed);
  }
  // re-enable tentacle removals.
  GRAB_LOCK;
  _disallow_removals--;
  return to_return;
}

outcome octopus::evaluate(infoton *request, const octopus_request_id &id,
    bool now)
{
  FUNCDEF("evaluate");
  periodic_cleaning();  // freshen up if it's that time.

  // check that the classifier is well formed.
  if (!request->classifier().length()) {
#ifdef DEBUG_OCTOPUS
    LOG("failed due to empty classifier.");
#endif
    WHACK_RETURN(tentacle::BAD_INPUT, request);
  }

  _molock->lock();

  // block tentacle removals while we're working.
  _disallow_removals++;

  // ensure that we pass this infoton through all the filters for vetting.
  for (int i = 0; i < _filters->length(); i++) {
    tentacle *current = (*_filters)[i];
#ifdef DEBUG_OCTOPUS_FILTERS
    LOG(a_sprintf("%d: checking ", i + 1) + current->group().text_form());
#endif

    // check if the infoton is addressed specifically by this filter.
    bool is_relevant = current->group().prefix_compare(request->classifier());

#ifdef DEBUG_OCTOPUS_FILTERS
    if (is_relevant)
      LOG(astring("found it to be relevant!  for ") + id.text_form())
    else
      LOG(astring("found it to not be relevant.  for ") + id.text_form());
#endif

    // this infoton is _for_ this filter.
    _molock->unlock();
      // unlock octopus to allow others to operate.

    byte_array transformed;
//hmmm: maybe there should be a separate filter method?
    outcome to_return = current->consume(*request, id, transformed);
      // pass the infoton into the current filter.

    if (is_relevant) {
      // the infoton was _for_ the current filter.  that means that we are
      // done processing it now.
#ifdef DEBUG_OCTOPUS_FILTERS
      LOG(astring("filter ") + current->group().text_form() + " consumed "
          "infoton from " + id.text_form() + " with result "
          + tentacle::outcome_name(to_return));
#endif
      WHACK(request);
      GRAB_LOCK;  // short re-establishment of the lock.
      _disallow_removals--;
      return to_return;
    } else {
      // the infoton was vetted by the filter.  make sure it was liked.
#ifdef DEBUG_OCTOPUS_FILTERS
      LOG(astring("filter ") + current->group().text_form() + " vetted "
          "infoton " + id.text_form() + " with result "
          + tentacle::outcome_name(to_return));
#endif
      if (to_return == tentacle::PARTIAL) {
        // if the infoton is partially complete, then we're allowed to keep
        // going.  this outcome means it was not prohibited.

        // make sure they didn't switch it out on us.
        if (transformed.length()) {
          // we need to substitute the transformed version for the original.
          string_array classif;
          byte_array decro;  // decrypted packed infoton.
          bool worked = infoton::fast_unpack(transformed, classif, decro);
          if (!worked) {
            LOG("failed to fast_unpack the transformed data.");
          } else {
            infoton *new_req = NIL;
            outcome rest_ret = restore(classif, decro, new_req);
            if (rest_ret == tentacle::OKAY) {
              // we got a good transformed version.
              WHACK(request);
              request = new_req;  // substitution complete.
            } else {
              LOG("failed to restore transformed infoton.");
            }
          }
        }

        _molock->lock();  // get the lock again.
        continue;
      } else {
        // this is a failure to process that object.
#ifdef DEBUG_OCTOPUS_FILTERS
        LOG(astring("filter ") + current->group().text_form() + " denied "
            "infoton from " + id.text_form());
#endif
        {
          GRAB_LOCK;  // short re-establishment of the lock.
          _disallow_removals--;
        }
        WHACK_RETURN(to_return, request);
      }
    }
  }

  // if we're here, then the infoton has been approved by all filters.

#ifdef DEBUG_OCTOPUS_FILTERS
  LOG(astring("all filters approved infoton: ") + id.text_form());
#endif

  // locate the appropriate tentacle for this request.
  tentacle *found = _tentacles->find(request->classifier());

  _molock->unlock();
    // from here in, the octopus itself is not locked up.  but we have sent
    // the signal that no one must remove any tentacles for now.

  if (!found) {
#ifdef DEBUG_OCTOPUS
    LOG(astring("tentacle not found for: ")
        + request->classifier().text_form());
#endif
    GRAB_LOCK;  // short re-establishment of the lock.
    _disallow_removals--;
    WHACK_RETURN(tentacle::NOT_FOUND, request);
  }
  // make sure they want background execution and that the tentacle can
  // support this.
  if (!now && found->backgrounding()) {
    // pass responsibility over to the tentacle.
    outcome to_return = found->enqueue(request, id);
    GRAB_LOCK;  // short re-establishment of the lock.
    _disallow_removals--;
    return to_return;
  } else {
    // call the tentacle directly.
    byte_array ignored;
    outcome to_return = found->consume(*request, id, ignored);
    WHACK(request);
    GRAB_LOCK;  // short re-establishment of the lock.
    _disallow_removals--;
    return to_return;
  }
}

void octopus::periodic_cleaning()
{
//  FUNCDEF("periodic_cleaning");
  time_stamp next_time;
  {
    auto_synchronizer l(*_clean_lock);
    next_time = *_next_cleaning;
  }
  if (next_time < time_stamp()) {
    // the bin locks itself, so we don't need to grab the lock here.
    _responses->clean_out_deadwood(); 
    auto_synchronizer l(*_clean_lock);
      // lock before modifying the time stamp; only one writer.
    _next_cleaning->reset(OCTOPUS_CHECKING_INTERVAL);
  }
}

tentacle *octopus::lock_tentacle(const string_array &tentacle_name)
{
  if (!tentacle_name.length()) return NIL;
  _molock->lock();
  tentacle *found = _tentacles->find(tentacle_name);
  if (!found) {
    _molock->unlock();
    return NIL;
  }
  return found;
}

octopus_entity octopus::issue_identity()
{
  return octopus_entity(*_name, application_configuration::process_id(), _sequencer->next_id(),
      _rando->inclusive(0, MAXINT32 / 4));
}

} //namespace.

