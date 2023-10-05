/*****************************************************************************\
*                                                                             *
*  Name   : test_string                                                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

//#define DEBUG_STRING
  // set this to enable debugging features of the string class.

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/enhance_cpp.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <loggers/critical_events.h>
#include <loggers/file_logger.h>
#include <mathematics/chaos.h>
#include <structures/object_packers.h>
#include <structures/string_array.h>
#include <structures/static_memory_gremlin.h>
#include <textual/byte_formatter.h>
#include <textual/string_convert.h>
#include <textual/string_manipulation.h>
#include <timely/earth_time.h>
#include <timely/time_stamp.h>
#include <unit_test/unit_base.h>

//#ifdef _MSC_VER
//  #include <comdef.h>
//#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace application;
using namespace basis;
using namespace loggers;
using namespace mathematics;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

//HOOPLE_STARTUP_CODE;

//#define DEBUG_STRING_TEST
  // uncomment for testing version.

const float TEST_RUNTIME_DEFAULT = .02 * MINUTE_ms;
  // the test, by default, will run for this long.

//////////////

class test_string : public application_shell, public unit_base
{
public:
  test_string() {}
  ~test_string() {}

  DEFINE_CLASS_NAME("test_string");

  virtual int execute();

  void run_test_01();
  void run_test_02();
  void run_test_03();
  void run_test_04();
  void run_test_05();
  void run_test_06();
  void run_test_07();
  void run_test_08();
  void run_test_09();
  void run_test_10();
  void run_test_11();
  void run_test_12();
  void run_test_13();
  void run_test_14();
  void run_test_15();
  void run_test_16();
  void run_test_17();
  void run_test_18();
  void run_test_19();
  void run_test_20();
  void run_test_21();
  void run_test_22();
  void run_test_23();
  void run_test_24();
  void run_test_25();
  void run_test_26();
  void run_test_27();
  void run_test_28();
  void run_test_29();
  void run_test_30();
  void run_test_31();
  void run_test_32();
  void run_test_33();
  void run_test_34();
  void run_test_35();
  void run_test_36();
  void run_test_37();
  void run_test_38();
  void run_test_39();
  void run_test_40();
  void run_test_41();
  void run_test_42();
};

//////////////

chaos rando;

#define LOG(s) EMERGENCY_LOG(program_wide_logger::get(), s)

#define WHERE __WHERE__.s()

// test: reports an error if the condition evaluates to non-zero.
#define test(expr) { \
  ASSERT_FALSE(expr, astring("operator test should work: ") + #expr); \
}

static basis::astring staticity_test("yo!");
  // used below to check whether static strings are looking right.

//////////////

void test_string::run_test_01()
{
  FUNCDEF("run_test_01");

//  const int TEST_EMPTY = 10000000;
//  time_stamp started;
//  for (int i = 0; i < TEST_EMPTY; i++) {
//    astring glob = astring::empty_string();
//  }
//  int duration = int(time_stamp().value() - started.value());
//  LOG(a_sprintf("duration of empty string test=%d ms", duration));

  // test simple string operations, like construction, length, equality.
  astring fred1("hyeargh!");
  astring fred2("matey.");
  astring fred3;
  fred3 = fred1;
  fred3 += fred2;
  astring fred4(fred2);

  ASSERT_EQUAL(fred1.length(), int(strlen(fred1.s())), "length should be correct (a).");
  ASSERT_EQUAL(fred2.length(), int(strlen(fred2.s())), "length should be correct (b).");
  ASSERT_EQUAL(fred3.length(), int(strlen(fred3.s())), "length should be correct (c).");
  ASSERT_EQUAL(fred4.length(), int(strlen(fred4.s())), "length should be correct (d).");

#ifdef DEBUG_STRING_TEST
  LOG("[ " + fred1 + " & " + fred2 + "] -> " + fred3);
#endif

  ASSERT_EQUAL(fred1, astring("hyeargh!"), "failure in comparison (a).");
  ASSERT_EQUAL(fred2, astring("matey."), "failure in comparison (b).");
  ASSERT_EQUAL(fred3, astring("hyeargh!matey."), "failure in comparison (c).");
  ASSERT_EQUAL(fred4, astring("matey."), "failure in comparison (d-1).");
  ASSERT_EQUAL(fred4, fred2, "failure in comparison (d-2).");

  a_sprintf nullo;
  ASSERT_EQUAL(nullo, astring(), "forward blank a_sprintf isn't blank.");
  ASSERT_EQUAL(astring(), nullo, "backward blank a_sprintf isn't blank.");
  ASSERT_EQUAL(nullo, astring::empty_string(), "forward blank a_sprintf isn't empty.");
  ASSERT_EQUAL(astring::empty_string(), nullo, "backward blank a_sprintf isn't empty.");
}

void test_string::run_test_02()
{
  FUNCDEF("run_test_02");
  // assorted tests involving strings as pointers.
  astring *fred1 = new astring("flipper ate");
  astring *fred2 = new astring(" my sandwich.");
  astring *fred3 = new astring;
  *fred3 = *fred1;
  *fred3 += *fred2;

  // testing adding a null to a string.
  *fred2 += (char *)NULL_POINTER;
  *fred3 += (char *)NULL_POINTER;

#ifdef DEBUG_STRING_TEST
  LOG(astring("[ ") + *fred1 + " & " + *fred2 + "] -> " + *fred3);
#endif

  ASSERT_EQUAL(*fred1, astring("flipper ate"), "flipper A failure in comparison");
  ASSERT_EQUAL(*fred2, astring(" my sandwich."), "sandwich A failure in comparison");
  ASSERT_EQUAL(*fred3, astring("flipper ate my sandwich."), "full f-s A failure in comparison");
  delete fred1;
  delete fred2;
  delete fred3;
}

void test_string::run_test_03()
{
  FUNCDEF("run_test_03");
  // tests some things about zap.
  astring fleermipe("hello my frobious.");
  fleermipe.zap(0, fleermipe.length() - 1);
  ASSERT_EQUAL(fleermipe.length(), 0, "length not 0 after deleting entire astring");
}

void test_string::run_test_04()
{
  FUNCDEF("run_test_04");
  astring test_string("this test string will be chopped up.");
#ifdef DEBUG_STRING_TEST
  LOG(astring("original is: ") + test_string);
#endif
  astring fred(test_string.s());
  fred.zap(0, fred.find('w'));

#ifdef DEBUG_STRING_TEST
  LOG(astring("now, the one chopped through 'w' is: ") + fred);
#endif

  ASSERT_EQUAL(fred, astring("ill be chopped up."), "first zap failed");

  astring blorg(test_string);
  blorg.zap(blorg.find('p'), blorg.length() - 1);
#ifdef DEBUG_STRING_TEST
  LOG(astring("now the one chopped from p to the end: ") + blorg);
#endif

  ASSERT_EQUAL(blorg, astring("this test string will be cho"), "second zap failed");

  astring fleen;
  fleen += test_string;
  fleen.zap(7, 14);
#ifdef DEBUG_STRING_TEST
  LOG(astring("now the one with 7 through 14 missing: ") + fleen);
#endif

  ASSERT_EQUAL(fleen, astring("this teg will be chopped up."), "third zap failed");

#ifdef DEBUG_STRING_TEST
  LOG(astring("original astring is now: ") + test_string);
#endif
  ASSERT_EQUAL(test_string, astring("this test string will be chopped up."),
      "original astring was changed");
}

void test_string::run_test_05()
{
  FUNCDEF("run_test_05");
#ifdef DEBUG_STRING_TEST
  LOG("about to test weird things:");
#endif
  astring frieda("glorp");
  astring jorb(frieda);
  astring *kleeg = new astring(jorb.s());
  astring plok = frieda;
  test(frieda != jorb);
  test(jorb != *kleeg);
  test(*kleeg != plok);
  test(plok != frieda);
  astring glorp("glorp");
  test(frieda != glorp);

  WHACK(kleeg);

#ifdef DEBUG_STRING_TEST
  LOG("strings matched up okay.");
#endif

  // test new features sprintf is relying upon.
  astring bubba("gumpternations");
  bubba += "_02193";
#ifdef DEBUG_STRING_TEST
  LOG(astring("bubba = ") + bubba);
#endif
  ASSERT_EQUAL(bubba, astring("gumpternations_02193"), "+= on char pointer failed.");

  astring bubelah("laksos");
  bubelah += '3';
  bubelah += '8';
  bubelah += '2';
#ifdef DEBUG_STRING_TEST
  LOG(astring("bubelah = ") + bubelah);
#endif
  ASSERT_EQUAL(bubelah, astring("laksos382"), "+= on char failed.");

  astring simple_spr0(astring::SPRINTF, "%% yoga splorch %%");
#ifdef DEBUG_STRING_TEST
  LOG(astring("simple sprintf 0 = ") + simple_spr0);
#endif
  ASSERT_EQUAL(simple_spr0, astring("% yoga splorch %"), "simple sprintf 0 is wrong");
  astring simple_spr1(astring::SPRINTF, "%d", 23);
#ifdef DEBUG_STRING_TEST
  LOG(astring("simple sprintf 1 = ") + simple_spr1);
#endif
  ASSERT_EQUAL(simple_spr1, astring("23"), "simple sprintf 1 is wrong");
  astring simple_spr2(astring::SPRINTF, "%s", "yoyo");
#ifdef DEBUG_STRING_TEST
  LOG(astring("simple sprintf 2 = ") + simple_spr2);
#endif
  ASSERT_EQUAL(simple_spr2, astring("yoyo"), "simple sprintf 2 is wrong");

  astring sprintest(astring::SPRINTF, "%s has got me up the %s some %d "
     "times, in %p with %d and %lu.", "marge", "ladder", 32, &kleeg,
     812377487L, 213123123L);
  astring sprintest2;
  sprintest2.reset(astring::SPRINTF, "%s has got me up the %s some %d "
     "times, in %p with %d and %lu.", "marge", "ladder", 32, &kleeg,
     812377487L, 213123123L);
  astring addr(astring::SPRINTF, "%p", &kleeg);
#ifdef DEBUG_STRING_TEST
  LOG("here is your astring sir...");
  LOG(sprintest);
  LOG("and addr we will see is...");
  LOG(addr);
#endif
  if (sprintest != astring(astring::SPRINTF, "marge has got me up the "
      "ladder some 32 times, in %s with 812377487 and 213123123.", addr.s()))
    "constructed astring is wrong";
  if (sprintest2 != astring(astring::SPRINTF, "marge has got me up the "
      "ladder some 32 times, in %s with 812377487 and 213123123.", addr.s()))
    "reset astring is wrong";
}

void test_string::run_test_06()
{
  FUNCDEF("run_test_06");
  astring bungee;
  bungee += "this astring";
  bungee += " has been constructed gradua";
  bungee += 'l';
  bungee += 'l';
  bungee += 'y';
  astring blorpun(astring::SPRINTF, " out of severa%c", 'l');
  bungee += blorpun;
  bungee += " different bits,\nincluding";
  astring freeple(astring::SPRINTF, "%s constructed %s blarg from %f ", " this", "silly", 3.14159265358);
  bungee += freeple;
  bungee += "radians awa";
  bungee += 'y';
  bungee += '.';
  bungee += "\nhow does it look?\n";
#ifdef DEBUG_STRING_TEST
  LOG(bungee);
#endif

//MessageBox(0, bungee.s(), "yo, got this", MB_ICONINFORMATION | MB_OK);
  ASSERT_EQUAL(bungee, astring("this astring has been constructed gradually out of several different bits,\nincluding this constructed silly blarg from 3.141593 radians away.\nhow does it look?\n"), "constructed astring is wrong");
}

void test_string::run_test_07()
{
  FUNCDEF("run_test_07");
  astring a("axes");
  astring b("bakesales");
  astring x("xylophone");

  test(a >= x);
  test(a == b);
  test(a > b);
  test(b >= x);
  test(x <= a);
  test(x != x);
  test(a != a);
#ifdef DEBUG_STRING_TEST
  LOG("comparisons worked");
#endif
}

void test_string::run_test_08()
{
  FUNCDEF("run_test_08");
#ifdef DEBUG_STRING_TEST
  LOG("now testing + operator");
#endif
  astring a("fred");
  astring b(" is");
  astring c(" his");
  astring d(" name.");
  astring e;
  e = a + b + c + d;
#ifdef DEBUG_STRING_TEST
  LOG(astring("result is: ") + e);
#endif
  astring f;
  f = d + c + b + a;
#ifdef DEBUG_STRING_TEST
  LOG(astring("reverse is: ") + f);
#endif
  astring g;
  g = a + c + d + b;
#ifdef DEBUG_STRING_TEST
  LOG(astring("tibetan style is: ") + g);
#endif
  ASSERT_EQUAL(e, astring("fred is his name."), "astring looks wrong");
  ASSERT_EQUAL(f, astring(" name. his isfred"), "astring looks wrong");
  ASSERT_EQUAL(g, astring("fred his name. is"), "astring looks wrong");
}

void test_string::run_test_09()
{
  FUNCDEF("run_test_09");
  astring bleer(astring::SPRINTF, "urghla burgla #%d\n", 23);
  char holder[50];
  for (int i = 0; i < bleer.length(); i++) {
    bleer.stuff(holder, i);
#ifdef DEBUG_STRING_TEST
    LOG(astring(astring::SPRINTF, "%d", i) + " has: " + holder);
#endif
    astring my_copy(bleer);
    my_copy.zap(i, bleer.length() - 1);
    ASSERT_EQUAL(my_copy, astring(holder), "should see no inaccurate stuff() call");
  }
}

void test_string::run_test_10()
{
  FUNCDEF("run_test_10");
#ifdef DEBUG_STRING_TEST
  LOG("The tenth ones:");
#endif
  astring george("this one will be mangled.");
  ASSERT_EQUAL(george.length(), int(strlen(george.s())), "length is incorrect (q).");
  astring tmp1(george.substring(1, 7));  // constructor.
  astring tmp2 = george.substring(10, george.length() - 1);  // constructor.
#ifdef DEBUG_STRING_TEST
  LOG(tmp1 + "... " + tmp2);
#endif
  ASSERT_INEQUAL(tmp1, tmp2, "bizarre equality occurred!");
  ASSERT_FALSE( (tmp1 > tmp2) || (tmp2 < tmp1), "bizarre comparison error.");
  ASSERT_EQUAL(george.length(), int(strlen(george.s())), "length is incorrect (z).");
#ifdef DEBUG_STRING_TEST
  LOG(george.substring(1, 7));
  LOG("... ");
  LOG(george.substring(10, george.length() - 1));
#endif
  ASSERT_EQUAL(george.length(), int(strlen(george.s())), "length is incorrect (a).");
  george.insert(14, "terribly ");
  ASSERT_EQUAL(george.length(), int(strlen(george.s())), "length is incorrect (b).");
#ifdef DEBUG_STRING_TEST
  LOG(george);
#endif
  astring chunky;
  astring mssr_boef_la_tet("eeyoy eye eye capn");
  mssr_boef_la_tet.substring(chunky, 2, 7);
  ASSERT_EQUAL(chunky, astring("yoy ey"), "contents wrong after substring");

  astring fred(george);
  ASSERT_TRUE(george.compare(fred, 0, 0, george.length() - 1, true), "did not work");
  ASSERT_TRUE(george.compare(fred, 8, 8, george.length() - 1 - 8, true), "partial did not work");

  astring taco1("iLikeTacosNSuch");
  astring taco2("iLikeTaCosNSuch");
  ASSERT_TRUE(taco1.compare(taco2, 0, 0, taco1.length() - 1, false),
      "tacos case-insensitive compare A did not work");
  ASSERT_FALSE(taco1.compare(taco2, 0, 0, taco1.length() - 1, true),
      "tacos case-sensitive compare B did not work");

  ASSERT_EQUAL(george.length(), int(strlen(george.s())), "length is incorrect (c).");
  george.zap(14, 22);
#ifdef DEBUG_STRING_TEST
  LOG(george);
#endif
  astring fred_part;
  fred_part = fred.substring(0, 13);
  ASSERT_EQUAL(fred_part.length(), int(strlen(fred_part.s())), "length incorrect (d).");
  ASSERT_TRUE(george.compare(fred_part, 0, 0, 13, true), "did not work");
  fred_part = fred.substring(23, fred.length() - 1);
  ASSERT_EQUAL(fred_part.length(), int(strlen(fred_part.s())), "length incorrect (e).");
  ASSERT_TRUE(george.compare(fred_part, 14, 0, fred_part.length() - 1, true), "did not work");
#ifdef DEBUG_STRING_TEST
  LOG("compares okay");
#endif
}

void test_string::run_test_11()
{
  FUNCDEF("run_test_11");
  astring empty;
  ASSERT_FALSE(empty.length(), "empty string judged not");
  ASSERT_TRUE(!empty, "not on empty string doesn't work");
  astring non_empty("grungee");
  ASSERT_TRUE(non_empty.length(), "non-empty string judged empty");
  ASSERT_FALSE(!non_empty, "not on non-empty string doesn't work");
}

void test_string::run_test_12()
{
  FUNCDEF("run_test_12");
  astring elf0(astring::SPRINTF, "%%_%%_%%");
  ASSERT_FALSE(strcmp(elf0.s(), "%_%_%"), "failed %% printing");

  char fred[6] = { 'h', 'y', 'a', 'r', 'g', '\0' };
  astring fred_copy(astring::UNTERMINATED, fred, 5);
  ASSERT_EQUAL(fred_copy.length(), 5, "length of copy is wrong");
  ASSERT_EQUAL(fred_copy, astring("hyarg"), "length of copy is wrong");

  char hugh[6] = { 'o', 'y', 'o', 'b', 'o', 'y' };
  astring hugh_copy(astring::UNTERMINATED, hugh, 3);
  ASSERT_EQUAL(hugh_copy.length(), 3, "length of copy is wrong");
  ASSERT_EQUAL(hugh_copy, astring("oyo"), "length of copy is wrong");

  astring another_copy;
  another_copy.reset(astring::UNTERMINATED, fred, strlen(fred));
  ASSERT_EQUAL(another_copy.length(), 5, "length of reset copy is wrong");
  ASSERT_EQUAL(another_copy, astring("hyarg"), "length of reset copy is wrong");
}

void test_string::run_test_13()
{
  FUNCDEF("run_test_13");
  // check for possible memory leaks in these combined ops....  13th.
  const astring churg("borjh sjh oiweoklj");
  astring pud = churg;
  astring flug = "kase iqk aksjir kljasdo";
  const char *nerf = "ausd qwoeui sof zjh qwei";
  astring snorp = nerf;
  pud = "snugbo";
  snorp = flug;
  astring pundit("murph");
  pundit = nerf;
}

void test_string::run_test_14()
{
  FUNCDEF("run_test_14");
  // test the new dynamic sprintf'ing for long strings... 14th.
  const int longish_size = 5000;
  char temp[longish_size];
  for (int i = 0; i < longish_size; i++)
    temp[i] = char(rando.inclusive(1, 255));
  temp[longish_size - 1] = '\0';
  a_sprintf longish("this is a really darned long string of stuff: %s,\nbut doesn't it look interesting?", temp);
  // still with us?
  longish.zap(longish.length() / 3, 2 * longish.length() / 3);
  longish += longish;
  ASSERT_EQUAL(longish.length(), int(strlen(longish.s())), "length is incorrect.");
}

void test_string::run_test_15()
{
  FUNCDEF("run_test_15");
  // test the new dynamic sprintf'ing for long strings... 15th.
  int try_1 = 32767;
  astring testy(astring::SPRINTF, "%d", try_1);
  ASSERT_INEQUAL(testy.convert(95), 95, "default value returned, so it failed.");
  ASSERT_EQUAL(testy.convert(95), try_1, "correct value was not returned.");

  long try_2 = 2938754L;
  testy = astring(astring::SPRINTF, "%ld", try_2);
  ASSERT_INEQUAL((double)testy.convert(98L), (double)98L, "default value returned, so it failed.");
  ASSERT_EQUAL((double)testy.convert(98L), (double)try_2, "correct value was not returned.");

  testy = astring(astring::SPRINTF, "%ld", try_2);
  ASSERT_INEQUAL((double)testy.convert(98L), (double)98L, "default value returned, so it failed.");
  ASSERT_EQUAL((double)testy.convert(98L), (double)try_2, "correct value was not returned.");

  float try_3 = float(2938.754);  // weird msvcpp error if don't cast.
  testy = astring(astring::SPRINTF, "%f", try_3);
  float found = testy.convert(float(98.6));
  ASSERT_INEQUAL(found, 98.6, "default value returned, so it failed.");
  ASSERT_EQUAL(found, try_3, "correct value was not returned.");

  {
    double try_4 = 25598437.712385;
    testy = astring(astring::SPRINTF, "%f", try_4);
    double found2 = testy.convert(double(98.6));
    ASSERT_INEQUAL(found2, 98.6, "default value returned, so it failed.");
    ASSERT_EQUAL(found2, try_4, "correct value was not returned.");
  }

  {
    double try_4 = 25598437.712385e38;
    testy = astring(astring::SPRINTF, "%f", try_4);
    double found2 = testy.convert(double(98.6));
    ASSERT_INEQUAL(found2, 98.6, "default value returned, so it failed.");
    ASSERT_EQUAL(found2, try_4, "correct value was not returned.");
  }
}

void test_string::run_test_16()
{
  FUNCDEF("run_test_16");
  // the 16th test group tests boundary checking.
  astring gorf("abc");
  for (int i = -3; i < 25; i++) gorf[i] = 't';
  ASSERT_EQUAL(gorf, astring("ttt"), "correct value was not returned (a).");
  ASSERT_EQUAL(gorf.length(), 3, "length is incorrect (a).");
  gorf.insert(3, astring("bru"));
  ASSERT_EQUAL(gorf, astring("tttbru"), "correct value was not returned (b).");
  ASSERT_EQUAL(gorf.length(), 6, "length is incorrect (b).");
  gorf.insert(35, astring("snu"));
  ASSERT_EQUAL(gorf, astring("tttbru"), "correct value was not returned (c).");
  ASSERT_EQUAL(gorf.length(), 6, "length is incorrect (c).");
  gorf.insert(-30, astring("eep"));
  ASSERT_EQUAL(gorf, astring("tttbru"), "correct value was not returned (d).");
  ASSERT_EQUAL(gorf.length(), 6, "length is incorrect (d).");
}

void test_string::run_test_17()
{
  FUNCDEF("run_test_17");
  // 17th test checks construction of temporaries.
/* this test set causes the obnoxious 16 bit codeguard error from hell, as
   does use of temporary objects in ostream << operators.  argh! */
  astring("jubjo");
  astring("obo");
  astring("flrrpominort").substring(3, 6);
}

