//////////////
// Name   : test_stopwatch
// Author : Chris Koeritz
// Rights : Copyright (c) 1991-$now By Author
//////////////
// This file is free software; you can modify/redistribute it under the terms
// of the GNU General Public License. [ http://www.gnu.org/licenses/gpl.html ]
// Feel free to send updates to: [ fred@gruntose.com ]
//////////////

#include <application/application_shell.h>
#include <application/hoople_main.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <loggers/console_logger.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <mathematics/chaos.h>
#include <structures/static_memory_gremlin.h>
#include <timely/stopwatch.h>
#include <timely/time_control.h>

using namespace basis;
using namespace application;
using namespace loggers;
using namespace mathematics;
using namespace structures;
using namespace timely;

#define DEBUG_TIMER

// ACCEPT: acceptable timer deviation from the total time to wait.
// NOTE: timer characteristics and the sleep characteristics for machines vary.
// it may be necessary to play with ACCEPT to get this program to accept the
// machine's particular characteristics.
//#define ACCEPT 0.01
//#define WIDER_ACCEPT 0.05
//#define WIDEST_ACCEPT 0.20
#define ACCEPT 0.9
#define WIDER_ACCEPT 0.95
#define WIDEST_ACCEPT 1.20

#define LOG(s) EMERGENCY_LOG(program_wide_logger::get(), s)

class test_stopwatch : public application_shell
{
public:
  test_stopwatch() : application_shell() {}
  DEFINE_CLASS_NAME("test_stopwatch");
  virtual int execute();
};

int test_stopwatch::execute()
{
  stopwatch fred_time;
  chaos randomizer;

  int to_sleep = randomizer.inclusive(20, 100);
    // needs longer ints for the last two checks...
  fred_time.start();
  time_control::sleep_ms(to_sleep);
  fred_time.halt();
#ifdef DEBUG_TIMER
  LOG(a_sprintf("sleep of %0.3f seconds took %d milliseconds\n",
      float(to_sleep) / 1000.0, fred_time.milliseconds()));
#endif
  if (absolute_value(to_sleep - fred_time.milliseconds())
      > ACCEPT * to_sleep)
    deadly_error(class_name(), "first", "unacceptable timer deviation");
  fred_time.reset();

  to_sleep = randomizer.inclusive(76, 420);
  fred_time.start();
  time_control::sleep_ms(to_sleep);
  fred_time.halt();
#ifdef DEBUG_TIMER
  LOG(a_sprintf("sleep of %0.3f seconds took %d milliseconds\n",
      float(to_sleep) / 1000.0, fred_time.milliseconds()));
#endif
  if (absolute_value(to_sleep - fred_time.milliseconds()) > ACCEPT * to_sleep)
    deadly_error(class_name(), "second", "unacceptable timer deviation");
  fred_time.reset();

  critical_events::alert_message("stopwatch:: works for those functions tested.");
  return 0;
}

HOOPLE_MAIN(test_stopwatch, )

