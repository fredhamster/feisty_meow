/*
*  Name   : test_system_preconditions
*  Author : Chris Koeritz
**
* Copyright (c) 1993-$now By Author.  This program is free software; you can  
* redistribute it and/or modify it under the terms of the GNU General Public 
* License as published by the Free Software Foundation; either version 2 of 
* the License or (at your option) any later version.  This is online at:
*     http://www.fsf.org/copyleft/gpl.html
* Please send any updates to: fred@gruntose.com
*/

#include "checkup.h"

#include <application/hoople_main.h>
#include <application/windoze_helper.h>
#include <basis/astring.h>
#include <basis/enhance_cpp.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <configuration/application_configuration.h>
#include <loggers/critical_events.h>
#include <loggers/console_logger.h>
#include <structures/version_record.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

#include <stdio.h>

using namespace application;
using namespace basis;
using namespace configuration;
using namespace loggers;
using namespace system_checkup;
using namespace structures;
using namespace unit_test;

class test_system_preconditions : virtual public unit_base, virtual public application_shell
{
public:
  test_system_preconditions() : application_shell() {}
  DEFINE_CLASS_NAME("test_system_preconditions");
  virtual int execute();
};

//////////////

#undef UNIT_BASE_THIS_OBJECT
#define UNIT_BASE_THIS_OBJECT c_parent

class burpee
{
public:
  burpee(unit_base &parent) : c_parent(parent), my_string(new astring) { *my_string = "balrog"; }
  DEFINE_CLASS_NAME("burpee");
  virtual ~burpee() {
    FUNCDEF("destructor");
    WHACK(my_string);
    ASSERT_FALSE(my_string, "whack test should not fail to clear string");
  }

protected:
  unit_base &c_parent;

private:
  astring *my_string;
};

#undef UNIT_BASE_THIS_OBJECT 

//////////////

#define UNIT_BASE_THIS_OBJECT c_parent

class florba : public burpee
{
public:
  florba(unit_base &parent) : burpee(parent), second_string(new astring)
      { *second_string = "loquacious"; }
  DEFINE_CLASS_NAME("florba");
  virtual ~florba() {
    FUNCDEF("destructor");
    WHACK(second_string); 
    ASSERT_FALSE(second_string, "whack test should clear string in derived class");
  }

private:
  astring *second_string;
};

#undef UNIT_BASE_THIS_OBJECT 

//////////////

// back to default now.
#define UNIT_BASE_THIS_OBJECT (*this)

struct testing_file_struct : public FILE {};

// NOTE: an important part of this test program is running it under something
// like boundschecker to ensure that there are no memory leaks caused by
// invoking WHACK.  apparently diab 3 is unable to implement WHACK correctly.

int test_system_preconditions::execute()
{
  FUNCDEF("execute")
  // let's see what this system is called.
  log(astring("The name of this software system is: ")
      + application_configuration::software_product_name());
  ASSERT_TRUE(strlen(application_configuration::software_product_name()),
      "product should not be blank");

  // and what this program is called.
  log(astring("The application is called: ") + application_configuration::application_name());
  ASSERT_TRUE(application_configuration::application_name().length(),
      "application name should not be blank");

  // testing compiler's ansi c++ compliance.
  for (int q = 0; q < 198; q++) {
    int treno = q;
    int malfoy = treno * 3;
//    log(a_sprintf("%d", malfoy));
  }
  // this should not be an error.  the scope of q should be within the loop and
  // not outside of it.
  int q = 24;
  ASSERT_FALSE(q > 190, "no weirdness should happen with compiler scoping");

  // test that the WHACK function operates properly.
  burpee *chunko = new burpee(*this);
  florba *lorkas = new florba(*this);
  burpee *alias = lorkas;

  WHACK(chunko);
  WHACK(alias);
  ASSERT_FALSE(chunko, "chunko whack test should succeed");
  ASSERT_FALSE(alias, "aliased lorkas whack test should succeed");
  ASSERT_TRUE(lorkas, "original lorkas should not have been cleared");
  lorkas = NIL;

  ASSERT_EQUAL((int)sizeof(testing_file_struct), (int)sizeof(FILE),
      "struct size test, sizeof testing_file_struct and sizeof FILE should not differ");

  // now do the crucial tests on the OS, platform, compiler, etc.
  ASSERT_TRUE(check_system_characteristics(*this),
      "required system characteristics should be found");

#ifdef __WIN32__
  known_operating_systems os = determine_OS();
  if (os == WIN_95)
    printf("This is windows 95.\n");
  else if (os == WIN_NT)
    printf("This is windows NT.\n");
  else if (os == WIN_2K)
    printf("This is windows 2000.\n");
  else if (os == WIN_XP)
    printf("This is windows XP.\n");
  else 
    printf("This OS is unknown.\n");
#endif

  version os_ver = application_configuration::get_OS_version();
  printf("OS version: %s\n", os_ver.text_form().s());

  return final_report();
}

HOOPLE_MAIN(test_system_preconditions, )

