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
#include <configuration/application_configuration.h>
#include <loggers/critical_events.h>
#include <loggers/logging_macros.h>
#include <loggers/program_wide_logger.h>
#include <filesystem/filename.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_array.h>
#include <textual/parser_bits.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace configuration;
using namespace mathematics;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

class test_filename : virtual public unit_base, public virtual application_shell
{
public:
  test_filename() : application_shell() {}
  DEFINE_CLASS_NAME("test_filename");
  virtual int execute();
  void clean_sequel(astring &sequel);
  astring virtual_root();
  void dump_string_array(const astring &title, const string_array &to_dump);
  bool verify_equal_string_array(const astring &group, const string_array &exemplar, const string_array &acolyte);
  bool prepare_string_arrays_for_filenames(const astring &common_bit, const astring &group,
      bool &exemplar_rooted, string_array &exemplar_pieces, bool &acolyte_rooted,
      string_array &acolyte_pieces);
};

void test_filename::clean_sequel(astring &sequel)
{ sequel.replace_all('\\', '/'); }

astring test_filename::virtual_root()
{
  astring virt_root = application_configuration::virtual_unix_root();
  if (!!virt_root && !filename::separator(virt_root[virt_root.length() - 1])) {
    // this is not terminated with a slash, which is possible for dosdows.
    // we'll make it a reliable directory component by adding a slash.
    virt_root += astring("/");
  }
  return virt_root;
}

void test_filename::dump_string_array(const astring &title, const string_array &to_dump)
{
  FUNCDEF("dump_string_array");
  LOG(title);
  for (int i = 0; i < to_dump.length(); i++) {
    LOG(a_sprintf("%d: '", i) + to_dump[i] + "'");
  }
}

/*
  due to some difference in behavior between the platforms, we need to turn
  rooted paths that work perfectly find on unix systems into a bizarre messed
  up c drive version for windows (based on the virtual unix system in place,
  although only cygwin is currently supported).  this assumes the virtual root
  is available...  we accomplish our testing a platform invariant way by by
  simulating the same operations filename does, but using our exemplar paths
  as the starting point.
*/

bool test_filename::verify_equal_string_array(const astring &group, const string_array &exemplar, const string_array &acolyte)
{
  FUNCDEF("verify_equal_string_array");

//temp debug
dump_string_array("exemplar", exemplar);
dump_string_array("acolyte", acolyte);

  // doing some extra assertions in here, to complain about what went wrong, but then we still need to return a success value.
  ASSERT_EQUAL(exemplar.length(), acolyte.length(), group + "the list was the wrong length");
  if (exemplar.length() != acolyte.length()) { return false; }

  for (int indy = 0; indy < exemplar.length(); indy++) {
    bool success = acolyte[indy].equal_to(exemplar[indy]);
    ASSERT_TRUE(success, group + a_sprintf("piece %d did not match: ", indy) + "'" 
        + acolyte[indy] + "' vs expected '" + exemplar[indy] + "'");
    if (!success) { return false; }  // fail fast.
  }
  return true;
}

