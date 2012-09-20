/*****************************************************************************\
*                                                                             *
*  Name   : cromp_server                                                      *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "cromp_common.h"
#include "cromp_security.h"
#include "cromp_server.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/mutex.h>
#include <loggers/program_wide_logger.h>
#include <octopus/entity_data_bin.h>
#include <octopus/entity_defs.h>
#include <octopus/identity_infoton.h>
#include <octopus/infoton.h>
#include <octopus/tentacle.h>
#include <octopus/unhandled_request.h>
#include <processes/ethread.h>
#include <processes/thread_cabinet.h>
#include <sockets/internet_address.h>
#include <sockets/tcpip_stack.h>
#include <sockets/spocket.h>
#include <structures/amorph.h>
#include <structures/unique_id.h>
#include <tentacles/key_repository.h>
#include <tentacles/login_tentacle.h>
#include <tentacles/encryption_tentacle.h>
#include <tentacles/encryption_wrapper.h>
#include <timely/time_control.h>

using namespace basis;
using namespace loggers;
using namespace octopi;
using namespace processes;
using namespace sockets;
using namespace structures;
using namespace timely;

namespace cromp {

//#define DEBUG_CROMP_SERVER
  // uncomment for noisy version.

const int DEAD_CLIENT_CLEANING_INTERVAL = 1 * SECOND_ms;
  // we will drop any clients that have disconnected this long ago.

const int MAXIMUM_ACTIONS_PER_CLIENT = 4000;
  // this is the maximum number of things we'll do in one run for a
  // client, including both sends and receives.

const int SEND_TRIES_ALLOWED = 1;
  // the number of attempts we will make to get outgoing data to send.

const int SEND_THRESHOLD = 512 * KILOBYTE;
  // if we pile up some data to this point in our client gathering, we'll
  // go ahead and start pushing it to the client.

const int EXTREME_SEND_TRIES_ALLOWED = 28;
  // if we're clogged, we'll push this many times to get data out.

const int MAXIMUM_BYTES_PER_SEND = 2 * MEGABYTE;
  // the maximum size we want our buffer to grow.

const int MAXIMUM_SIZE_BATCH = 384 * KILOBYTE;
  // the largest chunk of updates we'll try to grab at one time.

const int DROPPING_INTERVAL = 500;
  // the rate at which we'll check for dead clients and clean up.

const int DATA_AWAIT_TIMEOUT = 14;
  // how long the server zones out waiting for data.

const int ACCEPTANCE_SNOOZE = 60;
  // if the server sees no clients, it will take a little nap.

#undef LOG
#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger::get(), astring(to_print).s())

//////////////

// forward.
class cromp_client_record;

class cromp_data_grabber : public ethread
{
public:
  cromp_data_grabber(cromp_client_record &parent, octopus *octo)
      : ethread(), _parent(parent), _octo(octo) {}

  DEFINE_CLASS_NAME("cromp_data_grabber");

  virtual void perform_activity(void *);

private:
  cromp_client_record &_parent;
  octopus *_octo;
};
 
//////////////

class cromp_client_record : public cromp_common
{
public:
  cromp_client_record(cromp_server &parent, spocket *client, octopus *octo,
      login_tentacle &security)
  : cromp_common(client, octo),
    _parent(parent),
    _octo(octo),
    _ent(),
    _healthy(true),
    _fixated(false),
    _grabber(*this, octo),
    _waiting(),
    _still_connected(true),
    _security_arm(security)
  {
    internet_address local_addr = internet_address
        (internet_address::localhost(), client->stack().hostname(), 0);
    open_common(local_addr);  // open the common support for biz.
    _grabber.start(NIL);  // crank up our background data pump on the socket.
  }

  ~cromp_client_record() {
    croak();
  }

  DEFINE_CLASS_NAME("cromp_client_record");

  bool handle_client_needs(ethread &prompter) {
#ifdef DEBUG_CROMP_SERVER
    FUNCDEF("handle_client_needs");
    time_stamp start;
#endif
    if (!_healthy) return false;  // done.
    if (!spock()->connected()) {
      _still_connected = false;
      return false;  // need to stop now.
    }
    bool keep_going = true;
    int actions = 0;
    while (keep_going && (actions < MAXIMUM_ACTIONS_PER_CLIENT) ) {
      // make sure we don't overstay our welcome when the thread's supposed
      // to quit.
      if (prompter.should_stop()) return false;
      keep_going = false;  // only continue if there's a reason.
      bool ret = get_incoming_data(actions);  // look for requests.
      if (ret) keep_going = true;
      ret = push_client_replies(actions);  // send replies back to the client.
      if (ret) keep_going = true;
    }

#ifdef DEBUG_CROMP_SERVER
    if (actions > 10) {
      LOG(a_sprintf("actions=%d", actions));
      LOG(a_sprintf("%d pending send bytes, %d bytes accumulated, bin has "
          "%d items.", pending_sends(), accumulated_bytes(),
          octo()->responses().items_held()));
    }

    int duration = int(time_stamp().value() - start.value());
    if (duration > 200) {
      LOG(a_sprintf("duration=%d ms.", duration));
    }
#endif

    return true;
  }

  const octopus_entity &ent() const { return _ent; }

  // stops the background activity of this object and drops the connection
  // to the client.
  void croak() {
    FUNCDEF("croak");
    _grabber.stop();
    int actions = 0;
    while (get_incoming_data(actions)) {
      // keep receiving whatever's there already.  we are trying to drain
      // the socket before destroying it.
    }
    _healthy = false;
    // clean out any records for this goner.
    _security_arm.expunge(_ent);
    close_common();
  }

  bool healthy() const { return _healthy; }
    // this is true unless the object has been told to shut down.

  bool still_connected() const { return _still_connected; }
    // this is true unless the client side dropped the connection.

  cromp_server &parent() const { return _parent; }

  bool push_client_replies(int &actions) {
    FUNCDEF("push_client_replies");
    if (!healthy()) return false;
    if (ent().blank()) {
      // not pushing replies if we haven't even gotten a command yet.
#ifdef DEBUG_CROMP_SERVER
      LOG("not pushing replies for blank.");
#endif
      return false;
    }

    if (buffer_clog(MAXIMUM_BYTES_PER_SEND)) {
LOG("buffer clog being cleared now.");
      // the buffers are pretty full; we'll try later.
      push_outgoing(EXTREME_SEND_TRIES_ALLOWED);
      // if we're still clogged, then leave.
      if (buffer_clog(MAXIMUM_BYTES_PER_SEND)) {
LOG("could not completely clear buffer clog.");
        return true;
      }
LOG("cleared out buffer clog.");
    }

    int any_left = true;
    while (actions++ < MAXIMUM_ACTIONS_PER_CLIENT) {
      // make sure we're not wasting our time.
      if (!_octo->responses().items_held()) {
        any_left = false;
        break;
      }
      // make sure we don't ignore receptions.
      grab_anything(false);
      // try to grab a result for this entity.
      int num_located = _octo->responses().acquire_for_entity(ent(),
          _waiting, MAXIMUM_SIZE_BATCH);
      if (!num_located) {
        any_left = false;
        break;
      }

      // if we're encrypting, we need to wrap these as well.
      if (_parent.encrypting()) {
        for (int i = 0; i < _waiting.elements(); i++) {
          infoton *curr = _waiting[i]->_data;
          infoton *processed = _parent.wrap_infoton(curr,
              _waiting[i]->_id._entity);
          if (processed) _waiting[i]->_data = processed;  // replace infoton.
        }
      }

      outcome ret = pack_and_ship(_waiting, 0);
        // no attempt to send yet; we're just stuffing the buffer.
      if ( (ret != cromp_common::OKAY) && (ret != cromp_common::TIMED_OUT) ) {
//hmmm: what about keeping transmission as held in list; retry later on it?

//#ifdef DEBUG_CROMP_SERVER
        LOG(astring("failed to send package back to client: ")
            + cromp_common::outcome_name(ret));
//#endif
        any_left = false;
        break;
      }

      if (pending_sends() > SEND_THRESHOLD) {
#ifdef DEBUG_CROMP_SERVER
        LOG(astring("over sending threshold on ") + _ent.text_form());
#endif
        push_outgoing(SEND_TRIES_ALLOWED);
      }

    }
    // now that we've got a pile possibly, we'll try to send them out.
    push_outgoing(SEND_TRIES_ALLOWED);
    if (!spock()->connected()) {
#ifdef DEBUG_CROMP_SERVER
      LOG("noticed disconnection of client.");
#endif
      _still_connected = false;
    }
    return any_left;
  }

  bool get_incoming_data(int &actions) {
    FUNCDEF("get_incoming_data");
    if (!healthy()) return false;
    int first_one = true;
    bool saw_something = false;  // true if we got a packet.
    while (actions++ < MAXIMUM_ACTIONS_PER_CLIENT) {
      // pull in anything waiting.
      infoton *item = NIL;
      octopus_request_id req_id;
      outcome ret = retrieve_and_restore_any(item, req_id,
          first_one? DATA_AWAIT_TIMEOUT : 0);
      first_one = false;
      if (ret == cromp_common::TIMED_OUT) {
        actions--;  // didn't actually eat one.
        return false;
      } else if (ret != cromp_common::OKAY) {
#ifdef DEBUG_CROMP_SERVER
        LOG(astring("got error ") + cromp_common::outcome_name(ret));
#endif
        if (ret == cromp_common::NO_CONNECTION) {
#ifdef DEBUG_CROMP_SERVER
          LOG("noticed disconnection of client.");
#endif
          _still_connected = false;
        }
        actions--;  // didn't actually eat one.
        return false;  // get outa here.
      }
      // got a packet.
      saw_something = true;
      if (!_fixated) {
        if (req_id._entity.blank()) {
          LOG(astring("would have assigned ours to blank id! ")
              + req_id._entity.mangled_form());
          WHACK(item);
          continue;
        }
#ifdef DEBUG_CROMP_SERVER
        LOG(astring("cmd with entity ") + req_id._entity.mangled_form());
#endif
        if (_ent.blank()) {
          // assign the entity id now that we know it.
          _ent = req_id._entity;
#ifdef DEBUG_CROMP_SERVER
          LOG(astring("assigned own entity to ") + _ent.mangled_form());
#endif
        } else if (!_fixated && (_ent != req_id._entity) ) {
#ifdef DEBUG_CROMP_SERVER
          LOG(astring("fixated on entity of ") + req_id._entity.mangled_form()
              + " where we used to have " + _ent.mangled_form());
#endif
          _ent = req_id._entity;
          _fixated = true;
        }
      }  // connects to line after debug just below.
#ifdef DEBUG_CROMP_SERVER
        else if (_ent != req_id._entity) {
        // this checks the validity of the entity.
#ifdef DEBUG_CROMP_SERVER
        LOG(astring("seeing wrong entity of ") + req_id._entity.mangled_form()
            + " when we fixated on " + _ent.mangled_form());
#endif
        WHACK(item);
        continue;
      }
#endif
      // check again so we make sure we're still healthy; could have changed
      // state while getting a command.
      if (!healthy()) {
        WHACK(item);
        continue;
      }
      string_array classif = item->classifier();
        // hang onto the classifier since the next time we get a chance, the
        // object might be destroyed.

      // we pass responsibility for this item over to the octopus.  that's why
      // we're not deleting it once evaluate gets the item.
      ret = _octo->evaluate(item, req_id, _parent.instantaneous());
      if (ret != tentacle::OKAY) {
#ifdef DEBUG_CROMP_SERVER
        LOG(astring("failed to evaluate the infoton we got: ")
            + classif.text_form());
#endif
//hmmm: we have upgraded this response to be for all errors, since otherwise
//      clients will just time out waiting for something that's never coming.

        // we do a special type of handling when the tentacle is missing.  this
        // is almost always because the wrong type of request is being sent to
        // a server, or the server didn't register for all the objects it is
        // supposed to handle.
/////        if (ret == tentacle::NOT_FOUND) {
//#ifdef DEBUG_CROMP_SERVER
          LOG(astring("injecting unhandled note into response stream for ")
              + req_id.text_form() + ", got outcome " + outcome_name(ret));
//#endif
          _parent.send_to_client(req_id,
              new unhandled_request(req_id, classif, ret));
            // this will always work, although it's not a surety that the
            // client actually still exists.  probably though, since we're
            // just now handling this request.
/////        }
      }
    }
    return saw_something;  // keep going if we actually did anything good.
  }

private:
  cromp_server &_parent;  // the object that owns this client.
  octopus *_octo;
  octopus_entity _ent;  // the entity by which we know this client.
  bool _healthy;  // reports our current state of happiness.
  bool _fixated;  // true if the entity id has become firm.
  cromp_data_grabber _grabber;  // the data grabbing thread.
  infoton_list _waiting;
    // used by the push_client_replies() method; allocated once to avoid churn.
  bool _still_connected;
    // set to true up until we notice that the client disconnected.
  login_tentacle &_security_arm;  // provides login checking.
};

//////////////

void cromp_data_grabber::perform_activity(void *)
{
#ifdef DEBUG_CROMP_SERVER
  FUNCDEF("perform_activity");
#endif
  while (!should_stop()) {
//    time_stamp started;
    bool ret = _parent.handle_client_needs(*this);
//    int duration = int(time_stamp().value() - started.value());
    if (!ret) {
      // they said to stop.
#ifdef DEBUG_CROMP_SERVER
      LOG("done handling client needs.");
#endif
      _octo->expunge(_parent.ent());
      break;
    }
  }
}

//////////////

class cromp_client_list : public amorph<cromp_client_record>
{
public:
  int find(const octopus_entity &to_find) const {
    for (int i = 0; i < elements(); i++)
      if (to_find == get(i)->ent()) return i;
    return common::NOT_FOUND;
  }
};

//////////////

class client_dropping_thread : public ethread
{
public:
  client_dropping_thread (cromp_server &parent)
  : ethread(DROPPING_INTERVAL),
    _parent(parent) {}

  void perform_activity(void *formal(ptr)) {
    FUNCDEF("perform_activity");
    _parent.drop_dead_clients(); 
  }

private:
  cromp_server &_parent;  // we perform tricks for this object.
};

//////////////

class connection_management_thread : public ethread
{
public:
  connection_management_thread(cromp_server &parent)
  : ethread(),
    _parent(parent) {}

  void perform_activity(void *formal(ptr)) {
    FUNCDEF("perform_activity");
    _parent.look_for_clients(*this); 
  }

private:
  cromp_server &_parent;  // we perform tricks for this object.
};

//////////////

#undef LOCK_LISTS
#define LOCK_LISTS auto_synchronizer l(*_list_lock)
  // takes over access to the client list and root socket.

cromp_server::cromp_server(const internet_address &where,
    int accepting_threads, bool instantaneous, int max_per_ent)
: cromp_common(cromp_common::chew_hostname(where), max_per_ent),
  _clients(new cromp_client_list),
  _accepters(new thread_cabinet),
  _list_lock(new mutex),
  _next_droppage(new time_stamp(DEAD_CLIENT_CLEANING_INTERVAL)),
  _instantaneous(instantaneous),
  _where(new internet_address(where)),
  _accepting_threads(accepting_threads),
  _dropper(new client_dropping_thread(*this)),
  _enabled(false),
  _encrypt_arm(NIL),
  _default_security(new cromp_security),
  _security_arm(NIL)
{
  FUNCDEF("constructor");
}
 
cromp_server::~cromp_server()
{
  disable_servers();
  WHACK(_accepters);
  WHACK(_dropper);
  WHACK(_clients);
  WHACK(_next_droppage);
  WHACK(_where);
  WHACK(_default_security);
  WHACK(_list_lock);
  _encrypt_arm = NIL;
  _security_arm = NIL;
}

internet_address cromp_server::location() const { return *_where; }

bool cromp_server::get_sizes(const octopus_entity &id, int &items, int &bytes)
{ return octo()->responses().get_sizes(id, items, bytes); }

internet_address cromp_server::any_address(int port)
{
  const abyte any_list[] = { 0, 0, 0, 0 };
  return internet_address(byte_array(4, any_list), "", port);
}

astring cromp_server::responses_text_form() const
{ return octo()->responses().text_form(); }

int cromp_server::DEFAULT_ACCEPTERS() {
  // default number of listening threads; this is the maximum number of mostly
  // simultaneous connections that the server can pick up at a time.
  return 7;  // others are not generally so limited on resources.
}

infoton *cromp_server::wrap_infoton(infoton * &request,
    const octopus_entity &ent)
{
  FUNCDEF("wrap_infoton");
  if (!_enabled) return NIL;
  // identity is not wrapped with encryption; we need to establish and identity
  // to talk on a distinct channel with the server.  even if that identity were
  // compromised, the interloper should still not be able to listen in on the
  // establishment of an encryption channel.  also, the encryption startup
  // itself is not encrypted and we don't want to re-encrypt the wrapper.
  if (dynamic_cast<identity_infoton *>(request)
      || dynamic_cast<encryption_infoton *>(request)
      || dynamic_cast<encryption_wrapper *>(request)) return NIL;

#ifdef DEBUG_CROMP_SERVER
  LOG(astring("encrypting ") + request->text_form());
#endif

  octenc_key_record *key = _encrypt_arm->keys().lock(ent);
    // lock here is released a bit down below.
  if (!key) {
    LOG(astring("failed to locate key for entity ") + ent.text_form());
    return NIL;
  }
  byte_array packed_request;
  infoton::fast_pack(packed_request, *request);
  WHACK(request);
  encryption_wrapper *to_return = new encryption_wrapper;
  key->_key.encrypt(packed_request, to_return->_wrapped);
  _encrypt_arm->keys().unlock(key);
  return to_return;
}

outcome cromp_server::enable_servers(bool encrypt, cromp_security *security)
{
  FUNCDEF("enable_servers");
  if (encrypt) {
    // add the tentacles needed for encryption.
#ifdef DEBUG_CROMP_SERVER
    LOG(astring("enabling encryption for ") + class_name()
        + " on " + _where->text_form());
#endif
    _encrypt_arm = new encryption_tentacle;
    add_tentacle(_encrypt_arm, true);
    add_tentacle(new unwrapping_tentacle, false);
  }
  WHACK(_security_arm);  // in case being reused.
  if (security) {
    _security_arm = new login_tentacle(*security);
    add_tentacle(_security_arm, true);
  } else {
    _security_arm = new login_tentacle(*_default_security);
    add_tentacle(_security_arm, true);
  }
  open_common(*_where);  // open the common ground.

  _enabled = true;
  // try first accept, no waiting.
  outcome to_return = accept_one_client(false);
  if ( (to_return != common::NOT_FOUND) && (to_return != common::OKAY) ) {
    LOG(astring("failure starting up server: ") + outcome_name(to_return));
    return to_return;
  }

#ifdef DEBUG_CROMP_SERVER
  LOG(a_sprintf("adding %d accepting threads.", _accepting_threads));
#endif
  for (int i = 0; i < _accepting_threads; i++) {
    // crank in a new thread and tell it yes on starting it.
    _accepters->add_thread(new connection_management_thread(*this), true, NIL);
  }

  _dropper->start(NIL);
  return OKAY;
}

void cromp_server::disable_servers()
{
  FUNCDEF("disable_servers");
  if (!_enabled) return;
  _dropper->stop();  // signal the thread to leave when it can.
  _accepters->stop_all();  // signal the accepting threads to exit.
  if (_clients) {
    LOCK_LISTS;
      // make sure no one rearranges or uses the client list while we're
      // working on it.
    for (int i = 0; i < _clients->elements(); i++) {
      // stop the client's activities before the big shutdown.
      cromp_client_record *cli = (*_clients)[i];
      if (cli) cli->croak();
    }
  }

  close_common();  // zap the socket so that our blocked waiters get woken up.

  // now finalize the shutdown.  we don't grab the lock because we don't want
  // a deadlock, but we also shouldn't need to grab the lock.  by here, we have
  // cancelled all threads, no new clients should be able to be added, and the
  // destruction of this list will ensure that each client's thread really is
  // stopped.
  WHACK(_clients);

  _enabled = false;  // record our defunctivity.
}

int cromp_server::clients() const
{
  LOCK_LISTS;
  return _clients? _clients->elements() : 0;
}

bool cromp_server::disconnect_entity(const octopus_entity &id)
{
  FUNCDEF("disconnect_entity");
  if (!_enabled) return false;
  LOCK_LISTS;
  int indy = _clients->find(id);
  if (negative(indy)) return false;  // didn't find it.
  cromp_client_record *cli = (*_clients)[indy];
  // disconnect the client and zap its entity records.
  cli->croak();
  return true;
}

bool cromp_server::find_entity(const octopus_entity &id,
    internet_address &found)
{
  FUNCDEF("find_entity");
  if (!_enabled) return false;
  found = internet_address();
  LOCK_LISTS;
  int indy = _clients->find(id);
  if (negative(indy)) return false;  // didn't find it.
  cromp_client_record *cli = (*_clients)[indy];
    // pull out the address from the record at that index.
  found = cli->spock()->remote();
  return true;
}

outcome cromp_server::accept_one_client(bool wait)
{
#ifdef DEBUG_CROMP_SERVER
  FUNCDEF("accept_one_client");
#endif
  if (!_enabled) return common::INCOMPLETE;
  spocket *accepted = NIL;
//printf((timestamp(true, true) + "into accept\n").s());
  outcome ret = spock()->accept(accepted, wait);
//printf((timestamp(true, true) + "out of accept\n").s());
    // accept and wait for it to finish.
  if ( (ret == spocket::OKAY) && accepted) {
    // we got a new client to talk to.
    cromp_client_record *adding = new cromp_client_record(*this, accepted,
        octo(), *_security_arm);
#ifdef DEBUG_CROMP_SERVER
    LOG(a_sprintf("found a new client on sock %d.", accepted->OS_socket()));
#endif
    LOCK_LISTS;  // short term lock.
    _clients->append(adding);
    return OKAY;
  } else {
    if (ret == spocket::NO_CONNECTION)
      return NOT_FOUND;  // normal occurrence.
#ifdef DEBUG_CROMP_SERVER
    LOG(astring("error accepting client: ") + spocket::outcome_name(ret));
#endif
    return DISALLOWED;
  }
}

void cromp_server::look_for_clients(ethread &requestor)
{
  FUNCDEF("look_for_clients");
  if (!_enabled) return;
  // see if any clients have been accepted.
  while (!requestor.should_stop()) {
    outcome ret = accept_one_client(false);
    if ( (ret != OKAY) && (ret != NOT_FOUND) ) {
      // we got an error condition besides our normal set.
//#ifdef DEBUG_CROMP_SERVER
      LOG(astring("got real error on socket; leaving for good.")
          + spocket::outcome_name(ret));
//#endif
      break;
    }
    // if we weren't told we got a client, then we'll sleep.  if we did get
    // a client, we'll try again right away.
    if (ret != OKAY)
      time_control::sleep_ms(ACCEPTANCE_SNOOZE);
  }
}

outcome cromp_server::send_to_client(const octopus_request_id &id,
    infoton *data)
{
#ifdef DEBUG_CROMP_SERVER
  FUNCDEF("send_to_client");
#endif
  if (!_enabled) return common::INCOMPLETE;
  if (!octo()->responses().add_item(data, id)) {
#ifdef DEBUG_CROMP_SERVER
    LOG("failed to store result for client--no space left currently.");
#endif
    return TOO_FULL;
  }
  return OKAY;
}

/*outcome cromp_server::get_any_from_client(const octopus_entity &ent,
    infoton * &data, int timeout)
{
  FUNCDEF("get_from_client");
//hmmm: this implementation locks the lists; can't we get the client to do
//      most of the work for this?
  LOCK_LISTS;
  int indy = _clients->find(id._entity);
  if (negative(indy)) return NOT_FOUND;  // didn't find it.
  cromp_client_record *cli = (*_clients)[indy];
  octopus_request_id id;
  return cli->retrieve_and_restore_any(data, ent, timeout);
}
*/