void test_string::run_test_18()
{
#ifdef DEBUG_STRING_TEST
  FUNCDEF("run_test_18");
#endif
  // 18th test group checks windows related functions.
#ifdef AFXDLL
  AfxSetResourceHandle(GET_INSTANCE_HANDLE());
    // required for mfc to see the proper instance handle.

  // this tests the "constructor from resource".
  astring libname = rc_string(IDS_BASIS_NAME);
  ASSERT_EQUAL(libname, astring("Basis Library"),
      astring("library name is a mismatch: comes out as \"") + libname + "\".");
  #define IDS_SOME_BAD_UNKNOWN_STRING_HANDLE 30802
  astring bogus_name = rc_string(IDS_SOME_BAD_UNKNOWN_STRING_HANDLE);
  ASSERT_FALSE(bogus_name.length(), "bogus rc string had length");

  // tests conversions from CString to astring and back.
  astring george("yo yo yo");
  CString hal = convert(george);
  astring borgia = convert(hal);

#ifdef DEBUG_STRING_TEST
  LOG(astring("cstringy conversions: ") + george);
  LOG((const char *)hal);
  LOG(borgia);
#endif

  ASSERT_EQUAL(borgia, george, "got the wrong value");
#endif
}

void test_string::run_test_19()
{
  FUNCDEF("run_test_19");
  // 19th test group is devoted to anthony wenzel's % printing bug.
  astring problematic_example(astring::SPRINTF, "this should have %d% more "
      "stuffing than before!", 20);
//MessageBox(0, astring("got a string of \"%s!\"", problematic_example.s()).s(), "yo!", MB_OK);
  ASSERT_EQUAL(problematic_example, astring("this should have 20% more stuffing than before!"), "failure to print correct phrase");
}

