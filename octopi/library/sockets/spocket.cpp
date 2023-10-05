/*****************************************************************************\
*                                                                             *
*  Name   : spocket                                                           *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2001-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "internet_address.h"
#include "raw_socket.h"
#include "spocket.h"
#include "tcpip_stack.h"

#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/astring.h>
#include <basis/mutex.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <structures/static_memory_gremlin.h>
#include <timely/time_control.h>
#include <timely/time_stamp.h>

//hmmm: put this bag o headers into a similar thing to windoze helper.  maybe just have an os_helper file that combines both?
//#ifdef __UNIX__
  #include <arpa/inet.h>
  #include <errno.h>
  #include <netdb.h>
  #include <signal.h>
  #include <string.h>
  #include <sys/ioctl.h>
  #include <sys/socket.h>
  #include <sys/types.h>
  #include <termios.h>
  #include <unistd.h>
//#endif

using namespace basis;
using namespace loggers;
using namespace structures;
using namespace timely;

namespace sockets {

//#define DEBUG_SPOCKET
  // uncomment for noisy version.

#undef LOG
#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger::get(), to_print)

const int PENDING_CONNECTIONS_ALLOWED = 14;
  // we allow this many connections to queue up before they get rejected.
  // if the OS is windoze, this number is ignored if it's greater than the
  // hardcoded maximum of like 5.

const int RESOLVE_INTERVAL = 300;
  // we'll re-resolve the ip address at this rate.  this mainly comes into
  // play for the connect call, since the address passed in could have changed
  // or been invalid to start with.  we're not losing much by trying to
  // resolve the address again during connection time.

#define RECOGNIZE_DISCO \
  _client_bind = false; \
  _was_connected = false

// ensure that the socket is in a good state.
#define ENSURE_HEALTH(retval) \
  if (!was_connected()) return retval;  /* never has been. */ \
  if (!_socket) { RECOGNIZE_DISCO; return retval; /* not set. */ }

#define CHECK_BOGUS(retval) \
  if (is_bogus()) { return retval;  /* this spocket is junk. */ }

/*
#undef GRAB_LOCK
#ifdef __WIN32__
  // win32 seems to trip over selects unless we protect them.
  #define GRAB_LOCK auto_synchronizer l(*_select_lock)
    // and in truth, the locking turns out to be needed on win32 if we're
    // going to allow sharing a spocket across threads.  this is one of the
    // design goals so we're honor bound to support that.
#else
  #define GRAB_LOCK 
#endif
*/
  #define GRAB_LOCK 


//#ifdef __UNIX__
  SAFE_STATIC(mutex, __broken_pipe_synch, )
//#endif

spocket::spocket(const internet_address &where, sock_types type)
: _type(type),
  _socket(0),
  _server_socket(0),
  _was_connected(false),
  _client(false),
  _where(new internet_address(where)),
  _remote(new internet_address),
  _socks(new raw_socket),
  _stack(new tcpip_stack),
  _select_lock(new mutex),
  _last_resolve(new time_stamp),  // don't force an immediate resolve.
  _client_bind(false),
  _cli_bind(new internet_address)
{
  FUNCDEF("constructor");
  if ( (_type == BROADCAST) || (_type == UNICAST) ) {
    // casting types are never servers.
    _client = true;
  } else if ( (type == CONNECTED) || (type == BOGUS_SOCK) ) {
    // nothing special here currently.
  } else {
    // this is an unknown type.
    LOG(a_sprintf("unknown socket type %d; failing out.", _type));
//hmmm: without a validity flag of some sort, this doesn't mean much.
    return;
  }
}

spocket::~spocket()
{
  FUNCDEF("destructor");
#ifdef DEBUG_SPOCKET
  LOG(a_sprintf("closing spocket: ") + text_form());
#endif
  disconnect();
  WHACK(_where);
  WHACK(_socks);
  WHACK(_stack);
  WHACK(_remote);
  WHACK(_select_lock);
  WHACK(_last_resolve);
  WHACK(_cli_bind);
  _client_bind = false;
}

// where and remote don't need to be protected unless we revise the design of
// the class and allow a reset or re-open kind of method.
const internet_address &spocket::where() const { return *_where; }
const internet_address &spocket::remote() const { return *_remote; }

