#ifndef TEST_CROMP_CLASS
#define TEST_CROMP_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : broadcast_spocket_tester                                          *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Puts the spocket class in broadcast mode through some test paces.        *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <sockets/internet_address.h>
#include <sockets/spocket.h>
#include <sockets/tcpip_stack.h>

// this structure is filled by the tester during a send data call.
class testing_statistics
{
public:
  int total_runs;       // how many cycles did we successfully do?
  int bytes_sent;       // overall count of bytes sent.
  int bytes_received;   // overall count of bytes received.
  int send_time;        // time taken just to invoke "send" function.
  int receive_time;     // time taken just to invoke "recv" function.
  int round_trip_time;  // time taken between sending and receiving back.

  testing_statistics()
  : total_runs(0), bytes_sent(0), bytes_received(0), send_time(0),
    receive_time(0), round_trip_time(0) {}
};

// our main tester class.
class broadcast_spocket_tester
{
public:
  broadcast_spocket_tester(const sockets::internet_address &where,
          bool unicast = false);
    // constructs the tester object.  "where" provides information about either
    // this side (for a server) or the other side (for a client).
    // if "unicast" is true, then we test unicasts instead of broadcasts.

  ~broadcast_spocket_tester();
    // destroys the tester object.

  DEFINE_CLASS_NAME("broadcast_spocket_tester");

  bool connect();
    // gets ready for sending and reception.

  bool do_a_send(const sockets::internet_address &where_to, basis::abyte *buffer, int size,
          testing_statistics &stats);

  bool do_a_receive(int size_expected, testing_statistics &stats);

  bool perform_test(const sockets::internet_address &dest, int size, int count,
          testing_statistics &stats_to_fill);
    // sends "count" random data chunks of the "size" specified.  the measured
    // performance during this sending is reported in "stats_to_fill".

private:
  sockets::internet_address *_where;  // our communications endpoint.
  sockets::tcpip_stack *_stack;  // provides access to the operating system layers.
  sockets::spocket *_socket;  // does the communication for us.
  sockets::spocket *_root_server;  // used for server side testing.
  sockets::raw_socket *_raw;  // provides functions on sockets.
  bool _ucast;  // true if we're unicasting.
};

#endif