outcome cromp_server::get_from_client(const octopus_request_id &id,
    infoton * &data, int timeout)
{
  FUNCDEF("get_from_client");
  if (!_enabled) return common::INCOMPLETE;
//hmmm: this implementation locks the lists; can't we get the client to do
//      most of the work for this?
  LOCK_LISTS;
  int indy = _clients->find(id._entity);
  if (negative(indy)) return NOT_FOUND;  // didn't find it.
  cromp_client_record *cli = (*_clients)[indy];
  return cli->retrieve_and_restore(data, id, timeout);
}

void cromp_server::drop_dead_clients()
{
#ifdef DEBUG_CROMP_SERVER
  FUNCDEF("drop_dead_clients");
#endif
  if (!_enabled) return;
  // clean out any dead clients.

  {
    LOCK_LISTS;
    if (time_stamp() < *_next_droppage) return;  // not time yet.
  }

  LOCK_LISTS;  // keep locked from now on.
  for (int i = 0; i < _clients->elements(); i++) {
    cromp_client_record *cli = (*_clients)[i];
    if (!cli) {
#ifdef DEBUG_CROMP_SERVER
      LOG(astring("error in list structure."));
#endif
      _clients->zap(i, i);
      i--;   // skip back before deleted guy.
      continue;
    }
    if (!cli->still_connected() || !cli->healthy()) {
#ifdef DEBUG_CROMP_SERVER
      LOG(astring("dropping disconnected client ") + cli->ent().mangled_form());
#endif
      cli->croak();  // stop it from operating.

//hmmm: check if it has data waiting and complain about it perhaps.
      _clients->zap(i, i);
      i--;   // skip back before deleted guy.
      continue;
    }
  }

  _next_droppage->reset(DEAD_CLIENT_CLEANING_INTERVAL);
}

} //namespace.

