//////////////
// Name   : memory_hog
// Author : Chris Koeritz
//////////////
// Copyright (c) 2012-$now By Author.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation:
//     http://www.gnu.org/licenses/gpl.html
// or under the terms of the GNU Library license:
//     http://www.gnu.org/licenses/lgpl.html
// at your preference.  Those licenses describe your legal rights to this
// software, and no other rights or warranties apply.
// Please send updates for this code to: fred@gruntose.com -- Thanks, fred.
//////////////

//! This application lives to grub up lots of memory.
/*!
  You can specify how much memory the application should consume.
  Occasionally it will riffle through its hamstered away memory to try to
  ensure that it stays in real memory rather than virtual memory.
*/

#include <application/hoople_main.h>
#include <application/command_line.h>
#include <application/singleton_application.h>
#include <basis/enhance_cpp.h>
#include <loggers/program_wide_logger.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace loggers;
//using namespace processes;
using namespace structures;
using namespace unit_test;

const int MEMORY_CHECKING_INTERVAL = 0.5 * SECOND_ms;
  // this many milliseconds elapses between checks on shutdown conditions.

#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger().get(), astring(to_print))
  // define a macro that will send diagnostic output to the app's logger.

//////////////

class application_example : virtual public unit_base, virtual public application_shell
{
public:
  application_example();
  ~application_example();

  DEFINE_CLASS_NAME("application_example");

  virtual void handle_timer();
    //!< called by timer events from anchor window.

  virtual void handle_startup();
    //!< begins our service's activity.

  virtual void handle_shutdown();
    //!< catches the graceful shutdown so our objects get closed normally.

  virtual int execute();
    //!< the root of the program's activity.

  void print_instructions();
    //!< describes the available command line options for the ULS.

private:
  singleton_application _app_lock;  //!< our inter-application synchronizer.
};

//////////////

application_example::application_example()
: application_shell(),
  _app_lock(static_class_name())
{}

application_example::~application_example()
{}

void application_example::handle_startup()
{
  FUNCDEF("handle_startup");
  LOG("starting up now.");
}

void application_example::handle_shutdown()
{
  FUNCDEF("handle_shutdown");
  LOG("shutting down now.");
}

void application_example::handle_timer()
{
  FUNCDEF("handle_timer");
  LOG("timer blip.");
}

int application_example::execute()
{
  FUNCDEF("execute");
  command_line cmds(_global_argc, _global_argv);

LOG(a_sprintf("cmd line has %d entries", cmds.entries()));


//  ASSERT_EQUAL(astring(class_name()), astring("application_example"),
//      "simple application name check");

//  return final_report();
  return 0;
}

//////////////

HOOPLE_MAIN(application_example, )

