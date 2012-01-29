/*****************************************************************************\
*                                                                             *
*  Name   : test_bit_vector                                                   *
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

#include <application/hoople_main.h>
#include <basis/array.h>
#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <loggers/combo_logger.h>
#include <mathematics/chaos.h>
#include <structures/bit_vector.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

#include <memory.h>
#include <stdlib.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace mathematics;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger::get(), to_print)

#define MAX_TEST 100
#define FOOP_MAX 213

//////////////

class test_bit_vector : virtual public unit_base, virtual public application_shell
{
public:
  test_bit_vector() : unit_base() {}
  DEFINE_CLASS_NAME("test_bit_vector");
  virtual int execute();
};

HOOPLE_MAIN(test_bit_vector, );

//////////////

struct test_struct { basis::un_int store; int posn; int size; };

int test_bit_vector::execute()
{
  FUNCDEF("execute");
  SETUP_COMBO_LOGGER;

  const array<test_struct> unused;

  chaos randomizer;
  bit_vector foop(FOOP_MAX);

  for (int i = 0; i < MAX_TEST; i++) {
    // sets a random bit and finds that one.
    int rando = randomizer.inclusive(0, FOOP_MAX-1);
    foop.light(rando);
    int found = foop.find_first(true);
    ASSERT_EQUAL(found, rando, "find first locates first true");
    foop.clear(rando);

    foop.resize(FOOP_MAX);
    ASSERT_EQUAL(0, foop.find_first(0), "locating location of first zero");
    ASSERT_EQUAL(common::NOT_FOUND, foop.find_first(1), "showing there are no one bits");
    for (int i = 0; i < 12; i++) foop.light(i);
    ASSERT_EQUAL(12, foop.find_first(0), "finding first on partially set vector");

    foop.light(FOOP_MAX);  // shouldn't work, but shouldn't die.
    ASSERT_FALSE(foop.on(FOOP_MAX), "bit_on should not be lit past end of vector");

    // sets a bunch of random bits.
    for (int j = 0; j < 40; j++) {
      int rando = randomizer.inclusive(0, FOOP_MAX-1);
      foop.light(rando);
    }
    bit_vector foop2(FOOP_MAX, ((const byte_array &)foop).observe());
    ASSERT_EQUAL(foop, foop2, "after lighting, vectors should be identical");

    {
      // this block tests the subvector and int storage/retrieval routines.
      if (foop.bits() < 90) foop.resize(90);  // make sure we have room to play.

      array<test_struct> tests;
      test_struct t1 = { 27, 15, 5 };
      tests += t1;
      test_struct t2 = { 8, 25, 4 };
      tests += t2;
      test_struct t3 = { 1485, 34, 16 };
      tests += t3;
      test_struct t4 = { 872465, 50, 32 };
      tests += t4;

      for (int i = 0; i < tests.length(); i++) {
        ASSERT_TRUE(foop.set(tests[i].posn, tests[i].size, tests[i].store),
            "storing int in vector should work");

//hmmm: make this a test case!
//        bit_vector found = foop.subvector(tests[i].posn, tests[i].posn+tests[i].size-1);
//        LOG(astring(astring::SPRINTF, "contents found:\n%s", found.text_form().s()));

        basis::un_int to_check = foop.get(tests[i].posn, tests[i].size);
        if (to_check != tests[i].store)
          LOG(a_sprintf("int found at %d in vector (%u) is different than what was stored (%u).",
              i, to_check, tests[i].store));
        ASSERT_EQUAL((int)to_check, (int)tests[i].store, "should see expected int stored in vector");
      }
    }

    {
      // tests random resizings and resettings.
      int number_of_loops = randomizer.inclusive(50, 150);
      for (int i = 0; i < number_of_loops; i++) {
        int which_to_do = randomizer.inclusive(1, 3);
        switch (which_to_do) {
          case 1: {
            // resize.
            int new_size = randomizer.inclusive(0, 32000);
            foop.resize(new_size);
            break;
          }
          case 2: {
            // reset.
            int new_size = randomizer.inclusive(0, 32000);
            foop.reset(new_size);
            break;
          }
          case 3: {
            // random sets.
            int sets_to_do = randomizer.inclusive(40, 280);
            for (int i = 0; i < sets_to_do; i++) {
              int rando = randomizer.inclusive(0, foop.bits());
              if (randomizer.inclusive(0, 1)) foop.light(rando);
              else foop.clear(rando);
            }
            break;
          }
        }
      }
    }

    foop.reset(FOOP_MAX);  // to clear before next loop.
  }

  return final_report();
}

