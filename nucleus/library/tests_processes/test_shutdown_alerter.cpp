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

#include <filesystem/filename.h>
#include <structures/static_memory_gremlin.h>
#include <processes/shutdown_alerter.h>

HOOPLE_STARTUP_CODE;

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

int main(int formal(argc), char *formal(argv)[])
{
  my_anchor w;
  BASE_LOG(a_sprintf("timer will hit every %d ms.", TIMING_CYCLE));
  shutdown_alerter::launch_console(w,
      filename(application_configuration::application_name()).basename(), TIMING_CYCLE);
  return 0;
}

