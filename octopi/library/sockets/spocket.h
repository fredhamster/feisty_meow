#ifndef SPOCKET_CLASS
#define SPOCKET_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : spocket                                                           *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*                                                                             *
*******************************************************************************
* Copyright (c) 1989-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "tcpip_stack.h"

#include <basis/contracts.h>
#include <basis/mutex.h>
#include <timely/time_stamp.h>

namespace sockets {

// forward declarations.
class internet_address;
class raw_socket;

//! Abstraction for a higher-level BSD socket that is platform independent.
/*!
  The class works on Unix and Win32 style operating systems.  This class is named in honor of
  the venerable Vulcan Spock, which also avoid naming conflicts with the OS's socket() function.
*/

class spocket : public virtual basis::root_object
{
public:
  enum sock_types {
    CONNECTED,         // connected socket over TCP.
    BROADCAST,         // broadcasting socket over UDP.
    UNICAST,           // single-address targeted socket over UDP.
    BOGUS_SOCK         // special type that goes nowhere and provides no data.
  };

  spocket(const internet_address &where, sock_types type = CONNECTED);
    // constructs the spocket object.  "where" provides the network location
    // for either this side (for a server) or the other side (for a client).
    // the decision about this socket's orientation (client or server) will not
    // be made until either connect() or accept() are invoked.  note however
    // that BROADCAST sockets are only appropriate for use as a client; they
    // can receive and send broadcasts that way without needing a server role.

  ~spocket();
    // drops any connection that was made and destroys the spocket object.

  DEFINE_CLASS_NAME("spocket");

  bool healthy();
    // returns true if the spocket seems to be okay.

  const internet_address &where() const;
    // returns the location where this socket exists.

  const internet_address &remote() const;
    // returns the location that we have accepted from.

  enum outcomes {
    OKAY = basis::common::OKAY,
    TIMED_OUT = basis::common::TIMED_OUT,
    ACCESS_DENIED = basis::common::ACCESS_DENIED,
    BAD_INPUT = basis::common::BAD_INPUT,
    NONE_READY = basis::common::NONE_READY,
    PARTIAL = basis::common::PARTIAL,

    NO_CONNECTION = sockets::communication_commons::NO_CONNECTION,
    NO_ANSWER = sockets::communication_commons::NO_ANSWER,

    DEFINE_OUTCOME(NOT_SERVER, -39, "Accept was tried on a spocket that is "
        "not a root server")
  };
  
  static const char *outcome_name(const basis::outcome &to_name);
    // returns the text for "to_name" if it's a member of spocket outcomes.

  // informative functions...

  basis::astring text_form();
    // returns a readable version of the contents of the spocket.

  bool was_connected() const { return _was_connected; }
    // a simple check of whether a connection has been made on this object.
    // if this is not true, then the object is not usable yet.  this state
    // will also get set to false when the spocket gets disconnected.

  bool connected();
    // returns true if the spocket is "currently" connected.  this causes an
    // interrogation of the operating system and may take a short while.

  // these report what type of spocket this is.
  bool is_client() const { return _client; }
  bool is_server() const { return !_client; }
  bool is_root_server() const { return is_server() && !!_server_socket; }

  basis::un_int OS_socket() { return _socket; }
    // returns the low-level operating system form of our normal action socket.
    // this is zero for a root server.  note: this will still record what the
    // socket was after it is closed out, just for record keeping; do not
    // control the returned socket outside of this class.
  basis::un_int OS_root_socket() { return _server_socket; }
    // returns the OS form of our server socket, but only if this is a root
    // server type of socket.

  void bind_client(const internet_address &source);
    //!< when a client calls connect, this forces it to bind to "source".
    /*!< this has no effect on servers.  it is also disabled again when the
    client is disconnected, so it should always be done before every time
    connect() is called. */

  // major operations for connected mode sockets...

  basis::outcome connect(int communication_wait = 20 * basis::SECOND_ms);
    // acts as a client and connects to a destination.  the timeout is
    // specified in milliseconds by "communication_wait".  this can be as low
    // as zero if you don't want to wait.  TIMED_OUT is returned if the
    // connection hasn't finished within the connection_wait.  OKAY is returned
    // if the connection succeeded.  other outcomes denote failures.

