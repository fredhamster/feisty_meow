/*****************************************************************************\
*                                                                             *
*  Name   : spocket_tester                                                    *
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

#include "spocket_tester.h"

#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/astring.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <mathematics/chaos.h>
#include <sockets/internet_address.h>
#include <sockets/raw_socket.h>
#include <sockets/spocket.h>
#include <sockets/tcpip_definitions.h>
#include <sockets/tcpip_stack.h>
#include <timely/time_control.h>
#include <timely/time_stamp.h>

#include <errno.h>

//using namespace application;
using namespace basis;
//using namespace filesystem;
using namespace loggers;
using namespace mathematics;
using namespace sockets;
using namespace structures;
using namespace textual;
using namespace timely;
//using namespace unit_test;

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger().get(), astring(to_print))

const int MAXIMUM_WINSOCK_MTU = 100000;
  // the largest chunk of bytes we will receive at one time.

const int MAXIMUM_TRANSFER_WAIT = 40 * SECOND_ms;
  // the longest amount of time we wait in trying to receive data.

static abyte receive_buffer[MAXIMUM_WINSOCK_MTU + 1];
  // used for dumping received data into.

const int PAUSE_TIME = 200;
  // the snooze interval when we encounter socket underflow or overflow.

//#define DEBUG_SPOCKET_TESTER
  // uncomment for noisy version.

spocket_tester::spocket_tester(const internet_address &where)
: _where(new internet_address(where)),
  _stack(new tcpip_stack),
  _socket(NIL),
  _root_server(NIL),
  _raw(new raw_socket)
{
}

spocket_tester::~spocket_tester()
{
  WHACK(_socket);
  WHACK(_root_server);
  WHACK(_stack);
  WHACK(_where);
  WHACK(_raw);
}

bool spocket_tester::connect()
{
  if (!_socket) {
    _socket = new spocket(*_where);
  }
  outcome ret = spocket::NO_CONNECTION;
  while (true) {
    ret = _socket->connect();
    if (ret == spocket::OKAY) break;
    if (ret != spocket::NO_CONNECTION) break;
    time_control::sleep_ms(100);
  }
  return ret == spocket::OKAY;
}

bool spocket_tester::accept(bool wait)
{
  if (!_root_server) {
    _root_server = new spocket(*_where);
  }
  if (_socket) {
    LOG("already have a socket for accept!");
    return true;
  }
  outcome ret = spocket::NO_CONNECTION;
  while (true) {
    ret = _root_server->accept(_socket, false);
    if (ret == spocket::OKAY) break;
    if (ret != spocket::NO_CONNECTION) break;
    if (!wait) return true;  // we accepted at least once.
    time_control::sleep_ms(100);
  }
  return ret == spocket::OKAY;
}

bool spocket_tester::do_a_send(abyte *buffer, int size,
    testing_statistics &stats)
{
  time_stamp start_time;
  int len_sent;
  time_stamp when_to_leave(MAXIMUM_TRANSFER_WAIT);
  outcome worked;

#ifdef DEBUG_SPOCKET_TESTER
  LOG("into do a send");
#endif

  while (time_stamp() < when_to_leave) {
    worked = _socket->send(buffer, size, len_sent);
    if (worked == spocket::NONE_READY) {
////      time_control::sleep_ms(PAUSE_TIME);
      _socket->await_writable(PAUSE_TIME);
      continue;
    } else if (worked == spocket::PARTIAL) {
//danger danger if we get wrong info.
      buffer += len_sent;
      size -= len_sent;
      stats.bytes_sent += len_sent;
///      time_control::sleep_ms(PAUSE_TIME);
      _socket->await_writable(PAUSE_TIME);
      continue;
    } else break;
  }
#ifdef DEBUG_SPOCKET_TESTER
  LOG("got out of loop");
#endif

  stats.send_time += int(time_stamp().value() - start_time.value());
  stats.bytes_sent += len_sent;

  if ( (worked != spocket::OKAY) && (worked != spocket::PARTIAL) ) {
    LOG(astring("No data went out on the socket: ")
        + spocket::outcome_name(worked));
    return false;
  }
  if (len_sent != size) {
    LOG(a_sprintf("partial send on socket, %d bytes instead of %d, recurse.", 
        len_sent, size));
    return do_a_send(buffer + len_sent, size - len_sent, stats);
  }

  time_stamp end_time;
//  int time_taken = int(end_time.value() - start_time.value());

  return true;
}

bool spocket_tester::do_a_receive(int size_expected, testing_statistics &stats)
{
  time_stamp start_time;

#ifdef DEBUG_SPOCKET_TESTER
  LOG("into do a rcv");
#endif

  time_stamp when_to_leave(MAXIMUM_TRANSFER_WAIT);
  int full_length = 0;
  while ( (full_length < size_expected) && (time_stamp() < when_to_leave) ) {
    time_stamp start_of_receive;
    int len = MAXIMUM_WINSOCK_MTU;
    outcome ret = _socket->receive(receive_buffer, len);
    if (ret != spocket::OKAY) {
      if (ret == spocket::NONE_READY) {
if (len != 0) LOG(a_sprintf("supposedly nothing was received (%d bytes)", len));
///        time_control::sleep_ms(PAUSE_TIME);
        _socket->await_readable(PAUSE_TIME);
        continue;
      } else break;
    }
    // reset our time if we've gotten good data.
    if (ret == spocket::OKAY)
      when_to_leave.reset(MAXIMUM_TRANSFER_WAIT);

    int receive_duration = int(time_stamp().value()
        - start_of_receive.value());
    stats.receive_time += receive_duration;

#ifdef DEBUG_SPOCKET_TESTER
    LOG(a_sprintf("did recv, len=%d", len));
#endif

    if (!len) {
      LOG("Our socket has been disconnected.");
      return false;
    } else if (len < 0) {
      if (errno == SOCK_EWOULDBLOCK) continue;  // no data.
      LOG(astring("The receive failed with an error ")
          + critical_events::system_error_text(errno));
      return false;
    }
    full_length += len;
    stats.bytes_received += len;
  }

  if (full_length != size_expected)
    LOG(a_sprintf("Did not get the full size expected (wanted %d and "
        "got %d bytes).", size_expected, full_length));

  return true;
}

bool spocket_tester::perform_test(int size, int count,
    testing_statistics &stats)
{
#ifdef DEBUG_SPOCKET_TESTER
  LOG("into perf test");
#endif

  // the statics are used to generate our random buffer for sending.
  static abyte garbage_buffer[MAXIMUM_WINSOCK_MTU + 1];
  static bool garbage_initialized = false;
  chaos randomizer;

  // if our static buffer full of random stuff was never initialized, we do
  // so now.  this supports efficiently re-using the tester if desired.
  if (!garbage_initialized) {
    LOG("initializing random send buffer.");
    // note the less than or equal; we know we have one more byte to fill.
    for (int i = 0; i <= MAXIMUM_WINSOCK_MTU; i++)
      garbage_buffer[i] = randomizer.inclusive(0, 255);
    garbage_initialized = true;
    LOG("random send buffer initialized.");
  }

  // reset the statistical package.
  stats.total_runs = 0;
  stats.send_time = 0;
  stats.receive_time = 0;
  stats.bytes_sent = 0;
  stats.bytes_received = 0;

  // check that they aren't trying to do too big of a send.
  if (size > MAXIMUM_WINSOCK_MTU) {
    LOG("The size is over our limit.  To fix this, edit the "
        "send_data function.");
    return false;
  }

  // check that our socket is usable.
  if (!_socket) {
    LOG("One cannot send data on an uninitialized tester!");
    return false;
  }

  int runs_completed = 0;
    // counts up how many times we've done our test cycle.

  while (runs_completed < count) {
#ifdef DEBUG_SPOCKET_TESTER
    LOG(a_sprintf("iter %d", runs_completed));
#endif
    if (_socket->client()) {
      // we're doing the client side routine here.
      time_stamp trip_start;
#ifdef DEBUG_SPOCKET_TESTER
      LOG("client about to send");
#endif
      if (!do_a_send(garbage_buffer, size, stats)) {
        LOG("We failed on a send.  Now quitting.");
        return false;
      }
#ifdef DEBUG_SPOCKET_TESTER
      LOG("client about to rcv");
#endif
      if (!do_a_receive(size, stats)) {
        LOG("We failed on a receive.  Now quitting.");
        return false;
      }
      stats.round_trip_time += int(time_stamp().value() - trip_start.value());
    } else {
      // we're doing the server side routine here.
      time_stamp trip_start;
#ifdef DEBUG_SPOCKET_TESTER
      LOG("server about to rcv");
#endif
      if (!do_a_receive(size, stats)) {
        LOG("We failed on a receive.  Now quitting.");
        return false;
      }
#ifdef DEBUG_SPOCKET_TESTER
      LOG("server about to send");
#endif
      if (!do_a_send(garbage_buffer, size, stats)) {
        LOG("We failed on a send.  Now quitting.");
        return false;
      }
      stats.round_trip_time += int(time_stamp().value() - trip_start.value());
    }

    runs_completed++;  // finished a run.
    stats.total_runs++;  // count it in the overall stats too.
    if ( !(runs_completed % 10) )
      LOG(a_sprintf("Completed test #%d.", runs_completed));
  }

  return true;
}

