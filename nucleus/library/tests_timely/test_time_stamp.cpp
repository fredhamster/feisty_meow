/*****************************************************************************\
*                                                                             *
*  Name   : test_time_stamp                                                   *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1998-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/application_shell.h>
#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/guards.h>
#include <loggers/program_wide_logger.h>
#include <structures/static_memory_gremlin.h>
#include <timely/time_control.h>
#include <timely/time_stamp.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace loggers;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

//////////////

//#define DEBUG_TIME_STAMP

//const int RUN_DURATION = 5 * MINUTE_ms;
const int RUN_DURATION = 3 * SECOND_ms;
  // how long the test will run.

const int SLEEP_DURATION = 42;
  // the period of sleep between checks on the time stamp.

const int REPORT_INTERVAL = 500;  // every half second.
  // how often we show the current time stamp.

//////////////

class time_stamp_tester : virtual public unit_base, virtual public application_shell
{
public:
  time_stamp_tester() : application_shell() {}

  DEFINE_CLASS_NAME("time_stamp_tester");

  int execute() {
    time_stamp start;
    log(a_sprintf("initial time stamp: %0.0f", start.value()));

    time_stamp curr;
    time_stamp get_out(RUN_DURATION);
    time_stamp report_time(REPORT_INTERVAL);
    int iterations = RUN_DURATION / SLEEP_DURATION;
    while (iterations-- > 0) {
      if (time_stamp() < curr)
        deadly_error(class_name(), "compare test", "failure; now is less "
            "than earlier!");

//hmmm: how about some more verifications, like the sleep time we asked for is reflected (or nearly) in the time stamp difference?

      time_control::sleep_ms(SLEEP_DURATION);
      curr = time_stamp();
      if (time_stamp() >= report_time) {
        log(a_sprintf("current time stamp: %0.0f", time_stamp().value()));
        report_time.reset(REPORT_INTERVAL);
      }
    }

    time_stamp end;
    log(a_sprintf(" ending time stamp: %0.0f", end.value()));

    critical_events::alert_message("time_stamp:: works for those functions tested.");
    return 0;
  }
};

//////////////

HOOPLE_MAIN(time_stamp_tester, )

