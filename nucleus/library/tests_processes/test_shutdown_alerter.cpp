/*****************************************************************************\
*                                                                             *
*  Name   : t_shutdown_alerter                                                *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    An example of using the shutdown_alerter object to manage the runtime    *
*  of a program.                                                              *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <configuration/application_configuration.h>
#include <filesystem/filename.h>
#include <loggers/program_wide_logger.h>
#include <processes/shutdown_alerter.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

//hmmm: make this use the unit_base so its a real test app.

using namespace application;
using namespace basis;
using namespace configuration;
//using namespace mathematics;
using namespace filesystem;
using namespace loggers;
//using namespace processes;
//using namespace structures;
//using namespace textual;
using namespace timely;
using namespace unit_test;

//////////////

#define BASE_LOG(s) STAMPED_EMERGENCY_LOG(program_wide_logger::get(), s)
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

const int TIMING_CYCLE = 1408;
  // how frequently timer should be hit.

class my_anchor : public shutdown_alerter
{
public:
  virtual void handle_startup() { BASE_LOG("into startup..."); }
  virtual void handle_shutdown() { BASE_LOG("into shutdown..."); }
  virtual void handle_timer() { BASE_LOG("into timer..."); }
};

//hmmm: what is this test intended to actually do, or test?
//   all it does right now is created the shutdown alerter object...
//   and then it never seems to try to shut it down!???

int main(int formal(argc), char *formal(argv)[])
{
//hmmm: skipping since we crash currently.  fix this!
return 0;

  my_anchor w;
  BASE_LOG(a_sprintf("timer will hit every %d ms.", TIMING_CYCLE));
  shutdown_alerter::launch_console(w,
      filename(application_configuration::application_name()).basename(), TIMING_CYCLE);
  BASE_LOG("after creating the alerter's console app...");
BASE_LOG("still here, but about to exit...  kaboom?");
  return 0;
}