void test_string::run_test_20()
{
#ifdef DEBUG_STRING_TEST
  FUNCDEF("run_test_20");
#endif
  // 20th test group is devoted to another wenzelbug.

  // Hey, I just found out (in an ugly way) that the following doesn't work:
  char myText[] = "OK";
  astring myString(astring::SPRINTF, "%04s", myText);
#ifdef DEBUG_STRING_TEST
  LOG(astring("first try gets: ") + myString);
#endif

  char myText8[] = "OK";
  char my_text_4[90];
  sprintf(my_text_4, "%4s", myText8);
#ifdef DEBUG_STRING_TEST
  LOG(astring("second try gets: ") + astring(my_text_4));
#endif

  // Looks like you don't handle the "%04s" arguments properly.  I can make
  // it work as follows:
  char myText2[] = "OK";
  char myText3[50];
  sprintf(myText3, "%4s", myText2);
  astring myString2(myText3);
#ifdef DEBUG_STRING_TEST
  LOG(astring("third try gets: ") + myString2);
#endif
}

void test_string::run_test_21()
{
  FUNCDEF("run_test_21");
  // 21st test group checks out the strip spaces and replace functions.
  astring spacey("   dufunk  ");
  astring temp = spacey;
  temp.strip_spaces(astring::FROM_FRONT);
  ASSERT_EQUAL(temp, astring("dufunk  "), "created wrong string");
  temp = spacey;
  temp.strip_spaces(astring::FROM_END);
  ASSERT_EQUAL(temp, astring("   dufunk"), "created wrong string");
  temp = spacey;
  temp.strip_spaces(astring::FROM_BOTH_SIDES);
  ASSERT_EQUAL(temp, astring("dufunk"), "created wrong string");

  astring placemat("mary had a red hooded cobra she kept around her neck "
      "and it hissed at the people as they walked by her tent.");
  ASSERT_TRUE(placemat.replace("had", "bought"), "replace failed");
  ASSERT_TRUE(!placemat.replace("hoded", "bought"), "replace didn't fail but should have");
  ASSERT_TRUE(placemat.replace("she", "funkateria"), "replace failed");
  ASSERT_TRUE(placemat.replace("hooded", "hood"), "replace failed");
  ASSERT_TRUE(placemat.replace("cobra", "in the"), "replace failed");

  int indy = placemat.find("kept");
  ASSERT_FALSE(negative(indy), "couldn't find string");
  placemat[indy - 1] = '.';
  placemat.zap(indy, placemat.end());
  ASSERT_EQUAL(placemat, astring("mary bought a red hood in the funkateria."), "got wrong result string");
}

