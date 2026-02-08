/*****************************************************************************\
*                                                                             *
*  Name   : test_ini_parser                                                   *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <structures/string_table.h>
#include <application/application_shell.h>
#include <filesystem/byte_filer.h>
#include <loggers/console_logger.h>
#include <configuration/ini_parser.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace configuration;
//using namespace mathematics;
//using namespace filesystem;
using namespace loggers;
//using namespace structures;
//using namespace textual;
//using namespace timely;
using namespace unit_test;

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

const astring INI_FILE_1 = "\
[bork]\n\
norple=1\n\
train=12.5\n\
singhy=9   \r\n\
\n\
[twerf]\r\n\
noodles=fungus\n\
dora=34\n\
";

//#define READ_FILE_TEST
  // uncomment to read in a file and parse it.

class test_ini_parser : virtual public unit_base, virtual public application_shell
{
public:
  test_ini_parser() : application_shell(class_name()) {}
  DEFINE_CLASS_NAME("test_ini_parser");
  virtual int execute();
};

int test_ini_parser::execute()
{
  program_wide_logger::get().eol(log_base::NO_ENDING);

  ini_parser par(INI_FILE_1);

//astring dump;
//par.restate(dump);
//LOG(astring("table has:\n") + dump);

  string_table twerf;
  if (!par.get_section("twerf", twerf))
    deadly_error(class_name(), "get_section 1", "twerf section was not found");
//LOG(astring("twerf section is: ") + twerf.text_form());
  if (!twerf.find("noodles"))
    deadly_error(class_name(), "get_section 1", "item #1 was not found");
  if (*twerf.find("noodles") != "fungus")
    deadly_error(class_name(), "get_section 1", "item #1 found is incorrect");
  if (!twerf.find("dora"))
    deadly_error(class_name(), "get_section 1", "item #2 was not found");
  if (*twerf.find("dora") != "34")
    deadly_error(class_name(), "get_section 1", "item #2 found is incorrect");

  string_table bork;
  if (!par.get_section("bork", bork))
    deadly_error(class_name(), "get_section 2", "bork section was not found");
  if (!bork.find("norple"))
    deadly_error(class_name(), "get_section 2", "item #1 was not found");
  if (*bork.find("norple") != "1")
    deadly_error(class_name(), "get_section 2", "item #1 found is incorrect");
  if (!bork.find("train"))
    deadly_error(class_name(), "get_section 2", "item #2 was not found");
  if (*bork.find("train") != "12.5")
    deadly_error(class_name(), "get_section 2", "item #2 found is incorrect");
  if (!bork.find("singhy"))
    deadly_error(class_name(), "get_section 2", "item #3 was not found");
  if (*bork.find("singhy") != "9")
    deadly_error(class_name(), "get_section 2", "item #3 found is incorrect");

  astring new_ini;
  par.restate(new_ini);

  program_wide_logger::get().eol(log_base::CRLF_AT_END);
  LOG("");

#ifdef READ_FILE_TEST
  byte_filer input("c:/home/fungal.lld", "rb");
  int len = input.length();
  LOG(a_sprintf("fungal len is %d", len));
  astring jojo;
  input.read(jojo, len);
  //LOG("whole file is:");
  //LOG(jojo);

  ini_parser klug(jojo);
  astring dump2;
  klug.restate(dump2);
  LOG(dump2);
#endif

  critical_events::alert_message("ini_parser:: works for those functions tested.\n");
  return 0;
}

HOOPLE_MAIN(test_ini_parser, )

