/*****************************************************************************\
*                                                                             *
*  Name   : post_office                                                       *
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

#include "ethread.h"
#include "letter.h"
#include "mailbox.h"
#include "post_office.h"
#include "thread_cabinet.h"

#include <basis/mutex.h>
#include <configuration/application_configuration.h>
#include <loggers/program_wide_logger.h>
#include <structures/set.h>
#include <structures/amorph.h>
#include <structures/unique_id.h>
#include <textual/parser_bits.h>
#include <timely/time_stamp.h>

using namespace basis;
using namespace configuration;
using namespace loggers;
using namespace structures;
using namespace textual;
using namespace timely;

namespace processes {

//#define DEBUG_POST_OFFICE
  // uncomment if you want the noisy version.

#undef LOG
#define LOG(a) CLASS_EMERGENCY_LOG(program_wide_logger::get(), a)

const int CLEANING_INTERVAL = 14 * SECOND_ms;
  // the interval between cleaning of extra letters and dead mailboxes.

const int SNOOZE_TIME_FOR_POSTMAN = 42;
  // we'll snooze for this long if absolutely nothing happened during the
  // thread's activation.  if things are going on, our snooze time is reduced
  // by the length of time we were delivering items.

const int DELIVERIES_ALLOWED = 350;
  // the maximum number of deliveries we'll try to get done per thread run.

//////////////

//hmmm: arrhhh--maybe we need to spawn a thread per postal route.

class postal_carrier : public ethread
{
public:
  postal_carrier(post_office &parent, const unique_int &route)
  : ethread(SNOOZE_TIME_FOR_POSTMAN, ethread::SLACK_INTERVAL),
    _parent(parent),
    _route(route)
  {}

  DEFINE_CLASS_NAME("postal_carrier");

  void perform_activity(void *) {
    FUNCDEF("perform_activity");
    bool finished;
    try {
      finished = _parent.deliver_mail_on_route(_route, *this); 
    } catch(...) {
      LOG(astring("caught exception during mail delivery!"));
    }
    if (!finished) {
      // not finished delivering all items.
      reschedule();
    } else {
      reschedule(SNOOZE_TIME_FOR_POSTMAN);
    }
  }

private:
  post_office &_parent;
  unique_int _route;
};

//////////////

class postal_cache : public mailbox {};

//////////////

class tagged_mail_stop : public virtual text_formable
{
public:
  mail_stop *_route;
  unique_int _thread_id;
  unique_int _id;

  tagged_mail_stop(const unique_int &id = 0, mail_stop *route = NULL_POINTER,
          const unique_int &thread_id = 0)
      : _route(route), _thread_id(thread_id), _id(id) {}

  DEFINE_CLASS_NAME("tagged_mail_stop");

  virtual void text_form(basis::base_string &fill) const {
    fill.assign(text_form());
  }

  virtual astring text_form() const {
    return a_sprintf("%s: id=%d, addr=%08lx, thr_id=%d",
        static_class_name(), _id.raw_id(), _route, _thread_id.raw_id());
  }
};

//////////////

class route_map : public amorph<tagged_mail_stop>
{
public:
  tagged_mail_stop *find(const unique_int &id) {
    for (int i = 0; i < elements(); i++) {
      tagged_mail_stop *curr = borrow(i);
      if (curr && (curr->_id == id)) return curr;
    }
    return NULL_POINTER;
  }

  bool zap(const unique_int &id) {
    for (int i = 0; i < elements(); i++) {
      tagged_mail_stop *curr = borrow(i);
      if (curr && (curr->_id == id)) {
        amorph<tagged_mail_stop>::zap(i, i);
        return true;
      }
    }
    return false;
  }

};

//////////////

class letter_morph : public amorph<letter> {};

//////////////

post_office::post_office()
: _post(new mailbox),
  _routes(new route_map),
  _next_cleaning(new time_stamp),
  _threads(new thread_cabinet)
{
}

post_office::~post_office()
{
  stop_serving();
  WHACK(_post);
  WHACK(_routes);
  WHACK(_next_cleaning);
  WHACK(_threads);
}

void post_office::show_routes(astring &to_fill)
{
  auto_synchronizer l(c_mutt);
//hmmm: simplify this; just use the int_set returning func and print that.
  astring current_line;
  astring temp;
  if (_routes->elements())
    to_fill += astring("Mail Delivery Routes:") + parser_bits::platform_eol_to_chars();

  for (int i = 0; i < _routes->elements(); i++) {
    const tagged_mail_stop *tag = _routes->get(i);
    if (!tag) continue;
    temp = astring(astring::SPRINTF, "%d ", tag->_id.raw_id());
    if (current_line.length() + temp.length() >= 80) {
      current_line += parser_bits::platform_eol_to_chars();
      to_fill += current_line;
      current_line.reset();
    }
    current_line += temp;
  }
  // catch the last line we created.
  if (!!current_line) to_fill += current_line;
}

void post_office::stop_serving() { if (_threads) _threads->stop_all(); }

void post_office::show_mail(astring &output)
{
  output.reset();
  output += parser_bits::platform_eol_to_chars();
  output += astring("Mailbox Contents at ") + time_stamp::notarize(true)
      + parser_bits::platform_eol_to_chars() + parser_bits::platform_eol_to_chars();
  astring box_state;
  _post->show(box_state); 
  if (box_state.t()) output += box_state;
  else
    output += astring("No items are awaiting delivery.")
        + parser_bits::platform_eol_to_chars();
}

void post_office::drop_off(const unique_int &id, letter *package)
{
#ifdef DEBUG_POST_OFFICE
  FUNCDEF("drop_off");
  LOG(astring(astring::SPRINTF, "mailbox drop for %d: ", id)
      + package->text_form());
#endif
  _post->drop_off(id, package); 
#ifdef DEBUG_POST_OFFICE
  if (!route_listed(id)) {
    LOG(a_sprintf("letter for %d has no route!", id));
  }
#endif
}

bool post_office::pick_up(const unique_int &id, letter * &package)
{
#ifdef DEBUG_POST_OFFICE
  FUNCDEF("pick_up");
#endif
  bool to_return = _post->pick_up(id, package); 
#ifdef DEBUG_POST_OFFICE
  if (to_return)
    LOG(astring(astring::SPRINTF, "mailbox grab for %d: ", id)
        + package->text_form());
#endif
  return to_return;
}

bool post_office::route_listed(const unique_int &id)
{
  int_set route_set;
  get_route_list(route_set);
  return route_set.member(id.raw_id());
}

void post_office::get_route_list(int_set &route_set)
{
  auto_synchronizer l(c_mutt);

  // gather the set of routes that we should carry mail to.
  route_set.reset();

  if (!_routes->elements()) {
    // if there are no elements, why bother iterating?
    return;
  }

  for (int i = 0; i < _routes->elements(); i++) {
    const tagged_mail_stop *tag = _routes->get(i);
    if (!tag) continue;
    route_set.add(tag->_id.raw_id());
  }
}

void post_office::clean_package_list(post_office &formal(post),
    letter_morph &to_clean)
{
  FUNCDEF("clean_package_list");
  auto_synchronizer l(c_mutt);

  // recycle all the stuff we had in the list.
  while (to_clean.elements()) {
    letter *package = to_clean.acquire(0);
    to_clean.zap(0, 0);
    if (!package) {
      LOG("saw empty package in list to clean!");
      continue;
    }
    WHACK(package);
  }
}

bool post_office::deliver_mail_on_route(const unique_int &route,
    ethread &carrier)
{
  FUNCDEF("deliver_mail_on_route");
  auto_synchronizer l(c_mutt);

#ifdef DEBUG_POST_OFFICE
  time_stamp enter;
#endif
  if (carrier.should_stop()) return true;  // get out if thread was told to.

  int deliveries = 0;  // number of items delivered so far.
  letter_morph items_for_route;
    // holds the items that need to be sent to this route.

  // pickup all of the mail that we can for this route.
  while (deliveries < DELIVERIES_ALLOWED) {
    if (carrier.should_stop())
      return true;  // get out if thread was told to.
    letter *package;
    if (!_post->pick_up(route, package)) {
      // there are no more letters for this route.
      break;  // skip out of the loop.
    }
    deliveries++;  // count this item as a delivery.
    items_for_route.append(package);
  }

  if (!items_for_route.elements()) return true;  // nothing to handle.

  // locate the destination for this route.
  tagged_mail_stop *real_route = _routes->find(route);  // find the route.
  if (!real_route) {
    // we failed to find the route we wanted...
    LOG(astring(astring::SPRINTF, "route %d disappeared!", route.raw_id()));
    clean_package_list(*this, items_for_route);
    return true;
  }

  // now deliver what we found for this route.
  for (int t = 0; t < items_for_route.elements(); t++) {
    if (carrier.should_stop()) {
      // get out if thread was told to.
      return true;
    }
    letter *package = items_for_route.acquire(t);
    // hand the package out on the route.
    mail_stop::items_to_deliver pack(route, package);
    real_route->_route->invoke_callback(pack);
      // the callee is responsible for cleaning up.
  }

  bool finished_all = (deliveries < DELIVERIES_ALLOWED);
    // true if we handled everything we could have.

  if (carrier.should_stop()) return true;  // get out if thread was told to.

  // this bit is for the post office at large, but we don't want an extra
  // thread when we've got all these others handy.
  bool cleaning_time = time_stamp() > *_next_cleaning;
  if (cleaning_time) {
    _post->clean_up();  // get rid of dead mailboxes in main post office.
    _next_cleaning->reset(CLEANING_INTERVAL);
  }

  time_stamp exit;
#ifdef DEBUG_POST_OFFICE
  int duration = int(exit.value() - enter.value());
  if (duration > 20)
    LOG(a_sprintf("deliveries took %d ms.", duration));
#endif
  return finished_all;
}

bool post_office::register_route(const unique_int &id,
    mail_stop &carrier_path)
{
  auto_synchronizer l(c_mutt);

  tagged_mail_stop *found = _routes->find(id);
  if (found) return false;  // already exists.

  postal_carrier *po = new postal_carrier(*this, id);
  unique_int thread_id = _threads->add_thread(po, false, NULL_POINTER);
    // add the thread so we can record its id.
  tagged_mail_stop *new_stop = new tagged_mail_stop(id, &carrier_path,
      thread_id);
  _routes->append(new_stop);
    // add the mail stop to our listings.
  po->start(NULL_POINTER);
    // now start the thread so it can begin cranking.
  return true;
}

bool post_office::unregister_route(const unique_int &id)
{
  auto_synchronizer l(c_mutt);

  tagged_mail_stop *tag = _routes->find(id);
  if (!tag) return false;  // doesn't exist yet.
  unique_int thread_id = tag->_id;
  _routes->zap(id);
  _threads->zap_thread(thread_id);
  return true;
}

} //namespace.