void test_string::run_test_22()
{
  FUNCDEF("run_test_22");
  // 22nd test: morgan's find bug.
  astring B("buzz*buzz*");
  {
    int x = B.find('*');  // correctly finds the first *.
    ASSERT_EQUAL(x, 4, "got wrong index for first");
    x++;   
    x = B.find('*', x);  // correctly finds the second *.
    ASSERT_EQUAL(x, 9, "got wrong index for second");
    x++;  // now x == B.length().
    x = B.find('*', x);
      // error was: finds the second * again (and again and again and 
      // again...).  At this point it should return OUT_OF_RANGE.
    ASSERT_FALSE(x > 0, "got wrong outcome for third");
  }
  {
    int x = B.find("z*");  // correctly finds the first z*.
    ASSERT_EQUAL(x, 3, "got wrong index for fourth");
    x++;   
    x = B.find("z*", x);  // correctly finds the second *.
    ASSERT_EQUAL(x, 8, "got wrong index for fifth");
    x++;  // now x == B.length().
    x = B.find("z*", x);
      // error was: finds the second * again (and again and again and 
      // again...).  At this point it should return OUT_OF_RANGE.
    ASSERT_FALSE(x > 0, "got wrong outcome for sixth");
  }
}

void test_string::run_test_23()
{
  FUNCDEF("run_test_23");
  // 23rd test: test the new strip function.
  astring strip_string("!@#$%^&*()");
  
  astring no_stripper("this shouldn't change");
  no_stripper.strip(strip_string, astring::FROM_BOTH_SIDES);
  ASSERT_EQUAL(no_stripper, astring("this shouldn't change"), "first test failed comparison");

  astring strippee_orig("&$(*(@&@*()()!()@*(@(*fudge#((@(*@**#)(@#)(#");
  astring strippee(strippee_orig);
  strippee.strip(strip_string, astring::FROM_BOTH_SIDES);
  ASSERT_EQUAL(strippee, astring("fudge"), "second test failed comparison");

  strippee = strippee_orig;
  strippee.strip(strip_string, astring::FROM_FRONT);
  ASSERT_EQUAL(strippee, astring("fudge#((@(*@**#)(@#)(#"), "third test failed comparison");

  strippee = strippee_orig;
  strippee.strip(strip_string, astring::FROM_END);
  ASSERT_EQUAL(strippee, astring("&$(*(@&@*()()!()@*(@(*fudge"), "fourth test failed comparison");
}

