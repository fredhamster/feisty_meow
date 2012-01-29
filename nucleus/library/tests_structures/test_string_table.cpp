/*****************************************************************************\
*                                                                             *
*  Name   : test_string_table                                                 *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1994-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/byte_array.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <structures/matrix.h>
#include <structures/string_table.h>
#include <timely/time_stamp.h>
#include <loggers/console_logger.h>
#include <structures/static_memory_gremlin.h>
#include <textual/string_manipulation.h>
#include <application/hoople_main.h>
#include <unit_test/unit_base.h>

#include <stdio.h>
#include <stdlib.h>

using namespace application;
using namespace basis;
///using namespace configuration;
using namespace mathematics;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

//#define DEBUG_SYMBOL_TABLE
  // uncomment to get lots of noise out of base class.

//#define DEBUG_STRING_TABLE
  // uncomment for noisy version.

//#define TEST_SIZE_TABLE
  // uncomment for testing the size of a string_table.

//#define OLD_TEST
  // uncomment for the older version of symbol table.

const int test_iterations = 25;

const int FIND_ITERATIONS = 20;
const int MAXIMUM_RANDOM_ADDS = 50;

const int TEST_SIZE_TABLE_COUNT = 10000;
  // the number of string_table elements we create for checking how
  // big one of them is.

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger::get(), astring(to_print))

double time_in_add = 0;
double time_in_find = 0;
double time_in_pack = 0;
double time_in_unpack = 0;
basis::un_int operations = 0;

class test_string_table : public virtual application_shell, public virtual unit_base
{
public:
  test_string_table() {}
  DEFINE_CLASS_NAME("test_string_table");

  void ADD(string_table &syms, const astring &name, const astring &to_add);
  void FIND(const string_table &syms, const astring &name, const astring &to_find);

  void test_1();

  virtual int execute();
};

//////////////

void test_string_table::ADD(string_table &syms, const astring &name, const astring &to_add)
{
  FUNCDEF("ADD")
//LOG(astring("add of ") + name + " => " + to_add);
  time_stamp start;
  outcome added = syms.add(name, to_add);
  operations++;
  ASSERT_INEQUAL(added.value(), common::EXISTING, "should not already be in table");
  time_stamp end;
  time_in_add += end.value() - start.value();
}

void test_string_table::FIND(const string_table &syms, const astring &name, const astring &to_find)
{
  FUNCDEF("FIND")
  for (int i = 0; i < FIND_ITERATIONS; i++) {
    time_stamp start;
    astring *found = syms.find(name);
    operations++;
    time_stamp end;
    time_in_find += end.value() - start.value();
    ASSERT_TRUE(found, "should be in table");
    ASSERT_EQUAL(*found, to_find, "string in table should be correct");
  }
}

//////////////

void test_string_table::test_1()
{
  FUNCDEF("test_string_table");
#ifdef TEST_SIZE_TABLE
  {
    array<string_table> symso(TEST_SIZE_TABLE_COUNT);
    operations++;
    guards::alert_message(astring(astring::SPRINTF, "we should effectively "
        "have swamped out the size of other code in the program now.  the size "
        "should represent %d string_table instantiations.  take the current "
        "memory size (minus maybe 2 megs) and divide by %d and you will have "
        "a fairly accurate cost for instantiating a string table.  hit a key.",
        TEST_SIZE_TABLE_COUNT, TEST_SIZE_TABLE_COUNT));
    #ifdef _CONSOLE
    getchar();
    #elif defined(__UNIX__)
    getchar();
    #endif
  }
#endif

  string_table syms;
  string_table new_syms;
  string_table newer_syms;
  operations += 3;
  for (int qq = 0; qq < test_iterations; qq++) {
    syms.reset();  // still could be costly.
    operations++;
#ifdef DEBUG_STRING_TABLE
    LOG(astring(astring::SPRINTF, "index %d", qq));
#endif
    astring freudname("blurgh");
    astring freud("Sigmund Freud was a very freaked dude.");
    ADD(syms, freudname, freud);
    astring borgname("borg");
    astring borg("You will be assimilated.");
    ADD(syms, borgname, borg);
    astring xname("X-Men");
    astring x("The great unknown superhero cartoon.");
    ADD(syms, xname, x);
    astring aname("fleeny-brickle");
    astring a("lallax menick publum.");
    ADD(syms, aname, a);
    astring axname("ax");
    astring ax("Lizzy Borden has a very large hatchet.");
    ADD(syms, axname, ax);
    astring bloinkname("urg.");
    astring bloink("this is a short and stupid string");
    ADD(syms, bloinkname, bloink);
    astring faxname("fax");
    astring fax("alligators in my teacup.");
    ADD(syms, faxname, fax);
    astring zname("eagle ovaries");
    astring z("malfeasors beware");
    ADD(syms, zname, z);

    FIND(syms, freudname, freud);
    FIND(syms, borgname, borg);
    FIND(syms, xname, x);
    FIND(syms, aname, a);
    FIND(syms, axname, ax);
    FIND(syms, bloinkname, bloink);
    FIND(syms, faxname, fax);
    FIND(syms, zname, z);

    astring name;
    astring content;
    for (int y = 0; y < MAXIMUM_RANDOM_ADDS; y++) {
      name = string_manipulation::make_random_name(40, 108);
      content = string_manipulation::make_random_name(300, 1000);
      ADD(syms, name, content);
      FIND(syms, name, content);
    }

    // test copying of the string tables.
    string_table chronos = syms;
    operations++;
    {
      string_table mary = syms;
      operations++;
      string_table june = mary;
      operations++;
      ASSERT_TRUE(mary == syms, "copy test should compare properly");
      operations++;
    }
    ASSERT_TRUE(syms == chronos, "copy test original should not be harmed");
    operations++;

    {
      // test the bug we think we found in the operator =.
      string_table fred;
      fred.add("urp", "rarp");
      fred.add("hyurgh", "ralph");
      string_table med;
      med.add("urp", "rarp");
      med.add("hyurgh", "ralph");
      fred = med;  // the deadly assignment.
      fred = med;  // the deadly assignment.
      fred = med;  // the deadly assignment.
      fred = med;  // the deadly assignment.
      fred.add("urp", "rarp");
      fred.add("gurp", "flaarp");  // a new entry.
      astring *urp = fred.find("urp");
      astring *hyurgh = fred.find("hyurgh");
      ASSERT_TRUE(urp, "urp should not go missing");
      ASSERT_TRUE(hyurgh, "hyurgh should not go missing");
#ifdef DEBUG_STRING_TABLE
      LOG(astring("got urp as ") + (urp? *urp : "empty!!!!"));
      LOG(astring("got hyurgh as ") + (hyurgh? *hyurgh : "empty!!!!"));
#endif
      astring urp_val = fred[0];
        // AH HA!  this one finds it.  accessing via bracket or other methods
        // that use the internal get() method will fail.
        // if there is no outright crash, then the bug is gone.
#ifdef DEBUG_STRING_TABLE
      LOG(astring("got urp_val as ") + (urp_val.t()? urp_val : "empty!!!!"));
#endif
    }

#ifdef DEBUG_STRING_TABLE
////    LOG(astring(astring::SPRINTF,"This is the symbol table before any manipulation\n%s", syms.text_form()));
    LOG("now packing the symbol table...");
#endif
    byte_array packed_form;
    time_stamp start;
    syms.pack(packed_form);
    operations++;
    time_stamp end;
    time_in_pack += end.value() - start.value();
#ifdef DEBUG_STRING_TABLE
    LOG("now unpacking from packed form");
#endif
    start.reset();
    ASSERT_TRUE(new_syms.unpack(packed_form), "unpack test should succeed in unpacking");
    operations++;
    end.reset();  // click, off.
    time_in_unpack += end.value() - start.value();

#ifdef DEBUG_STRING_TABLE
///    LOG(astring(astring::SPRINTF, "unpacked form has:\n%s", new_syms->text_form().s()));
#endif
    ASSERT_FALSE(! (syms == new_syms), "unpacked test symbol tables should be same");
    operations++;

#ifdef DEBUG_STRING_TABLE
    LOG("now deleting old symbol table...");
#endif

#ifdef DEBUG_STRING_TABLE
///    LOG(astring(astring::SPRINTF, "got the unpacked form, and dumping it:\n%s", new_syms->text_form().s()));
    LOG("packing the symbol table again...");
#endif
    byte_array packed_again(0);
    start.reset();  // click, on.
    new_syms.pack(packed_again);
    operations++;
    end.reset();  // click, off.
    time_in_pack += end.value() - start.value();
#ifdef DEBUG_STRING_TABLE
    LOG("now unpacking from packed form again...");
#endif
    start = time_stamp();
    newer_syms.unpack(packed_again);
    operations++;
    end = time_stamp();
    time_in_unpack += end.value() - start.value();
#ifdef DEBUG_STRING_TABLE
    LOG(astring(astring::SPRINTF, "got the unpacked form, and dumping "
        "it:\n%s", newer_syms.text_form().s()));
#endif
    ASSERT_TRUE(new_syms == newer_syms, "unpacked test should not be different symbol tables");
    operations++;

#ifdef DEBUG_STRING_TABLE
    LOG("now deleting new and newer symbol table...");
#endif
  }
}

//////////////

int test_string_table::execute()
{
#ifdef DEBUG_STRING_TABLE
  LOG(astring("starting test: ") + time_stamp::notarize(false));
#endif
  test_1();
#ifdef DEBUG_STRING_TABLE
  LOG(astring("done test: ") + time_stamp::notarize(false));
  LOG(astring(astring::SPRINTF, "time in add=%f", time_in_add));
  LOG(astring(astring::SPRINTF, "time in find=%f", time_in_find));
  LOG(astring(astring::SPRINTF, "time in pack=%f", time_in_pack));
  LOG(astring(astring::SPRINTF, "time in unpack=%f", time_in_unpack));
  LOG(astring(astring::SPRINTF, "total operations=%u", operations));
#endif

  return final_report();
}

HOOPLE_MAIN(test_string_table, )

