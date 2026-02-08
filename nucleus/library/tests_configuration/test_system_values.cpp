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

#include <basis/function.h>
#include <basis/guards.h>
#include <basis/istring.h>
#include <basis/portable.h>
#include <opsystem/application_shell.h>
#include <loggers/console_logger.h>
#include <data_struct/static_memory_gremlin.h>
#include <opsystem/system_values.h>

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger(), s)

class test_system_values : public application_shell
{
public:
  test_system_values()
      : application_shell(class_name()),
        events(system_values::EVENT_VALUES()),
        filters(system_values::FILTER_VALUES()),
        outcomes(system_values::OUTCOME_VALUES())
  {}

  IMPLEMENT_CLASS_NAME("test_system_values");
  virtual int execute();

private:
  system_values events;
  system_values filters;
  system_values outcomes;
};

int test_system_values::execute()
{
  FUNCDEF("execute");

  log("Outcome Values");
  log("==============");
  log(outcomes.text_form());

  log("Filter Values");
  log("=============");
  log(filters.text_form());

  log("Event Values");
  log("============");
  log(events.text_form());

  return 0;
}

HOOPLE_MAIN(test_system_values, )

