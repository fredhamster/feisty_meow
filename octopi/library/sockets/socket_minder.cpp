/*****************************************************************************\
*                                                                             *
*  Name   : socket_minder                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1999-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "raw_socket.h"
#include "socket_data.h"
#include "socket_minder.h"
#include "tcpip_stack.h"

#include <basis/mutex.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <processes/ethread.h>
#include <processes/os_event.h>
#include <structures/set.h>
#include <structures/amorph.h>
#include <structures/unique_id.h>
#include <textual/parser_bits.h>

#include <errno.h>
#ifdef __WIN32__
  #include <ws2tcpip.h>
#endif
#ifdef __UNIX__
  #include <arpa/inet.h>
  #include <sys/socket.h>
#endif

using namespace basis;
using namespace loggers;
using namespace processes;
using namespace structures;
using namespace textual;
using namespace timely;

namespace sockets {

//#define DEBUG_SOCKET_MINDER
  // uncomment for noisiness.

#undef LOG
#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger::get(), to_print)

const int SOCKET_CHECK_INTERVAL = 50;
  // we will scoot around in our sockets this frequently.

const int SOCKMIND_MAXIMUM_RECEIVES = 10;
  // we'll receive this many items from the socket in one go.

const int MAXIMUM_TRANSFER_CHUNK = 512 * KILOBYTE;
  // largest block of data we'll try to deal with at one time.

const int CONN_ALERT_INTERVAL = 100;
  // this is the most frequently that we will generate a connection checking
  // event.

const int MULTI_SELECT_TIMEOUT = 250;
  // the snooze on select will occur for this long.  during this interval,
  // it is likely no new sockets will be considered.

//////////////

class socket_data_amorph : public amorph<socket_data> {};

//////////////

class socket_minder_prompter : public ethread
{
public:
  socket_minder_prompter(socket_minder &parent)
  : ethread(SOCKET_CHECK_INTERVAL, ethread::SLACK_INTERVAL),
    _parent(parent)
  {
    start(NIL);
  }

  ~socket_minder_prompter() {
    stop();  // shut down our thread.
  }

  virtual void perform_activity(void *formal(ptr)) { _parent.snoozy_select(); }

private:
  socket_minder &_parent;  // the object we're hooked to.
};

//////////////

socket_minder::socket_minder(post_office &post, int parent_route,
    int event_type, int message)
: _post(post),
  _parent_route(parent_route),
  _event_type(event_type),
  _lock(new mutex),
  _socket_list(new socket_data_amorph),
  _socks(new raw_socket),
  _stack(new tcpip_stack),
  _message(message),
  _pending_sox(new int_set),
  _prompter(new socket_minder_prompter(*this))
{
  _prompter->start(NIL);
}

socket_minder::~socket_minder()
{
  _prompter->stop();
  WHACK(_prompter);
  WHACK(_socket_list);
  WHACK(_lock);
  WHACK(_pending_sox);
  WHACK(_socks); 
  WHACK(_stack);
}

void socket_minder::disengage()
{
  _prompter->stop();
}

astring socket_minder::text_form() const
{
  auto_synchronizer l(*_lock);
  astring to_return;

  for (int i = 0; i < _socket_list->elements(); i++) {
    const socket_data *curr = _socket_list->get(i);
    to_return += curr->text_form();
    if (i != _socket_list->elements() - 1)
      to_return += parser_bits::platform_eol_to_chars();
  }

  return to_return;
}

void socket_minder::snoozy_select()
{
  FUNCDEF("snoozy_select");
  int_array read_sox;
  int_array write_sox;
  int_array pending;

  get_sockets(read_sox, write_sox, pending);

  // process any with pending connections right now, rather than later.
  for (int p = 0; p < pending.length(); p++) {
    socket_data *sd = lock_socket_data(pending[p]);
    if (!sd) continue;  // something hosed there.
    handle_pending_connecters(*sd);
    unlock_socket_data(sd);
  }

  // now select on all of our sockets simultaneously.
  int ret = _socks->select(read_sox, write_sox, MULTI_SELECT_TIMEOUT);
  if (!ret || (!read_sox.length() && !write_sox.length()) ) {
    return;  // nothing happened.
  }

  // rotate through the lists and push socket_minders around as needed.
  // any sockets we have events for but no socket_data are orphans and will
  // be ignored.

  // check read sockets.
  for (int r = 0; r < read_sox.length(); r++) {
    const int sock = read_sox[r];
    if (owns_socket(sock)) {
      socket_data *sd = lock_socket_data(sock);
      if (!sd) continue;  // something hosed there.
      push_receives(*sd, SI_READABLE);
      unlock_socket_data(sd);
      read_sox.zap(r, r);
      r--;  // skip back before deleted guy.
    }
  }

  // check write sockets.
  for (int w = 0; w < write_sox.length(); w++) {
    const int sock = write_sox[w];
    if (owns_socket(sock)) {
      socket_data *sd = lock_socket_data(sock);
      if (!sd) continue;  // something hosed there.
      push_sends(*sd, SI_WRITABLE);
      unlock_socket_data(sd);
      write_sox.zap(w, w);
      w--;  // skip back before deleted guy.
    }
  }
}

void socket_minder::get_sockets(int_array &read_sox, int_array &write_sox,
    int_array &pendings) const
{
  auto_synchronizer l(*_lock);
  for (int i = 0; i < _socket_list->elements(); i++) {
    socket_data *sd = _socket_list->borrow(i);
    if (sd->_connection_pending) {
      // this is not ready for sends and receives yet.
      pendings += sd->_socket;
    } else {
      // always add sockets to test if they have data waiting.
      read_sox += sd->_socket;
      // only check on writability if there is data pending for sending.
      if (sd->_partially_sent.length())
        write_sox += sd->_socket;
    }
  }
}

bool socket_minder::owns_socket(int socket) const
{
  auto_synchronizer l(*_lock);
  for (int i = 0; i < _socket_list->elements(); i++) {
    if (_socket_list->borrow(i)->_socket == socket) return true;
  }
  return false;
}

socket_data *socket_minder::lock_socket_data(int socket)
{
  _lock->lock();
  for (int i = 0; i < _socket_list->elements(); i++)
    if (_socket_list->borrow(i)->_socket == socket)
      return _socket_list->borrow(i);
  // this is a failure to get here; there was no suitable socket.
  _lock->unlock();
  return NIL;
}

void socket_minder::unlock_socket_data(socket_data *to_unlock)
{
  if (!to_unlock) return;
//can't affect it now.  to_unlock = NIL;
  _lock->unlock();
}

bool socket_minder::add_socket_data(int socket, bool server, int server_socket,
    bool connected_mode, bool connection_pending)
{
  auto_synchronizer l(*_lock);
  socket_data *harpo = lock_socket_data(socket);
  if (harpo) {
    unlock_socket_data(harpo);
    return false;
  }
  socket_data *new_data = new socket_data(socket, server, server_socket,
      connected_mode);
  _socks->set_non_blocking(socket);
    // ensure the new guy is treated as non-blocking.  unix does not seem
    // to inherit this from the parent.
  new_data->_connection_pending = connection_pending;
  _socket_list->append(new_data);
  return true;
}

bool socket_minder::remove_socket_data(int socket)
{
  FUNCDEF("remove_socket_data");
  auto_synchronizer l(*_lock);
  for (int i = 0; i < _socket_list->elements(); i++) {
    if (_socket_list->borrow(i)->_socket == socket) {
      // give the socket a last chance to get its data out.
      evaluate_interest(*_socket_list->borrow(i));
      _socket_list->zap(i, i);
      return true;
    }
  }
//  LOG(a_sprintf("couldn't find socket %d.", socket));
  return false;
}

bool socket_minder::register_interest(int socket, int interests)
{
#ifdef DEBUG_SOCKET_MINDER
  FUNCDEF("register_interest");
#endif
  socket_data *harpo = lock_socket_data(socket);
#ifdef DEBUG_SOCKET_MINDER
  LOG(astring(astring::SPRINTF, "registering interest of %d for socket "
      "%d.", interests, socket));
#endif
  if (!harpo) return false;
  harpo->_registered_interests = interests;
  unlock_socket_data(harpo);
  return true;
}

bool socket_minder::is_connection_pending(int socket)
{
  socket_data *harpo = lock_socket_data(socket);
  if (!harpo) return false;
  bool to_return = harpo->_connection_pending;
  unlock_socket_data(harpo);
  return to_return;
}

bool socket_minder::set_connection_pending(int socket, bool pending)
{
  socket_data *harpo = lock_socket_data(socket);
  if (!harpo) return false;
  harpo->_connection_pending = pending;
  unlock_socket_data(harpo);
  return true;
}

void socket_minder::fire_event(int to_fire, int at_whom,
    basis::un_int parm1, basis::un_int parm2)
{
  _post.drop_off(at_whom, new OS_event(_event_type, to_fire, parm1, parm2));
}

void socket_minder::put_pending_server(int to_put, bool at_head)
{
  if (!to_put) return;  // bogus.
  auto_synchronizer l(*_lock);
  if (at_head) {
    _pending_sox->insert(0, 1);
    (*_pending_sox)[0] = to_put;
  } else {
    *_pending_sox += to_put;
  }
}

bool socket_minder::zap_pending_server(int socket)
{
  auto_synchronizer l(*_lock);
  if (!_pending_sox->member(socket)) return false;
  _pending_sox->remove(socket);
  return true;
}

int socket_minder::get_pending_server()
{
  auto_synchronizer l(*_lock);
  if (!_pending_sox->length()) return 0;
  int to_return = _pending_sox->get(0);
  _pending_sox->zap(0, 0);
  return to_return;
}

bool socket_minder::handle_pending_connecters(socket_data &to_peek)
{
  FUNCDEF("handle_pending_connecters");
  if (!to_peek._connection_pending) return false;  // not needed here.

  if (to_peek._last_conn_alert > time_stamp(-CONN_ALERT_INTERVAL)) {
    // not time yet.
    return false;
  }
  to_peek._last_conn_alert.reset();

  // force the issue; there is no simple way to portably know whether
  // the socket got a connection or not, so just say it did.
  if (!to_peek._is_server
      && (to_peek._registered_interests & SI_CONNECTED) ) {
    // deal with a client first.  this just means, zing an event.
#ifdef DEBUG_SOCKET_MINDER
    LOG(a_sprintf("sending client SI_CONNECTED event on parent %d",
        _parent_route));
#endif
    fire_event(_message, _parent_route, to_peek._socket, SI_CONNECTED);
  } else if (to_peek._is_server
      && (to_peek._registered_interests & SI_CONNECTED) ) {
    // special handling for servers.  we accept the waiting guy if he's
    // there.  otherwise we don't send the event.
    sockaddr sock_addr;
    socklen_t sock_len = sizeof(sock_addr);
    int new_sock = int(::accept(to_peek._socket, &sock_addr, &sock_len));
      // check for a new socket.
    if (new_sock != INVALID_SOCKET) {
      LOG(a_sprintf("accept got a client socket %d.", new_sock));
      if (_pending_sox->member(new_sock)) {
        LOG(a_sprintf("already have seen socket %d in pending!", new_sock));
      } else {
        *_pending_sox += new_sock;
#ifdef DEBUG_SOCKET_MINDER
        LOG(a_sprintf("sending server SI_CONNECTED event on parent %d",
            _parent_route));
#endif
        fire_event(_message, _parent_route, to_peek._socket, SI_CONNECTED);
      }
    } else if (_pending_sox->length()) {
      // there are still pending connectees.
      fire_event(_message, _parent_route, to_peek._socket, SI_CONNECTED);
    }
  }
  // also, if the connection is still pending, we don't want to select on
  // it yet.
  return true;
}

bool socket_minder::evaluate_interest(socket_data &to_peek)
{
  FUNCDEF("evaluate_interest");
  if (to_peek._connection_pending) {
    return handle_pending_connecters(to_peek);
  }

  int sel_mode = 0;

  int states = _socks->select(to_peek._socket, sel_mode);

  if (!states) {
    return true;  // nothing to report.
  }

  if (! (states & SI_ERRONEOUS) && ! (states & SI_DISCONNECTED) ) {
    push_sends(to_peek, states);
    push_receives(to_peek, states);
  }

  if ( (to_peek._registered_interests & SI_ERRONEOUS)
      && (states & SI_ERRONEOUS) ) {
    // there's some kind of bad problem on the socket.
    LOG(a_sprintf("socket %d has status of erroneous.", to_peek._socket));
//hmmm: what to do?  generate an event?
  }

  if ( (to_peek._registered_interests & SI_DISCONNECTED)
      && (states & SI_DISCONNECTED) ) {
    // we lost our state of connectedness.
    fire_event(_message, _parent_route, to_peek._socket,
        SI_DISCONNECTED);
    return true;  // get out now.
  }

  return true;
}

void socket_minder::push_sends(socket_data &to_poke, int states)
{
  FUNCDEF("push_sends");
  if (to_poke._connection_pending) {
    LOG("not supposed to try this when not connected yet...");
  }

#ifdef DEBUG_SOCKET_MINDER
  if (to_poke._partially_sent.length()) {
    LOG(a_sprintf("socket %d: %d bytes to send.", to_poke._socket,
        to_poke._partially_sent.length()));
  }
#endif

  int error_code = 0;

  if ( (states & SI_WRITABLE) && to_poke._partially_sent.length()) {
    // write to the socket since we see an opportunity.
    byte_array &to_send = to_poke._partially_sent;
    int len_sent = send(to_poke._socket, (char *)to_send.observe(),
        to_send.length(), 0);
    if (!len_sent) {
      // nothing got sent.
      LOG(a_sprintf("didn't send any data on socket %d.", to_poke._socket));
    } else if (len_sent == SOCKET_ERROR) {
      // something bad happened.
      error_code = critical_events::system_error();
    } else {
#ifdef DEBUG_SOCKET_MINDER
      LOG(a_sprintf("updating that %d bytes got sent out of %d to send.",
          len_sent, to_send.length()));
      if (to_send.length() != len_sent)
        LOG(a_sprintf("size to send (%d) not same as size sent (%d).",
            to_send.length(), len_sent));
#endif
      // update the partially sent chunk for the bit we sent out.
      to_send.zap(0, len_sent - 1);
    }
  }

  // handle errors we have seen.
  if (error_code) {
    if (error_code != SOCK_EWOULDBLOCK)
      LOG(astring(astring::SPRINTF, "error on socket %d is %s.",
          to_poke._socket, _stack->tcpip_error_name(error_code).s()));

    switch (error_code) {
      case SOCK_ENOTSOCK:  // fall-through.
      case SOCK_ECONNABORTED:  // fall-through.
      case SOCK_ECONNRESET: {
        // the connection got hammerlocked somehow.
        LOG(a_sprintf("due to %s condition, closing socket %d.",
            _stack->tcpip_error_name(error_code).s(), to_poke._socket));
        fire_event(_message, _parent_route, to_poke._socket,
            SI_DISCONNECTED);
        to_poke._partially_sent.reset();  // clear with no connection.
        break;
      }
    }
  }
}

void socket_minder::push_receives(socket_data &to_poke, int states)
{
  FUNCDEF("push_receives");
  if (to_poke._connection_pending) {
    LOG("not supposed to try this when not connected yet...");
  }

#ifdef DEBUG_SOCKET_MINDER
  if (to_poke._partially_received.length())
    LOG(a_sprintf("socket %d: %d bytes already received.", to_poke._socket,
        to_poke._partially_received.length()));
#endif

  int error_code = 0;

  if ( (states & SI_READABLE) && to_poke._connected_mode) {
    // grab any data that's waiting on the connection-oriented socket.

    bool got_something = true;  // hopeful for now.
    to_poke._receive_buffer.reset(MAXIMUM_TRANSFER_CHUNK);
      // allocate space where we'll get new data.
    int count = 0;
    while (got_something && (count++ < SOCKMIND_MAXIMUM_RECEIVES)) {
      got_something = false;  // now get all pessimistic.
      int len = recv(to_poke._socket, (char *)to_poke._receive_buffer.observe(),
          to_poke._receive_buffer.length(), 0);
      if (len > 0) {
#ifdef DEBUG_SOCKET_MINDER
        LOG(a_sprintf("received %d bytes on socket %d.", len, to_poke._socket));
#endif
        // we received some actual data; we're happy again.
        got_something = true;
        // zap any extra off.
        if (len < MAXIMUM_TRANSFER_CHUNK)
          to_poke._receive_buffer.zap(len, to_poke._receive_buffer.last());
        // add in what we were given.
        to_poke._partially_received += to_poke._receive_buffer;
        to_poke._receive_buffer.reset(MAXIMUM_TRANSFER_CHUNK);
          // reset to the right size for trying some more.
      } else {
        error_code = critical_events::system_error();

        // reset the states flag based on current state.
        states = _socks->select(to_poke._socket,
            raw_socket::SELECTING_JUST_READ);
        if (states & SI_DISCONNECTED) {
          error_code = SOCK_ECONNRESET;  // make like regular disconnect.
          LOG(a_sprintf("noticed disconnection on socket %d.",
              to_poke._socket));
          // close the socket; we use a temporary because the close method
          // wants to actually store zero in the socket.
          basis::un_int deader = to_poke._socket;
          _socks->close(deader);
        }

      }
    }

    if ( !(states & SI_DISCONNECTED)
        && to_poke._partially_received.length()) { 
#ifdef DEBUG_SOCKET_MINDER
      LOG(a_sprintf("firing readable now: sock=%d", to_poke._socket));
#endif
      fire_event(_message, _parent_route, to_poke._socket, SI_READABLE);
    }
  } else if ( (states & SI_READABLE) && !to_poke._connected_mode) {
    // datagram style; we need to still alert the parent.
    fire_event(_message, _parent_route, to_poke._socket,
        SI_READABLE);
  }

  // handle errors we have seen.
  if (error_code) {
    if (error_code != SOCK_EWOULDBLOCK)
      LOG(astring(astring::SPRINTF, "error on socket %d is %s.",
          to_poke._socket, _stack->tcpip_error_name(error_code).s()));

    switch (error_code) {
      case SOCK_ENOTSOCK:  // fall-through.
      case SOCK_ECONNABORTED:  // fall-through.
      case SOCK_ECONNRESET: {
        // the connection got hammerlocked somehow.
        LOG(a_sprintf("due to %s condition, closing socket %d.",
            _stack->tcpip_error_name(error_code).s(), to_poke._socket));
        fire_event(_message, _parent_route, to_poke._socket,
            SI_DISCONNECTED);
        basis::un_int deader = to_poke._socket;
        _socks->close(deader);
        to_poke._partially_sent.reset();  // clear with no connection.
        break;
      }
    }
  }
}

} //namespace.

