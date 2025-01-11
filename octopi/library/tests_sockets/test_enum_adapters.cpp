/*
*  Name   : test_enumerate_adapters                                           *
*  Author : Chris Koeritz                                                     *
*  Purpose: Makes sure that the adapter enumerator function is working properly.
**
* Copyright (c) 2003-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/functions.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <sockets/tcpip_stack.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_array.h>
#include <unit_test/unit_base.h>

#include <stdio.h>

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

class test_enum_adapaters : public virtual unit_base, virtual public application_shell
{
public:
  test_enum_adapaters() {}
  DEFINE_CLASS_NAME("test_enum_adapaters");
  virtual int execute();
};

int test_enum_adapaters::execute()
{
  FUNCDEF("execute");
  tcpip_stack stack;

  string_array ips;
  bool did_it = stack.enumerate_adapters(ips);
  if (!did_it)
    continuable_error(class_name(), func, "could not enumerate adapters");

  for (int i = 0; i < ips.length(); i++) {
    log(a_sprintf("%d: ", i+1) + ips[i]);
  }

  return final_report();
}

HOOPLE_MAIN(test_enum_adapaters, )

