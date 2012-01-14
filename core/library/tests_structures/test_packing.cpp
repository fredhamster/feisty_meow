/*
*  Name   : test_object_packing
*  Author : Chris Koeritz
**
* Copyright (c) 1996-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <loggers/critical_events.h>
#include <mathematics/chaos.h>
#include <mathematics/double_plus.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

#include <math.h>

using namespace application;
using namespace basis;
using namespace loggers;
using namespace mathematics;
using namespace structures;
using namespace unit_test;

#define GENERATE_TEST_NAME(group, message) \
  (astring(group) + " test group: " + message)

#define TRY_ON(type, value, group) { \
  byte_array temp_array; \
  attach(temp_array, type(value)); \
  type output; \
/*log(astring(astring::SPRINTF, "parms are: type=%s value=%s group=%s", #type, #value, #group));*/ \
  ASSERT_TRUE(detach(temp_array, output), \
    GENERATE_TEST_NAME(group, "should unpack " #type " okay")); \
  ASSERT_TRUE(output == type(value), \
    GENERATE_TEST_NAME(group, #type " value should match")); \
  ASSERT_FALSE(temp_array.length(), \
    GENERATE_TEST_NAME(group, #type " detached should be empty")); \
}

#define TRY_ON_OBSCURE(type, value, group) { \
  byte_array temp_array; \
  obscure_attach(temp_array, type(value)); \
  type output; \
/*log(astring(astring::SPRINTF, "parms are: type=%s value=%s group=%s", #type, #value, #group));*/ \
  ASSERT_TRUE(obscure_detach(temp_array, output), \
    GENERATE_TEST_NAME(group, "should obscure unpack " #type " okay")); \
  ASSERT_TRUE(output == type(value), \
    GENERATE_TEST_NAME(group, #type " value should obscure match")); \
  ASSERT_FALSE(temp_array.length(), \
    GENERATE_TEST_NAME(group, #type " obscure detached should be empty")); \
}

#define TRY_ON_F(type, value, group) { \
  byte_array temp_array; \
  attach(temp_array, type(value)); \
  type output; \
/*log(astring(astring::SPRINTF, "parms are: type=%s value=%s group=%s", #type, #value, #group));*/ \
  ASSERT_TRUE(detach(temp_array, output), \
      GENERATE_TEST_NAME(group, "should unpack " #type " fine")); \
/*  double_plus<type> a(output); \
  double_plus<type> b(value); */ \
  /*double diff = maximum(output, value) - minimum(output, value);*/ \
  int exponent_1, exponent_2; \
  double mantissa_1 = frexp(output, &exponent_1); \
  double mantissa_2 = frexp(output, &exponent_2); \
  ASSERT_FALSE( (mantissa_1 != mantissa_2) || (exponent_1 != exponent_2), \
      GENERATE_TEST_NAME(group, #type " value should match just so")); \
  ASSERT_FALSE(temp_array.length(), \
      GENERATE_TEST_NAME(group, #type " detached should have no data left")); \
}

class test_object_packing : virtual public unit_base, virtual public application_shell
{
public:
  test_object_packing() : application_shell() {}
  ~test_object_packing() {}

  DEFINE_CLASS_NAME("test_object_packing");

  int execute();
};

//////////////

int test_object_packing::execute()
{
  FUNCDEF("execute");
  {
    #define TEST "first"
    TRY_ON(int, 2383, TEST);
    TRY_ON(int, -18281, TEST);
//    TRY_ON(long, 337628, TEST);
//    TRY_ON(long, -987887, TEST);
    TRY_ON(short, 12983, TEST);
    TRY_ON(short, -32700, TEST);
    TRY_ON(int, 2988384, TEST);
    TRY_ON(int, 92982984, TEST);
//    TRY_ON(un_long, 388745, TEST);
//    TRY_ON(un_long, 993787, TEST);
    TRY_ON(basis::un_short, 12983, TEST);
    TRY_ON(basis::un_short, 48377, TEST);
    TRY_ON_OBSCURE(un_int, -23948377, TEST);
    TRY_ON_OBSCURE(un_int, 28938, TEST);
    #undef TEST
  }
  {
    #define TEST "second"
    TRY_ON(int, 0, TEST);
    TRY_ON(int, MAXINT32, TEST);
    TRY_ON(int, MININT32, TEST);
    TRY_ON(abyte, 0, TEST);
    TRY_ON(abyte, MAXBYTE, TEST);
    TRY_ON(abyte, MINBYTE, TEST);
    TRY_ON(char, 0, TEST);
    TRY_ON(char, MAXCHAR, TEST);
    TRY_ON(char, MINCHAR, TEST);
//    TRY_ON(long, 0, TEST);
//    TRY_ON(long, MAXLONG, TEST);
//    TRY_ON(long, MINLONG, TEST);
    TRY_ON(short, 0, TEST);
    TRY_ON(short, MAXINT16, TEST);
    TRY_ON(short, MININT16, TEST);
    TRY_ON(basis::un_int, 0, TEST);
    un_int max_u_int = MAXINT32 | MININT32;
    TRY_ON(basis::un_int, max_u_int, TEST);
    TRY_ON(basis::un_int, max_u_int - 1, TEST);
    TRY_ON(basis::un_int, max_u_int - 2, TEST);
    TRY_ON(basis::un_int, max_u_int - 3, TEST);
//    un_long max_u_long = MAXLONG | MINLONG;
//    TRY_ON(un_long, 0, TEST);
//    TRY_ON(un_long, max_u_long, TEST);
//    TRY_ON(un_long, max_u_long - 1, TEST);
//    TRY_ON(un_long, max_u_long - 2, TEST);
//    TRY_ON(un_long, max_u_long - 3, TEST);
    basis::un_short max_u_short = MAXINT16 | MININT16;
    TRY_ON(basis::un_short, 0, TEST);
    TRY_ON(basis::un_short, max_u_short, TEST);
    TRY_ON(basis::un_short, max_u_short - 1, TEST);
    TRY_ON(basis::un_short, max_u_short - 2, TEST);
    TRY_ON(basis::un_short, max_u_short - 3, TEST);
    #undef TEST
  }
  {
    #define TEST "third"
    // new bit for floating point packing.
    TRY_ON_F(double, 0.0, TEST);
    TRY_ON_F(double, 1.0, TEST);
    TRY_ON_F(double, -1.0, TEST);
    TRY_ON_F(double, 1.1, TEST);
    TRY_ON_F(double, -1.1, TEST);
    TRY_ON_F(double, 1983.293, TEST);
    TRY_ON_F(double, -1983.293, TEST);
    TRY_ON_F(double, 984.293e20, TEST);
    TRY_ON_F(double, -984.293e31, TEST);

    const int MAX_FLOAT_ITERS = 100;
    int iters = 0;
    while (iters++ < MAX_FLOAT_ITERS) {
      double dividend = randomizer().inclusive(1, MAXINT32 / 2);
      double divisor = randomizer().inclusive(1, MAXINT32 / 2);
      double multiplier = randomizer().inclusive(1, MAXINT32 / 2);
      double rand_float = (dividend / divisor) * multiplier;
      if (randomizer().inclusive(0, 1) == 1)
        rand_float = -1.0 * rand_float;
      TRY_ON_F(double, rand_float, "third--loop");
//log(a_sprintf("%f", rand_float));
    }
    #undef TEST
  }

  {
    #define TEST "fourth"
    // new test for char * packing.
    const char *tunnel_vision = "plants can make good friends.";
    const char *fresnel_lense = "chimney sweeps carry some soot.";
    const char *snoopy = "small white dog with black spots.";
    byte_array stored;
    int fregose = 38861;
    double perky_doodle = 3799.283e10;
    const char *emptyish = "";
    int jumboat = 998;
    // now stuff the array with some things.
    attach(stored, fregose);
    attach(stored, tunnel_vision);
    attach(stored, snoopy);
    attach(stored, perky_doodle);
    attach(stored, fresnel_lense);
    attach(stored, emptyish);
    attach(stored, jumboat);
    // time to restore those contents.
    astring tunnel_copy;
    astring fresnel_copy;
    astring snoopy_copy;
    int freg_copy;
    double perk_copy;
    astring emp_copy;
    int jum_copy;
    ASSERT_TRUE(detach(stored, freg_copy),
        GENERATE_TEST_NAME(TEST, "first int failed to unpack"));
    ASSERT_TRUE(detach(stored, tunnel_copy),
        GENERATE_TEST_NAME(TEST, "first string failed to unpack"));
    ASSERT_TRUE(detach(stored, snoopy_copy),
        GENERATE_TEST_NAME(TEST, "second string failed to unpack"));
    ASSERT_TRUE(detach(stored, perk_copy),
        GENERATE_TEST_NAME(TEST, "first double failed to unpack"));
    ASSERT_TRUE(detach(stored, fresnel_copy),
        GENERATE_TEST_NAME(TEST, "third string failed to unpack"));
    ASSERT_TRUE(detach(stored, emp_copy),
        GENERATE_TEST_NAME(TEST, "fourth string failed to unpack"));
    ASSERT_TRUE(detach(stored, jum_copy),
        GENERATE_TEST_NAME(TEST, "second int failed to unpack"));
    // now test contents.
    ASSERT_EQUAL(freg_copy, fregose,
        GENERATE_TEST_NAME(TEST, "first int had wrong contents"));
    ASSERT_EQUAL(tunnel_copy, astring(tunnel_vision),
        GENERATE_TEST_NAME(TEST, "first string had wrong contents"));
    ASSERT_EQUAL(snoopy_copy, astring(snoopy),
        GENERATE_TEST_NAME(TEST, "second string had wrong contents"));
    ASSERT_EQUAL(perk_copy, perky_doodle,
        GENERATE_TEST_NAME(TEST, "first double had wrong contents"));
    ASSERT_EQUAL(fresnel_copy, astring(fresnel_lense),
        GENERATE_TEST_NAME(TEST, "third string had wrong contents"));
    ASSERT_EQUAL(emp_copy, astring(emptyish),
        GENERATE_TEST_NAME(TEST, "fourth string had wrong contents"));
    ASSERT_EQUAL(jum_copy, jumboat,
        GENERATE_TEST_NAME(TEST, "second int had wrong contents"));
    ASSERT_FALSE(stored.length(),
        GENERATE_TEST_NAME(TEST, "array still had contents after detaching"));
    #undef TEST
  }

//  critical_events::alert_message("packable:: works for those functions tested.");
  return final_report();
}

//////////////

HOOPLE_MAIN(test_object_packing, );