void test_string::run_test_24()
{
  FUNCDEF("run_test_24");
#ifndef __GNU_WINDOWS__
#ifdef __WIN32__
  // 24th test group tests _bstr_t conversions.
  _bstr_t beast("abcdefgh");
#ifdef DEBUG_STRING_TEST
  LOG(astring("the beast is ") + beast.operator char *() +
      astring(astring::SPRINTF, " with length %d", beast.length()));
#endif
  astring convert = beast;
#ifdef DEBUG_STRING_TEST
  LOG(astring("the convert is ") + convert
      + astring(astring::SPRINTF, " with length %d", convert.length()));
#endif
  ASSERT_EQUAL(convert, astring("abcdefgh"), "first test failed comparison");

  astring jethro("i want a hog sandwich");
  _bstr_t pork = string_convert::to_bstr_t(jethro);
  ASSERT_FALSE(strcmp(pork.operator char *(), jethro.s()), "second test failed comparison");
#endif
#endif
}

void test_string::run_test_25()
{
  FUNCDEF("run_test_25");
  // 25th test group tests simple comparisons.
  astring fred = "asdoiuaoisud";
  ASSERT_INEQUAL(fred, astring(), "first test failed comparison");
  astring bub = fred;
  ASSERT_EQUAL(fred, bub, "second test failed comparison");
  fred = "";
  ASSERT_EQUAL(fred, astring(), "third test failed comparison");
  ASSERT_FALSE(fred.length(), "fourth test failed comparison");
}

