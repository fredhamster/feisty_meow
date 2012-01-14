/*
*  Name   : test_ucast_spocket
*  Author : Chris Koeritz
*  Purpose: This is the "main" program for our sockets tester.  It parses command
*  line parameters and starts up the tester class.
**
* Copyright (c) 2001-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include "bcast_spocketer.h"

#include <application/hoople_main.h>
#include <basis/byte_array.h>
#include <basis/astring.h>
#include <loggers/program_wide_logger.h>
#include <processes/launch_process.h>
#include <structures/static_memory_gremlin.h>
#include <sockets/internet_address.h>
#include <timely/time_control.h>
#include <unit_test/unit_base.h>

#include <stdio.h>
#include <string.h>

using namespace application;
using namespace basis;
using namespace loggers;
using namespace mathematics;
using namespace processes;
using namespace sockets;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger().get(), astring(to_print))

const int INITIAL_DELAY = 1;  // number of seconds before starting sends.

typedef abyte ip_address_holder[4];

class test_ucast_spocket : public virtual application_shell, public virtual unit_base
{
public:
  test_ucast_spocket() {}
  DEFINE_CLASS_NAME("test_ucast_spocket");
  virtual int execute();

  bool parse_address(const astring &input, ip_address_holder &ip_address);
};

//hmmm: extract this to function in sockets.
bool test_ucast_spocket::parse_address(const astring &input, ip_address_holder &ip_address)
{
  int index = 0;  // current storage position in our array.
  int current_byte = 0;
  const char *last_period = 0;  // helps check for non-empty numbers.
  bool got_digit = false;
  for (const char *address = input.s(); *address; address++) {
    if ( (*address <= '9') && (*address >= '0') ) {
      current_byte *= 10;  // shift over.
      current_byte += *address - '0';  // add in current character.
      got_digit = true;
    } else if (*address == '.') {
      got_digit = false;
      if (last_period + 1 == address) {
        LOG("The IP address entry has an empty digit.  Exiting.");
        return false;
      }
      last_period = address;  // set our last period location.
      if (current_byte > 255) {
        LOG("The IP address entry has an illegal abyte.  Exiting.");
        return false;
      }
      ip_address[index] = abyte(current_byte);  // store current byte.
      current_byte = 0;  // reset.
      index++;  // next place in array.
      if (index > 3) break;
        // stop if there are too many periods, but keep accumulated address.
    } else {
      LOG("The IP address entry has illegal characters.  Exiting.");
      return false;
    }
  }
  // catch the case where we ran out of chars before adding the last byte.
  if ( (index == 3) && got_digit) {
    if (current_byte > 255) {
      LOG("The IP address entry has an illegal abyte.  Exiting.");
      return false;
    }
    ip_address[index] = current_byte;
  } else if (index < 4) {
    LOG("The IP address entry is too short.  Exiting.");
    return false;
  }
  return true;
}

int test_ucast_spocket::execute()
{
//this preamble with command line parsing is identical to bcast version,
//and will be very close to regular spocket version.
//get it extracted to a helper class for use by all three.
  FUNCDEF("execute");
  ip_address_holder ip_address;  // accumulates the source address.
  ip_address_holder dest_addr;  // where to send stuff to.
  int rcv_port = 0;
  int send_port = 0;
  int send_size = 0;
  int send_count = 0;

  const char *DEFAULT_HOST = "127.0.0.1";
  const int DEFAULT_PORT = 12342;
  const int DEFAULT_SEND_SIZE = 1008;
  const int DEFAULT_SEND_COUNT = 10;

  if (_global_argc < 7) {
    if (_global_argc > 1) {
      LOG("\
This program takes six command line arguments to begin operation.\n\
These arguments (in order) are:\n\
\tIP address for src\tIn the form w.x.y.z\n\
\tIP address for dest\tIn the form w.x.y.z\n\
\tReceive Port number\tAs a short integer\n\
\tSending Port number\tAs a short integer\n\
\tSend size\t\tThe size of the data to exchange.\n\
\tSend count\t\tThe number of \"packets\" to exchange.\n\
Note: it is expected that the testers have equal send sizes; this\n\
allows the receiver to know when it's gotten all the data that's\n\
expected during a cycle.");
      return 1;  // bail if they provided anything; otherwise we test.
    } else {
      parse_address(DEFAULT_HOST, ip_address);
      parse_address(DEFAULT_HOST, dest_addr);
      rcv_port = DEFAULT_PORT;
      send_port = DEFAULT_PORT + 1;
      send_size = DEFAULT_SEND_SIZE;
      send_count = DEFAULT_SEND_COUNT;
    }
  }

  // only parse the parameters if we got enough from the user; otherwise we accept our
  // defaults to do a simple test run.
  if (_global_argc >= 7) {

    if (!parse_address(_global_argv[1], ip_address)) {
      LOG("failed to parse source address.");
      return 9283;
    }

    LOG(a_sprintf("\tParsed a source of: \"%d.%d.%d.%d\".",
        (int)ip_address[0], (int)ip_address[1], (int)ip_address[2],
        (int)ip_address[3]));

    if (!parse_address(_global_argv[2], dest_addr)) {
      LOG("failed to parse dest address.");
      return 9283;
    }

    LOG(a_sprintf("\tParsed a destination of: \"%d.%d.%d.%d\".",
        (int)dest_addr[0], (int)dest_addr[1], (int)dest_addr[2],
        (int)dest_addr[3]));

    // parse the third parameter: the port.
    if (sscanf(_global_argv[3], "%d", &rcv_port) < 1) {
      LOG("The port entry is malformed.  Exiting.");
      return 3;
    }
    LOG(a_sprintf("\tGot a receive port of %d.", rcv_port));

    // parse the fourth parameter: the port.
    if (sscanf(_global_argv[4], "%d", &send_port) < 1) {
      LOG("The port entry is malformed.  Exiting.");
      return 3;
    }
    LOG(a_sprintf("\tGot a send port of %d.", send_port));

    // parse the fifth parameter: the size of the sends.
    if (sscanf(_global_argv[5], "%d", &send_size) < 1) {
      LOG("The send size entry is malformed.  Exiting.");
      return 5;
    }
    LOG(a_sprintf("\tGot a send size of %d.", send_size));

    // parse the sixth parameter: the number of sends.
    if (sscanf(_global_argv[6], "%d", &send_count) < 1) {
      LOG("The send count entry is malformed.  Exiting.");
      return 5;
    }
    LOG(a_sprintf("\tGot a send count of %d.", send_count));
  }

  if (_global_argc == 1) {
    // launch a paired duplicate of our test so we can chat.
    launch_process zingit;
    un_int kidnum;
    un_int result = zingit.run(_global_argv[0],
        astring(DEFAULT_HOST) + " " +  DEFAULT_HOST + " "
        /* we have reversed the send and receive ports. */
        + a_sprintf("%d", DEFAULT_PORT + 1) + " " + a_sprintf("%d", DEFAULT_PORT)
        + " " + a_sprintf("%d", DEFAULT_SEND_SIZE) + " " + a_sprintf("%d", DEFAULT_SEND_COUNT),
        launch_process::RETURN_IMMEDIATELY, kidnum);
    ASSERT_EQUAL(result, 0, "launching paired process should start successfully");
  }

  // package our parameters in a form the tester likes.
  internet_address to_pass(byte_array(4, ip_address), "", rcv_port);
  internet_address dest(byte_array(4, dest_addr), "", send_port);

  // now, construct our tester object.
  broadcast_spocket_tester tester(to_pass, true);

  // choose the appropriate action based on our role.
  bool outcome = tester.connect();
  if (!outcome) {
    LOG(astring("Failed to connect on the tester."));
    return 10;
  }

  LOG(a_sprintf("you now have %d seconds; get other side ready.",
      INITIAL_DELAY));
  time_control::sleep_ms(INITIAL_DELAY * SECOND_ms);
  LOG("starting test");

  // so, we're connected.  try sending the test packages out.
  testing_statistics stats;  // to be filled by the tester.
  outcome = tester.perform_test(dest, send_size, send_count * 2, stats);
    // multiply send_count since we count each side as one.
  if (!outcome) {
    LOG("Failed out of send_data; maybe other side terminated.");
  }

  stats.total_runs /= 2;  // cut down to the real number of tests.

  if (!stats.total_runs)
    stats.total_runs = 1;

  // now report on the stats that we get from the data sending.
  LOG(a_sprintf("Report for %d completed test cycles.", stats.total_runs));
  LOG("");
  LOG("\t\tsend stats\t\treceive stats");
  LOG("\t\t----------\t\t-------------");
  LOG(a_sprintf("bytes\t\t%d\t\t\t%d", stats.bytes_sent,
      stats.bytes_received));
  LOG(a_sprintf("time\t\t%d\t\t\t%d", stats.send_time, stats.receive_time));
  LOG(a_sprintf("avg. bytes\t%d\t\t\t%d", stats.bytes_sent
      / stats.total_runs / 2, stats.bytes_received / stats.total_runs / 2));
  LOG("");
  LOG(a_sprintf("round trip time: %d ms", stats.round_trip_time));
//hmmm: use the bandwidth measurer object!!!
  double bandwidth = double(stats.bytes_sent + stats.bytes_received)
      / stats.round_trip_time / 1024.0 * 1000.0;
  LOG(a_sprintf("bandwidth overall: %f K/s", bandwidth));

  if (_global_argc == 1) return final_report();
  else return 0;  // no unit test report for non-top-level process
}

HOOPLE_MAIN(test_ucast_spocket, );

