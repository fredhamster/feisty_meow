/*
*  Name   : test_sequence_tracker
*  Author : Chris Koeritz
*  Purpose: Runs a couple of tests on the sequence tracker object.
**
* Copyright (c) 2003-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <application/hoople_main.h>
#include <basis/byte_array.h>
#include <basis/astring.h>
#include <loggers/program_wide_logger.h>
#include <mathematics/chaos.h>
#include <sockets/machine_uid.h>
#include <sockets/sequence_tracker.h>
#include <structures/set.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

//#include <stdio.h>
//#include <string.h>

using namespace application;
using namespace basis;
using namespace loggers;
using namespace mathematics;
using namespace sockets;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger().get(), astring(to_print))

class test_sequence_tracker : public virtual unit_base, virtual public application_shell
{
public:
  test_sequence_tracker() {}
  DEFINE_CLASS_NAME("test_sequence_tracker");
  virtual int execute();
};

int test_sequence_tracker::execute()
{
  FUNCDEF("execute");
  // some arbitrary ip addresses.
  abyte arb1[] = { 127, 0, 0, 1 };
  abyte arb2[] = { 192, 168, 0, 1 };
  abyte arb3[] = { 28, 42, 56, 253 };

  machine_uid eep(machine_uid::TCPIP_LOCATION, byte_array(4, arb1));
  machine_uid op(machine_uid::TCPIP_LOCATION, byte_array(4, arb2));
  machine_uid ork(machine_uid::TCPIP_LOCATION, byte_array(4, arb3));

  sequence_tracker chevy(1 * MINUTE_ms, 10 * MINUTE_ms);

  int_set eep_set;
  int adds = randomizer().inclusive(400, 900);
  int starter = 12092;
  while (adds--) {
    int seq = starter + randomizer().inclusive(1, 129);
    eep_set += seq;
    chevy.add_pair(eep, seq);
  }

  int_set op_set;
  adds = randomizer().inclusive(200, 600);
  starter = 1222;
  while (adds--) {
    int seq = starter + randomizer().inclusive(1, 129);
    op_set += seq;
    chevy.add_pair(op, seq);
  }

  int_set ork_set;
  adds = randomizer().inclusive(200, 600);
  starter = 992981;
  while (adds--) {
    int seq = starter + randomizer().inclusive(1, 129);
    ork_set += seq;
    chevy.add_pair(ork, seq);
  }

  int i;
  for (i = 0; i < eep_set.elements(); i++) {
    int seq = eep_set[i];
    if (!chevy.have_seen(eep, seq)) {
      log(a_sprintf("missing sequence is %d", seq));
      log(chevy.text_form(true));
      deadly_error("test_sequence_tracker", "eep check", "missing sequence");
    }
  }
  for (i = 0; i < op_set.elements(); i++) {
    int seq = op_set[i];
    if (!chevy.have_seen(op, seq)) {
      log(a_sprintf("missing sequence is %d", seq));
      log(chevy.text_form(true));
      deadly_error("test_sequence_tracker", "op check", "missing sequence");
    }
  }
  for (i = 0; i < ork_set.elements(); i++) {
    int seq = ork_set[i];
    if (!chevy.have_seen(ork, seq)) {
      log(a_sprintf("missing sequence is %d", seq));
      log(chevy.text_form(true));
      deadly_error("test_sequence_tracker", "ork check", "missing sequence");
    }
  }

  return final_report();
}

HOOPLE_MAIN(test_sequence_tracker, )

