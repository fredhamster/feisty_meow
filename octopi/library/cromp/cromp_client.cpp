/*****************************************************************************\
*                                                                             *
*  Name   : cromp_client                                                      *
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

#include "cromp_client.h"
#include "cromp_common.h"
#include "cromp_transaction.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/mutex.h>
#include <configuration/application_configuration.h>
#include <crypto/rsa_crypto.h>
#include <loggers/program_wide_logger.h>
#include <mathematics/chaos.h>
#include <octopus/entity_defs.h>
#include <octopus/identity_infoton.h>
#include <octopus/unhandled_request.h>
#include <processes/ethread.h>
#include <sockets/internet_address.h>
#include <sockets/machine_uid.h>
#include <sockets/spocket.h>
#include <sockets/tcpip_stack.h>
#include <structures/static_memory_gremlin.h>
#include <structures/roller.h>
#include <tentacles/encryption_tentacle.h>
#include <tentacles/encryption_wrapper.h>
#include <tentacles/entity_registry.h>
#include <tentacles/key_repository.h>
#include <tentacles/login_tentacle.h>
#include <tentacles/security_infoton.h>
#include <timely/time_control.h>
#include <timely/time_stamp.h>

using namespace basis;
using namespace configuration;
using namespace crypto;
using namespace loggers;
using namespace mathematics;
using namespace octopi;
using namespace processes;
using namespace sockets;
using namespace structures;
using namespace timely;

namespace cromp {

//#define DEBUG_CROMP_CLIENT
  // uncomment for noisier version.

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

const int MAX_CONN_ATTEMPTS = 3;
  // the number of times we'll retry connecting for certain errors.

const int INTERCONNECTION_SNOOZE = 200;
  // we will pause this long if the initial connection attempt failed, and
  // in between each attempt thereafter except the last.

// grab control of the class, preventing multiple threads from trampling data.
#define AUTO_LOCK \
  auto_synchronizer l(*_lock)

// make sure the client is in an operational state.
#define CHECK_LOCKOUT \
  if (_disallowed) { \
    /* we can't do anything now due to the state of the connection. */ \
    return NO_CONNECTION; \
  }

