/*****************************************************************************\
*                                                                             *
*  Name   : cromp_common                                                      *
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

// NOTES:
// 
//   for a cromp_common that is "normal", the base octopus will be used for
// restoring infotons.
//   for a dependent cromp_common with a singleton and preexisting socket,
// the socket will be used for communications and the singleton octopus will
// be used for restore().
//
//   there are a few tiers of methods here.  the lowest-level tier can be
// called by any other functions except those in the lowest-level (so being on
// tier A implies that a method may not call other methods in tier A, but being
// on a tier X allows calling of all existent tiers X-1, X-2, ...).

//   last verified that conditions stated in header about variables protected
// by accumulator lock are true: 12/30/2002.

#include "cromp_common.h"
#include "cromp_transaction.h"

#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/astring.h>
#include <basis/mutex.h>
#include <crypto/rsa_crypto.h>
#include <loggers/program_wide_logger.h>
#include <octopus/entity_data_bin.h>
#include <octopus/entity_defs.h>
#include <octopus/infoton.h>
#include <octopus/octopus.h>
#include <octopus/tentacle.h>
#include <octopus/unhandled_request.h>
#include <sockets/internet_address.h>
#include <sockets/machine_uid.h>
#include <sockets/spocket.h>
#include <sockets/tcpip_stack.h>
#include <structures/static_memory_gremlin.h>
#include <tentacles/encryption_infoton.h>
#include <textual/byte_formatter.h>
#include <timely/time_stamp.h>

using namespace basis;
using namespace crypto;
using namespace loggers;
using namespace octopi;
using namespace sockets;
using namespace structures;
using namespace textual;
using namespace timely;

namespace cromp {

//#define DEBUG_CROMP_COMMON
  // uncomment for debugging info.

#undef LOG
#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger::get(), astring(to_print).s())

const int STALENESS_PERIOD = 2 * MINUTE_ms;
  // if data sits in the buffer this long without us seeing more, we assume
  // it's gone stale.

const int SEND_DELAY_TIME = 200;
  // if the send failed initially, we'll delay this long before trying again.

const int DATA_AWAIT_SNOOZE = 80;
  // we sleep for this long while we await data.

const int QUICK_CROMP_SNOOZE = 28;
  // we take a quick nap if we're looking for some data and it's not there
  // for us yet.

const int CROMP_BUFFER_CHUNK_SIZE = 256 * KILOBYTE;
  // the initial allocation size for buffers.

const int MAXIMUM_RECEIVES = 70;
  // the maximum number of receptions before we skip to next phase.

const int MAXIMUM_SEND = 128 * KILOBYTE;
  // the largest chunk we try to send at a time.  we want to limit this
  // rather than continually asking the OS to consume a big transmission.

const int CLEANUP_INTERVAL = 28 * SECOND_ms;
  // this is how frequently we'll flush out items from our data bin that
  // are too old.

const int cromp_common::HOSTCHOP = 6;
  // we take this many characters as the readable textual portion of the
  // hostname.

double cromp_common::_bytes_sent_total = 0.0;
double cromp_common::_bytes_received_total = 0.0;

  SAFE_STATIC_CONST(rsa_crypto, _hidden_localhost_only_key,
      (encryption_infoton::RSA_KEY_SIZE))
  const rsa_crypto &cromp_common::localhost_only_key() {
#ifdef DEBUG_CROMP_COMMON
    FUNCDEF("localhost_only_key");
#endif
    static bool initted = false;
#ifdef DEBUG_CROMP_COMMON
    bool was_initted = initted;
#endif
    initted = true;
#ifdef DEBUG_CROMP_COMMON
    if (!was_initted)
      LOG("started creating localhost RSA key.");
#endif
    const rsa_crypto &to_return = _hidden_localhost_only_key();
#ifdef DEBUG_CROMP_COMMON
    if (!was_initted)
      LOG("done creating localhost RSA key.");
#endif
    return to_return;
  }

cromp_common::cromp_common(const astring &host, int max_per_ent)
: _commlink(NULL_POINTER),
  _octopus(new octopus(host, max_per_ent)),
  _singleton(NULL_POINTER),
  _requests(new entity_data_bin(max_per_ent)),
  _accum_lock(new mutex),
  _last_data_seen(new time_stamp),
  _accumulator(new byte_array(CROMP_BUFFER_CHUNK_SIZE, NULL_POINTER)),
  _sendings(new byte_array(CROMP_BUFFER_CHUNK_SIZE, NULL_POINTER)),
  _receive_buffer(new byte_array(CROMP_BUFFER_CHUNK_SIZE, NULL_POINTER)),
  _still_flat(new byte_array(CROMP_BUFFER_CHUNK_SIZE, NULL_POINTER)),
  _last_cleanup(new time_stamp)
{
  FUNCDEF("constructor [host/max_per_ent]");
  // clear pre-existing space.
  _accumulator->reset();
  _sendings->reset();
  _receive_buffer->reset();
  _still_flat->reset();
}

cromp_common::cromp_common(spocket *preexisting, octopus *singleton)
: _commlink(preexisting),
  _octopus(singleton),
  _singleton(singleton),
  _requests(new entity_data_bin(singleton?
      singleton->responses().max_bytes_per_entity()
          : DEFAULT_MAX_ENTITY_QUEUE)),
  _accum_lock(new mutex),
  _last_data_seen(new time_stamp),
  _accumulator(new byte_array(CROMP_BUFFER_CHUNK_SIZE, NULL_POINTER)),
  _sendings(new byte_array(CROMP_BUFFER_CHUNK_SIZE, NULL_POINTER)),
  _receive_buffer(new byte_array(CROMP_BUFFER_CHUNK_SIZE, NULL_POINTER)),
  _still_flat(new byte_array(CROMP_BUFFER_CHUNK_SIZE, NULL_POINTER)),
  _last_cleanup(new time_stamp)
{
  FUNCDEF("constructor [preexisting/singleton]");
  if (!_octopus) {
    // they passed us a bad singleton.  carry on as best we can.
    LOG("singleton passed as NULL_POINTER; constructing new octopus instead.");
    internet_address local(internet_address::localhost(), "localhost", 0);
    _octopus = new octopus(chew_hostname(local), DEFAULT_MAX_ENTITY_QUEUE);
  }
  // clear pre-existing space.
  _accumulator->reset();
  _sendings->reset();
  _receive_buffer->reset();
  _still_flat->reset();
}

cromp_common::~cromp_common()
{
  FUNCDEF("destructor");
  close_common();  // shuts down our socket and other stuff.
  if (_singleton) {
    _singleton = NULL_POINTER;  // reset the pointer we had.
    _octopus = NULL_POINTER;  // ditto.
  } else {
    // this one was ours so we need to clean it up.
    WHACK(_octopus);
  }
  WHACK(_accumulator);
  WHACK(_sendings);
  WHACK(_commlink);
  WHACK(_requests);
  WHACK(_last_cleanup);
  WHACK(_last_data_seen);
  WHACK(_receive_buffer);
  WHACK(_still_flat);
  WHACK(_accum_lock);
}

spocket *cromp_common::spock() const { return _commlink; }

int cromp_common::default_port() { return 10008; }

outcome cromp_common::add_tentacle(tentacle *to_add, bool filter)
{ return _octopus->add_tentacle(to_add, filter); }

int cromp_common::pending_sends() const
{
  auto_synchronizer l(*_accum_lock);
  return _sendings->length();
}

int cromp_common::accumulated_bytes() const
{
  auto_synchronizer l(*_accum_lock);
  return _accumulator->length();
}

astring cromp_common::chew_hostname(const internet_address &addr,
    internet_address *resolved_form)
{
#ifdef DEBUG_CROMP_COMMON
  FUNCDEF("chew_hostname");
  LOG(astring("addr coming in ") + addr.text_form());
#endif
  tcpip_stack stack;
  bool worked;
  internet_address res1 = stack.fill_and_resolve(addr.hostname, addr.port,
      worked);
  if (worked) {
    if (resolved_form) *resolved_form = res1;
#ifdef DEBUG_CROMP_COMMON
    LOG(astring("resolved addr ") + res1.text_form());
#endif
  } else {
#ifdef DEBUG_CROMP_COMMON
    LOG(astring("failed to resolve host=") + addr.hostname);
#endif
  }

  // get a readable form of the host.
  astring just_host = res1.normalize_host();
  while (just_host.length() < HOSTCHOP) just_host += "-";  // filler.
  machine_uid converted = res1.convert();
  astring to_return = just_host.substring(0, HOSTCHOP - 1);
  to_return += converted.compact_form();

#ifdef DEBUG_CROMP_COMMON
  LOG(astring("returning machid ") + converted.text_form() + ", packed as "
      + parser_bits::platform_eol_to_chars()
      + byte_formatter::text_dump((abyte *)to_return.s(),
            to_return.length() + 1));
#endif

  return to_return;
}

astring cromp_common::responses_text_form() const
{ return _requests->text_form(); }

internet_address cromp_common::other_side() const
{
  if (!_commlink) return internet_address();
  return _commlink->where();
}

int cromp_common::max_bytes_per_entity() const
{ return _requests->max_bytes_per_entity(); }

void cromp_common::max_bytes_per_entity(int max_bytes_per_entity)
{
  _requests->max_bytes_per_entity(max_bytes_per_entity);
  _octopus->responses().max_bytes_per_entity(max_bytes_per_entity);
}

void cromp_common::conditional_cleaning()
{
  FUNCDEF("conditional_cleaning");
  if (time_stamp(-CLEANUP_INTERVAL) > *_last_cleanup) {
    _requests->clean_out_deadwood();
      // flush any items that are too old.
    _last_cleanup->reset();
      // record that we just cleaned up.
  }
}

outcome cromp_common::open_common(const internet_address &where)
{
#ifdef DEBUG_CROMP_COMMON
  FUNCDEF("open_common");
#endif
  if (_singleton && _commlink)
    return OKAY;  // done if this uses pre-existing objects.

  if (_commlink) WHACK(_commlink);  // clean up any pre-existing socket.

  internet_address other_side = where;

#ifdef DEBUG_CROMP_COMMON
  LOG(astring("opening at ") + other_side.text_form());
#endif
  _commlink = new spocket(other_side);
//hmmm: check socket health.

  return OKAY;
}

outcome cromp_common::close_common()
{
  if (_commlink) _commlink->disconnect();  // make the thread stop bothering.
  return OKAY;
}

const char *cromp_common::outcome_name(const outcome &to_name)
{
  switch (to_name.value()) {
    case TOO_FULL: return "TOO_FULL";
    case PARTIAL: return "PARTIAL";
    default: return communication_commons::outcome_name(to_name);
  }
}

outcome cromp_common::pack_and_ship(const infoton_list &requests,
    int max_tries)
{
  FUNCDEF("pack_and_ship [multiple]");
  if (!_commlink) return BAD_INPUT;  // they haven't opened this yet.
  conditional_cleaning();
  {
    auto_synchronizer l(*_accum_lock);  // lock while packing.
    for (int i = 0; i < requests.elements(); i++) {
      if (!requests[i] || !requests[i]->_data) {
        // this is a screw-up by someone.
        LOG("error in infoton_list; missing data element.");
        continue;
      }
      cromp_transaction::flatten(*_sendings, *requests[i]->_data,
          requests[i]->_id);
    }
  }

  return push_outgoing(max_tries);
}

bool cromp_common::buffer_clog(int max_buff) const
{
  auto_synchronizer l(*_accum_lock);
  return _sendings->length() >= max_buff;
}

outcome cromp_common::pack_and_ship(const infoton &request,
    const octopus_request_id &item_id, int max_tries)
{
#ifdef DEBUG_CROMP_COMMON
  FUNCDEF("pack_and_ship [single]");
#endif
  if (!_commlink) return BAD_INPUT;  // they haven't opened this yet.
  conditional_cleaning();

#ifdef DEBUG_CROMP_COMMON
  LOG(astring("sending req ") + item_id.mangled_form());
#endif

  {
    auto_synchronizer l(*_accum_lock);  // lock while packing.
    cromp_transaction::flatten(*_sendings, request, item_id);
  }

  return push_outgoing(max_tries);
}

outcome cromp_common::push_outgoing(int max_tries)
{
  FUNCDEF("push_outgoing");

  if (!max_tries) return cromp_common::OKAY;
    // no tries means we're done already.

  grab_anything(false);  // suck any data in that happens to be waiting.

  outcome to_return = cromp_common::TOO_FULL;
  int attempts = 0;
  while ( (attempts++ < max_tries) && (to_return == cromp_common::TOO_FULL) ) {
    to_return = send_buffer();
    grab_anything(false);  // suck any data in that happens to be waiting.
    if (to_return == cromp_common::OKAY)
      break;  // happy returns.
    if (to_return == cromp_common::PARTIAL) {
      // we sent all we tried to but there's more left.
      attempts = 0;  // skip back since we had a successful attempt.
      to_return = cromp_common::TOO_FULL;
        // reset so that we treat this by staying in the send loop.
      continue;  // jump back without waiting.
    }
    if (to_return == cromp_common::TOO_FULL) {
      // we can't send any more yet so delay for a bit to see if we can get
      // some more out.
      time_stamp stop_pausing(SEND_DELAY_TIME);
      while (time_stamp() < stop_pausing) {
LOG("into too full looping...");
        if (!_commlink->connected()) break;
        grab_anything(true);  // suck any data in that happens to be waiting.
        // snooze a bit until we think we can write again.
        outcome ret = _commlink->await_writable(QUICK_CROMP_SNOOZE);
        if (ret != spocket::NONE_READY)
          break;
      }
    } else {
      LOG(astring("failed send: ") + cromp_common::outcome_name(to_return));
      break;
    }
  }
  return to_return;
}

// rules for send_buffer: this function is in the lowest-level tier for using
// the spocket.  it is allowed to be called by anyone.  it must not call any
// other functions on the cromp_common class.
outcome cromp_common::send_buffer()
{
#ifdef DEBUG_CROMP_COMMON
  FUNCDEF("send_buffer");
#endif
  auto_synchronizer l(*_accum_lock);

  // all done if nothing to send.
  if (!_sendings->length())
    return OKAY;

  int size_to_send = minimum(_sendings->length(), MAXIMUM_SEND);
#ifdef DEBUG_CROMP_COMMON
//  LOG(a_sprintf("sending %d bytes on socket %d.", size_to_send,
//      _commlink->OS_socket()));
#endif
  int len_sent = 0;
  outcome to_return;
  outcome send_ret = _commlink->send(_sendings->observe(), size_to_send,
      len_sent);
  switch (send_ret.value()) {
    case spocket::OKAY: {
      // success.
#ifdef DEBUG_CROMP_COMMON
//      LOG(a_sprintf("really sent %d bytes on socket %d.", len_sent,
//          _commlink->OS_socket()));
#endif
      _bytes_sent_total += len_sent;
      to_return = OKAY;
      break;
    }
    case spocket::PARTIAL: {
      // got something done hopefully.
#ifdef DEBUG_CROMP_COMMON
      LOG(a_sprintf("partial send of %d bytes (of %d desired) on socket %d.",
          len_sent, size_to_send, _commlink->OS_socket()));
#endif
      _bytes_sent_total += len_sent;
      to_return = PARTIAL;
      break;
    }
    case spocket::NONE_READY: {
      // did nothing useful.
#ifdef DEBUG_CROMP_COMMON
      LOG(a_sprintf("too full to send any on socket %d.",
          _commlink->OS_socket()));
#endif
      len_sent = 0;  // reset just in case.
      to_return = TOO_FULL;
      break;
    }
    default: {
      // other things went wrong.
#ifdef DEBUG_CROMP_COMMON
      LOG(astring("failing send with ") + spocket::outcome_name(send_ret));
#endif
      len_sent = 0;  // reset just in case.

//hmmm: these are unnecessary now since it's the same set of outcomes.
      if (send_ret == spocket::NO_CONNECTION) to_return = NO_CONNECTION;
      else if (send_ret == spocket::TIMED_OUT) to_return = TIMED_OUT;
//any other ideas?
      else to_return = DISALLOWED;

#ifdef DEBUG_CROMP_COMMON
      LOG(astring("failed to send--got error ") + outcome_name(to_return));
#endif
      break;
    }
  }

  if ( (to_return == PARTIAL) || (to_return == OKAY) ) {
    // accomodate our latest activity on the socket.
    _sendings->zap(0, len_sent - 1);  // sent just some of it.
  }

  return to_return;
}

outcome cromp_common::retrieve_and_restore_root(bool get_anything,
    infoton * &item, octopus_request_id &req_id, int timeout)
{
  FUNCDEF("retrieve_and_restore_root");
  item = NULL_POINTER;
  if (!_commlink) return BAD_INPUT;  // they haven't opened this yet.
  octopus_request_id tmp_id;
  time_stamp leaving_time(timeout);

  conditional_cleaning();

  do {
    // check if it's already in the bin from someone else grabbing it.
    if (get_anything)
      item = _requests->acquire_for_any(req_id);
    else
      item = _requests->acquire_for_identifier(req_id);
    if (item)
      return OKAY;

    // check to see if there's any data.
    grab_anything(timeout? true : false);

    push_outgoing(1);
//hmmm: parameterize the push?

    // check again just to make sure.  this is before we check the timeout,
    // since we could squeak in with something before that.
    if (get_anything)
      item = _requests->acquire_for_any(req_id);
    else
      item = _requests->acquire_for_identifier(req_id);
    if (item)
      return OKAY;

    if (!timeout) return TIMED_OUT;
      // timeout is not set so we leave right away.

    if (!_commlink->connected()) return NO_CONNECTION;

    // keep going if we haven't seen it yet and still have time.
  } while (time_stamp() < leaving_time);
  return TIMED_OUT;
}

outcome cromp_common::retrieve_and_restore(infoton * &item,
    const octopus_request_id &req_id_in, int timeout)
{
  octopus_request_id req_id = req_id_in;
  return retrieve_and_restore_root(false, item, req_id, timeout);
}

outcome cromp_common::retrieve_and_restore_any(infoton * &item,
    octopus_request_id &req_id, int timeout)
{ return retrieve_and_restore_root(true, item, req_id, timeout); }

// rules: snarf_from_socket is in the second lowest-level tier.  it must not
// call any other functions on cromp_common besides the send_buffer and
// process_accumulator methods.
void cromp_common::snarf_from_socket(bool wait)
{
#ifdef DEBUG_CROMP_COMMON
  FUNCDEF("snarf_from_socket");
#endif
  if (wait) {
#ifdef DEBUG_CROMP_COMMON
//    LOG(a_sprintf("awaiting rcptblty on socket %d.", _commlink->OS_socket()));
#endif
    // snooze until data seems ready for chewing or until we time out.
    time_stamp stop_pausing(DATA_AWAIT_SNOOZE);
    while (time_stamp() < stop_pausing) {
      if (!_commlink->connected()) return;
      outcome wait_ret = _commlink->await_readable(QUICK_CROMP_SNOOZE);
      if (wait_ret != spocket::NONE_READY)
        break;
      send_buffer();  // push out some data in between.
    }
  }

  outcome rcv_ret = spocket::OKAY;
  // this loop scrounges as much data as possible, within limits.
  int receptions = 0;
  while ( (rcv_ret == spocket::OKAY) && (receptions++ < MAXIMUM_RECEIVES) ) {
    int rcv_size = CROMP_BUFFER_CHUNK_SIZE;
    {
      auto_synchronizer l(*_accum_lock);
      _receive_buffer->reset();  // clear pre-existing junk.
      rcv_ret = _commlink->receive(*_receive_buffer, rcv_size);
#ifdef DEBUG_CROMP_COMMON
      if ( (rcv_ret == spocket::OKAY) && rcv_size) {
        LOG(a_sprintf("received %d bytes on socket %d", rcv_size,
            _commlink->OS_socket()));
      } else if (rcv_ret != spocket::NONE_READY) {
        LOG(a_sprintf("no data on sock %d--outcome=", _commlink->OS_socket())
            + spocket::outcome_name(rcv_ret));
      }
#endif
      if ( (rcv_ret == spocket::OKAY) && rcv_size) {
        // we got some data from the receive, so store it.
        _bytes_received_total += _receive_buffer->length();
        *_accumulator += *_receive_buffer;  // add to overall accumulator.
        _last_data_seen->reset();
      }
    }

    send_buffer();
      // force data to go out also.
  }
}

void cromp_common::grab_anything(bool wait)
{
  snarf_from_socket(wait);  // get any data that's waiting.
  process_accumulator();  // retrieve any commands we see.
}

#define CHECK_STALENESS \
  if (*_last_data_seen < time_stamp(-STALENESS_PERIOD)) { \
    LOG("would resynch data due to staleness."); \
    _accumulator->zap(0, 0);  /* roast first byte */ \
    cromp_transaction::resynchronize(*_accumulator); \
    _last_data_seen->reset(); \
    continue; \
  }

