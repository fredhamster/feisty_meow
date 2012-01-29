/*
*  Name   : test_filename
*  Author : Chris Koeritz
**
* Copyright (c) 1993-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#define DEBUG_FILENAME_TEST

#include <application/hoople_main.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <filesystem/filename.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_array.h>
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

class test_filename : virtual public unit_base, public virtual application_shell
{
public:
  test_filename() : application_shell() {}
  DEFINE_CLASS_NAME("test_filename");
  virtual int execute();
  void clean_sequel(astring &sequel);
};

void test_filename::clean_sequel(astring &sequel)
{ sequel.replace_all('\\', '/'); }

int test_filename::execute()
{
  FUNCDEF("execute")
  {
    // first test group.
    filename gorgeola("");
    ASSERT_FALSE(gorgeola.exists(), "an empty filename should not exist");
  }

  {
    // second test group.
    astring GROUP = "separate-- ";
    filename turkey("/omega/ralph/turkey/buzzard.txt");
    string_array pieces;
    turkey.separate(pieces);
    ASSERT_TRUE(pieces[1].equal_to("omega"), GROUP + "the first piece didn't match.");
    ASSERT_TRUE(pieces[2].equal_to("ralph"), GROUP + "the second piece didn't match.");
    ASSERT_TRUE(pieces[3].equal_to("turkey"), GROUP + "the third piece didn't match.");
    ASSERT_TRUE(pieces[4].equal_to("buzzard.txt"), GROUP + "the fourth piece didn't match.");
    ASSERT_EQUAL(pieces.length(), 5, GROUP + "the list was the wrong length");
  }

  {
    // third test group.
    astring GROUP = "third: test compare_prefix ";
    filename turkey("/omega/ralph/turkey/buzzard.txt");
    filename murpin1("/omega");
    filename murpin2("/omega/ralph");
    filename murpin3("/omega/ralph/turkey");
    filename murpin4("/omega/ralph/turkey/buzzard.txt");
    filename murpin_x1("ralph/turkey/buzzard.txt");
    filename murpin_x2("/omega/ralph/turkey/buzzard.txt2");
    filename murpin_x3("/omega/turkey/buzzard.txt");
    filename murpin_x4("/omega/ralph/turkey/b0/buzzard.txt");
    filename murpin_x5("moomega/ralph/turkey");

    astring sequel;
    ASSERT_TRUE(murpin1.compare_prefix(turkey, sequel), GROUP + "first should match but didn't");
    clean_sequel(sequel);
    ASSERT_TRUE(sequel.equal_to("ralph/turkey/buzzard.txt"), GROUP + "first sequel was wrong");
    ASSERT_TRUE(murpin2.compare_prefix(turkey, sequel), GROUP + "second should match but didn't");
    clean_sequel(sequel);
    ASSERT_TRUE(sequel.equal_to("turkey/buzzard.txt"), GROUP + "second sequel was wrong");
    ASSERT_TRUE(murpin3.compare_prefix(turkey, sequel), GROUP + "third should match but didn't");
    clean_sequel(sequel);
    ASSERT_TRUE(sequel.equal_to("buzzard.txt"), GROUP + "third sequel was wrong");
    ASSERT_TRUE(murpin4.compare_prefix(turkey, sequel), GROUP + "fourth should match but didn't");
    ASSERT_FALSE(sequel.t(), GROUP + "fourth had a sequel but shouldn't");

    ASSERT_FALSE(murpin_x1.compare_prefix(turkey, sequel),
        GROUP + "x-first should not match but did");
    ASSERT_FALSE(sequel.t(),
        GROUP + "x-first had a sequel but shouldn't");
    ASSERT_FALSE(murpin_x2.compare_prefix(turkey, sequel),
        GROUP + "x-second should not match but did");
    ASSERT_FALSE(sequel.t(),
        GROUP + "x-second had a sequel but shouldn't");
    ASSERT_FALSE(murpin_x3.compare_prefix(turkey, sequel),
        GROUP + "x-third should not match but did");
    ASSERT_FALSE(sequel.t(),
        GROUP + "x-third had a sequel but shouldn't");
    ASSERT_FALSE(murpin_x4.compare_prefix(turkey, sequel),
        GROUP + "x-fourth should not match but did");
    ASSERT_FALSE(sequel.t(),
        GROUP + "x-fourth had a sequel but shouldn't");
    ASSERT_FALSE(murpin_x5.compare_prefix(turkey, sequel),
        GROUP + "x-fifth should not match but did");
    ASSERT_FALSE(sequel.t(),
        GROUP + "x-fifth had a sequel but shouldn't");

    // check that the functions returning no sequel are still correct.
    ASSERT_TRUE(murpin1.compare_prefix(turkey), GROUP + "the two versions differed!");
    ASSERT_FALSE(murpin_x1.compare_prefix(turkey), GROUP + "x-the two versions differed!");
  }

  {
    // fourth test group.
    astring GROUP = "fourth: test compare_suffix ";
    filename turkey("/omega/ralph/turkey/buzzard.txt");
    filename murpin1("turkey\\buzzard.txt");
    filename murpin2("turkey/buzzard.txt");
    filename murpin3("ralph/turkey/buzzard.txt");
    filename murpin4("omega/ralph/turkey/buzzard.txt");
    filename murpin5("/omega/ralph/turkey/buzzard.txt");

    ASSERT_TRUE(murpin1.compare_suffix(turkey), GROUP + "compare 1 failed");
    ASSERT_TRUE(murpin2.compare_suffix(turkey), GROUP + "compare 2 failed");
    ASSERT_TRUE(murpin3.compare_suffix(turkey), GROUP + "compare 3 failed");
    ASSERT_TRUE(murpin4.compare_suffix(turkey), GROUP + "compare 4 failed");
    ASSERT_TRUE(murpin5.compare_suffix(turkey), GROUP + "compare 5 failed");

    ASSERT_FALSE(turkey.compare_suffix(murpin1), GROUP + "compare x.1 failed");
  }

  {
    // fifth test group.
    // tests out the canonicalization method on any parameters given on
    // the command line, including the program name.
    astring GROUP = "fifth: canonicalize command-line paths ";
//    log(GROUP, ALWAYS_PRINT);
    for (int i = 0; i < application::_global_argc; i++) {
      filename canony(application::_global_argv[i]);
//      log(a_sprintf("parm %d:\n\tfrom \"%s\"\n\t  to \"%s\"", i, application::_global_argv[i],
//           canony.raw().s()), ALWAYS_PRINT);

//hmmm: the above wasn't really a test so much as a look at what we did.
//      we should run the canonicalizer against a set of known paths so we can know what to
//      expect.

    }
  }

  {
    // sixth test group.
    astring GROUP = "sixth: testing pop and push ";
    // test dossy paths.
    filename test1("c:/flug/blumen/klemper/smooden");
//log(astring("base=") + test1.basename(), ALWAYS_PRINT);
    ASSERT_EQUAL(test1.basename(), astring("smooden"), GROUP + "basename 1 failed");
//log(astring("got past basename 1 test that was failing."));
    ASSERT_EQUAL(test1.dirname(), filename("c:/flug/blumen/klemper"),
        GROUP + "d-dirname 1 failed");
//log(astring("got past a test or so after that."));
    filename test2 = test1;
    astring popped = test2.pop();
    ASSERT_EQUAL(popped, astring("smooden"), GROUP + "dpop 1 return failed");
    ASSERT_EQUAL(test2, filename("c:/flug/blumen/klemper"), GROUP + "dpop 1 failed");
    test2.pop();
    test2.pop();
    ASSERT_EQUAL(test2, filename("c:/flug"), GROUP + "dpop 2 failed");
    popped = test2.pop();
    ASSERT_EQUAL(popped, astring("flug"), GROUP + "dpop 1 return failed");
    ASSERT_EQUAL(test2, filename("c:/"), GROUP + "dpop 3 failed");
    test2.pop();
    ASSERT_EQUAL(test2, filename("c:/"), GROUP + "dpop 3 failed");
    test2.push("flug");
    test2.push("blumen");
    test2.push("klemper");
    ASSERT_EQUAL(test2, filename("c:/flug/blumen/klemper"), GROUP + "dpush 1 failed");
    // test unix paths.
    filename test3("/flug/blumen/klemper/smooden");
    ASSERT_EQUAL(test3.basename(), astring("smooden"), GROUP + "basename 1 failed");
    ASSERT_EQUAL(test3.dirname(), filename("/flug/blumen/klemper"),
        GROUP + "u-dirname 1 failed");
    filename test4 = test3;
    popped = test4.pop();
    ASSERT_EQUAL(popped, astring("smooden"), GROUP + "upop 1 return failed");
    ASSERT_EQUAL(test4, filename("/flug/blumen/klemper"), GROUP + "upop 1 failed");
    test4.pop();
    test4.pop();
    ASSERT_EQUAL(test4, filename("/flug"), GROUP + "upop 2 failed");
    popped = test4.pop();
    ASSERT_EQUAL(popped, astring("flug"), GROUP + "upop 1 return failed");
    ASSERT_EQUAL(test4, filename("/"), GROUP + "upop 3 failed");
    test4.pop();
    ASSERT_EQUAL(test4, filename("/"), GROUP + "upop 3 failed");
    test4.push("flug");
    test4.push("blumen");
    test4.push("klemper");
    ASSERT_EQUAL(test4, filename("/flug/blumen/klemper"), GROUP + "upush 1 failed");
  }
  {
    // seventh test group.
    astring GROUP = "seventh: testing pack and unpack ";
    filename test1("/usr/local/athabasca");
    byte_array packed;
    int size_guess = test1.packed_size();
    test1.pack(packed);
    ASSERT_EQUAL(size_guess, packed.length(), GROUP + "packed_size 1 failed");
    filename test2;
    ASSERT_TRUE(test2.unpack(packed), GROUP + "unpack 1 failed");
    ASSERT_EQUAL(test2, test1, GROUP + "packed contents differ, 1 failed");
  }

  return final_report();
}

HOOPLE_MAIN(test_filename, )