/*
  helper method constructs string arrays for the filename with common_bit as
  the portion after the root ('/').  the exemplar array is generated
  independently from the acolyte string array to ensure that it is correctly
  constructed (with a virtual root and then a non-rooted chunk).
*/
bool test_filename::prepare_string_arrays_for_filenames(const astring &common_bit, const astring &group,
    bool &exemplar_rooted, string_array &exemplar_pieces,
    bool &acolyte_rooted, string_array &acolyte_pieces)
{
  FUNCDEF("prepare_string_arrays_for_filenames")
  bool to_return = true;  // success until we learn otherwise.

  // generate the acolyte, which will be tested again, very straightforwardly.
  // it is a non-rooted string, so we just slap the virtual root in front.
  filename acolyte_fn(virtual_root() + common_bit);
  acolyte_fn.separate(acolyte_rooted, acolyte_pieces);

  // generate the exemplar without allowing filename to operate on the whole
  // string.  we get the virtual root first and operate on it as a filename,
  // then we slap on the unrooted portion to get the unmanipulated form.
  filename(virtual_root()).separate(exemplar_rooted, exemplar_pieces);
  {
    string_array common_pieces;
    bool common_rooted;
    filename(common_bit).separate(common_rooted, common_pieces);
    ASSERT_FALSE(common_rooted, group + "the common_rooted value is erreonous");
    if (common_rooted) { to_return = false; }
    // conjoin the rooty pieces with the common bits, hopefully hitting both platforms' sweet spots.
    exemplar_pieces += common_pieces;
  }

  return to_return;
}

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

    astring GROUP = "testing separate() ";
    astring common_bit = "omega/ralph/turkey/buzzard.txt";
    string_array turkey_pieces;
    bool turkey_rooted;
    string_array exemplar_pieces;
    bool exemplar_rooted;
    bool worked = test_filename::prepare_string_arrays_for_filenames(common_bit, GROUP,
        exemplar_rooted, exemplar_pieces, turkey_rooted, turkey_pieces);

    ASSERT_EQUAL(turkey_rooted, exemplar_rooted, GROUP + "the turkey_rooted value is erreonous.");
    ASSERT_TRUE(verify_equal_string_array(GROUP, exemplar_pieces, turkey_pieces), "the turkey array differs from exemplar");
  }

  {
    // third test group.
    astring GROUP = "third: test compare_prefix ";
    filename turkey(virtual_root() + "omega/ralph/turkey/buzzard.txt");
    filename murpin1(virtual_root() + "omega");
    filename murpin2(virtual_root() + "omega/ralph");
    filename murpin3(virtual_root() + "omega/ralph/turkey");
    filename murpin4(virtual_root() + "omega/ralph/turkey/buzzard.txt");
    filename murpin_x1("ralph/turkey/buzzard.txt");
    filename murpin_x2(virtual_root() + "omega/ralph/turkey/buzzard.txt2");
    filename murpin_x3(virtual_root() + "omega/turkey/buzzard.txt");
    filename murpin_x4(virtual_root() + "omega/ralph/turkey/b0/buzzard.txt");
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
    filename turkey(virtual_root() + "omega/ralph/turkey/buzzard.txt");
    filename murpin1("turkey\\buzzard.txt");
    filename murpin2("turkey/buzzard.txt");
    filename murpin3("ralph/turkey/buzzard.txt");
    filename murpin4("omega/ralph/turkey/buzzard.txt");
    filename murpin5(virtual_root() + "omega/ralph/turkey/buzzard.txt");

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
    filename test3(virtual_root() + "flug/blumen/klemper/smooden");
    ASSERT_EQUAL(test3.basename(), astring("smooden"), GROUP + "basename 1 failed");
    ASSERT_EQUAL(test3.dirname(), filename(virtual_root() + "flug/blumen/klemper"),
        GROUP + "u-dirname 1 failed");
    filename test4 = test3;
    popped = test4.pop();
    ASSERT_EQUAL(popped, astring("smooden"), GROUP + "upop 1 return failed");
    ASSERT_EQUAL(test4, filename(virtual_root() + "flug/blumen/klemper"), GROUP + "upop 1 failed");
    test4.pop();
    test4.pop();
    ASSERT_EQUAL(test4, filename(virtual_root() + "flug"), GROUP + "upop 2 failed");
    popped = test4.pop();
    ASSERT_EQUAL(popped, astring("flug"), GROUP + "upop 2 return failed");
    ASSERT_EQUAL(test4, filename(virtual_root()), GROUP + "upop 3 failed");
    test4.pop();
    filename special_popped = filename(virtual_root());
    special_popped.pop();
    ASSERT_EQUAL(test4, special_popped, GROUP + "upop 4 failed");
    test4 = filename(virtual_root());
    test4.push("flug");
    test4.push("blumen");
    test4.push("klemper");
    ASSERT_EQUAL(test4, filename(virtual_root() + "flug/blumen/klemper"), GROUP + "upush 1 failed");
  }
  {
    // seventh test group.
    astring GROUP = "seventh: testing pack and unpack ";
    filename test1(virtual_root() + "usr/local/athabasca");
    byte_array packed;
    int size_guess = test1.packed_size();
    test1.pack(packed);
    ASSERT_EQUAL(size_guess, packed.length(), GROUP + "packed_size 1 failed");
    filename test2;
    ASSERT_TRUE(test2.unpack(packed), GROUP + "unpack 1 failed");
    ASSERT_EQUAL(test2, test1, GROUP + "packed contents differ, 1 failed");
  }
