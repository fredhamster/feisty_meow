/*
 * Name   : time_set_effective_id
 * Author : Chris Koeritz
 * Purpose:
 *
 *   A simple test of how long it takes to call set effective id on Unix.
 *   For this to really work as designed, it should be run as root.
 *
 * Copyright (c) 2003-$now By Author.  This program is free software; you can 
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either version 2 of
 * the License or (at your option) any later version.  This is online at:
 *     http://www.fsf.org/copyleft/gpl.html
 * Please send any updates to: fred@gruntose.com
 */

#include <basis/functions.h>
#include <basis/astring.h>
#include <structures/set.h>
#include <timely/time_stamp.h>
#include <application/hoople_main.h>
#include <loggers/console_logger.h>
#include <loggers/file_logger.h>
#include <structures/static_memory_gremlin.h>

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

class time_set_effective_id : public application_shell
{
public:
  time_set_effective_id() : application_shell() {}
  DEFINE_CLASS_NAME("time_set_effective_id");
  int execute();
};

int time_set_effective_id::execute()
{
  FUNCDEF("execute");
  SETUP_COMBO_LOGGER;

  int test_runs = 1000000;

  time_stamp start;  // start of test.
  for (int i = 0; i < test_runs; i++) {
    // set effective id to fred.
    int ret = seteuid(1008);
    if (ret != 0) {
      LOG("failure to change effective user id to normal user.");
      exit(1);
    }
    // set effective id to root.
    ret = seteuid(0);
    if (ret != 0) {
      LOG("failure to change effective user id to root.");
      exit(1);
    }
  }
  time_stamp completion_time;  // end of test.

  double durat = completion_time.value() - start.value();
  double secs = durat / 1000.0;   // convert to seconds.
  LOG(a_sprintf("test run took %0.2f milliseconds or %0.2f seconds or %0.2f minutes.", durat, secs, secs / 60.0));
  // divide by two because we're doing two calls above.
  LOG(a_sprintf("individual call takes %0.0f milliseconds.", durat / double(test_runs) / 2.0));

  return 0;
}

HOOPLE_MAIN(time_set_effective_id, )

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
#endif // __BUILD_STATIC_APPLICATION__

