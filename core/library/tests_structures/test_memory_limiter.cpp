/*****************************************************************************\
*                                                                             *
*  Name   : test_memory_limiter                                               *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Tests that the memory_limiter is keeping track of the memory users       *
*  accurately.                                                                *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

//#define DEBUG_MEMORY_LIMITER
  // uncomment for debugging version.

#include <application/hoople_main.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <configuration/ini_configurator.h>
#include <mathematics/chaos.h>
#include <structures/memory_limiter.h>
#include <structures/set.h>
#include <structures/static_memory_gremlin.h>
#include <timely/time_stamp.h>
#include <unit_test/unit_base.h>

#ifdef DEBUG_MEMORY_LIMITER
  #include <stdio.h>
#endif

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace mathematics;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger().get(), astring(to_print))

const int MAXIMUM_MEM_OVERALL = 1 * MEGABYTE;
const int MAXIMUM_MEM_PER_OWNER = 100 * KILOBYTE;

const int RUN_TIME = .8 * SECOND_ms;

//////////////

class test_memory_limiter : virtual public unit_base, virtual public application_shell
{
public:
  test_memory_limiter() {}
  DEFINE_CLASS_NAME("test_memory_limiter");
  virtual int execute();
};

//////////////

struct mem_record {
  int parent;
  int allocated;

  mem_record(int parent_in = 0, int allocated_in = 0)
      : parent(parent_in), allocated(allocated_in) {}
};

struct memorial : array<mem_record> {};

//////////////

int test_memory_limiter::execute()
{
  FUNCDEF("execute");
  time_stamp when_to_leave(RUN_TIME);
  time_stamp start;
  memorial wtc;
  memory_limiter to_test(MAXIMUM_MEM_OVERALL, MAXIMUM_MEM_PER_OWNER);
  int allocations = 0;
  int deletions = 0;
  basis::un_int total_allocated = 0;
  basis::un_int total_deleted = 0;
  while (time_stamp() < when_to_leave) {
    int to_do = randomizer().inclusive(1, 100);
    if (to_do < 50) {
      // add a new record.
      int alloc = randomizer().inclusive(1, 1 * MEGABYTE);
//isolate min max alloc
      int parent = randomizer().inclusive(1, 120);
//isolate min max parents

      if (!to_test.okay_allocation(parent, alloc))
        continue;  // no space right now.
      wtc += mem_record(parent, alloc);
      allocations++;
      total_allocated += alloc;
    } else if (to_do < 88) {
      // remove an existing record.
      if (!wtc.length()) continue;  // nothing to remove.
      int indy = randomizer().inclusive(0, wtc.length() - 1);
      mem_record to_gone = wtc[indy];
      wtc.zap(indy, indy);
      ASSERT_TRUE(to_test.record_deletion(to_gone.parent, to_gone.allocated),
          "first case failed to record deletion!");
      deletions++;
      total_deleted += to_gone.allocated;
    } else {
//do something funky, like allocate part of one into another...
    }
  }

  // now clear everything left in our list.
  for (int i = 0; i < wtc.length(); i++) {
    mem_record to_gone = wtc[i];
    ASSERT_TRUE(to_test.record_deletion(to_gone.parent, to_gone.allocated),
        "second case failed to record deletion!");
    deletions++;
    total_deleted += to_gone.allocated;
  }

  // now check that the memory limiter has returned to camber.

  ASSERT_FALSE(to_test.overall_usage(), "final checks: there is still memory in use!");

  ASSERT_EQUAL(to_test.overall_space_left(), MAXIMUM_MEM_OVERALL,
      "final checks: the free space is not correct!");

  int_set remaining = to_test.individuals_listed();
  ASSERT_FALSE(remaining.elements(), "final checks: there were still uncleared individuals!");

  time_stamp end;

  LOG("stats for this run:");
  LOG(astring(astring::SPRINTF, "\trun time %f ms",
      end.value() - start.value()));
  LOG(astring(astring::SPRINTF, "\tallocations %d, total memory allocated %d",
      allocations, total_allocated));
  LOG(astring(astring::SPRINTF, "\tdeletions %d, total memory deleted %d",
      deletions, total_deleted));

  return final_report();
}

//////////////

HOOPLE_MAIN(test_memory_limiter, );