#ifdef __WIN32__
  {
    // eighth test group is only for windows side.
    astring GROUP = "eighth: cygwin and msys paths ";
    filename test1("/cygdrive/q/marbles");
    ASSERT_EQUAL(test1, astring("q:/marbles"), GROUP + "test 1 failed");
    filename test2("/cygdrive/r");
    ASSERT_EQUAL(test2, astring("r:/"), GROUP + "test 2 failed");
    filename test3("/cygdrive/r/");
    ASSERT_EQUAL(test3, astring("r:/"), GROUP + "test 3 failed");
    // this is a broken pattern, which we don't expect to resolve to a drive.
    filename test4("/cygdrive//");
    ASSERT_EQUAL(test4, virtual_root() + "cygdrive", GROUP + "test 4 failed");
    // another broken pattern.
    filename test5("/cygdrive/");
    ASSERT_EQUAL(test5, virtual_root() + "cygdrive", GROUP + "test 5 failed");
    // and one more.  not great tests, but whatever.
    filename test6("/cygdrive");
    ASSERT_EQUAL(test6, virtual_root() + "cygdrive", GROUP + "test 6 failed");
    filename test7(virtual_root() + "klaunspendle");
    ASSERT_EQUAL(test7, astring(virtual_root() + "klaunspendle"), GROUP + "test 7 failed");
    filename test8("z:/klaunspendle");
    ASSERT_EQUAL(test8, astring("z:/klaunspendle"), GROUP + "test 8 failed");

    filename test10("/q/borkage");
    ASSERT_EQUAL(test10, astring("q:/borkage"), GROUP + "test 10 failed");
    filename test11("/q/r");
    ASSERT_EQUAL(test11, astring("q:/r"), GROUP + "test 11 failed");
    filename test12("/q/r/");
    ASSERT_EQUAL(test12, astring("q:/r"), GROUP + "test 12 failed");
    filename test13("/q/r/x");
    ASSERT_EQUAL(test13, astring("q:/r/x"), GROUP + "test 13 failed");
    filename test14("/r/");
    ASSERT_EQUAL(test14, astring("r:/"), GROUP + "test 14 failed");
    filename test15("/r");
    ASSERT_EQUAL(test15, astring("r:/"), GROUP + "test 15 failed");

    bool ex_rooted, ac_rooted;
    string_array exemplar, acolyte;
    ASSERT_TRUE(prepare_string_arrays_for_filenames(astring(""), GROUP,
        ex_rooted, exemplar, ac_rooted, acolyte), GROUP + "test 16 failed prep");
    ASSERT_TRUE(verify_equal_string_array(GROUP, exemplar, acolyte), GROUP + "test 16 failed compare");

    filename test17("r/");
    ASSERT_EQUAL(test17, astring("r/"), GROUP + "test 17 failed");
    filename test18(virtual_root() + "kr/soop");
    ASSERT_EQUAL(test18, astring(virtual_root() + "kr/soop"), GROUP + "test 18 failed");
  }
#endif

  return final_report();
}

HOOPLE_MAIN(test_filename, )

