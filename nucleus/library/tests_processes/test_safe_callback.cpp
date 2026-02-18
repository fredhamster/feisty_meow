/*****************************************************************************\
*                                                                             *
*  Name   : test_safe_callback                                                *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Tests the safe callback object slightly.  Mainly this is to ensure that  *
*  no memory leaks are being caused by the object and that minimal support is *
*  provided by the object.                                                    *
*                                                                             *
*******************************************************************************
* Copyright (c) 1998-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/guards.h>
#include <configuration/application_configuration.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
//#include <loggers/file_logger.h>
#include <processes/safe_callback.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace configuration;
//using namespace mathematics;
//using namespace filesystem;
using namespace loggers;
using namespace processes;
//using namespace structures;
//using namespace textual;
using namespace timely;
using namespace unit_test;

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), astring(s))

//////////////

const astring LOGFILE_NAME = application_configuration::make_logfile_name
    ("t_safe_callback.log");
  // where our debugging output goes by default.

///#define LOG(to_print) log.log(timestamp(true) + astring(to_print))
  // our macro for logging with a timestamp.

//hmmm: this tester doesn't really do very much at all.
//  how about a more aggressive test, which has a bunch of threads using
//  safe callback?

class cb_tester : public safe_callback
{
public:
  cb_tester() : safe_callback() {}
  ~cb_tester() { end_availability(); }

  virtual void real_callback(callback_data_block &) 
      { /* do nothing. */ }
};

//hmmm: move this to using a class based approach, based on unit_test and application_shell.

int main(int formal(argc), char *formal(argv)[])
{
///  file_logger log(LOGFILE_NAME);

  cb_tester testing;

  critical_events::alert_message("safe_callback:: works for all functions tested.");
  return 0;
}

