/*
*  Name   : test_section_manager
*  Author : Chris Koeritz
*  Purpose: Tests that the section manager is writing sections properly and keeping its
 table of contents correctly.
**
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

//#define DEBUG_SECTION_MANAGER
  // uncomment for debugging version.

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/guards.h>
#include <configuration/ini_configurator.h>
#include <configuration/section_manager.h>
#include <structures/string_table.h>
#include <structures/string_array.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace configuration;
using namespace filesystem;
using namespace loggers;
using namespace mathematics;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger().get(), astring(to_print))

//#ifdef DEBUG_SECTION_MANAGER
//  #include <stdio.h>
//#endif

class test_section_manager : public virtual unit_base, virtual public application_shell
{
public:
  test_section_manager() {}
  DEFINE_CLASS_NAME("test_section_manager");
  virtual int execute();
};

//////////////

int test_section_manager::execute()
{
  FUNCDEF("execute");
  {
    astring TEST = "First Test";
    ini_configurator ini("t_section_manager_1.ini", ini_configurator::AUTO_STORE);
    section_manager mangler(ini, "TOC", "BORK:");
    // clean up the ini file for our testing....
    string_array names;
    if (mangler.get_section_names(names)) {
      for (int i = 0; i < names.length(); i++) mangler.zap_section(names[i]);
      ini.delete_section("TOC");  // remove table of contents.
    }

    // now add some entries...
    string_table contents1;
    contents1.add("oink", "bozoot");
    contents1.add("gink", "rinkum");
    contents1.add("sorty", "figulat");
    contents1.add("crinkish", "wazir");
    ASSERT_TRUE(mangler.add_section("burny", contents1),
        TEST + ": couldn't add the first section!");
    string_table temp_1;
    ASSERT_TRUE(mangler.find_section("burny", temp_1),
        TEST + ": couldn't retrieve the first section!");
#ifdef DEBUG_SECTION_MANAGER
    printf("first section has:\n%s\n", temp_1.text_form().s());
    printf("we want:\n%s\n", contents1.text_form().s());
#endif
    ASSERT_EQUAL(temp_1, contents1, TEST + ": first section's contents are incorrect!");
    contents1.add("glurp", "locutusburger");
    ASSERT_FALSE(mangler.add_section("burny", contents1),
        TEST + ": incorrectly allowing re-add of first section!");
    ASSERT_TRUE(mangler.replace_section("burny", contents1),
        TEST + ": failing to replace first section!");
    temp_1.reset();
    ASSERT_TRUE(mangler.find_section("burny", temp_1),
        TEST + ": couldn't retrieve the first section (2)!");
    ASSERT_EQUAL(temp_1, contents1, TEST + ": first section's contents are incorrect (2)!");

    string_table contents2;
    contents2.add("tsingha", "tsinglo");
    contents2.add("chunk", "midgets");
    contents2.add("burn", "barns in texas");
    contents2.add("chump", "will not be elected");
    contents2.add("geezerplant", "water weekly");
    ASSERT_TRUE(mangler.add_section("itchy", contents2),
        TEST + ": couldn't add the second section!");
    string_table temp_2;
    ASSERT_TRUE(mangler.find_section("itchy", temp_2),
        TEST + ": couldn't retrieve the second section!");
    ASSERT_EQUAL(temp_2, contents2, TEST + ": second section's contents are incorrect!");
    // test that first section is still there with second having been added.
    ASSERT_TRUE(mangler.find_section("burny", temp_1),
        TEST + ": couldn't retrieve the first section (3)!");
    ASSERT_EQUAL(temp_1, contents1, TEST + ": first section's contents are incorrect (3)!");

//more!
  }    
  {
//    astring TEST = "Second Test";
  }    

  return final_report();
}

//////////////

HOOPLE_MAIN(test_section_manager, );

