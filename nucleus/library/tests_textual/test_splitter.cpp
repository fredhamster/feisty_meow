/*
*  Name   : test_splitter
*  Author : Chris Koeritz
*  Purpose: Checks out the line splitting support to make sure it is working.
**
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
*/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <mathematics/chaos.h>
#include <structures/static_memory_gremlin.h>
#include <textual/string_manipulation.h>
#include <unit_test/unit_base.h>

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

class test_splitter : public virtual unit_base, virtual public application_shell
{
public:
  test_splitter() {}
  DEFINE_CLASS_NAME("test_splitter");
  int execute();
};

astring show_limits(int min_col, int max_col)
{
  astring to_return;
  for (int i = 0; i <= max_col; i++) {
    if (i < min_col) to_return += " ";
    else to_return += "+";
  }
  return to_return;
}

#define SHOW_SPLIT(str, low, high) \
  string_manipulation::split_lines(str, output, low, high); \
  LOG(""); formal(temp)\
  LOG(show_limits(low, high)); \
  LOG(output + "<<<<"); \
  LOG(show_limits(low, high))

int test_splitter::execute()
{
  FUNCDEF("execute");

  astring split_1 = "This is a fairly long paragraph that will be split a few different ways to check the splitting logic.  It will be interesting to see how things like spaces, and punctuation, are handled.";
  
  astring output;

  SHOW_SPLIT(split_1, 0, 1);
  SHOW_SPLIT(split_1, 0, 10);
  SHOW_SPLIT(split_1, 10, 30);
  SHOW_SPLIT(split_1, 20, 35);
  SHOW_SPLIT(split_1, 4, 50);
  SHOW_SPLIT(split_1, 8, 12);

  astring split_2 = "Here's another string.\r\nThere are embedded carriage returns after every sentence.\r\nSo, this should be a good test of how those things are handled when they're seen in the body of the text.\r\nThe next one is, hey, guess what?\r\nIt's a simple LF instead of CRLF; here it comes:\nHow did that look compared the others?\nThe previous was another bare one.\r\nWhen can I stop writing this stupid paragraph?\r\nSoon hopefully.";

  SHOW_SPLIT(split_2, 5, 20);
  SHOW_SPLIT(split_2, 0, 30);
  SHOW_SPLIT(split_2, 8, 11);
  SHOW_SPLIT(split_2, 28, 41);
  SHOW_SPLIT(split_2, 58, 79);

  astring split_3 = "This string exists for just one purpose; it will be showing how the thing handles a really long word at the end.  And that word is... califragilisticexpialadociuosberriesinatreearerottingnowsomamacasscanyoupleasehelpme";

  SHOW_SPLIT(split_3, 0, 5);
  SHOW_SPLIT(split_3, 10, 30);

  astring split_4 = "This string\n\n\nhas multiple CRs gwan on.\r\n\r\nDoes this cause problems?\n\n\n\n";

  SHOW_SPLIT(split_4, 3, 10);
  SHOW_SPLIT(split_4, 8, 20);

  return final_report();
}

HOOPLE_MAIN(test_splitter, );