tcpip_stack &spocket::stack() const { return *_stack; }

// doesn't need to be protected since the sockets are being treated as simple
// ints and since _where currently does not get destroyed.
astring spocket::text_form()
{
  FUNCDEF("text_form");
  astring to_return = is_client()? "client" :
      (is_root_server()? "root-server" : "server");
  to_return += " spocket: ";
  if (connected()) {
    to_return += "connected, ";
  } else {
    if (was_connected()) to_return += "unconnected (was once), ";
    else to_return += "never-connected, ";
  }
  to_return += a_sprintf("socket=%u, ", _socket);
  if (is_root_server()) {
    to_return += a_sprintf("root-socket=%u, ", _server_socket);
  }
  to_return += _where->text_form().s();
  return to_return;
}

void spocket::bind_client(const internet_address &addr)
{
  _client_bind = true;
  *_cli_bind = addr;
}

const char *spocket::outcome_name(const outcome &to_name)
{
  switch (to_name.value()) {
    case NOT_SERVER: return "NOT_SERVER";
    default: return communication_commons::outcome_name(to_name);
  }
}

outcome spocket::disconnect()
{
  FUNCDEF("disconnect");
  RECOGNIZE_DISCO; 
  if (_socket) {
#ifdef DEBUG_SPOCKET
    LOG(a_sprintf("closing socket %d", _socket));
#endif
    _socks->close(_socket);
    _socket = 0;
  }
  if (_server_socket) {
#ifdef DEBUG_SPOCKET
    LOG(a_sprintf("closing server socket %d", _server_socket));
#endif
    _socks->close(_server_socket);
    _server_socket = 0;
  }
  return OKAY;
}

bool spocket::connected()
{
  FUNCDEF("connected");
  ENSURE_HEALTH(false);

  if (_type != CONNECTED) return was_connected();

  if (!_socket) return false;

  // do examination on spocket.
  int sel_mode = 0;
  GRAB_LOCK;

  try {
    int ret = _socks->select(_socket, sel_mode);
    if (ret == 0) {
      return true;  // we are happy.
    }
    if ( (ret & SI_DISCONNECTED) || (ret & SI_ERRONEOUS) ) {
      RECOGNIZE_DISCO; 
      return false;
    }
    return true;
  } catch (...) {
    LOG("caught exception thrown from select, returning false.");
    return false;
  }
}

outcome spocket::await_readable(int timeout)
{
  FUNCDEF("await_readable");
  CHECK_BOGUS(NO_CONNECTION);
  ENSURE_HEALTH(NO_CONNECTION);
  GRAB_LOCK;
  int mode = raw_socket::SELECTING_JUST_READ;
  int ret = _socks->select(_socket, mode, timeout);
  if (ret & SI_READABLE) return OKAY;
    // we found something to report.
  if (ret & SI_DISCONNECTED) {
    RECOGNIZE_DISCO; 
    return NO_CONNECTION;
  }
  return _socket? NONE_READY : NO_CONNECTION;
    // nothing is ready currently.
}

outcome spocket::await_writable(int timeout)
{
  FUNCDEF("await_writable");
  CHECK_BOGUS(NO_CONNECTION);
  ENSURE_HEALTH(NO_CONNECTION);
  GRAB_LOCK;
  int mode = raw_socket::SELECTING_JUST_WRITE;
  int ret = _socks->select(_socket, mode, timeout);
  if (ret & SI_WRITABLE) return OKAY;
    // we found something to report.
  if (ret & SI_DISCONNECTED) {
    RECOGNIZE_DISCO; 
    return NO_CONNECTION;
  }
  return _socket? NONE_READY : NO_CONNECTION;
    // nothing is ready currently.
}

