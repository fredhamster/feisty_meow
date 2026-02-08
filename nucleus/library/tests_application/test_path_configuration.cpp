/*****************************************************************************\
*                                                                             *
*  Name   : test_application_configuration                                           *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/hoople_main.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <loggers/console_logger.h>
#include <configuration/application_configuration.h>
#include <structures/static_memory_gremlin.h>
//#include <unit_test/unit_base.h>

//using namespace application;
using namespace basis;
using namespace configuration;
//using namespace mathematics;
//using namespace filesystem;
using namespace loggers;
//using namespace structures;
//using namespace textual;
//using namespace timely;
//using namespace unit_test;

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

//hmmm: ugly old main() without using the hoople machinery.  ack.
astring static_class_name() { return "test_path_configuration"; }

HOOPLE_STARTUP_CODE;

int main(int argc, char *argv[])
{
  FUNCDEF("main")
  astring jammed;
  for (int i = 0; i < argc; i++)
    jammed += astring(argv[i]) + " ";
  LOG(astring("command line=") + jammed);

  astring app_dir = application_configuration::application_directory();
  LOG(astring("app dir is: ") + app_dir);

  critical_events::alert_message(astring(static_class_name()) + ": works for those functions tested.");
  return 0;
}

