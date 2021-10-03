/*****************************************************************************\
*                                                                             *
*  Name   : test_symbol_tree                                                  *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Creates a symbol_tree and performs some operations on it to assure       *
*  basic functionality.                                                       *
*                                                                             *
*******************************************************************************
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <loggers/program_wide_logger.h>
#include <mathematics/chaos.h>
#include <nodes/symbol_tree.h>
#include <structures/static_memory_gremlin.h>
#include <textual/string_manipulation.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace mathematics;
using namespace nodes;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger().get(), astring(to_print))

#define DEBUG_SYMBOL_TREE

// how many nodes we add to the tree.
//const int MAX_NODES_TESTED = 40000;
//hmmm: TEMPORARY!!!
const int MAX_NODES_TESTED = 2;

class test_symbol_tree : public unit_base, public application_shell
{
public:
  test_symbol_tree() : unit_base() {}
  DEFINE_CLASS_NAME("test_symbol_tree");
  int execute();
};

int test_symbol_tree::execute()
{
  FUNCDEF("execute");

  try {
    // creates a crazy tree with only one branch per node, but hugely deep.
    symbol_tree *t = new symbol_tree("blork");
    symbol_tree *curr = t;
    for (int i = 0; i < MAX_NODES_TESTED; i++) {
      // if the current node has any branches, we'll jump on one as the next
      // place.
      if (curr->branches()) {
        // move to a random branch.
        int which = randomizer().inclusive(0, curr->branches() - 1);
        curr = (symbol_tree *)curr->branch(which);
      }
      astring rando = string_manipulation::make_random_name(1, 10);
      curr->add(new symbol_tree(rando));
    }
LOG("about to whack dynamic tree...");
    WHACK(t);
LOG("dynamic tree whacked.");
  } catch (...) {
    LOG("crashed during tree stuffing.");
    return 1;
  }

//hmmm: create a more balanced tree structure...
//      perform known operations and validate shape of tree.

  return final_report();
}

//////////////

HOOPLE_MAIN(test_symbol_tree, )