outcome spocket::connect(int communication_wait)
{
  FUNCDEF("connect");
  CHECK_BOGUS(NO_CONNECTION);
  {
    GRAB_LOCK;  // short lock.
    if ( (was_connected() && !_client) || _server_socket) {
#ifdef DEBUG_SPOCKET
      LOG("this object was already opened as a server!");
#endif
      return BAD_INPUT;
    }
    _client = true;  // set our state now that we're sure this is okay.
    _was_connected = false;  // reset this, since we're connecting now.
  }

  if (!_socket) {
    // the socket was never created (or was cleaned up previously).  this is
    // where we create the socket so we can communicate.
#ifdef DEBUG_SPOCKET
    LOG(astring("creating socket now for ") + _where->text_form());
#endif
    GRAB_LOCK;
    int sock_type = SOCK_STREAM;
    int proto = IPPROTO_TCP;

    if ( (_type == BROADCAST) || (_type == UNICAST) ) {
      sock_type = SOCK_DGRAM;
      proto = IPPROTO_IP;
    }
    _socket = int(::socket(AF_INET, sock_type, proto));
    if ( (_socket == basis::un_int(INVALID_SOCKET)) || !_socket) {
      _socket = 0;
      LOG("Failed to open the client's connecting spocket.");
      return ACCESS_DENIED;
    }

    // mark the spocket for _blocking_ I/O.  we want connect to sit there
    // until it's connected or returns with an error.
    _socks->set_non_blocking(_socket, false);

    if (_type == BROADCAST) {
      if (!_socks->set_broadcast(_socket)) return ACCESS_DENIED;
        // mark the socket for broadcast capability.
    }

    if (!_socks->set_reuse_address(_socket)) return ACCESS_DENIED;
      // mark the socket so we don't get bind errors on in-use conditions.
  }

  if (_type == CONNECTED) {
    GRAB_LOCK;
    // turn on the keepalive timer so that loss of the connection will
    // eventually be detected by the OS.  the duration that is allowed to
    // elapse before a dead connection is noticed varies with the operating
    // system and is not configured at this level.
    if (!_socks->set_keep_alive(_socket)) {
#ifdef DEBUG_SPOCKET
      LOG("couldn't set watchdog timer on socket.");
#endif
    }

//hmmm: doesn't this need to be done for bcast too?

    // create the spocket address that we will connect to.
    if (strlen(_where->hostname)
//        && (_where->is_nil_address() 
//            || (*_last_resolve < time_stamp(-RESOLVE_INTERVAL) ) ) ) {
//
//moving to always re-resolving before a connect.  otherwise we have somewhat
//hard to predict behavior about when the re-resolve will happen.
          ) {
      // we know we need to resolve if the address is NULL_POINTER or if the re-resolve
      // interval has elapsed.
      astring full_host;
      byte_array ip_addr = _stack->full_resolve(_where->hostname, full_host);
      if (ip_addr.length()) {
        ip_addr.stuff(internet_address::ADDRESS_SIZE, _where->ip_address);
        LOG(astring("successfully re-resolved address--") + _where->text_form());
      }
      *_last_resolve = time_stamp();  // reset since we just resolved.
    }

    // special code for forcing a client to bind.
    if (_client_bind) {
      sockaddr sock = _stack->convert(*_cli_bind);

#ifdef DEBUG_SPOCKET
      LOG(a_sprintf("binding client socket %d to ", _socket)
          + inet_ntoa(((sockaddr_in *)&sock)->sin_addr));
#endif

      // now, the socket address is bound to our socket.
      if (negative(bind(_socket, &sock, sizeof(sock)))) {
        LOG(a_sprintf("error binding socket %d to ", _socket)
            + inet_ntoa(((sockaddr_in *)&sock)->sin_addr));
      }
    }

  } else if ( (_type == BROADCAST) || (_type == UNICAST) ) {
    // this is the last piece of preparation for a broadcast or unicast socket.
    // there's no real connection, so we just need to get it bound and ready
    // to fling packets.
    GRAB_LOCK;
    sockaddr sock = _stack->convert(*_where);

#ifdef DEBUG_SPOCKET
    LOG(a_sprintf("binding socket %d to ", _socket)
        + inet_ntoa(((sockaddr_in *)&sock)->sin_addr));
#endif

    // now, the socket address is bound to our socket.
    if (negative(bind(_socket, &sock, sizeof(sock)))) {
      LOG(a_sprintf("error binding socket %d to ", _socket)
          + inet_ntoa(((sockaddr_in *)&sock)->sin_addr));
    }

    // that's it for broadcast preparation.  we should be ready.
    _was_connected = true;
    return OKAY;
  }

  // the following is for connected mode only.

  sockaddr sock = _stack->convert(*_where);

  // attempt the connection now.

//hmmm: error returns are done differently on bsd, right?
//hmmm: perhaps hide the base connect in a func that sets our internal
//      error variable and then allows comparison to enums we provide.

  time_stamp abort_time(communication_wait);

  bool connected = false;  // did we connect.

  int sock_len = sizeof(sock);

  while (time_stamp() < abort_time) {
    // make the low-level socket connection.
    int ret = ::connect(_socket, &sock, sock_len);
    if (ret != SOCKET_ERROR) {
      connected = true;
      _socks->set_non_blocking(_socket, true);
      break;
    }

    basis::un_int last_error = critical_events::system_error();

    // if we're already done, then make this look like a normal connect.
    if (last_error == SOCK_EISCONN) {
      connected = true;
      break;
    }

    if ( (last_error != SOCK_EWOULDBLOCK)
        && (last_error != SOCK_EINPROGRESS) ) {
      // this seems like a real error here.
#ifdef DEBUG_SPOCKET
      LOG(a_sprintf("Connect failed (error %s or %d) on address:",
          critical_events::system_error_text(last_error).s(), last_error)
          + _where->text_form());
#endif
      if (last_error == SOCK_ECONNREFUSED) return NO_ANSWER;
//hmmm: fix more of the possibilities to be sensible outcomes?
      return ACCESS_DENIED;
    }

    if (time_stamp() >= abort_time) break;  // skip before sleeping if T.O.

    // snooze for a bit before trying again.
    time_control::sleep_ms(10);
  }

  if (connected) {
#ifdef DEBUG_SPOCKET
    LOG(a_sprintf("socket %d connected to server.", _socket));
#endif
    GRAB_LOCK;  // short lock.
    _was_connected = true;
    return OKAY;
  }

  return TIMED_OUT;
}