  basis::outcome accept(spocket * &sock, bool wait);
    // makes this spocket act as a root server and accepts a connection from a
    // client if possible.  a root server is a spocket object that manages a
    // server spocket but which does not allow any normal sending or receiving.
    // only root servers can have accept called on them.  the "sock" will be
    // a normal server spocket which can be used to send and receive if it
    // got connected.  for "sock" to be valid, it must return as NULL_POINTER
    // and the returned outcome must be OKAY.  if no new connections are
    // available, then NO_CONNECTION is returned.  if the "wait" flag is true,
    // then the accept on the root server will block until a connection is
    // accepted and the returned spocket will be made non-blocking.  if "wait"
    // is false, then no blocking will occur at all.  note that multiple
    // threads can invoke this without tripping over the protective locking;
    // once the root socket is initialized, accept will not lock the spocket.

  basis::outcome disconnect();
    // closes the connection.  the state is returned to the post-construction
    // state, i.e. it will appear that this object had never connected yet.

  // these report which side of the connection this is functioning as.
  bool client() const { return _client; }
  bool server() const { return !_client; }

  // send and receive functions...
  //
  //   if the outcome from one of these is NO_CONNECTION, then somehow the
  // connection has dropped or never succeeded.

  basis::outcome send(const basis::abyte *buffer, int size, int &len_sent);
    // sends "size" bytes from the "buffer".  the number actually sent is
    // stored in "len_sent".  this can only be used for CONNECTED type sockets.

  basis::outcome send(const basis::byte_array &to_send, int &len_sent);
    // this version takes a byte_array.

  basis::outcome send_to(const internet_address &where_to, const basis::abyte *buffer,
          int size, int &len_sent);
    // this version is used for sending when the socket type is BROADCAST
    // or UNICAST.

  basis::outcome send_to(const internet_address &where_to, const basis::byte_array &to_send,
          int &len_sent);
    // clone of above, using byte_array.

  basis::outcome receive(basis::abyte *buffer, int &size);
    // attempts to retrieve data from the spocket and place it in the "buffer".
    // the "size" specifies how much space is available.  if successful, the
    // buffer will be filled with data and "size" will report how much there
    // actually is.

  basis::outcome receive(basis::byte_array &buffer, int &size);
    // this version uses a byte_array for the "buffer".  the "size" expected
    // must still be set and "size" will still report the bytes read.
//hmmm: could remove the size parameter for byte array type.

  // these methods are used for BROADCAST and other non-connected types of
  // spockets.  they report the data as the above receive methods do, but they
  // also report the sender.
  basis::outcome receive_from(basis::abyte *buffer, int &size,
          internet_address &where_from);
  basis::outcome receive_from(basis::byte_array &buffer, int &size,
          internet_address &where_from);

  basis::outcome await_readable(int timeout);
    // pauses this caller until data arrives.  this is a blocking call that
    // could potentially not return until the "timeout" elapses (measured in
    // milliseconds).  if "timeout" is zero, then this really doesn't do
    // anything.  if data is available, then OKAY is returned.

  basis::outcome await_writable(int timeout);
    // pauses this caller until the socket can be written to again.  could
    // potentially not return until the "timeout" elapses (measured in
    // milliseconds).  if "timeout" is zero, then this really doesn't do
    // anything.  if the socket is able to send again, then OKAY is returned.
    // otherwise NONE_READY is returned.

  tcpip_stack &stack() const;
    // provides access to the spocket's tcpip stack to derivative objects.

  bool is_bogus() const { return _type == BOGUS_SOCK; }
    //!< returns true when this object is bogus.
    /*!< a spocket constructed as BOGUS_SOCK does not open a network
    connection to anywhere, and it also never sends or receives any data.
    it allows code based on spockets to be disabled when appropriate, so
    that the spocket is still constructed and all methods can be invoked,
    but it does nothing. */

private:
  sock_types _type;  // records what kind of socket we'll create.
  basis::un_int _socket;  // our socket that we communicate on.
  basis::un_int _server_socket;  // our serving socket, if we're a root server.
  bool _was_connected;  // did we ever successfully connect?
  bool _client;  // true if we are acting as a client.
  internet_address *_where;  // our addressing info.
  internet_address *_remote;  // our addressing info.
  raw_socket *_socks;  // provides low-level socket functionality.
  tcpip_stack *_stack;  // provides access to socket facilities.
  basis::mutex *_select_lock;  // protects concurrent access to socket.
  timely::time_stamp *_last_resolve;
    // tracks when we last tried a resolve on our address.  if they try to
    // reconnect and we haven't done this in a while, we'll re-resolve the
    // socket.
  bool _client_bind;  //!< force the client to bind on an address.
  internet_address *_cli_bind;  //!< where the client binds.

  // not allowed.
  spocket(const spocket &);
  spocket &operator =(const spocket &);
};

} //namespace.

#endif

