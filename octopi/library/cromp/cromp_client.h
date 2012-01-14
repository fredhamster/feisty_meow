#ifndef CROMP_CLIENT_CLASS
#define CROMP_CLIENT_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : cromp_client                                                      *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Supplies primitive operations for requesting services of a CROMP-based   *
*  server application.  Tentacles in cromp clients are only used for          *
*  restoring original objects, but they are required for that.  Keep in mind  *
*  that for communication to be possible, a cromp server and its cromp client *
*  must possess tentacles that grok the same infotons.  They do not need to   *
*  be the same objects and probably shouldn't be.  It makes sense to          *
*  implement an unpacking / cloning tentacle and then derive the full-service *
*  request processing tentacle from it.                                       *
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

#include <crypto/blowfish_crypto.h>
#include <sockets/internet_address.h>
#include <structures/roller.h>
#include <tentacles/encryption_tentacle.h>
#include <tentacles/encryption_wrapper.h>
#include <tentacles/entity_registry.h>
#include <timely/time_stamp.h>

namespace cromp {

// forward:
class asynch_connection_thread;

class cromp_client : public cromp_common
{
public:
  enum constraints {
    DEFAULT_MAX_CONNECT_WAIT = 28 * basis::SECOND_ms,
      // the server had better connect within this time limit or it will
      // be considered missing in action.
    DEFAULT_MAX_RESPONSE_WAIT = 4 * basis::MINUTE_ms
      // a server should be able to answer in this interval or something is
      // really wrong with the system.
  };

  cromp_client(const sockets::internet_address &destination,
          int connection_wait = DEFAULT_MAX_CONNECT_WAIT,
          int max_per_ent = DEFAULT_MAX_ENTITY_QUEUE);
    // will connect to a cromp_server on the host specified by "destination".

  virtual ~cromp_client();

  DEFINE_CLASS_NAME("cromp_client");

  basis::astring instance_name() const {
    return basis::astring(class_name()) + ": " + entity().text_form();
  }

  const octopi::octopus_entity &entity() const;
    // returns our identity within the octopus server.

  void enable_encryption();
    // this turns on the encryption.  this should be done before the first
    // connect or login invocations.  once it's enabled, it stays enabled.

  void reset(const sockets::internet_address &destination,
          int connection_wait = DEFAULT_MAX_CONNECT_WAIT,
          int max_per_ent = DEFAULT_MAX_ENTITY_QUEUE);
    // disconnects from any previous server and reconstructs this client.
    // the client will be left in an unconnected and unauthenticated state.
    // any pre-existing tentacles will still be hooked up to the object.
    // use connect() to re-establish the connection to the server.

  basis::outcome connect(const basis::byte_array &verification = blank_verification());
    // attempts to connect to the server.  OKAY is returned if this succeeds.

  static const basis::byte_array &blank_verification();  // empty verification.

  basis::outcome asynch_connect();
    // this is a non-blocking connect.  it behaves like connect(), but should
    // never take more than a few milliseconds to return.  if the client is
    // already connected, then nothing is done.  otherwise no operations will
    // be permitted until the reconnection has succeeded.  a call to regular
    // connect() cancels the asynchronous connect.

  bool connected() const;
    // returns true if we think we are connected to the server.

  basis::outcome disconnect();
    // disconnects from the cromp server and releases all connection resources.

  virtual basis::outcome login();
    // attempts to log in to the server.  we must already have connected
    // when this is called.  the "verification" is a protocol specific
    // package of info that can be used to validate the login.

  //////////////

  basis::outcome submit(const octopi::infoton &request, octopi::octopus_request_id &item_id,
          int max_tries = 80);
    // requests a transaction from the cromp_server described by "request".
    // the return value is OKAY if the request was successfully sent or it
    // will be another outcome that indicates a failure of transmission.
    // the "max_tries" is the number of times to try getting the send out;
    // if asynchronous sends are allowed to accumulate, then 1 works here.