// tries to get a particular type of object back from an infoton response.
#define CAST_REPLY(type, varname, newvar, retval) \
  type *newvar = dynamic_cast<type *>(varname); \
  if (!newvar) { \
    LOG("failed to cast " #varname " to appropriate type, " #type "."); \
    WHACK(varname); \
    return retval; \
  }

//////////////

class asynch_connection_thread : public ethread
{
public:
  asynch_connection_thread(cromp_client &parent)
      : ethread(), _parent(parent) {}
  ~asynch_connection_thread() { stop(); }
  void perform_activity(void *formal(ptr)) {
    FUNCDEF("perform_activity");
    while (!should_stop()) {
      if (_parent.connected()) {
        LOG(_parent.instance_name() + " got connected.");
        break;  // done?
      }
      // invoke the real connection maker.  we should be synchronized wrt
      // multiple threads since the "_disallowed" flag is set before this
      // thread is ever started.  no one that locks the cromp_client will
      // get a chance to interfere.
      LOG(_parent.instance_name() + " still unconnected; trying connect now.");
      _parent.locked_connect();
      LOG(_parent.instance_name()
          + " done calling connect.");
    }
    // single shot thread is exiting now.
    _parent._disallowed = false;
  }

private:
  cromp_client &_parent;
};

//////////////

cromp_client::cromp_client(const internet_address &addr, int connection_wait,
    int max_per_ent)
: cromp_common(cromp_common::chew_hostname(addr), max_per_ent),
  _encrypting(false),
  _connection_wait(connection_wait),
  _lock(new mutex),
  _ent(new octopus_entity(randomize_entity())),
  _req_id(new int_roller(1, MAXINT32 - 20)),
  _identified(false),
  _authorized(false),
  _disallowed(false),
  _asynch_connector(NIL),
  _channel_secured(false),
  _crypto(new blowfish_crypto(encryption_infoton::BLOWFISH_KEY_SIZE)),
  _encrypt_arm(NIL),
  _guardian(new blank_entity_registry),
  c_verification(new byte_array)
{
#ifdef DEBUG_CROMP_CLIENT
  FUNCDEF("constructor");
  LOG(astring("initial entity=") + _ent->mangled_form());
#endif
  open_common(addr);

  // add simple security handling.
  add_tentacle(new login_tentacle(*_guardian));
    // add a non-filtering tentacle for checking security.  we mainly need
    // this to be able to unpack answers from the server.
}

cromp_client::~cromp_client()
{
  FUNCDEF("destructor");
  disconnect();
  close_common();
  _identified = false;
  _authorized = false;
  WHACK(_ent);
  WHACK(_req_id);
  _channel_secured = false;
  WHACK(_crypto);
  WHACK(_guardian);
  WHACK(c_verification);
  WHACK(_lock);
}

bool cromp_client::connected() const { return spock()->connected(); }

const byte_array &cromp_client::verification() const
{ return *c_verification; }

void cromp_client::enable_encryption()
{
  FUNCDEF("enable_encryption");
  AUTO_LOCK;

#ifdef DEBUG_CROMP_CLIENT
  LOG(astring("enabling encryption for ") + class_name() + " on "
      + other_side().text_form());
#endif
  _encrypting = true;

  // plug in the encryption support.
  if (other_side().is_localhost()) {
    // if the address is localhost, then we will use the standard key.
    byte_array temp_priv_key;
    localhost_only_key().private_key(temp_priv_key);
    _encrypt_arm = new encryption_tentacle(temp_priv_key);
//hmmm: there is a risk that if they reset to a new address we'd still be
//      using the slightly less secure local key.  could be ameliorated by
//      zapping the encryption tentacle for a reset and readding it if it
//      existed?
  } else
    _encrypt_arm = new encryption_tentacle(encryption_infoton::RSA_KEY_SIZE);
  add_tentacle(_encrypt_arm, true);
  add_tentacle(new unwrapping_tentacle, false);
}

void cromp_client::stop_asynch_thread()
{
#ifdef DEBUG_CROMP_CLIENT
  FUNCDEF("stop_asynch_thread");
#endif
  if (_asynch_connector) {
#ifdef DEBUG_CROMP_CLIENT
    LOG(instance_name() + " stopping thread.");
#endif
    _asynch_connector->cancel();  // send it a nudge before we grab control.
    AUTO_LOCK;  // lock the class to prevent interference.
    _asynch_connector->stop();
    WHACK(_asynch_connector);
  }
  _disallowed = false;  // no longer running the background thread.
}

void cromp_client::reset(const internet_address &addr, int connection_wait,
    int max_per_ent)
{
#ifdef DEBUG_CROMP_CLIENT
  FUNCDEF("reset");
#endif
  stop_asynch_thread();
  AUTO_LOCK;
  close_common();  // shut down the low-level stuff.
  max_bytes_per_entity(max_per_ent);
  *_ent = randomize_entity();
  _req_id->set_current(1);
  _identified = false;
  _authorized = false;
  _channel_secured = false;
  _connection_wait = connection_wait;
  _disallowed = false;
#ifdef DEBUG_CROMP_CLIENT
  LOG(astring("resetting entity=") + _ent->mangled_form());
#endif
  open_common(addr);
}

const octopus_entity &cromp_client::entity() const
{
  AUTO_LOCK;
  return *_ent;
}

SAFE_STATIC(tcpip_stack, _hidden_stack, )

octopus_entity cromp_client::randomize_entity() const
{
  astring host = cromp_common::chew_hostname(internet_address
      (byte_array::empty_array(), _hidden_stack().hostname(), 0), NIL);
  chaos randomizer;
  return octopus_entity(host, application_configuration::process_id(),
      randomizer.inclusive(0, MAXINT32 / 3),
      randomizer.inclusive(0, MAXINT32 / 3));
}

octopus_request_id cromp_client::next_id()
{
  AUTO_LOCK;
  return octopus_request_id(*_ent, _req_id->next_id());
}

outcome cromp_client::synchronous_request(const infoton &to_send,
    infoton * & received, octopus_request_id &item_id,
    int timeout)
{
  FUNCDEF("synchronous_request");
  received = NIL;
  outcome ret = submit(to_send, item_id);
  if (ret != OKAY) {
    LOG(astring("failed to submit request: ") + outcome_name(ret) + " on " + to_send.text_form());
    return ret;
  }
  ret = acquire(received, item_id, timeout);
  if (ret != OKAY) {
    LOG(astring("failed to acquire response: ") + outcome_name(ret) + " for " + to_send.text_form());
    return ret;
  }
  return OKAY;
}

SAFE_STATIC(byte_array, _empty_blank_verif, );
const byte_array &cromp_client::blank_verification()
{ return _empty_blank_verif(); }

outcome cromp_client::login()
{
  FUNCDEF("login");
  CHECK_LOCKOUT;
  if (!_identified) {
    _channel_secured = false;
    // we need to secure an identity with the server.
    identity_infoton identity;
    octopus_request_id item_id = octopus_request_id::randomized_id();
    infoton *response;
    outcome ret = synchronous_request(identity, response, item_id);
    if (ret != OKAY) return ret;

    CAST_REPLY(identity_infoton, response, ide_reply, NO_SERVER);
    if (!ide_reply->_new_name.blank()) {
#ifdef DEBUG_CROMP_CLIENT
      LOG(astring("setting new entity to: ")
          + ide_reply->_new_name.mangled_form());
#endif
      AUTO_LOCK;
      *_ent = ide_reply->_new_name;
      _identified = true;
    } else {
#ifdef DEBUG_CROMP_CLIENT
      LOG("identity request failed: got blank name.");
#endif
    }
    WHACK(ide_reply);
  }

  if (_encrypting && !_channel_secured) {
    // now the encryption needs to be cranked up.

    if (!_encrypt_arm)
      LOG("there's no encryption arm!!!!");

    encryption_infoton encro;
    {
      AUTO_LOCK;
      encro.prepare_public_key(_encrypt_arm->private_key());
    }

    infoton *response;
    octopus_request_id item_id;
    outcome ret = synchronous_request(encro, response, item_id);
    if (ret != OKAY) return ret;

    CAST_REPLY(encryption_infoton, response, enc_reply, ENCRYPTION_MISMATCH);
      // this is a reasonable answer (mismatch), because a non-encrypting
      // server should tell us a general failure response, since it shouldn't
      // understand the request.

    // handle the encryption infoton by feeding our tentacle the new key.
    byte_array transformed;
    ret = _encrypt_arm->consume(*enc_reply, item_id, transformed);
    if (ret != OKAY) {
      LOG(astring("failed to process encryption infoton for ")
          + item_id.text_form());
      WHACK(enc_reply);  // nothing to give out.
      return ret;
    }
    WHACK(enc_reply);

    octenc_key_record *reco = _encrypt_arm->keys().lock(item_id._entity);
    if (!reco) {
      LOG(astring("failed to locate key for ") + item_id._entity.text_form());
      return NOT_FOUND;
    }
    _crypto->set_key(reco->_key.get_key(),
        encryption_infoton::BLOWFISH_KEY_SIZE);
    _encrypt_arm->keys().unlock(reco);
    _channel_secured = true;
  }

  if (!_authorized) {
    // we need to go through whatever authentication is used by the server.
    security_infoton::login_modes login_type = security_infoton::LI_LOGIN;
    security_infoton securinfo(login_type, OKAY, *c_verification);
    octopus_request_id item_id;
    infoton *response;
    outcome ret = synchronous_request(securinfo, response, item_id);
    unhandled_request *temp_unh = dynamic_cast<unhandled_request *>(response);
    if (temp_unh) {
#ifdef DEBUG_CROMP_CLIENT
      LOG(astring("got an unhandled request with reason: ")
          + common::outcome_name(temp_unh->_reason));
#endif
      return temp_unh->_reason;  // return the original reason.
    }
    CAST_REPLY(security_infoton, response, sec_reply, NO_SERVER);
    outcome success = sec_reply->_success;
    WHACK(sec_reply);
    if (success == tentacle::OKAY) {
      AUTO_LOCK;
      _authorized = true;
#ifdef DEBUG_CROMP_CLIENT
      LOG(astring("login request succeeded, now logged in."));
#endif
    } else {
#ifdef DEBUG_CROMP_CLIENT
      LOG(astring("login request failed."));
#endif
      return success;
    }
  }

  return OKAY;
}

outcome cromp_client::connect(const byte_array &verification)
{
  FUNCDEF("connect");
  stop_asynch_thread();
  AUTO_LOCK;  // protect from multiple connect attempts.
  *c_verification = verification;
  return locked_connect();
}

outcome cromp_client::asynch_connect()
{
  FUNCDEF("asynch_connect");
  if (connected()) return OKAY;  // why bother?
  if (_asynch_connector) return NO_CONNECTION;  // in progress.
//#ifdef DEBUG_CROMP_CLIENT
  LOG(instance_name() + " entry.");
//#endif
  {
    AUTO_LOCK;
      // protect this block only; we want to unlock before thread gets started.
    if (connected()) return OKAY;  // done already somehow.
    if (_asynch_connector) {
      LOG("logic error: asynchronous connector already exists.");
      return NO_CONNECTION;
    }
    _disallowed = true;
    _asynch_connector = new asynch_connection_thread(*this);
  }
  _asynch_connector->start(NIL);
//#ifdef DEBUG_CROMP_CLIENT
  LOG(instance_name() + " exit.");
//#endif
  return NO_CONNECTION;
}

outcome cromp_client::locked_connect()
{
  FUNCDEF("locked_connect");
  if (!spock()) return BAD_INPUT;
  if (connected()) return OKAY;  // already connected.

  locked_disconnect();  // clean out any previous connection.
  *_ent = randomize_entity();  // reset the login id.

  int attempts = 0;
  while (attempts++ < MAX_CONN_ATTEMPTS) {
#ifdef DEBUG_CROMP_CLIENT
    LOG(instance_name() + " calling spocket connect.");
#endif
    outcome ret = spock()->connect(_connection_wait);
#ifdef DEBUG_CROMP_CLIENT
    LOG(instance_name() + " done calling spocket connect.");
#endif
    if (ret == spocket::OKAY) {
#ifdef DEBUG_CROMP_CLIENT
      LOG("finished connection...  now affirming identity.");
#endif
      return login();
    }
    if (ret == spocket::TIMED_OUT) return TIMED_OUT;
    if ( (ret == spocket::NO_ANSWER) || (ret == spocket::ACCESS_DENIED) ) {
      // clean up.  this is a real case of something hosed.
      locked_disconnect();
      return NO_SERVER;
    }
#ifdef DEBUG_CROMP_CLIENT
    LOG(a_sprintf("error gotten=%s", spocket::outcome_name(ret)));
#endif

    if (attempts < MAX_CONN_ATTEMPTS - 1)
      time_control::sleep_ms(INTERCONNECTION_SNOOZE);
  }
  LOG(instance_name() + " failed to connect.");
  locked_disconnect();  // clean up.
  return NO_CONNECTION;
}

outcome cromp_client::disconnect()
{
  stop_asynch_thread();
  AUTO_LOCK;
  return locked_disconnect();
}

void cromp_client::keep_alive_pause(int duration, int interval)
{
  if (duration < 0) duration = 0;
  if (interval < 0) interval = 40;
  if (interval > duration) interval = duration;
  
  // keep looping on the cromp stimulation methods until the time has elapsed.
  time_stamp leave_at(duration);
  while (time_stamp() < leave_at) {
    push_outgoing(1);
    grab_anything(false);
    // we'll only sleep if they didn't give us a zero duration.
    if (duration)
      time_control::sleep_ms(interval);  // snooze a hopefully short time.
  }
}

outcome cromp_client::locked_disconnect()
{
  if (!spock()) return BAD_INPUT;
  outcome ret = spock()->disconnect();
  _identified = false;
  _authorized = false;
  _channel_secured = false;
  *_ent = octopus_entity();  // reset the login id.
  if (ret != spocket::OKAY) {
//hmmm: any other outcomes to return?
    return OKAY;
  }
  return OKAY;
}

bool cromp_client::wrap_infoton(const infoton &request,
    encryption_wrapper &wrapped)
{
#ifdef DEBUG_CROMP_CLIENT
  FUNCDEF("wrap_infoton");
#endif
  if (!_channel_secured) return false;
  // identity is not wrapped with encryption; we need to establish and identity
  // to talk on a distinct channel with the server.  even if that identity were
  // compromised, the interloper should still not be able to listen in on the
  // establishment of an encryption channel.
  bool is_ident = !!dynamic_cast<const identity_infoton *>(&request);
  bool is_encrypt = !!dynamic_cast<const encryption_infoton *>(&request);
  bool is_wrapper = !!dynamic_cast<const encryption_wrapper *>(&request);
  if (!is_ident && !is_encrypt && !is_wrapper) {
    // check that we have already got a channel to speak over.  otherwise, we
    // can't do any encrypting of messages yet.
#ifdef DEBUG_CROMP_CLIENT
    LOG(astring("encrypting ") + request.text_form());
#endif
    byte_array packed_request;
    infoton::fast_pack(packed_request, request);
    _crypto->encrypt(packed_request, wrapped._wrapped);
    return true;
  } else return false;  // we didn't need or want to wrap it.
}

outcome cromp_client::submit(const infoton &request,
    octopus_request_id &item_id, int max_tries)
{
#ifdef DEBUG_CROMP_CLIENT
  FUNCDEF("submit");
#endif
  CHECK_LOCKOUT;
  item_id = next_id();
  bool is_ident = !!dynamic_cast<const identity_infoton *>(&request);
  if (!_identified && !is_ident) return BAD_INPUT;

  if (_encrypting && _channel_secured) {
    // if we're encrypting things, then we need to encrypt this too.  this
    // assumes that authentication is wrapped by encryption, which is the sane
    // thing to do.  identity is not wrapped that way though; we need to
    // establish and identity to talk on a distinct channel with the server.
    // even if that identity were compromised, the interloper would still not
    // be able to listen in on the establishment of an encryption channel.

    encryption_wrapper real_request;
    bool wrapped_okay = wrap_infoton(request, real_request);
    if (wrapped_okay) {
      outcome to_return = cromp_common::pack_and_ship(real_request, item_id,
          max_tries);
      return to_return;
    }
    // if it didn't wrap okay, we fall through to a normal send, because it's
    // probably an encryption or identity infoton, which needs to go through
    // without being wrapped.
  } else if (_encrypting) {
#ifdef DEBUG_CROMP_CLIENT
    LOG("the channel has not been secured yet.");
#endif
  }

  outcome to_return = cromp_common::pack_and_ship(request, item_id, max_tries);
  return to_return;
}

outcome cromp_client::acquire(infoton * &response,
    const octopus_request_id &cmd_id, int timeout)
{
  FUNCDEF("acquire");
  CHECK_LOCKOUT;
  outcome to_return = cromp_common::retrieve_and_restore(response, cmd_id,
      timeout);

  unhandled_request *intermed = dynamic_cast<unhandled_request *>(response);
  if (intermed) {
    // override the return value with the real outcome of a failed operation.
    to_return = intermed->_reason;
LOG(astring("using unhandled request's intermediate result: ") + outcome_name(to_return));
  }

  decrypt_package_as_needed(to_return, response, cmd_id);

  return to_return;
}

outcome cromp_client::acquire_any(infoton * &response,
    octopus_request_id &cmd_id, int timeout)
{
#ifdef DEBUG_CROMP_CLIENT
  FUNCDEF("acquire_any");
#endif
  CHECK_LOCKOUT;
  outcome to_return = cromp_common::retrieve_and_restore_any(response, cmd_id,
      timeout);

  unhandled_request *intermed = dynamic_cast<unhandled_request *>(response);
  if (intermed) {
    // override the return value with the real outcome of a failed operation.
    to_return = intermed->_reason;
  }

  decrypt_package_as_needed(to_return, response, cmd_id);

  return to_return;
}

void cromp_client::decrypt_package_as_needed(outcome &to_return,
    infoton * &response, const octopus_request_id &cmd_id)
{
  FUNCDEF("decrypt_package_as_needed");
  if (dynamic_cast<encryption_wrapper *>(response)) {
    if (!_encrypt_arm) {
      LOG(astring("received an encryption_wrapper but we are not "
          "encrypting, on ") + cmd_id.text_form());
      to_return = ENCRYPTION_MISMATCH;
      return;
    }
    byte_array transformed;
    outcome ret = _encrypt_arm->consume(*response, cmd_id, transformed);
    if ( (ret != OKAY) && (ret != PARTIAL) ) {
      LOG(astring("failed to decrypt wrapper for ") + cmd_id.text_form());
      to_return = ret;
      return;
    }

    string_array classif;
    byte_array decro;  // decrypted packed infoton.
    bool worked = infoton::fast_unpack(transformed, classif, decro);
    if (!worked) {
      LOG("failed to fast_unpack the transformed data.");
      to_return = ENCRYPTION_MISMATCH;  // what else would we call that?
    } else {
      infoton *new_req = NIL;
      outcome rest_ret = octo()->restore(classif, decro, new_req);
      if (rest_ret == tentacle::OKAY) {
        // we got a good transformed version.
        WHACK(response);
        response = new_req;  // substitution complete.
      } else {
        LOG("failed to restore transformed infoton.");
        to_return = ENCRYPTION_MISMATCH;  // what else would we call that?
      }
    }
  }
}

} //namespace.


