#ifndef CROMP_SERVER_CLASS
#define CROMP_SERVER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : cromp_server                                                      *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Services request commands from CROMP clients.  Derived objects supply    *
*  the specific services that a CROMP server implements by providing a set    *
*  of tentacles that handle the requests.  These are added directly to the    *
*  server's octopus base class.                                               *
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

#include <octopus/octopus.h>
#include <processes/ethread.h>
#include <timely/time_stamp.h>
#include <sockets/internet_address.h>

#include <tentacles/encryption_tentacle.h>
#include <tentacles/login_tentacle.h>
#include <processes/thread_cabinet.h>

namespace cromp {

// forward.
class client_dropping_thread;
class connection_management_thread;
class cromp_client_list;
class cromp_client_record;
class cromp_security;
class cromp_transaction;

class cromp_server : public cromp_common
{
public:
  cromp_server(const sockets::internet_address &where,
          int accepting_threads = DEFAULT_ACCEPTERS(),
          bool instantaneous = true,
          int max_per_entity = DEFAULT_MAX_ENTITY_QUEUE);
    // creates a server that will open servers on the location "where".  the
    // number of connections that can be simultaneously handled is passed in
    // "accepting_threads".  if the "instantaneous" parameter is true, then
    // any infotons that need to be handled will be passed to the octopus for
    // immediate handling rather than being handled later on a thread.

  virtual ~cromp_server();

  DEFINE_CLASS_NAME("cromp_server");

  int clients() const;  // returns number of active clients.

  static sockets::internet_address any_address(int port);
    // returns an internet_address that should work on any interface that a
    // host has.

  sockets::internet_address location() const;
    // returns the network location where this server is supposed to reside,
    // passed in the constructor.

  bool instantaneous() const { return _instantaneous; }
    // reports whether this server uses immediate handling or delayed handling.

  bool enabled() const { return _enabled; }
    // reports whether this server has been cranked up or not yet.

  basis::outcome enable_servers(bool encrypt, cromp_security *security = NULL_POINTER);
    // this must be called after construction to start up the object before it
    // will accept client requests.  if "encrypt" is on, then packets will
    // be encrypted and no unencrypted packets will be allowed.  if the
    // "security" is passed as NULL_POINTER, then a default security manager is created
    // and used.  otherwise, the specified "security" object is used and will
    //  _not_ be destroyed when this object goes away.

  void disable_servers();
    // shuts down the server sockets that this object owns and disables the
    // operation of this object overall.  the next step is destruction.

  bool find_entity(const octopi::octopus_entity &id, sockets::internet_address &found);
    // given an "id" that is currently connected, find the network address
    // where it originated and put it in "found".  true is returned if the
    // entity was located.

  bool disconnect_entity(const octopi::octopus_entity &id);
    //!< returns true if the "id" can be found and disconnected.

  basis::outcome send_to_client(const octopi::octopus_request_id &id, octopi::infoton *data);
    // blasts a command back to the client identified by the _entity member in
    // the "id".  this allows the controlling object of the server object to
    // asynchronously inject data into the client's incoming stream.  the
    // client had better know what's going on or this will just lead to
    // ignored data that eventually gets trashed.

  basis::outcome get_from_client(const octopi::octopus_request_id &id, octopi::infoton * &data,
      int timeout);
    // attempts to locate a command with the request "id".  if it is found
    // within the timeout period, then "data" is set to the command.  the
    // "data" pointer must be deleted after use.

//  basis::outcome get_any_from_client(const octopi::octopus_entity &ent, octopi::infoton * &data,
//      int timeout);
    // attempts to grab any waiting package for the entity "ent".

  bool get_sizes(const octopi::octopus_entity &id, int &items, int &bytes);
    // promoted from our octopus' entity_data_bin in order to provide some
    // accounting of what is stored for the "id".

  basis::astring responses_text_form() const;
    // returns a printout of the responses being held here.

  static int DEFAULT_ACCEPTERS();
    // the default number of listening threads.

  // internal use only...

  void look_for_clients(processes::ethread &requester);
    // meant to be called by an arbitrary number of threads that can block
    // on accepting a new client.  control flow will not return from this
    // method until the thread's cancel() method has been called.

  void drop_dead_clients();
    // called by a thread to manage dead or dying connections.

  bool encrypting() const { return !!_encrypt_arm; }
    // true if this object is encrypting transmissions.

  octopi::infoton *wrap_infoton(octopi::infoton * &request, const octopi::octopus_entity &ent);
    // when we're encrypting, this turns "request" into an encryption_wrapper.
    // if NULL_POINTER is returned, then nothing needed to happen to the "request".

private:
  cromp_client_list *_clients;  // the active set of clients.
  processes::thread_cabinet *_accepters;  // the list of accepting threads.
  basis::mutex *_list_lock;  // protects our lists.
  timely::time_stamp *_next_droppage;  // times cleanup for dead clients.
  bool _instantaneous;  // true if the octopus gets driven hard.
  sockets::internet_address *_where;  // where does the server live.
  int _accepting_threads;  // the number of accepters we keep going.
  client_dropping_thread *_dropper;  // cleans our lists.
  bool _enabled;  // records if this is running or not.
  octopi::encryption_tentacle *_encrypt_arm;  // the handler for encryption.
  cromp_security *_default_security;  // used in lieu of other provider.
  octopi::login_tentacle *_security_arm;  // handles security for the logins.

  basis::outcome accept_one_client(bool wait);
    // tries to get just one accepted client.  if "wait" is true, then the
    // routine will pause until the socket accept returns.
};

} //namespace.

#endif


