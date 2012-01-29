/*****************************************************************************\
*                                                                             *
*  Name   : application_shell                                                 *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "application_shell.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <configuration/application_configuration.h>
#include <loggers/combo_logger.h>
#include <loggers/program_wide_logger.h>
#include <timely/time_stamp.h>

#include <stdio.h>
#ifdef __UNIX__
  #include <limits.h>
  #include <unistd.h>
#endif

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

using namespace basis;
using namespace configuration;
using namespace loggers;
using namespace mathematics;
using namespace textual;
using namespace timely;

namespace application {

const int MAXIMUM_COMMAND_LINE = 32 * KILOBYTE;
  // maximum command line that we'll deal with here.

application_shell *not_so_hidden_pointer = NULL;  // fodder for the single instance function.

application_shell *application_shell::single_instance() { return not_so_hidden_pointer; }

application_shell::application_shell()
: c_rando(),
  c_exit_value(120)  // if this is never changed from the default, that in itself is an error.
{
  // we create a combo logger for program-wide usage.
  SETUP_COMBO_LOGGER;
  not_so_hidden_pointer = this;  // setup the single instance method.
}

application_shell::~application_shell()
{
  if (not_so_hidden_pointer == this) not_so_hidden_pointer = NULL;  // only scorch it when it's us.
}

outcome application_shell::log(const base_string &to_print, int filter)
{
  outcome to_return = common::OKAY;
  if (program_wide_logger::get().member(filter)) {
    astring temp_log(to_print);
    if (temp_log.length())
      temp_log.insert(0, time_stamp::notarize(true));
    to_return = program_wide_logger::get().log(temp_log, filter);
  }
  return to_return;
}

int application_shell::execute_application()
{
  try {
    c_exit_value = execute();
  } catch (const char *message) {
    printf("caught exception:\n%s\n", message);
  } catch (astring message) {
    printf("caught exception:\n%s\n", message.s());
  } catch (...) {
    printf("caught exception: unknown type!\n");
  }
  return c_exit_value;
}

} //namespace.

