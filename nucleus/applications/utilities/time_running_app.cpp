/*
 * Name   : time_running_app
 * Author : Chris Koeritz
 * Purpose:
 *
 *   A simple test of how long it takes to run a fairly heavy application.
 *
 * Copyright (c) 2003-$now By Author.  This program is free software; you can 
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either version 2 of
 * the License or (at your option) any later version.  This is online at:
 *     http://www.fsf.org/copyleft/gpl.html
 * Please send any updates to: fred@gruntose.com
 */

#include <basis/astring.h>
#include <basis/environment.cpp>
#include <basis/functions.h>
#include <structures/set.h>
#include <timely/time_stamp.h>
#include <application/hoople_main.h>
#include <loggers/console_logger.h>
#include <loggers/file_logger.h>
#include <structures/static_memory_gremlin.h>

#include <stdio.h>

using namespace application;
using namespace basis;
using namespace loggers;
using namespace filesystem;
using namespace textual;
using namespace timely;

#undef BASE_LOG
#define BASE_LOG(to_print) EMERGENCY_LOG(program_wide_logger::get(), astring(to_print))
#undef LOG
#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger::get(), astring(to_print))

class time_running_app : public application_shell
{
public:
  time_running_app() : application_shell() {}
  DEFINE_CLASS_NAME("time_running_app");
  int execute();
};

int time_running_app::execute()
{
  FUNCDEF("execute");
  SETUP_COMBO_LOGGER;

  int test_runs = 10000;

  time_stamp start;  // start of test.
  astring bins = environment::get("FEISTY_MEOW_DIR") + "/production/binaries";
  astring app = bins + "/example_application";
  // save real stdout.
  int real_stdout = dup(1);
  // dump app's output.
  freopen("/dev/null", "w", stdout);
  for (int i = 0; i < test_runs; i++) {
    // run the app; sorry about the relative path; change to this dir first.
    int ret = system(app.s());
    if (ret != 0) {
      // restore real stdout.
      dup2(real_stdout, 1);
      LOG(astring("got an error running the app: ") + app);
      exit(1);
    }
  }
  time_stamp completion_time;  // end of test.
  // restore real stdout.
  dup2(real_stdout, 1);

  double durat = completion_time.value() - start.value();
  double secs = durat / 1000.0;   // convert to seconds.
  LOG(a_sprintf("test run took %0.2f milliseconds or %0.2f seconds or %0.2f minutes.", durat, secs, secs / 60.0));
  LOG(a_sprintf("individual call takes %0.0f milliseconds.", durat / double(test_runs)));

  return 0;
}

HOOPLE_MAIN(time_running_app, )

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
#endif // __BUILD_STATIC_APPLICATION__

