/*
*  Name   : test_chaos
*  Author : Chris Koeritz
**
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

//#define DEBUG_CHAOS

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <loggers/console_logger.h>
#include <mathematics/chaos.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace mathematics;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#define MAX_RANDOM_BINS 40
#define MAX_TEST_CYCLES 10008
#define AVG_EXPECTED_PER_BIN (double(MAX_TEST_CYCLES) / double(MAX_RANDOM_BINS))
#define VARIATION_ALLOWED (AVG_EXPECTED_PER_BIN * 0.1)
#define ANOMALIES_ALLOWED (MAX_RANDOM_BINS / 4)

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger::get(), astring(to_print))

class test_chaos : virtual public unit_base, virtual public application_shell
{
public:
  test_chaos() : application_shell() {}
  DEFINE_CLASS_NAME("test_chaos");
  virtual int execute();
};

int test_chaos::execute()
{
  FUNCDEF("execute");
#ifdef DEBUG_CHAOS
  LOG(a_sprintf("average expected=%f, variation allowed=%f",
      AVG_EXPECTED_PER_BIN, VARIATION_ALLOWED));
#endif
  int results[MAX_RANDOM_BINS];
  for (int k = 0; k < MAX_RANDOM_BINS; k++) results[k] = 0;
  chaos randomizer;

  for (int i = 0; i < MAX_TEST_CYCLES; i++) {
    // first test if exclusivity is ensured...
    int res = randomizer.exclusive(0, MAX_RANDOM_BINS - 1);
    ASSERT_FALSE( (res <= 0) || (res >= MAX_RANDOM_BINS - 1),
        "exclusive test should not go out of bounds");
    // then test for our statistics.
    int base = randomizer.inclusive(-1000, 1000);
      // pick a base for the number below.
    res = randomizer.inclusive(base, base + MAX_RANDOM_BINS - 1);
    ASSERT_FALSE( (res < base) || (res > base + MAX_RANDOM_BINS - 1),
        "inclusive test should not go out of bounds");
//LOG(a_sprintf("adding it to %d bin", res - base));
    results[res - base]++;
  }
#ifdef DEBUG_CHAOS
  LOG("Anomalies:");
#endif
  int failed_any = false;
  for (int j = 0; j < MAX_RANDOM_BINS; j++) {
    if (absolute_value(results[j] - AVG_EXPECTED_PER_BIN) > VARIATION_ALLOWED) {
      failed_any++;
#ifdef DEBUG_CHAOS
      LOG(astring(astring::SPRINTF, "%d: difference=%f",
          j, double(results[j] - AVG_EXPECTED_PER_BIN)));
#endif
    }
  }
#ifdef DEBUG_CHAOS
  if (!failed_any) LOG("None")
  else LOG(a_sprintf("Saw %d anomalies of %d allowed.", failed_any, ANOMALIES_ALLOWED));
#endif

  ASSERT_FALSE(failed_any > ANOMALIES_ALLOWED,
      "probability anomalies should be less than the allowed number");
  return final_report();
}

HOOPLE_MAIN(test_chaos, )

