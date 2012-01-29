//////////////
// Name   : Simple Application Example
// Author : Chris Koeritz
//////////////
// Copyright (c) 2006-$now By Author.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation:
//     http://www.gnu.org/licenses/gpl.html
// or under the terms of the GNU Library license:
//     http://www.gnu.org/licenses/lgpl.html
// at your preference.  Those licenses describe your legal rights to this
// software, and no other rights or warranties apply.
// Please send updates for this code to: fred@gruntose.com -- Thanks, fred.
//////////////

//! An example of a bare-bones hoople application.
/*!
  This application provides a very simple example for how we create programs
  based on the HOOPLE code.
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

const int CHECKING_INTERVAL = 4 * SECOND_ms;
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

  bool already_running();
    //!< true if this program is already running.

  virtual void handle_timer();
    //!< called by timer events from anchor window.

  virtual void handle_startup();
    //!< begins our service's activity.

  virtual void handle_shutdown();
    //!< catches the graceful shutdown so our objects get closed normally.

  virtual int execute();
    //!< the root of the program's activity.

  int print_instructions();
    //!< describes the available command line options for this program.

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

int application_example::print_instructions()
{
  FUNCDEF("print_instructions");
  LOG("no instructions at this time.");
  return 1;
}

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

bool application_example::already_running()
{ return _app_lock.already_running(); }

int application_example::execute()
{
  FUNCDEF("execute");
  command_line cmds(_global_argc, _global_argv);

//hmmm: test for command line options that are supported.

  // make sure this app is not running already.
  if (already_running()) {
    return 0;
  }

//need anchor window online for this.
//  anchor_window::launch(*this, GET_INSTANCE_HANDLE(), class_name(), CHECKING_INTERVAL);

  ASSERT_EQUAL(astring(class_name()), astring("application_example"),
      "simple application name check");

  return final_report();
}

//////////////

HOOPLE_MAIN(application_example, )