outcome spocket::accept(spocket * &sock, bool wait)
{
  FUNCDEF("accept");
  CHECK_BOGUS(NO_CONNECTION);
  if (_type != CONNECTED) return BAD_INPUT;

  // we don't lock in here; we should not be locking on the server socket.

  sock = NULL_POINTER;  // reset.

  if (_socket) {
#ifdef DEBUG_SPOCKET
    LOG("tried to accept on a client spocket.");
#endif
    return NOT_SERVER;
  }
  _client = false;

  if (!_server_socket) {
    _server_socket = int(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
#ifdef DEBUG_SPOCKET
    LOG(a_sprintf("srv sock is %d", _server_socket));
    LOG(astring("creating server socket now for ") + _where->text_form());
#endif

    if (_server_socket == basis::un_int(INVALID_SOCKET)) {
      LOG("Failed to open the serving spocket.");
      return BAD_INPUT;
    }

    // mark the socket so we don't get bind errors on in-use conditions.
    if (!_socks->set_reuse_address(_server_socket)) 
      LOG("Failed to mark the socket for re-use.");

    // create the spocket address for where we exist.
    sockaddr sock = _stack->convert(*_where);

    // now, the spocket address is bound to our spocket.
    int sock_len = sizeof(sock);
    if (bind(_server_socket, (sockaddr *)&sock, sock_len) < 0) {
      LOG(astring("Error on bind of ") + critical_events::system_error_text(critical_events::system_error()));
      _socks->close(_server_socket);
      return ACCESS_DENIED;
    }

    // now listen for a connection on our spocket.
    if (listen(_server_socket, PENDING_CONNECTIONS_ALLOWED) < 0) {
      LOG(astring("Listen failed with error of ")
          + critical_events::system_error_text(critical_events::system_error()));
      _socks->close(_server_socket);
      return ACCESS_DENIED;
    }
  }

  // do the kind of accept they want; either block on it or don't.
  // since our server socket is never used for sends or receives, we pretty
  // much control it completely and this is safe.
  if (!wait) {
    _socks->set_non_blocking(_server_socket, true);
      // mark our socket as non-blocking so we don't get stuck in accepts.
  } else {
    _socks->set_non_blocking(_server_socket, false);
      // mark our socket as blocking; we will be paused until accept occurs.
  }

  // now try accepting a connection on the spocket.
  sockaddr new_sock;
  socklen_t sock_len = sizeof(new_sock);
  basis::un_int accepted = int(::accept(_server_socket, &new_sock, &sock_len));
  int error = critical_events::system_error();
  if (!accepted || (accepted == INVALID_SOCKET)) {
    if (error == SOCK_EWOULDBLOCK) return NO_CONNECTION;
#ifdef DEBUG_SPOCKET
    LOG(astring("Accept got no client, with an error of ")
        + critical_events::system_error_text(error));
#endif
    return ACCESS_DENIED;
  }

  // mark the new spocket for non-blocking I/O.
  _socks->set_non_blocking(accepted, true);

//move to socks object!
  int sock_hop = 1;
  if (setsockopt(accepted, SOL_SOCKET, SO_KEEPALIVE, (char *)&sock_hop,
      sizeof(sock_hop)) < 0) {
#ifdef DEBUG_SPOCKET
    LOG("couldn't set watchdog timer on socket.");
#endif
  }

#ifdef DEBUG_SPOCKET
  LOG(astring("accepted a client on our socket: ") + _where->text_form());
#endif

// NOTE: normally, our network code sets the spocket to be kept alive (using
//       keep alives), but we are trying to have a minimal spocket usage and
//       a minimal network load for this test scenario.

  // create the spocket address that we will connect to.
  sock = new spocket(*_where);
  *sock->_remote = _stack->convert(new_sock);
  sock->_socket = accepted;
  sock->_server_socket = 0;  // reset to avoid whacking.
  sock->_was_connected = true;
  return OKAY;
}

outcome spocket::send(const byte_array &to_send, int &len_sent)
{
  return send(to_send.observe(), to_send.length(), len_sent);
}

outcome spocket::send(const abyte *buffer, int size, int &len_sent)
{
  FUNCDEF("send");
  CHECK_BOGUS(OKAY);
  if (_type != CONNECTED) return BAD_INPUT;
  GRAB_LOCK;
  ENSURE_HEALTH(NO_CONNECTION);

  len_sent = ::send(_socket, (char *)buffer, size, 0);
  int error_code = critical_events::system_error();
  if (!len_sent) {
#ifdef DEBUG_SPOCKET
    LOG("No data went out on the spocket.");
#endif
    return PARTIAL;
  }
  if (len_sent == SOCKET_ERROR) {
    if (error_code == SOCK_EWOULDBLOCK) {
#ifdef DEBUG_SPOCKET
      LOG("would block, will try later...");
      if (len_sent > 0)
        LOG("HEY HEY!  some was sent but we were not counting it!!!");
#endif
      return NONE_READY;
    }
#ifdef DEBUG_SPOCKET
    LOG(astring("Error ") + critical_events::system_error_text(error_code)
        + " occurred during the send!");
#endif
    if (!connected()) return NO_CONNECTION;
#ifdef DEBUG_SPOCKET
    LOG(a_sprintf("forcing disconnect on socket %d.", _socket));
#endif
    // we're trying this new approach here...  we found that the socket doesn't
    // really know that it got disconnected in some circumstances.
    disconnect();
    return ACCESS_DENIED;
  }
  if (len_sent != size) {
    // only sent part of the buffer.
#ifdef DEBUG_SPOCKET
    LOG(a_sprintf("sent %d bytes out of %d.", len_sent, size));
#endif
    return PARTIAL;
  }

  return OKAY;
}

outcome spocket::send_to(const internet_address &where_to,
    const byte_array &to_send, int &len_sent)
{
  return send_to(where_to, to_send.observe(), to_send.length(), len_sent);
}

outcome spocket::send_to(const internet_address &where_to, const abyte *to_send,
    int size, int &len_sent)
{
  FUNCDEF("send_to");
  CHECK_BOGUS(OKAY);
  if (_type == CONNECTED) return BAD_INPUT;
  sockaddr dest = _stack->convert(where_to);
  int ret = sendto(_socket, (char *)to_send, size, 0, &dest, sizeof(dest));
  int error = critical_events::system_error();
  if (ret < 0) {
    if (error == SOCK_EWOULDBLOCK) return NONE_READY;  // no buffer space?
    LOG(astring("failed to send packet; error ")
        + _stack->tcpip_error_name(error));
    return ACCESS_DENIED;
  }
  if (ret != size) {
    LOG(astring("didn't send whole datagram!"));
  }
  len_sent = ret;
  return OKAY;
}

outcome spocket::receive(byte_array &buffer, int &size)
{
  FUNCDEF("receive");
  CHECK_BOGUS(NONE_READY);
  if (_type != CONNECTED) return BAD_INPUT;
  if (size <= 0) return BAD_INPUT;
  buffer.reset(size);
  outcome to_return = receive(buffer.access(), size);
  // trim the buffer to the actual received size.
  if (to_return == OKAY)
    buffer.zap(size, buffer.last());
  return to_return;
}

outcome spocket::receive(abyte *buffer, int &size)
{
  FUNCDEF("receive");
  CHECK_BOGUS(NONE_READY);
  if (_type != CONNECTED) return BAD_INPUT;
  ENSURE_HEALTH(NO_CONNECTION);
  int expected = size;
  size = 0;
  if (expected <= 0) return BAD_INPUT;
  GRAB_LOCK;
  int len = recv(_socket, (char *)buffer, expected, 0);
  if (!len) {
    // check to make sure we're not disconnected.
    int ret = _socks->select(_socket, raw_socket::SELECTING_JUST_READ);
    if (ret & SI_DISCONNECTED) {
      RECOGNIZE_DISCO; 
      return NO_CONNECTION;
    }
    // seems like more normal absence of data.
    return NONE_READY;
  } else if (len < 0) {
    if (critical_events::system_error() == SOCK_EWOULDBLOCK) return NONE_READY;
#ifdef DEBUG_SPOCKET
    LOG(astring("The receive failed with an error ")
        + critical_events::system_error_text(critical_events::system_error()));
#endif
    if (!connected()) return NO_CONNECTION;
    return ACCESS_DENIED;
  }
  size = len;
  return OKAY;
}

outcome spocket::receive_from(byte_array &buffer, int &size,
    internet_address &where_from)
{
  FUNCDEF("receive_from");
  where_from = internet_address();
  CHECK_BOGUS(NONE_READY);
  if (_type == CONNECTED) return BAD_INPUT;
  if (size <= 0) return BAD_INPUT;
  buffer.reset(size);
  outcome to_return = receive_from(buffer.access(), size, where_from);
  // trim the buffer to the actual received size.
  if (to_return == OKAY)
    buffer.zap(size, buffer.last());
  return to_return;
}

outcome spocket::receive_from(abyte *buffer, int &size,
    internet_address &where_from)
{
  FUNCDEF("receive_from");
  where_from = internet_address();
  CHECK_BOGUS(NONE_READY);
  if (_type == CONNECTED) return BAD_INPUT;
  ENSURE_HEALTH(NO_CONNECTION);
  int expected = size;
  size = 0;
  if (expected <= 0) return BAD_INPUT;
  GRAB_LOCK;
  sockaddr from;
  socklen_t fromlen = sizeof(from);
  int len = recvfrom(_socket, (char *)buffer, expected, 0, &from, &fromlen);
  int err = critical_events::system_error();
  if (!len) return NONE_READY;
  else if (len < 0) {
#ifdef DEBUG_SPOCKET
    LOG(a_sprintf("actual sys err value=%d", err));
#endif
    if (err == SOCK_EWOULDBLOCK) return NONE_READY;
    if (err == SOCK_ECONNRESET) return NONE_READY;
      // this seems to be a necessary windoze kludge; we're not connected
      // and never were but it says this idiotic garbage about the connection
      // being reset.
#ifdef DEBUG_SPOCKET
    LOG(astring("The recvfrom failed with an error ")
        + critical_events::system_error_text(err));
#endif
    if (!connected()) return NO_CONNECTION;
    return ACCESS_DENIED;
  }
  where_from = _stack->convert(from);
  size = len;
  return OKAY;
}

} //namespace.