void test_string::run_test_26()
{
  FUNCDEF("run_test_26");
  // 26th test group does simple time_stamp::notarize operations.  these are more for
  // ensuring boundschecker gets to see some of this.
  astring t2 = time_stamp::notarize(false);
  astring t4 = time_stamp::notarize(true);
}

void test_string::run_test_27()
{
  FUNCDEF("run_test_27");
  // 27th test group plays around with idate in an attempt to get
  // boundschecker to complain.
  timely::day_in_year d1 = date_now();
  timely::clock_time t1 = time_now();
  timely::time_locus dt1 = now();
  astring sd1 = d1.text_form();
  astring st1 = t1.text_form();
  astring sdt1 = dt1.text_form_long();
}

void test_string::run_test_28()
{
  FUNCDEF("run_test_28");
  // 28th test group does sprintfs on shorts and such.
  basis::un_int in1 = 27;
  basis::un_short in2 = 39;
  char in3 = 'Q';
  astring testy(astring::SPRINTF, "%u:%hu:%c", in1, in2, in3);
  ASSERT_EQUAL(testy, astring("27:39:Q"), "fourth test failed comparison");
}

void test_string::run_test_29()
{
  FUNCDEF("run_test_29");
  // 29th test group tries out the packing support.
  astring a("would an onion smell so sweet?");
  byte_array p1;
  a.pack(p1);
  astring b;
  ASSERT_TRUE(b.unpack(p1), "first unpack failed");
  ASSERT_EQUAL(b, a, "first comparison failed");
  a = "128 salamanders cannot be wrong.";
  a.pack(p1);
  ASSERT_TRUE(b.unpack(p1), "second unpack failed");
  ASSERT_EQUAL(b, a, "second comparison failed");
}

void standard_sprintf_test(const char *parm_string)
{
  FUNCDEF("standard_sprintf_test");
  astring print_into(' ', 20000);
  print_into[0] = '\0';
//check these!!!:
  int i1 = int(rando.inclusive(0, 23945));
  long l1 = long(rando.inclusive(-2394, 2998238));
  un_int u1 = basis::un_int(rando.inclusive(0, 124395));
  un_short s1 = basis::un_short(rando.inclusive(0, 65535));
  sprintf(print_into.s(), "%d %ld %u %hu %s", i1, l1, u1, s1, parm_string);
  sprintf(print_into.s(), "%c %d %c %s %s %lu", char(rando.inclusive('a', 'z')),
      int(rando.inclusive(0, 23945)), char(rando.inclusive('A', 'Z')),
      parm_string, parm_string, basis::un_long(rando.inclusive(0, 2998238)));
}

void test_string::run_test_30()
{
  // 30th test group checks astring sprintf.
  FUNCDEF("run_test_30");
  astring parm_string = string_manipulation::make_random_name(40, 140);
  astring print_into(' ', 20000);
  print_into[0] = '\0';
  int i1 = int(rando.inclusive(0, 23945));
  long l1 = long(rando.inclusive(-2394, 2998238));
  un_int u1 = basis::un_int(rando.inclusive(0, 124395));
  un_short s1 = basis::un_short(rando.inclusive(0, 65535));
  char test_same[20010];
  sprintf(test_same, "%d %ld %u %hu %s", i1, l1, u1, s1, parm_string.s());
  print_into.sprintf("%d %ld %u %hu %s", i1, l1, u1, s1, parm_string.s());
  ASSERT_EQUAL(astring(test_same), print_into, "sprintf should get same results as standard");
//do test for this one too.
  print_into.sprintf("%c %d %c %s %s %lu", char(rando.inclusive('a', 'z')),
      int(rando.inclusive(0, 23945)), char(rando.inclusive('A', 'Z')),
      parm_string.observe(), parm_string.s(), basis::un_long(rando.inclusive(0, 2998238)));
}

void test_string::run_test_31()
{
  FUNCDEF("run_test_31");
  // testing of character repeat constructor.
  astring dashes('-', 20);
  astring non_plusses('+', 0);
  astring plusses('+', 1);
  ASSERT_EQUAL(dashes.length(), 20, astring("dashes has wrong length!"));
  ASSERT_EQUAL(dashes, astring("--------------------"), astring("dashes has wrong contents!  '") + dashes + "' vs. '" + astring("--------------------'"));
  ASSERT_FALSE(non_plusses.length(), astring("non_plusses has wrong length!"));
  ASSERT_EQUAL(plusses.length(), 1, astring("plusses has wrong length!"));
  ASSERT_EQUAL(plusses, astring("+"), astring("plusses has wrong contents!"));
  ASSERT_EQUAL(plusses, astring('+', 1), astring("plusses second compare failed!"));
}

void test_string::run_test_32()
{
  FUNCDEF("run_test_32");
  // tests creating a huge string and ripping it apart.

  const int CHUNK_SIZE = 2 * MEGABYTE;
    // the block factor for our string.  we expect not to go above this during
    // the testing.
  const int MIN_ADDITION = 10000;  const int MAX_ADDITION = 80000;
    // these are the largest chunk we add to a string at a time.
  const int BUILD_AND_BURN_ITERATIONS = 1;
    // number of times to test building up and tearing down.

  // the string we build up and tear down.
  astring slab;

//hmmm: maybe have a mixed phase where tearing and adding
//      happens frequently and interspersed.

  for (int iters = 0; iters < BUILD_AND_BURN_ITERATIONS; iters++) {

    int size = 0;  // independent count of expected size.
    // we don't want to bother allocating the big chunk more than once, so
    // we'll add to the string until it's within range of going over.
    while (slab.length() < CHUNK_SIZE - MAX_ADDITION - 20) {
//make this into add_a_chunk
      astring addition = string_manipulation::make_random_name(MIN_ADDITION,
          MAX_ADDITION);
      slab += addition;
      size += addition.length();
      ASSERT_EQUAL(size, slab.length(), astring("size is incorrect after add!"));
    }

    // now we eat it up.
    while (slab.length()) {
//make this into zap_a_chunk
      int zap_start = rando.inclusive(0, slab.end());
      int zap_end = rando.inclusive(0, slab.end());
      flip_increasing(zap_start, zap_end);
      int range_length = zap_end - zap_start + 1;
      ASSERT_FALSE(negative(range_length), astring("flip_increasing failed!"));
      slab.zap(zap_start, zap_end);
      size -= range_length;
      ASSERT_EQUAL(size, slab.length(), astring("size is incorrect after zap!"));
    }
  }
}

