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

#define BASE_LOG(to_print) EMERGENCY_LOG(program_wide_logger().get(), astring(to_print))
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

  int print_instructions();
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

int application_example::print_instructions()
{
  FUNCDEF("print_instructions");
  BASE_LOG("\
\nUsage: memory_hog {memorySize}\n\
\n\
  This application will consume a requested amount of memory until either\n\
(1) the applications is interrupted or terminated, or (2) the application\n\
is no longer given memory when it requests it (whether due to account limits\n\
or to a lack of remaining allocatable memory), or (3) the requested amount\n\
has been allocated.\n\
  The application will then sit back idly and occasionally riffle through its\n\
big bag of memory in an attempt to keep most of it in physical memory rather\n\
than virtual memory.\n\
  This is not a hostile act unless used unwisely; this program is intended\n\
to get an unloaded machine into a predictable memory state.\n\
  **DO NOT use this program without system administrator permission.**"
);
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

int application_example::execute()
{
  FUNCDEF("execute");
  command_line cmds(_global_argc, _global_argv);

LOG(a_sprintf("cmd line has %d entries", cmds.entries()));

  if (cmds.entries() < 1) {
    BASE_LOG("You need to provide a number on the command line....");
    return print_instructions();
  }

  astring size_as_text = cmds.get(0).text();

LOG(a_sprintf("hoo hah!  got a text thingy of: %s", size_as_text.s()));

  if (size_as_text.find('.') < 0) {
    // we don't see this as floating point, but we will move on as if it's
    // an integer and can become floating point.
    size_as_text += ".0";
  }

  double how_fat = size_as_text.convert(double(0.0));

LOG(a_sprintf("got a number from user of: %f", how_fat));

//okay, now that we've got that handled,
// we want to have a hash table of byte arrays.
//   int hash or whatever?  we want it indexable and fast access on a unique id.
// that will be our consumption tactic.
// we will allocate some chunk size in the byte arrays.  maybe max of a meg.
// then we just add as many byte arrays as we need to to get to the requested amount.
// don't just allocate them all at the same size; vary it a bunch, like say 20% of
//   our maximum allotance.
// so randomly pull memory until we get what we want.
// if we get a memory failure, slack off until we can try to get more.
// if we cannot get memory within certain number of failures, report this and bail out.
//   we do not want to crash the machine.
// once we've got our memory, start playing with it.
// every so often,
//   pick a random number of items to play with,
//     go to each item (which is a byte array),
//       pick a random segment of the byte array to look at,
//         read the contents of the memory there.  that's it, nothing stressful.
// repeat the memory play until forever.
//   enhancement, allow them to provide a run time.  time out after that elapses.

  return 0;
}

//////////////

HOOPLE_MAIN(application_example, )

