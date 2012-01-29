/*
*  Name   : test_xml_generator
*  Author : Chris Koeritz
*  Purpose: Checks out whether the XML writer seems to be functional.
**
* Copyright (c) 2007-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
*/

#include <application/hoople_main.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <loggers/program_wide_logger.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_table.h>
#include <textual/xml_generator.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
//using namespace filesystem;
using namespace loggers;
using namespace mathematics;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#define LOG(s) EMERGENCY_LOG(program_wide_logger::get(), s)

class test_xml_generator : public virtual unit_base, virtual public application_shell
{
public:
  test_xml_generator() {}
  DEFINE_CLASS_NAME("test_xml_generator");
  int execute();
};

#define OPERATE_XML(func, args, test_name) { \
  outcome ret = ted.func args; \
  ASSERT_EQUAL(ret.value(), xml_generator::OKAY, \
      astring(test_name) + astring(": failed to ") + #func); \
}

int test_xml_generator::execute()
{
  FUNCDEF("execute");
  xml_generator ted;
  #define TEST "boilerplate"

  string_table attribs;
  attribs.add("bluebird", "petunia chowder");
  OPERATE_XML(add_header, ("glommage", attribs), TEST);

  OPERATE_XML(open_tag, ("Recipe"), TEST);

  OPERATE_XML(open_tag, ("Name"), TEST);
  OPERATE_XML(add_content, ("Lime Jello Marshmallow Cottage Cheese Surprise"),
      TEST);
  OPERATE_XML(close_tag, ("Name"), TEST);
  
  OPERATE_XML(open_tag, ("Description"), TEST);
  OPERATE_XML(add_content, ("My grandma's favorite (may she rest in peace)."),
      TEST);
  OPERATE_XML(close_tag, ("Description"), TEST);

  #undef TEST
  #define TEST "stirring ingredients"
  OPERATE_XML(open_tag, ("Ingredients"), TEST);

  //////////////

  OPERATE_XML(open_tag, ("Ingredient"), TEST);

  attribs.reset();
  attribs.add("unit", "box");
  OPERATE_XML(open_tag, ("Qty", attribs), TEST);
  OPERATE_XML(add_content, ("1"), TEST);
  OPERATE_XML(close_tag, ("Qty"), TEST);

  OPERATE_XML(open_tag, ("Item"), TEST);
  OPERATE_XML(add_content, ("lime gelatin"), TEST);
  OPERATE_XML(close_tag, ("Item"), TEST);

  OPERATE_XML(close_tag, ("Ingredient"), TEST);

  //////////////

  OPERATE_XML(open_tag, ("Ingredient"), TEST);

  attribs.reset();
  attribs.add("unit", "g");
  OPERATE_XML(open_tag, ("Qty", attribs), TEST);
  OPERATE_XML(add_content, ("500"), TEST);
  OPERATE_XML(close_tag, ("Qty"), TEST);

  OPERATE_XML(open_tag, ("Item"), TEST);
  OPERATE_XML(add_content, ("multicolored tiny marshmallows"), TEST);
  OPERATE_XML(close_tag, ("Item"), TEST);

  OPERATE_XML(close_tag, ("Ingredient"), TEST);

  //////////////

  #undef TEST
  #define TEST "closing the bowl"

  OPERATE_XML(close_tag, ("Ingredients"), TEST);

  astring generated = ted.generate();
  LOG(astring("XML generated is as follows:"));
  LOG(generated);

  return final_report();
}

HOOPLE_MAIN(test_xml_generator, )