void test_string::run_test_33()
{
  FUNCDEF("run_test_33");
  // 33rd test group exercises the replace functions.
  {
    astring to_modify("\\\\feeby\\path\\yo");
    ASSERT_TRUE(to_modify.replace("\\", "/"), "failed to replace the string");
    ASSERT_EQUAL(to_modify, astring("/\\feeby\\path\\yo"), "produced wrong resultant string");
    while (to_modify.replace("\\", "/")) {}
    ASSERT_EQUAL(to_modify, astring("//feeby/path/yo"), "produced wrong final string");
  }
  {
    astring to_modify("\\superduper\\dynamo\\looper");
    ASSERT_TRUE(to_modify.replace("\\", "/"), "failed to replace the string");
    ASSERT_EQUAL(to_modify, astring("/superduper\\dynamo\\looper"), "produced wrong resultant string");
    while (to_modify.replace("\\", "/")) {}
    ASSERT_EQUAL(to_modify, astring("/superduper/dynamo/looper"), "produced wrong final string");
  }
  {
    astring id = "/SRV=1/SRC=1";
    astring id1 = id;
    while (id1.replace("/", " ")) {}
//    LOG(astring("replacing / with ' ' in first test (was ") + id +
//        ") gives " + id1);
    ASSERT_EQUAL(id1, astring(" SRV=1 SRC=1"), "produced wrong final string");

    astring id2 = id;
    while (id2.replace("=", ":")) {}
//    LOG(astring("replacing = with : in second test (was ") + id +
//        ") gives " + id2);
    ASSERT_EQUAL(id2, astring("/SRV:1/SRC:1"), "produced wrong final string");
  }
}

void test_string::run_test_34()
{
  FUNCDEF("run_test_34");

//not in use right now.

}

void test_string::run_test_35()
{
  FUNCDEF("run_test_35");
  // test the shrink method.
  astring termo('R', 2812);
  ASSERT_EQUAL(termo.length(), 2812, "length should be as requested");
  termo[1008] = '\0';
  termo.shrink();
  ASSERT_EQUAL(termo.get_implementation().internal_real_length(), 1010, a_sprintf("failure in shrunken size: " "wanted 1010 and got %d.", termo.get_implementation().internal_real_length()));
  astring termo2('R', 1008);
  ASSERT_EQUAL(termo, termo2, "wrong value produced");
}

void test_string::run_test_36()
{
  FUNCDEF("run_test_36");
  // test the text form on a string array (which is mildly related to strings;
  // this could be moved to a common templates test someday).
  string_array torpid;
  torpid += "york";
  torpid += "burger";
  torpid += "petunia";
  torpid += "dumptruck";
  ASSERT_EQUAL(torpid.text_form(), astring("\"york\",\"burger\",\"petunia\",\"dumptruck\""), "wrong value computed");
  string_array sacral;
  sacral += "gumboat";
  ASSERT_EQUAL(sacral.text_form(), astring("\"gumboat\""), "wrong value computed");

  string_array paknid;
  paknid += "gorboochy";
  paknid += "rangolent";
  byte_array packed;
  structures::pack_array(packed, paknid);

  string_array upnort;
  ASSERT_TRUE(structures::unpack_array(packed, upnort), "failed to unpack");
  ASSERT_FALSE(packed.length(), "array still has bytes!");

  string_array stongent;
  stongent += "pemulack";
  stongent += "bluzzent";
  stongent += "crupto";
  stongent += "floonack";
  stongent += "atoona";
  packed.reset();
  structures::pack_array(packed, stongent);

  string_array belzorp;
  ASSERT_TRUE(structures::unpack_array(packed, belzorp), "failed to unpack");
  ASSERT_FALSE(packed.length(), "array still has bytes!");
}

void test_string::run_test_37()
{
  FUNCDEF("run_test_37");
  // 37th test group used to try out the old packing support, but now is
  // just the same as test 29.  it would be good to make this different.
  astring a("would an onion smell so sweet?");
  byte_array p1;
  a.pack(p1);
  astring b;
  ASSERT_TRUE(b.unpack(p1), "first unpack failed");
  ASSERT_EQUAL(b, a, "first comparison failed");
  a = "128 salamanders cannot be wrong.";
  a.pack(p1);
  ASSERT_TRUE(b.unpack(p1), "second unpack failed");
  ASSERT_EQUAL(b, a, "second comparison failed");
}

void test_string::run_test_38()
{
  FUNCDEF("run_test_38");
  double to_print = 2.345;
  a_sprintf non_deadly("%.1f", to_print);
///  LOG(astring("printed: ") + non_deadly);

  to_print = 1.797E+254;
    // this value breaks things.

  char bucket[2000];
  bucket[0] = '\0';
  sprintf(bucket, "%.1f", to_print);
///  LOG(astring("sprintf printed: ") + bucket);

  a_sprintf deadly("%.1f", to_print);
///  LOG(astring("printed: ") + deadly);
}

void test_string::run_test_39()
{
  FUNCDEF("run_test_39");
  const char *find_set = "!?.;";
  astring test_1 = "how do i get to balthazar square?  it stinks!";
  ASSERT_EQUAL(test_1.find_any(find_set), 32, "first find returned wrong result");
  ASSERT_EQUAL(test_1.find_any(find_set, 33), 44, "second find returned wrong result");
  ASSERT_EQUAL(test_1.find_any(find_set, 40, true), 32, "third find returned wrong result");
}