// process_accumulator should do nothing besides chewing on the buffer.
// this puts it in the lowest-level tier.

void cromp_common::process_accumulator()
{
  FUNCDEF("process_accumulator");
  infoton *item = NULL_POINTER;
  octopus_request_id req_id;

  string_array clas;

  if (!_accumulator->length()) return;

  // a little gymnastics to get a large buffer on the first try.
  byte_array temp_chow_buffer(CROMP_BUFFER_CHUNK_SIZE, NULL_POINTER);
  temp_chow_buffer.reset();

  int cmds_found = 0;

  while (_accumulator->length()) {
LOG(a_sprintf("eating command %d", cmds_found++));
    {
      // first block tries to extract data from the accumulator.
      auto_synchronizer l(*_accum_lock);
      // there are some contents; let's look at them.
      int packed_length = 0;
      outcome peek_ret = cromp_transaction::peek_header(*_accumulator,
          packed_length);
      if ( (peek_ret == cromp_transaction::WAY_TOO_SMALL)
          || (peek_ret == cromp_transaction::PARTIAL) ) {
        // not ready yet.
        CHECK_STALENESS;
        return;
      } else if (peek_ret != cromp_transaction::OKAY) {
        LOG(astring("error unpacking--peek error=")
            + cromp_transaction::outcome_name(peek_ret));
        // try to get to a real command.
        _accumulator->zap(0, 0);  // roast first byte.
        if (cromp_transaction::resynchronize(*_accumulator)) continue;
        return;
      }

#ifdef DEBUG_CROMP_COMMON
      LOG("seeing command ready");
#endif
      // temp buffer for undoing cromp transaction.
      if (!cromp_transaction::unflatten(*_accumulator, *_still_flat, req_id)) {
        LOG("failed to unpack even though peek was happy!");
        // try to get to a real command.
        _accumulator->zap(0, 0);  // roast first byte.
        if (cromp_transaction::resynchronize(*_accumulator)) continue;
        return;
      }
#ifdef DEBUG_CROMP_COMMON
      LOG(astring("got req id of ") + req_id.mangled_form());
#endif

      // now unwrap the onion a bit more to find the real object being sent.
      if (!infoton::fast_unpack(*_still_flat, clas, temp_chow_buffer)) {
        // try to resynch on transaction boundary.
        LOG("failed to get back a packed infoton!");
        _accumulator->zap(0, 0);  // roast first byte.
        if (cromp_transaction::resynchronize(*_accumulator)) continue;
        return;
      }
#ifdef DEBUG_CROMP_COMMON
      LOG(astring("got classifier of ") + clas.text_form());
#endif
    } // end of protected area.

    // restore the infoton from the packed form.
    outcome rest_ret = octo()->restore(clas, temp_chow_buffer, item);
    if (rest_ret != tentacle::OKAY) {
#ifdef DEBUG_CROMP_COMMON
      LOG(astring("our octopus couldn't restore the packed data! ")
          + outcome_name(rest_ret));
#endif
      // publish an unhandled request back to the requestor.
      _requests->add_item(new unhandled_request(req_id, clas, rest_ret),
          req_id);
    } else {
      // we finally have reached a point where we have a valid infoton.
      if (_requests->add_item(item, req_id))
        cmds_found++;
#ifdef DEBUG_CROMP_COMMON
      else
        LOG("failed to add item to bin due to space constraints.");
#endif
    }
LOG(a_sprintf("ate command %d", cmds_found));
  }
///  LOG(a_sprintf("added %d commands", cmds_found));
}

bool cromp_common::decode_host(const astring &coded_host, astring &hostname,
    machine_uid &machine)
{
  if (coded_host.length() < HOSTCHOP) return false;  // not big enough.
  hostname = coded_host.substring(0, cromp_common::HOSTCHOP - 1);
  const astring compact_uid = coded_host.substring(cromp_common::HOSTCHOP,
      coded_host.length() - 1);
  machine = machine_uid::expand(compact_uid);
  if (!machine.valid()) return false;
  return true;
}

} //namespace.

