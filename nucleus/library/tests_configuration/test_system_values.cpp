/*****************************************************************************\
*                                                                             *
*  Name   : test_system_values                                                *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/astring.h>

#include <application/application_shell.h>
#include <application/hoople_main.h>
#include <loggers/console_logger.h>
#include <structures/static_memory_gremlin.h>
#include <configuration/system_values.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace configuration;
using namespace loggers;
using namespace textual;
using namespace unit_test;

#define LOG(s) EMERGENCY_LOG(program_wide_logger::get(), astring(s))

class test_system_values : virtual public unit_base, virtual public application_shell
{
public:
  test_system_values()
      : application_shell(),
        events(system_values::EVENT_VALUES()),
        filters(system_values::FILTER_VALUES()),
        outcomes(system_values::OUTCOME_VALUES())
  {}

  DEFINE_CLASS_NAME("test_system_values");
  virtual int execute();

private:
  system_values events;
  system_values filters;
  system_values outcomes;
};

int test_system_values::execute()
{
  FUNCDEF("execute");

  LOG("Outcome Values");
  LOG("==============");
  LOG(outcomes.text_form());

  LOG("Filter Values");
  LOG("=============");
  LOG(filters.text_form());

  LOG("Event Values");
  LOG("============");
  LOG(events.text_form());

  critical_events::alert_message(astring(class_name()) + ": works for those functions tested.");

  return 0;
}

HOOPLE_MAIN(test_system_values, )

