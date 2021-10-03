/*****************************************************************************\
*                                                                             *
*  Name   : t_node                                                            *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Tests out the node base class.                                           *
*                                                                             *
*******************************************************************************
* Copyright (c) 1989-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

//hmmm: make this a more aggressive and realistic test.  try implementing
//      some list algorithms or graph algorithms to push node around.

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <loggers/console_logger.h>
#include <nodes/node.h>
#include <structures/static_memory_gremlin.h>
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

class test_node : public virtual unit_base, public virtual application_shell
{
public:
  test_node() {}
  DEFINE_CLASS_NAME("test_node");
  virtual int execute();
};

void bogon(byte_array *fred)
{
  if (fred) LOG("eep")
  else LOG("eek");
}

int test_node::execute()
{
  FUNCDEF("execute");

  byte_array blank;
  basket<byte_array> fred(2, blank);
  basket<byte_array> george(2, blank);
  basket<byte_array> f_end1(0);
  basket<byte_array> f_end2(0);
  basket<byte_array> g_end1(0);
  basket<byte_array> g_end2(0);

  node root;

  // add some links to the linkless root.
  root.insert_link(0, &fred);
  root.insert_link(1, &george);

  // set the pre-existing links to our end points.
  fred.set_link(0, &f_end1);
  fred.set_link(1, &f_end2);
  george.set_link(0, &g_end1);
  george.set_link(1, &g_end2);

//do some testing on those results!!!

  return final_report();
}

HOOPLE_MAIN(test_node, );

