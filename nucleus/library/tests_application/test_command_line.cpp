/*****************************************************************************\
*                                                                             *
*  Name   : test_command_line                                                 *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Tests the command_line class by showing our parameters.                  *
*                                                                             *
*******************************************************************************
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/astring.h>

#include <application/application_shell.h>
#include <application/command_line.h>
#include <application/hoople_main.h>
#include <loggers/console_logger.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <filesystem/filename.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_array.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
//using namespace configuration;
//using namespace mathematics;
using namespace filesystem;
using namespace loggers;
using namespace structures;
//using namespace textual;
//using namespace timely;
using namespace unit_test;

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

class test_command_line : virtual public unit_base, virtual public application_shell
{
public:
  test_command_line() : application_shell() {}
  DEFINE_CLASS_NAME("test_command_line");
  int execute();
};

int test_command_line::execute()
{
  FUNCDEF("execute");

  LOG("these are the commands we got passed...");
  string_array cmds = command_line::get_command_line();
  for (int i = 0; i < cmds.length(); i++) {
    LOG(a_sprintf("%02d: cmd=%s", i, cmds[i].s()));
  }

  // test 1 is a simple probe on the actual arguments to this program.
  {
    command_line cl1(application::_global_argc, application::_global_argv);
    filename prog(cl1.program_name());
    log(astring("got a program name of: [") + prog.dirname() + astring("] ")
        + prog.basename());
    log(astring("got parms of:"));
    for (int i = 0; i < cl1.entries(); i++) {
      command_parameter got = cl1.get(i);
      log(astring(astring::SPRINTF, "%d: type=%s text=%s", i,
          (got.type()==command_parameter::VALUE)? "VALUE"
              : (got.type()==command_parameter::CHAR_FLAG)? "CHAR_FLAG"
              : (got.type()==command_parameter::STRING_FLAG)? "STRING_FLAG"
              : "UNKNOWN",
          got.text().s()));
    }
  }

  // test 2 ensures that our new special flag ending support (standard unix
  // flag of --) is performing.
  {
    astring cmd_line = "tossedout spumeco -g -q --fleem -r -- spugnats.txt crumbole.h";
    command_line cl2(cmd_line);

    command_parameter spumeco = cl2.get(0);
//log(a_sprintf("parm 0: type=%d text=%s", spumeco.type(), spumeco.text().s()));
    if (spumeco.type() != command_parameter::VALUE)
      non_continuable_error(class_name(), "test 2", "spumeco is wrong type");
    if (spumeco.text() != astring("spumeco"))
      non_continuable_error(class_name(), "test 2", "spumeco is erroneous");

    command_parameter gflag = cl2.get(1);
    if (gflag.type() != command_parameter::CHAR_FLAG)
      non_continuable_error(class_name(), "test 2", "G flag had wrong type");
    if (gflag.text() != astring("g"))
      non_continuable_error(class_name(), "test 2", "G flag had wrong value");

    command_parameter qflag = cl2.get(2);
    if (qflag.type() != command_parameter::CHAR_FLAG)
      non_continuable_error(class_name(), "test 2", "Q flag had wrong type");
    if (qflag.text() != astring("q"))
      non_continuable_error(class_name(), "test 2", "Q flag had wrong value");

    command_parameter fleemflag = cl2.get(3);
    if (fleemflag.type() != command_parameter::STRING_FLAG)
      non_continuable_error(class_name(), "test 2", "fleem flag had wrong type");
    if (fleemflag.text() != astring("fleem"))
      non_continuable_error(class_name(), "test 2", "fleem flag had wrong value");

    command_parameter rflag = cl2.get(4);
    if (rflag.type() != command_parameter::CHAR_FLAG)
      non_continuable_error(class_name(), "test 2", "R flag had wrong type");
    if (rflag.text() != astring("r"))
      non_continuable_error(class_name(), "test 2", "R flag had wrong value");

    command_parameter spugval = cl2.get(5);
    if (spugval.type() != command_parameter::VALUE)
      non_continuable_error(class_name(), "test 2", "spugval had wrong type");
    if (spugval.text() != astring("spugnats.txt"))
      non_continuable_error(class_name(), "test 2", "spugval had wrong value");

    command_parameter crumval = cl2.get(6);
    if (crumval.type() != command_parameter::VALUE)
      non_continuable_error(class_name(), "test 2", "crumval had wrong type");
    if (crumval.text() != astring("crumbole.h"))
      non_continuable_error(class_name(), "test 2", "crumval had wrong value");

    command_parameter bogus = cl2.get(7);
    if (bogus.type() != command_parameter::BOGUS_ITEM)
      non_continuable_error(class_name(), "test 2", "bogus parameter had wrong type");
  }

//more tests!

  critical_events::alert_message("command_line:: works for those functions tested.");
  return 0;
}

HOOPLE_MAIN(test_command_line, )

