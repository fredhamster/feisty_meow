#ifndef SOCKET_DATA_CLASS
#define SOCKET_DATA_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : socket_data                                                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Tracks the partially transmitted data for transports based on a socket   *
*  metaphor, where a socket is merely a numerical designation of the channel. *
*    Note: this is a heavy-weight header.  don't include it in other headers. *
*                                                                             *
*******************************************************************************
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/astring.h>
#include <timely/time_stamp.h>

namespace sockets {

class socket_data
{
public:
  int _socket;  // the number of the socket we are managing here.
  basis::byte_array _partially_sent;  // accumulates bytes from partial sends.
  basis::byte_array _partially_received;  // accumulates bytes from partial receives.
  basis::byte_array _receive_buffer;  // temporary that's used for reading.
  bool _is_server;  // true if this socket is for a server.
  int _registered_interests;
    // the events being watched for on this socket.  the bitwise or'ed items
    // in this are from the socket_interests enum.
  bool _connection_pending;
    // true if a connect or accept is pending.  the default is true since we
    // do not want to try probing the socket until it has been connected, for
    // sockets in connected mode.
  int _server_socket;  // non-zero if socket was accepted on root server socket.
  bool _connected_mode;  // true if this is a connected type of socket.
  timely::time_stamp _last_conn_alert;
    // when the connection was last given a check for a connected state.

  socket_data(int socket = 0, bool server = true, int server_socket = 0,
      bool connected_mode = true)
      : _socket(socket), _is_server(server), _registered_interests(0),
        _connection_pending(true), _server_socket(server_socket),
        _connected_mode(connected_mode) {}
  ~socket_data() {}

  bool server() const { return _is_server; }
  bool client() const { return !_is_server; }

  basis::astring text_form() const;
    // returns a descriptive list of the data contained here.
};

//////////////

// implementations.

basis::astring socket_data::text_form() const
{
  return basis::a_sprintf("socket=%d, type=%s, send_pend=%d, recv_pend=%d, "
      "interests=%s, conn_pending=%s",
      _socket, _is_server? "server":"client", _partially_sent.length(),
      _partially_received.length(),
      raw_socket::interest_name(_registered_interests).s(),
      _connection_pending? "true":"false");
}

} //namespace.

#endif

