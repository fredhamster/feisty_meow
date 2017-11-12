/*
*  Name   : test_doubly_linked_list
*  Author : Chris Koeritz
**
* Copyright (c) 1993-$now By Author.  This program is free software; you can
* redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either version 2 of
* the License or (at your option) any later version.  This is online at:
*     http://www.fsf.org/copyleft/gpl.html
* Please send any updates to: fred@gruntose.com
*/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/guards.h>
#include <configuration/application_configuration.h>
#include <loggers/program_wide_logger.h>
#include <mathematics/chaos.h>
#include <nodes/node.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>
#include <nodes/doubly_linked_list.h>

using namespace application;
using namespace basis;
using namespace configuration;
using namespace loggers;
using namespace mathematics;
using namespace nodes;
using namespace structures;
using namespace unit_test;

//#define DEBUG_LIST
  // uncomment this line to get more debugging output.

const int DEFAULT_ITERATIONS = 50;
  // the default number of times we run through our phase loop.

typedef basket<int> t_node;
  // the object we store in the list, a templated integer.

#define CASTER(bare_node) static_cast<const t_node *>(bare_node)
  // turns a node pointer into our special t_node.

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger::get(), to_print)

//////////////

class test_doubly_linked_list : virtual public unit_base, virtual public application_shell
{
public:
  test_doubly_linked_list() : unit_base() {}
  DEFINE_CLASS_NAME("test_list");
  virtual int execute();
};

HOOPLE_MAIN(test_doubly_linked_list, );

//////////////

int test_doubly_linked_list::execute()
{
  FUNCDEF("execute");

  doubly_linked_list the_list;
  chaos randomizer;

  int iterations_left = DEFAULT_ITERATIONS;
  while (iterations_left-- > 0) {

    // run through the phases below as many times as we are told.

    {
      // phase for adding a random number into the list.
      int to_add = randomizer.inclusive(0, 100000);

      // seek the correct insertion place to keep the list ordered.
      doubly_linked_list::iterator iter = the_list.head();
      while (!iter.is_tail() && iter.observe()
          && (CASTER(iter.observe())->stored() <= to_add) )
        iter++;
      the_list.insert(iter, new t_node(2, to_add));
    }

    {
      // test the list invariant (which is that all elements should be sorted
      // in non-decreasing order).
      doubly_linked_list::iterator iter = the_list.tail();
      // initialize our comparator.
      int bigger = CASTER(iter.observe())->stored();
      // loop backwards until we hit the head.
      while (!iter.is_head()) {
        // check that the last value is not less than the current value.
        ASSERT_FALSE(bigger < CASTER(iter.observe())->stored(),
            "invariant check should not find a mal-ordering in the list");
        bigger = CASTER(iter.observe())->stored();
        iter--;
      }
    }

    {
      // if the conditions are favorable, we whack at least one element out of
      // the list.
      if (randomizer.inclusive(1, 100) < 20) {
        int elem = the_list.elements();
        int to_whack = randomizer.inclusive(0, elem - 1);
        
        // start at the head of the list...
        doubly_linked_list::iterator iter = the_list.head();
        // and jump to the element we chose.
        the_list.forward(iter, to_whack);
        ASSERT_EQUAL(the_list.index(iter), to_whack,
            "forward should not see logic error where index of element to zap is incorrect");
        ASSERT_FALSE(iter.is_tail(),
            "forward should not see logic error where we get to the tail somehow");
        the_list.zap(iter);
      }
    }

  }

#ifdef DEBUG_LIST
  doubly_linked_list::iterator iter = the_list.head();
  log(astring(""));
  log(astring("list contents:"));
  int indy = 0;
  while (!iter.is_tail()) {
    int item = CASTER(iter.observe())->stored();
    log(a_sprintf("item #%d: %d", indy, item));
    indy++;
    iter++;
  }
#endif

  return final_report();
}


