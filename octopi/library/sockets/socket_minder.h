#ifndef SOCKET_MINDER_CLASS
#define SOCKET_MINDER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : socket_minder                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Provides a base for activities that tend a communication element called  *
*  a socket.  Sockets are numerically identified, but the number is only      *
*  unique on the specific medium of interest.  The socket minder also tracks  *
*  the send and receive status and buffers for the socket.                    *
*                                                                             *
*******************************************************************************
* Copyright (c) 1999-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/contracts.h>
#include <processes/mailbox.h>
#include <processes/post_office.h>

namespace sockets {

// forward declarations.
class raw_socket;
class socket_data;
class socket_data_amorph;
class socket_minder_prompter;
class tcpip_stack;

//////////////

class socket_minder : public virtual basis::root_object
{
public:
  socket_minder(processes::post_office &post, int parent_route, int event_type,
          int message);
    // the "parent_route" is where we will send asynchronous tcpip events,
    // which will be stored in the "post".  the "event_type" is the identifier
    // for the OS_events that will be generated and the "message" is the id
    // stored inside the events.

  virtual ~socket_minder();

  void disengage();
    // this method should be invoked before the socket_minder is destroyed.
    // it ensures that the object is released from background event delivery.

  DEFINE_CLASS_NAME("socket_minder");

  basis::astring text_form() const;
    // returns a dump of the minder's current state.

  socket_data *lock_socket_data(int socket);
    // locates the data for the "socket" specified.  the list is left locked
    // unless NULL_POINTER is returned.

  void unlock_socket_data(socket_data *to_unlock);
    // unlocks the list of socket data again and zeroes "to_unlock".

  bool add_socket_data(int socket, bool server, int server_socket,
          bool connected_mode, bool connection_pending);
    // adds a new record for the "socket" which is possibly a "server".  only
    // fails with false when the socket is already listed.  the
    // "connected_mode" should be true if this socket is connection-oriented.

  bool remove_socket_data(int socket);
    // rips out the data held for "socket".  only fails with false when the
    // record couldn't be found to delete.

  bool set_connection_pending(int socket, bool pending);
    // changes the state of pending connection for the "socket".  if "pending"
    // is true, then the socket is either trying to accept connections or it
    // is trying to connect.  if false, then it is no longer attempting this.

  bool is_connection_pending(int socket);
    // returns the current state of pending-ness for the "socket".  false could
    // also mean the socket cannot be found.

  bool register_interest(int socket, int interests);
    // sets the events to watch for on the "socket" from the "interests".
    // these are values merged bitwise from the socket_interests enum.  they
    // must be interpreted appropriately by different kinds of transports.  the
    // evaluate_interest() method must be over-ridden for the interests to
    // actually be implemented.  registering with "interests" as zero will
    // reset all interests to be... disinterested.

  virtual bool evaluate_interest(socket_data &to_examine);
    // this can be over-ridden by a derived socket minder to handle needs
    // besides simple bsd socket management.  but the implementation provided
    // here will generate events upon occurrence of a registered interest on
    // the socket.  an event letter is sent to the appropriate parent id
    // containing the event that was noticed.  true is returned if "to_examine"
    // was successfully evaluated.  any changes made to the socket's data are
    // recorded.  since the socket minder is locked during this upcall, it is
    // important not to cause any deadlocks by careless additional locking.

  int get_pending_server();
    // returns a non-zero socket number if a server socket was accepted on
    // and is waiting to be processed.

  bool zap_pending_server(int socket);
    // removes the "socket" from the pending servers list, if present.

  void put_pending_server(int to_put, bool at_head);
    // stores a pending server socket into the list, either since it just got
    // noticed as an accepted socket or because it cannot be processed yet.

  void get_sockets(basis::int_array &read_sox, basis::int_array &write_sox,
          basis::int_array &pending) const;
    // reports the set of sockets that this minder is handling in "read_sox",
    // since we assume any socket could be checked for pending received data.
    // if there is pending data to send, then they are put into the "write_sox"
    // to be checked for writability.  the existing contents of the lists are
    // not cleared, so this function can be used to join the lists of several
    // socket_minders together.

  bool owns_socket(int socket) const;
    // returns true if this minder owns the "socket" in question.

  void push_sends(socket_data &to_poke, int states);
  void push_receives(socket_data &to_poke, int states);
    // if the state is right, we'll try our hand at grabbing some data.  if the
    // "states" include SI_READABLE, then we'll try and receive on the socket
    // in push_receives.  if they include SI_WRITABLE, then we'll send out
    // pending data in push_sends.

  bool handle_pending_connecters(socket_data &to_peek);
    // returns true if the socket "to_peek" is awaiting a connection and we
    // have prompted that activity appropriately.  false means that this
    // socket is not awaiting a connection.

  void snoozy_select();
    // goes into a select on all applicable sockets and waits until one
    // of them has activity before waking up.

private:
  processes::post_office &_post;  // manages recycling of letters for us.
  int _parent_route;  // the identifier of our parent in the postal system.
  int _event_type;  // what we generate OS_events as.
  basis::mutex *_lock;  // maintains list integrity.
  socket_data_amorph *_socket_list;  // the table of information per socket.
  raw_socket *_socks;  // lets us access the OS's socket subsystem.
  tcpip_stack *_stack;  // provides tcpip protocol stack.
  int _message;  // the message type stored inside the generated OS_events.
  structures::int_set *_pending_sox;  // accepted servers that are waiting to be processed.
  socket_minder_prompter *_prompter;  // snoozes on sockets awaiting activity.

  void fire_event(int to_fire, int at_whom, basis::un_int parm1, basis::un_int parm2);

  // not implemented:
  socket_minder(const socket_minder &sm);
  socket_minder &operator =(const socket_minder &sm);
};

} //namespace.

#endif

