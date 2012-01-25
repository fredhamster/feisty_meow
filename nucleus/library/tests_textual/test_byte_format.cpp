/*
*  Name   : test_byte_format
*  Author : Chris Koeritz
*  Purpose: Puts the byte formatting utilities through their paces.
**
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
*/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <mathematics/chaos.h>
#include <loggers/program_wide_logger.h>
#include <textual/byte_formatter.h>
#include <unit_test/unit_base.h>
#include <structures/static_memory_gremlin.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace mathematics;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger::get(), astring(to_print))

class test_byte_format : virtual public unit_base, virtual public application_shell
{
public:
  test_byte_format() {}
  DEFINE_CLASS_NAME("test_byte_format");
  int execute();
};

int test_byte_format::execute()
{
  FUNCDEF("execute");
  {
    // test out the byte shifting functions on a simple known array.
    byte_array source;
    source += 0x23; source += 0x5f; source += 0xa8; source += 0x2d; 
    source += 0xe2; source += 0x61; source += 0x90; source += 0x2d; 
    source += 0xf2; source += 0x38; 
    astring shifty;
    byte_formatter::bytes_to_shifted_string(source, shifty);
//    LOG(a_sprintf("A: shifted our %d bytes into: ", source.length())
//        + shifty);
    byte_array source_copy;
    byte_formatter::shifted_string_to_bytes(shifty, source_copy);
    ASSERT_EQUAL(source_copy, source, "A: shifting corrupted the bytes.");
  }

  {
    // test out the byte shifting functions on a random array.
    for (int run = 0; run < 100; run++) {
      byte_array source;
      int size = randomizer().inclusive(1, 20923);
      for (int i = 0; i < size; i++) {
        source += abyte(randomizer().inclusive(0, 255));
      }
      astring shifty;
      byte_formatter::bytes_to_shifted_string(source, shifty);
//      LOG(a_sprintf("B: shifted our %d bytes into: ", source.length())
//          + shifty);
      byte_array source_copy;
      byte_formatter::shifted_string_to_bytes(shifty, source_copy);
      ASSERT_EQUAL(source_copy, source, "B: shifting corrupted the bytes.");
    }
  }

  {
    astring burf("alia bodalia petunia");
    astring dump(byte_formatter::text_dump((abyte *)burf.s(), burf.length() + 1));
//doofus!  make this compare the output with expectations.
    LOG("dumped form is:");
    LOG("");
    LOG(dump);
  }
  {
    abyte fodder[] = { 0x83, 0x0d, 0x93, 0x21, 0x82, 0xfe, 0xef, 0xdc, 0xb9,
        0xa9, 0x21, 0x54, 0x83, 0x38, 0x65, 0x59, 0x99, 0xff, 0x00, 0xa0,
        0x29, 0x03 };
    byte_array fred(sizeof(fodder), fodder);
    astring as_text1;
    byte_formatter::bytes_to_string(fred, as_text1, true);
    LOG(astring("got string #1 of: ") + as_text1);
    astring as_text2;
    byte_formatter::bytes_to_string(fred, as_text2, false);
    LOG(astring("got string #2 of: ") + as_text2);
    byte_array convert1;
    byte_formatter::string_to_bytes(as_text1, convert1);
    byte_array convert2;
    byte_formatter::string_to_bytes(as_text2, convert2);
    ASSERT_EQUAL(fred, convert1, "first abyte conversion: failed due to inequality");
    ASSERT_TRUE(fred == convert2, "second abyte conversion: failed due to inequality");
    // now a harder test.
    astring as_text3("muggulo x83d x93, x21, x82, xfe, xef, xdc, xb9, "
        "xa9, x21, x54, x83, x38, x65, x59, x99, xff, x00a0, x293");
    byte_array harder_convert1;
    byte_formatter::string_to_bytes(as_text3, harder_convert1);
astring back3;
byte_formatter::bytes_to_string(harder_convert1, back3);
LOG(astring("got third: ") + back3);

    astring as_text4("muggulo x83d x93, x21, x82, xfe, xef, xdc, xb9, "
        "xa9, x21, x54, x83, x38, x65, x59, x99, xff, x00a0, x293gikkor");
    byte_array harder_convert2;
    byte_formatter::string_to_bytes(as_text4, harder_convert2);
astring back4;
byte_formatter::bytes_to_string(harder_convert2, back4);
LOG(astring("got fourth: ") + back4);
    ASSERT_EQUAL(fred, harder_convert1, "third abyte conversion: failed due to inequality");
    ASSERT_EQUAL(fred, harder_convert2, "fourth abyte conversion: failed due to inequality");

    abyte fodder2[] = {
0x04, 0x00, 0x06, 0x00, 0x0a, 0x02, 0x03, 0x00, 0x06, 0x00, 0x48, 0x01, 0x1c, 0x00, 0x2c, 0x00, 0x04, 0x00, 0x09, 0x00, 0x17, 0x00, 0xff, 0xff, 0x00, 0x00,
0x00, 0x00, 0x09, 0x00 };
    fred = byte_array(sizeof(fodder2), fodder2);
    astring as_text5("040006000a020300060048011c002c00040009001700ffff000000000900");
    byte_array harder_convert3;
    byte_formatter::string_to_bytes(as_text5, harder_convert3);
astring back5;
byte_formatter::bytes_to_string(harder_convert3, back5);
LOG(astring("got fifth: ") + back5);
    ASSERT_EQUAL(fred, harder_convert3, "fifth abyte conversion: failed due to inequality");

#ifndef EMBEDDED_BUILD
    // now a test of parse_dump.
    astring fred_dump;
    byte_formatter::text_dump(fred_dump, fred, 0x993834);
LOG("fred dump...");
LOG(fred_dump);
    byte_array fred_like;
    byte_formatter::parse_dump(fred_dump, fred_like);
    ASSERT_EQUAL(fred, fred_like, "parse_dump test: failed due to inequality");
#endif
  }
  {
    // an endian tester.
    basis::un_short test1 = 0x3c5f;
    LOG("0x3c5f in intel:");
    LOG(byte_formatter::text_dump((abyte *)&test1, 2));
    basis::un_int test2 = 0x9eaad0cb;
    LOG("0x9eaad0cb in intel:");
    LOG(byte_formatter::text_dump((abyte *)&test2, 4));

    // now see what they look like as packables.
    byte_array testa;
    structures::attach(testa, test1);
    LOG("0x3c5f in package:");
    LOG(byte_formatter::text_dump(testa));
    byte_array testb;
    structures::attach(testb, test2);
    LOG("0x9eaad0cb in package:");
    LOG(byte_formatter::text_dump(testb));
  }

  return final_report();
}

HOOPLE_MAIN(test_byte_format, );