  basis::outcome acquire(octopi::infoton * &response, const octopi::octopus_request_id &cmd_id,
          int timeout = DEFAULT_MAX_RESPONSE_WAIT);
    // attempts to receive a "response" to a previously submitted request
    // with the "cmd_id".  if "timeout" is non-zero, then the response will
    // be awaited for "timeout" milliseconds.  otherwise the function returns
    // immediately.  note that "response" will only be generated properly given
    // a tentacle that knows the infoton type received.  this requires the
    // cromp_client to have tentacles registered for all of the types to be
    // exchanged.  this can be used for asynchronous retrieval if the timeout
    // is zero and one knows a previously requested "cmd_id".

  //////////////

  // grittier functions...

  basis::outcome synchronous_request(const octopi::infoton &to_send, octopi::infoton * &received,
          octopi::octopus_request_id &item_id,
          int timeout = DEFAULT_MAX_RESPONSE_WAIT);
    // submits the infoton "to_send" and waits for the reply.  if there is
    // a reply in the allotted time, then "received" is set to that.  the
    // "item_id" is set to the request id assigned to the request.

  basis::outcome acquire_any(octopi::infoton * &response, octopi::octopus_request_id &cmd_id,
          int timeout = DEFAULT_MAX_RESPONSE_WAIT);
    // a request to retrieve any waiting data for the client.  similar to
    // acquire above, but does not require one to know the command id.

  octopi::octopus_request_id next_id();
    // generates the next identifier.  this can be used as a unique identifier
    // for derived objects since the id is issued from our server.  it is
    // not unique across different types of cromp servers though, nor across
    // cromp servers running on different hosts.

  void decrypt_package_as_needed(basis::outcome &to_return, octopi::infoton * &response,
          const octopi::octopus_request_id &cmd_id);
    // this ensures that if the "response" is encrypted, it will be decrypted
    // and replaced with the real object intended.  this method is invoked
    // automatically by acquire(), but if the cromp_common retrieve methods are
    // used directly, this should be done to the response before using it.

  void keep_alive_pause(int duration = 0, int interval = 40);
    // pauses this thread but keeps calling into the cromp support to ensure
    // items get delivered or received when possible.  the call will snooze
    // for at least "duration" milliseconds with individual sleeps being
    // allowed the "interval" milliseconds.

  const basis::byte_array &verification() const;
    // returns the verification token held currently, if it's been set.

private:
  bool _encrypting;  // true if this connection should be encrypted.
  int _connection_wait;  // the length of time we'll allow a connection.
  basis::mutex *_lock;  // protects our data members below.
  octopi::octopus_entity *_ent;  // our identity within the octopus server.
  structures::int_roller *_req_id;  // the numbering of our nth request is depicted here.
  bool _identified;  // true if our initial identifier verification succeeded.
  bool _authorized;  // true if our login was successful.
  bool _disallowed;  // nothing is allowed right now except asynch connecting.
  friend class asynch_connection_thread;  // solely so it can use r_p_c method.
  asynch_connection_thread *_asynch_connector;  // b-ground connection thread.
  bool _channel_secured;  // true if an encrypted connection has been made.
  crypto::blowfish_crypto *_crypto;  // tracks our key, once we have one.
  octopi::encryption_tentacle *_encrypt_arm;  // processes encryption for us.
  octopi::blank_entity_registry *_guardian;  // simple security support.
  basis::byte_array *c_verification;  // verification token we were given.

  void stop_asynch_thread();  // stops the background connector.

  basis::outcome locked_connect();
    // called by the other types of connection processes to get the work done.

  octopi::octopus_entity randomize_entity() const;
    // provides a junk entity for our temporary identifier that we'll use
    // until the server gives us a new one.

  basis::outcome locked_disconnect();
    // assumes that the client is locked.  this is the real disconnect.

  bool wrap_infoton(const octopi::infoton &request, octopi::encryption_wrapper &wrapped);
    // wraps an infoton for sending over an encrypted connection.
};

} //namespace.

#endif