void test_string::run_test_40()
{
  FUNCDEF("run_test_40");
  int test_num = 1;
  #define test_name() a_sprintf("test %d: ", test_num)
  {
    astring target = "xabab";
    astring from = "ab";
    astring to = "dc";
    ASSERT_TRUE(target.replace_all(from, to), test_name() + "didn't find from string");
    ASSERT_EQUAL(target, astring("xdcdc"), test_name() + "didn't replace properly");
    test_num++;
  }
  {
    astring target = "xabab";
    astring from = "ab";
    astring to = "ab";
    ASSERT_TRUE(target.replace_all(from, to), test_name() + "didn't find from string");
    ASSERT_EQUAL(target, astring("xabab"), test_name() + "didn't replace properly");
    test_num++;
  }
  {
    astring target = "xabab";
    astring from = "ab";
    astring to = "a";
    ASSERT_TRUE(target.replace_all(from, to), test_name() + "didn't find from string");
    ASSERT_EQUAL(target, astring("xaa"), test_name() + "didn't replace properly");
    test_num++;
  }
  {
    astring target = "ababx";
    astring from = "ab";
    astring to = "a";
    ASSERT_TRUE(target.replace_all(from, to), test_name() + "didn't find from string");
    ASSERT_EQUAL(target, astring("aax"), test_name() + "didn't replace properly");
    test_num++;
  }
  {
    astring target = "suzzle rumpetzzes gnargle rezztor";
    astring from = "zz";
    astring to = "zzz";
    ASSERT_TRUE(target.replace_all(from, to), test_name() + "didn't find from string");
    ASSERT_EQUAL(target, astring("suzzzle rumpetzzzes gnargle rezzztor"), test_name() + "didn't replace properly");
    test_num++;
  }
  {
    astring target = "qqqq";
    astring from = "q";
    astring to = "qqq";
    ASSERT_TRUE(target.replace_all(from, to), test_name() + "didn't find from string");
    ASSERT_EQUAL(target, astring("qqqqqqqqqqqq"), test_name() + "didn't replace properly");
    test_num++;
  }
  {
    astring target = "glorg snorp pendle funk";
    astring from = " ";
    astring to = "";
    ASSERT_TRUE(target.replace_all(from, to), test_name() + "didn't find from string");
    ASSERT_EQUAL(target, astring("glorgsnorppendlefunk"), test_name() + "didn't replace properly");
    test_num++;
  }
}

void test_string::run_test_41()
{
  FUNCDEF("run_test_41");
  int test_num = 0;
  #define test_name() a_sprintf("test %d: ", test_num)
  {
    test_num++;
    astring target = "xabab";
    const char *finding1 = "ab";
    ASSERT_EQUAL(target.find_non_match(finding1, 0, false), 0, test_name() + "didn't find right location A");
    const char *finding2 = "xb";
    ASSERT_EQUAL(target.find_non_match(finding2, target.length() - 1, true), 3, test_name() + "didn't find right location B");
    const char *finding3 = "c";
    ASSERT_EQUAL(target.find_non_match(finding3, 0, false), 0, test_name() + "wrong answer for test C");
    ASSERT_EQUAL(target.find_non_match(finding3, target.length() - 1, true), target.length() - 1, 
      test_name() + "wrong answer for test D");
  }
  {
    test_num++;
    astring target = "abcadefghoota";
    const char *finding1 = "bcdfghot";
    ASSERT_EQUAL(target.find_non_match(finding1, 0, false), 0, test_name() + "didn't find right location A");
    ASSERT_EQUAL(target.find_non_match(finding1, 1, false), 3, test_name() + "didn't find right location B");
    ASSERT_EQUAL(target.find_non_match(finding1, target.length() - 1, true), target.length() - 1, test_name() + "didn't find right location C");
    ASSERT_EQUAL(target.find_non_match(finding1, target.length() - 2, true), 5, test_name() + "didn't find right location D");
    ASSERT_EQUAL(target.find_non_match(finding1, 3, false), 3, test_name() + "didn't find right location E");
    ASSERT_EQUAL(target.find_non_match(finding1, 4, false), 5, test_name() + "didn't find right location F");

  }

}

// exercise the middle, right and left methods.
void test_string::run_test_42()
{
  FUNCDEF("run_test_42");

  astring hobnob("all the best robots are bending robots");

  ASSERT_EQUAL(hobnob.middle(5, 7), astring("he best"), "failed to find middle of string");
  ASSERT_EQUAL(hobnob.right(10), astring("ing robots"), "failed to find right side of string");
  ASSERT_EQUAL(hobnob.left(6), astring("all th"), "failed to find right side of string");
}

//////////////

int test_string::execute()
{
  FUNCDEF("execute");

//  ASSERT_EQUAL(0, 1, "fake assertion to test jenkins log parsing");
  ASSERT_EQUAL(1, 1, "non-fake assertion to test jenkins log parsing");

  ASSERT_EQUAL(staticity_test, astring("yo!"), "wrong contents");

  time_stamp end_time(TEST_RUNTIME_DEFAULT);
    // when we're done testing.

  int i = 0;  // iteration counter.
  while (time_stamp() < end_time) {
    // we run the test until our time runs out.
    i++;  // next iteration.
#ifdef DEBUG_STRING_TEST
    LOG(astring(astring::SPRINTF, "index %d", i));
#endif

    // beginning of test sets.
    run_test_01();
    run_test_02();
    run_test_03();
    run_test_04();
    run_test_05();
    run_test_06();
    run_test_07();
    run_test_08();
    run_test_09();
    run_test_10();
    run_test_11();
    run_test_12();
    run_test_13();
    run_test_14();
    run_test_15();
    run_test_16();
    run_test_17();
    run_test_18();
    run_test_19();
    run_test_20();
    run_test_21();
    run_test_22();
    run_test_23();
    run_test_24();
    run_test_25();
    run_test_26();
    run_test_27();
    run_test_28();
    run_test_29();
//too slow    run_test_30();
    run_test_31();
    run_test_32();
    run_test_33();
//retired    run_test_34();
    run_test_35();
    run_test_36();
    run_test_37();
    run_test_38();
    run_test_39();
    run_test_40();
    run_test_41();
    run_test_42();
  }

  return final_report();
}

HOOPLE_MAIN(test_string, )


