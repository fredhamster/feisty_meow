/*****************************************************************************\
*                                                                             *
*  Name   : test_boilerplate                                                  *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Puts an object through its pacess--this is intended to provide the basic *
*  framework for a unit test using the hoople testing framework.              *
*                                                                             *
*******************************************************************************
* Copyright (c) 2011-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/hoople_main.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <loggers/combo_logger.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

#include <memory.h>
#include <stdlib.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace unit_test;

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger::get(), to_print)

//////////////

class test_boilerplate : virtual public unit_base, virtual public application_shell
{
public:
  test_boilerplate() : unit_base() {}
  DEFINE_CLASS_NAME("test_boilerplate");
  virtual int execute();
};

HOOPLE_MAIN(test_boilerplate, );

//////////////

int test_boilerplate::execute()
{
  FUNCDEF("execute");
//do some testing
  ASSERT_TRUE(true, "true is somehow not true?");
  return final_report();
}

