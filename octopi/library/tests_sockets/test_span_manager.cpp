/*****************************************************************************\
*                                                                             *
*  Name   : test_span_manager                                                 *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/hoople_main.h>
#include <basis/array.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <loggers/file_logger.h>
#include <sockets/span_manager.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
//using namespace filesystem;
using namespace loggers;
//using namespace mathematics;
using namespace sockets;
using namespace structures;
//using namespace textual;
//using namespace timely;
using namespace unit_test;

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger().get(), astring(to_print))

#define DEBUG_SPAN_MANAGER
  // uncomment for noisier output.

#define MAX_SPANS 8

// INIT_STUFF gets the macros ready for use.
#define INIT_STUFF int_array stuffer;

// STUFF puts two numbers (a span) into the next place in the storage array.
#define STUFF(a, b) { \
  ASSERT_FALSE(stuffer.length() / 2 + 1 > MAX_SPANS, "STUFF: too many for spans"); \
  stuffer.concatenate(a); stuffer.concatenate(b); \
}

// SEND updates the span manager with the current state of the storage array and resets the
// counter for the next set of spans.
#define SEND(send_to) \
  send_to.update(stuffer); stuffer.reset(0);
///  stuffer.number_of_spans = stuff_index / 2; \ old

// COMP compares the list of spans at position "i" with a value "to_compare".
#define COMP(to_compare, i) { \
  ASSERT_EQUAL(to_compare, rec_list[i], "comparison found incorrect"); \
  if (to_compare != rec_list[i]) LOG(a_sprintf("failed at index %d", i)); \
}

class test_span_manager : public virtual unit_base, public virtual application_shell
{
public:
  test_span_manager() {}
  DEFINE_CLASS_NAME("test_span_manager");
  virtual int execute();
};

int test_span_manager::execute()
{
  FUNCDEF("execute");
  span_manager fred(452);

  INIT_STUFF;

  // stuffs a bunch of spans into the storage array.
  STUFF(8, 8);
  STUFF(27, 29);
  STUFF(28, 31);
  STUFF(3, 5);
  STUFF(80, 83);
  STUFF(96, 123);
  STUFF(3, 6);
  STUFF(212, 430);
  SEND(fred);
  
  // stuffs the rest few in.
  STUFF(13, 17);
  STUFF(151, 250);
  SEND(fred);

#ifdef DEBUG_SPAN_MANAGER
  LOG(astring(astring::SPRINTF, "received sequence so far is %d",
      fred.received_sequence()));
  LOG("making received list:");
#endif

  int_array rec_list;
  fred.make_received_list(rec_list);

#ifdef DEBUG_SPAN_MANAGER
  LOG(fred.print_received_list());
#endif

  // the loop goes through the list of received spans and sees if they're
  // in the right places.
  for (int i = 0; i < rec_list.length(); i += 2) {
    switch (i) {
    case 0: COMP(3, i); COMP(6, i+1); break;
    case 2: COMP(8, i); COMP(8, i+1); break;
    case 4: COMP(13, i); COMP(17, i+1); break;
    case 6: COMP(27, i); COMP(31, i+1); break;
    case 8: COMP(80, i); COMP(83, i+1); break;
    case 10: COMP(96, i); COMP(123, i+1); break;
    case 12: COMP(151, i); COMP(430, i+1); break;
    }
  }

  ASSERT_EQUAL(fred.missing_sequence(), 0, "missing: should not have wrong sequence");

  // some more items are stuffed in, including the beginning few.
  STUFF(0, 5);
  STUFF(7, 7);
  STUFF(9, 12);
  SEND(fred);
  ASSERT_EQUAL(fred.missing_sequence(), 18, "missing 2: should not have wrong sequence");
#ifdef DEBUG_SPAN_MANAGER
  LOG("here are the missing ones now:");
  LOG(fred.print_missing_list());
#endif
  fred.make_missing_list(rec_list);
  // this loop looks into the list of missing places and makes sure they're
  // the right holes.
  for (int j = 0; j < rec_list.length(); j+=2) {
    switch (j) {
      case 0: COMP(18, j); COMP(26, j+1); break;
      case 2: COMP(32, j); COMP(79, j+1); break;
      case 4: COMP(84, j); COMP(95, j+1); break;
      case 6: COMP(124, j); COMP(150, j+1); break;
      case 8: COMP(431, j); COMP(451, j+1); break;
    }
  }

  STUFF(0, 451);
  SEND(fred);
#ifdef DEBUG_SPAN_MANAGER
  LOG("should be filled out now:");
  LOG(fred.print_received_list());
#endif
  ASSERT_EQUAL(fred.received_sequence(), 451, "received sequence should be filled out");

  return final_report();
}

HOOPLE_MAIN(test_span_manager, );

