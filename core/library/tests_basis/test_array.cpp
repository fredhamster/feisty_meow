/*****************************************************************************\
*                                                                             *
*  Name   : test_array                                                        *
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

//#define DEBUG_ARRAY
  // when this flag is turned on, extra checking will be done in the allocator.

#include <application/hoople_main.h>
#include <basis/array.h>
#include <basis/byte_array.h>
#include <basis/enhance_cpp.h>
#include <basis/functions.h>
#include <basis/astring.h>
#include <loggers/console_logger.h>
#include <loggers/critical_events.h>
#include <mathematics/chaos.h>
#include <mathematics/double_plus.h>
#include <timely/time_stamp.h>
#include <unit_test/unit_base.h>

///#include <stdio.h>//temp

using namespace application;
using namespace basis;
using namespace loggers;
using namespace mathematics;
using namespace timely;
using namespace unit_test;

//const float MAX_TEST_DURATION_ms = 28 * SECOND_ms;
const float MAX_TEST_DURATION_ms = 200;
  // each of the tests calling on the templated tester will take this long.

const int MAX_SIMULTANEOUS_OBJECTS = 42;  // the maximum arrays tested.

const int MIN_OBJECT = -30; // the smallest array we'll create.
const int MAX_OBJECT = 98; // the largest array we'll create.

const int MIN_BLOCK = 100;  // the smallest exemplar we'll use.
const int MAX_BLOCK = MAX_OBJECT * 2;  // the largest exempler.

//#define DEBUG_TEST_ARRAY
  // uncomment for noisy version.

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

static chaos a_randomizer;

//////////////

class test_array : public application_shell, public unit_base
{
public:
  test_array()
  : application_shell(),
    unit_base()
  {
//hmmm: should go into app shell
////SETUP_CONSOLE_LOGGER;
}
  DEFINE_CLASS_NAME("test_array");
  virtual int execute();
  void dump_array(array<void *> &ar, const char *name);
  void test_arrays_of_void_pointer();
  void test_iteration_speed();

  template <class contents>
  void array_tester(test_array &ta, const contents &formal(bogus), basis::un_short flags);

};

//////////////

void test_array::dump_array(array<void *> &ar, const char *name)
{
#ifdef DEBUG_TEST_ARRAY
  FUNCDEF("dump_array");
  LOG(a_sprintf("array named \"%s\" has:", name)); 
  for (int i = 0; i < ar.length(); i++) {
    LOG(a_sprintf("\t%4d: %d", i, (int)ar[i]));
  }
#else
  if (ar.length() && name) {}
#endif
}

void test_array::test_arrays_of_void_pointer()
{
  FUNCDEF("void pointer test");
  const int MAX_VOID_ARRAY = 20;
  array<void *> argh(MAX_VOID_ARRAY, NIL, byte_array::SIMPLE_COPY
      | byte_array::EXPONE | byte_array::FLUSH_INVISIBLE);
  array<void *> argh2(argh);
  ASSERT_EQUAL(argh.length(), MAX_VOID_ARRAY, "check first array length");
  ASSERT_EQUAL(argh2.length(), MAX_VOID_ARRAY, "check copied array length");
  int wrong_counter = 0;
  for (int o = 0; o < MAX_VOID_ARRAY; o++)
    if (argh[o] != argh2[o]) wrong_counter++;
  ASSERT_EQUAL(wrong_counter, 0, "compare array contents");

  // fill it with values.
  int starter;
  for (int i = 0; i < MAX_VOID_ARRAY; i++)
    argh[i] = (void *)(&starter + i);
  dump_array(argh, "first version");

  // check the values.
  wrong_counter = 0;
  for (int j = 0; j < MAX_VOID_ARRAY; j++)
    if (argh[j] != (void *)(&starter + j) ) wrong_counter++;
  ASSERT_EQUAL(wrong_counter, 0, "compare values that were set");

  // add a constant to the values.
  for (int k = 0; k < MAX_VOID_ARRAY; k++)
    argh[k] = (void *)((int *)argh[k] + 23);
  dump_array(argh, "second version");

  // check assignment.
  argh2 = argh;
  wrong_counter = 0;
  for (int n = 0; n < MAX_VOID_ARRAY; n++)
    if (argh2[n] != (void *)(&starter + n + 23)) wrong_counter++;
  ASSERT_EQUAL(wrong_counter, 0, "compare values that were assigned");

  // now test that values are kept in place after rearrangement.
  argh.zap(3, 4);
  dump_array(argh, "third version");
  wrong_counter = 0;
  for (int l = 0; l < 3; l++)
    if (argh[l] != (void *)(&starter + l + 23)) wrong_counter++;
  ASSERT_EQUAL(wrong_counter, 0, "zap low values test");
  wrong_counter = 0;
  for (int m = 3; m < MAX_VOID_ARRAY - 2; m++)
    if (argh[m] != (void *)(&starter + m + 2 + 23)) wrong_counter++;
  ASSERT_EQUAL(wrong_counter, 0, "zap high values test");
//hmmm: add more tests if void pointer arrays seem in question.
}

//////////////

static astring blank_string;

// jethro is a simple object for containment below.
class jethro
{
public:
  jethro(const astring &i = blank_string) : _truck(i) {}

  astring _truck;

  bool operator ==(const jethro &tc) const { return tc._truck == _truck; }
};

//////////////

// the test_content object is an object with proper copy constructor
// and assignment operator that also has deep contents.
class test_content
{
public:
  abyte _q;
  astring _ted;
  astring _jed;
  int_array _ned;

  test_content(abyte q = 3)
  : _q(q), _ted("bl"), _jed("orp"),
    _ned(12, NIL)
/*    _med(3, 2),
    _red(2, 4) */
  {
    for (int i = 0; i < _ned.length(); i++)
      _ned[i] = -i;
/*
    for (int r = 0; r < _med.rows(); r++)
      for (int c = 0; c < _med.columns(); c++)
        _med[r][c] = jethro(astring((r*c) % 256, 1));
    for (int j = 0; j < _red.rows(); j++)
      for (int k = 0; k < _red.columns(); k++)
        _red[j][k] = j * k;
*/
  }

  bool operator ==(const test_content &tc) const {
    if (tc._q != _q) return false;
    if (tc._ted != _ted) return false;
    if (tc._jed != _jed) return false;
    if (tc._ned.length() != _ned.length()) return false;
    for (int i = 0; i < _ned.length(); i++)
      if (tc._ned[i] != _ned[i]) return false;
    
/*
    if (tc._med.rows() != _med.rows()) return false;
    if (tc._med.columns() != _med.columns()) return false;
    for (int c = 0; c < _med.columns(); c++)
      for (int r = 0; r < _med.rows(); r++)
        if (tc._med.get(r, c) != _med.get(r, c)) return false;

    if (tc._red.rows() != _red.rows()) return false;
    if (tc._red.columns() != _red.columns()) return false;
    for (int j = 0; j < _red.rows(); j++)
      for (int k = 0; k < _red.columns(); k++)
        if (tc._red.get(j, k) != _red.get(j, k)) return false;
*/

    return true;
  }

  operator abyte() const { return _q; }
};

//////////////

template <class contents>
bool compare_arrays(const array<contents> &a, const array<contents> &b)
{
  if (a.length() != b.length()) return false;
  for (int i = 0; i < a.length(); i++)
    if (a[i] != b[i]) return false;
  return true;
}

//////////////

// this is a templated test for arrays, with some requirements.  the contents
// object must support a constructor that takes a simple byte, whether
// that's meaningful for the object or not.  if your type to test doesn't
// have that, derive a simple object from it, give it that constructor, and
// then it can be used below.  the object must also support comparison with
// == and != for this test to be used.  it must also provide a conversion
// to integer that returns the value passed to the constructor.

template <class contents>
void test_array::array_tester(test_array &ta, const contents &formal(bogus), basis::un_short flags)
{
  FUNCDEF("array_tester");
  // the space that all training for arrays comes from.
  contents *junk_space = new contents[MAX_OBJECT + MAX_BLOCK];
  for (int i = 0; i < MAX_OBJECT - 1; i++)
    junk_space[i] = contents(a_randomizer.inclusive('a', 'z'));
  junk_space[MAX_OBJECT + MAX_BLOCK - 1] = '\0';

  array<contents> *testers[MAX_SIMULTANEOUS_OBJECTS];
  for (int c = 0; c < MAX_SIMULTANEOUS_OBJECTS; c++) {
    // set up the initial array guys.
    testers[c] = new array<contents>(a_randomizer.inclusive(MIN_OBJECT, MAX_OBJECT),
        NIL, flags);
    // copy the randomized junk space into the new object.
    for (int i = 0; i < testers[c]->length(); i++)
      testers[c]->put(i, junk_space[i]);
  }

  // these are the actions we try out with the array during the test.
  // the first and last elements must be identical to the first and last
  // tests to perform.
  enum actions { first, do_train = first, do_size, do_assign,
      do_access, do_zap, do_resizer, do_shrink, do_reset, do_indices,
      do_concatenating, do_concatenating2, do_subarray, do_insert,
      do_overwrite, do_stuff, do_memory_paring, do_snarf, do_flush,
      do_observe, last = do_observe };

  time_stamp exit_time(MAX_TEST_DURATION_ms);
  while (time_stamp() < exit_time) {
    int index = a_randomizer.inclusive(0, MAX_SIMULTANEOUS_OBJECTS - 1);
    int choice = a_randomizer.inclusive(first, last);
    switch (choice) {
      case do_train: {
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_train"));
#endif
        int new_size = a_randomizer.inclusive(MIN_OBJECT, MAX_OBJECT);
        testers[index]->retrain(new_size, (contents *)junk_space);
        int wrong_counter = 0;
        for (int i = 0; i < new_size; i++)
          if (junk_space[i] != (*testers[index])[i]) wrong_counter++;
        ASSERT_EQUAL(wrong_counter, 0, "test contents after retrain");
        break;
      }
      case do_size: {
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_size"));
#endif
        array<contents> old_version = *testers[index];
        bool at_front = bool(a_randomizer.inclusive(0, 1));
        int new_size = a_randomizer.inclusive(MIN_OBJECT, MAX_OBJECT);
        bool smaller = new_size < old_version.length();
        int difference = absolute_value(new_size - old_version.length());
        testers[index]->resize(new_size,
            at_front? old_version.NEW_AT_BEGINNING
                : old_version.NEW_AT_END);
        if (!smaller && difference) {
          // neuter the contents in the new section so we can compare.  this
          // space is filled with whatever the object's constructor chooses.
          if (at_front) {
            for (int i = 0; i < difference; i++)
              testers[index]->put(i, 'Q');
          } else {
            for (int i = old_version.length();
                i < old_version.length() + difference; i++)
              testers[index]->put(i, 'Q');
          }
        }
        // now compute an equivalent form of what the state should be.
        array<contents> equivalent(0, NIL, flags);
        if (at_front) {
          if (smaller) {
            equivalent = old_version.subarray(difference,
                old_version.length() - 1);
          } else {
            array<contents> blank(difference, NIL, flags);
            for (int i = 0; i < blank.length(); i++)
              blank[i] = 'Q';
            equivalent = blank + old_version;
          }
        } else {
          if (smaller) {
            equivalent = old_version.subarray(0, old_version.length()
                - difference - 1);
          } else {
            array<contents> blank(difference, NIL, flags);
            for (int i = 0; i < blank.length(); i++)
              blank[i] = 'Q';
            equivalent = old_version + blank;
          }
        }
        ASSERT_EQUAL(equivalent.length(), testers[index]->length(),
            "check length of resized form");
        ASSERT_TRUE(compare_arrays(*testers[index], equivalent),
            "check contents of resized form");
        break;
      }
      case do_assign: {
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_assign"));
#endif
        array<contents> arrh = *testers[index];  // copy old value.
        int to_assign = a_randomizer.inclusive(0, MAX_SIMULTANEOUS_OBJECTS - 1);
        *testers[index] = *testers[to_assign];
        ASSERT_TRUE(compare_arrays(*testers[index], *testers[to_assign]),
            "check result of assign copying array");
        *testers[to_assign] = arrh;  // recopy contents to new place.
        ASSERT_TRUE(compare_arrays(*testers[to_assign], arrh),
            "check result of second assign");
        break;
      }
      case do_access: {
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_access"));
#endif
        int start = a_randomizer.inclusive(0, testers[index]->length());
        int end = a_randomizer.inclusive(0, testers[index]->length());
        flip_increasing(start, end);
        array<contents> accumulator(0, NIL, flags);
        for (int i = start; i < end; i++) {
          contents c = contents(a_randomizer.inclusive(1, 255));
          testers[index]->access()[i] = c;
          accumulator += c;
        }
        for (int j = start; j < end; j++)
          ASSERT_EQUAL(accumulator[j - start], (*testers[index])[j],
              "comparison accessing at index must be equal");
        break;
      }
      case do_indices: {
#ifndef CATCH_ERRORS  // only works if not catching errors.
  #ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_indices"));
  #endif
        // does some tests on bad indices.
        contents c1 = testers[index]->operator[](-50);
        contents c2 = testers[index]->operator[](-MAX_OBJECT);
        bool test = (c1 == c2);
        ASSERT_TRUE(test, "invalid values should be the same");
        int tests = a_randomizer.inclusive(100, 500);
        for (int i = 0; i < tests; i++) {
          int indy = a_randomizer.inclusive(-1000, MAX_OBJECT * 3);
          // testing if we can access without explosions.
          contents c3 = testers[index]->operator[](indy);
          contents c4 = c3;
          ASSERT_EQUAL(c3, c4, "for do_indices, values should be the same");
        }
#endif
        break;
      }
      case do_observe: {
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_observe"));
#endif
        // tests contents returned by observe.
        int start = a_randomizer.inclusive(0, testers[index]->length());
        int end = a_randomizer.inclusive(0, testers[index]->length());
        flip_increasing(start, end);
        double total_1 = 0;
        for (int i = start; i < end; i++)
          total_1 += testers[index]->observe()[i];
        double total_2 = 0;
        for (int j = end - 1; j >= start; j--)
          total_2 += testers[index]->observe()[j];
        ASSERT_EQUAL(total_1, total_2, "totals should match up");
        break;
      }
      case do_resizer: {
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_resizer"));
#endif
        // tests whether the array will reuse space when it should be able to.
        array<contents> &arrh = *testers[index];
        // fill with known data.
        int i;
        for (i = 0; i < arrh.length(); i++)
          arrh[i] = contents((i + 23) % 256);
        // record the old starting point.
        const contents *start = arrh.internal_block_start();
        int zap_amount = a_randomizer.inclusive(1, arrh.length() - 1);
        // now take out a chunk from the array at the beginning.
        arrh.zap(0, zap_amount - 1);
        // test the contents.
        for (i = 0; i < arrh.length(); i++)
          ASSERT_EQUAL(arrh[i], contents((i + 23 + zap_amount) % 256),
              "the resized form should have same contents after zap");
        // now add back in the space we ate.
        arrh.resize(arrh.length() + zap_amount, arrh.NEW_AT_END);
        // check the pointer; it should not have changed.
        ASSERT_EQUAL(start, arrh.internal_block_start(),
            "the resized form should have same start address");
        // test the contents again.  they should start at zero and have the
        // same zap_amount offset.  the stuff past the original point shouldn't
        // be tested since we haven't set it.
        for (i = 0; i < arrh.length() - zap_amount; i++)
          ASSERT_EQUAL(arrh[i], contents((i + 23 + zap_amount) % 256),
              "the resized form should still have same contents");
        // now a test of all values through the zap_amount.
        arrh.zap(0, zap_amount - 1);
        for (i = 0; i < zap_amount; i++) {
          arrh.resize(arrh.length() + 1, arrh.NEW_AT_END);
          ASSERT_EQUAL(start, arrh.internal_block_start(),
              "the slowly resized form should have same start address");
        }
        // test the contents once more.  they should start at zero and have
        // double the zap_amount offset.  the stuff past the original point
        // shouldn't be tested since we haven't set it.
        for (i = 0; i < arrh.length() - 2 * zap_amount; i++)
          ASSERT_EQUAL(arrh[i], contents((i + 23 + 2 * zap_amount) % 256),
              "the slowly resized form should have same contents");

        // the tests below should be nearly identical to the ones above, but
        // they use the NEW_AT_BEGINNING style of copying.

        // fill with known data.
        for (i = 0; i < arrh.length(); i++)
          arrh[i] = contents((i + 23) % 256);
        // record the old starting point.
        start = arrh.internal_block_start();
        zap_amount = a_randomizer.inclusive(1, arrh.length() - 1);
        // now take out a chunk from the array at the beginning.
        arrh.zap(0, zap_amount - 1);
        // test the contents.
        for (i = 0; i < arrh.length(); i++)
          ASSERT_EQUAL(arrh[i], contents((i + 23 + zap_amount) % 256),
              "the resized form with known data should have right contents");
        // now add back in the space we ate.
        arrh.resize(arrh.length() + zap_amount,
            arrh.NEW_AT_BEGINNING);
        // check the pointer; it should not have changed.
        ASSERT_EQUAL(start, arrh.internal_block_start(),
            "the resized form with known data should have same start address");
        // test the contents again.  they should start at zap_amount but have
        // the same zap_amount offset.  the stuff before the original point
        // shouldn't be tested since we haven't set it.
        for (i = zap_amount; i < arrh.length(); i++)
          ASSERT_EQUAL(arrh[i], contents((i + 23) % 256),
              "the known data resized form should have same contents");
        // now a test of all values through the zap_amount.
        arrh.zap(0, zap_amount - 1);
        for (i = 0; i < zap_amount; i++) {
          arrh.resize(arrh.length() + 1, arrh.NEW_AT_BEGINNING);
          ASSERT_EQUAL(start, arrh.internal_block_start(),
              "the known data slowly resized form should have same start address");
        }
        // test the contents once more.  the zap_amount offset should stay the
        // same since we clobbered the place we added originally, then added
        // more space in.  so we skip the first part still.
        for (i = zap_amount; i < arrh.length(); i++)
          ASSERT_EQUAL(arrh[i], contents((i + 23) % 256),
              "the known data slowly resized form should have same contents");
        break;
      }
      case do_zap: {
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_zap"));
#endif
        int start;
        int end;
        bool erroneous = false;
        int chose = a_randomizer.inclusive(1, 100);
        if (chose <= 90) {
          // there's a ninety percent chance we pick a range that's valid.
          start = a_randomizer.inclusive(0, testers[index]->length() - 1);
          end = a_randomizer.inclusive(0, testers[index]->length() - 1);
        } else if (chose <= 95) {
          // and a 5 percent chance we pick to choose the zero index as our
          // start; this gets at the code for fast zapping.
          start = 0;
          end = a_randomizer.inclusive(0, testers[index]->length() - 1);
        } else {
          // and a 5 percent chance we'll allow picking a bad index.  the
          // actual chance of picking a bad one is less.
          erroneous = true;
          start = a_randomizer.inclusive(-2, testers[index]->length() + 3);
          end = a_randomizer.inclusive(-2, testers[index]->length() + 3);
        }
        flip_increasing(start, end);
        array<contents> old_version = *testers[index];
        testers[index]->zap(start, end);
        if (!erroneous) {
          array<contents> old_head = old_version.subarray(0, start - 1);
          array<contents> old_tail = old_version.subarray(end + 1,
              old_version.length() - 1);
          array<contents> equivalent = old_head + old_tail;
          ASSERT_EQUAL(equivalent.length(), testers[index]->length(),
              "the zapped form should not have erroneous length");
          ASSERT_TRUE(compare_arrays(*testers[index], equivalent),
              "the zapped form should not have erroneous contents");
        }
        break;
      }
      case do_reset: {
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_reset"));
#endif
        int junk_start = a_randomizer.inclusive(MIN_BLOCK, MAX_BLOCK);
        int junk_end = a_randomizer.inclusive(MIN_BLOCK, MAX_BLOCK);
        flip_increasing(junk_start, junk_end);
        int len = junk_end - junk_start + 1;
        testers[index]->reset(len, (contents *)&junk_space[junk_start]);
        ASSERT_EQUAL(testers[index]->length(), len, "reset should have proper length");
        for (int i = 0; i < len; i++)
          ASSERT_EQUAL(testers[index]->get(i), junk_space[junk_start + i],
              "reset should copy data");
        break;
      }
      case do_shrink: {
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_shrink"));
#endif
        array<contents> garp = *testers[index];
        testers[index]->shrink();
        int new_diff = testers[index]->internal_real_length()
            - testers[index]->length();
        ASSERT_TRUE(compare_arrays(garp, *testers[index]), "shrink should keep contents");
        // now force a real shrinkage.
        if (testers[index]->length() < 5) continue;  // need some room.
        // pick an element to keep that is not first or last.
        int kept = a_randomizer.inclusive(1, testers[index]->last() - 1);
        // whack all but the lucky element.
        testers[index]->zap(kept + 1, testers[index]->last());
        testers[index]->zap(0, kept - 1);
        // shrink it again, now a big shrink.
        testers[index]->shrink();
        // check the difference now.
        new_diff = testers[index]->internal_real_length()
            - testers[index]->length();
        ASSERT_FALSE(new_diff > 1, "massive shrink size should be correct");

        // restore the original contents and block size.
        *testers[index] = garp;
        break;
      }
      case do_concatenating: {
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_concatenating"));
#endif
        for (int i = 0; i < a_randomizer.inclusive(1, 20); i++) {
          contents new_c = contents(a_randomizer.inclusive('a', 'z'));
          testers[index]->concatenate(new_c);
          ASSERT_EQUAL(new_c, testers[index]->get(testers[index]->last()),
              "value should be equal after concatenate");
        }
        int indy = a_randomizer.inclusive(0, MAX_SIMULTANEOUS_OBJECTS - 1);
        array<contents> flirpan = *testers[indy];
        int start = a_randomizer.inclusive(0, flirpan.length() - 1);
        int end = a_randomizer.inclusive(0, flirpan.length() - 1);
        flip_increasing(start, end);
        flirpan = flirpan.subarray(start, end);
        array<contents> grumzor = *testers[index];  // old copy.
        testers[index]->concatenate(flirpan);
        array<contents> bubula = grumzor + flirpan;
        ASSERT_TRUE(compare_arrays(bubula, *testers[index]),
            "contents should be correct after concatenate or concatenation");
        contents first_value;
        contents second_value;
        if (testers[index]->length() >= 1)
          first_value = testers[index]->get(0);
        if (testers[index]->length() >= 2)
          second_value = testers[index]->get(1);
        const int max_iters = a_randomizer.inclusive(1, 42);
        for (int j = 0; j < max_iters; j++) {
          contents new_c = contents(a_randomizer.inclusive('a', 'z'));
          *testers[index] = *testers[index] + new_c;
          // correct our value checks if new indices became available.
          if (testers[index]->length() == 1) {
            first_value = testers[index]->get(0);
          } else if (testers[index]->length() == 2) {
            second_value = testers[index]->get(1);
          }

          ASSERT_EQUAL(new_c, testers[index]->get(testers[index]->last()),
              "value should not be wrong after concatenation");

          ASSERT_FALSE((testers[index]->length() >= 1) && (first_value != testers[index]->get(0)),
              "first value should not be corrupted");
          ASSERT_FALSE((testers[index]->length() >= 2) && (second_value != testers[index]->get(1)),
              "second value should not be corrupted");

          *testers[index] += new_c;
          // we don't have to correct the first value any more, but might have
          // to correct the second.
          if (testers[index]->length() == 2) {
            second_value = testers[index]->get(1);
          }
          ASSERT_EQUAL(new_c, testers[index]->get(testers[index]->last()),
              "value should be right after second concatenation");
          ASSERT_FALSE( (testers[index]->length() >= 1) && (first_value != testers[index]->get(0)),
              "first value should be uncorrupted after second concat");
          ASSERT_FALSE((testers[index]->length() >= 2) && (second_value != testers[index]->get(1)),
              "second value should be uncorrupted after second concat");
        }
        break;
      }
      case do_concatenating2: {
        // this tests the new concatenate content array method for array.
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_concatenating2"));
#endif
        array<contents> &flirpan = *testers[index];
        int new_len = a_randomizer.inclusive(0, 140);
        contents *to_add = new contents[new_len];
        for (int i = 0; i < new_len; i++) {
          int rando = a_randomizer.inclusive(0, MAX_BLOCK - 1);
          to_add[i] = junk_space[rando];
        }
        int old_len = flirpan.length();
        flirpan.concatenate(to_add, new_len);
        for (int i = 0; i < new_len; i++) {
          ASSERT_EQUAL(flirpan[i + old_len], to_add[i],
              "value should not be wrong after content array concatenation");
        }
        delete [] to_add;  // clean up.
        break;
      }
      case do_subarray: {
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_subarray"));
#endif
        array<contents> flirpan = *testers[index];
        int start = a_randomizer.inclusive(0, flirpan.length() - 1);
        int end = a_randomizer.inclusive(0, flirpan.length() - 1);
        flip_increasing(start, end);
        flirpan = flirpan.subarray(start, end);
        for (int i = 0; i < end - start; i++)
          ASSERT_EQUAL(flirpan[i], testers[index]->get(i + start),
              "subarray should produce right array");
        break;
      }
      case do_memory_paring: {
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_memory_paring"));
#endif
        for (int i = 0; i < MAX_SIMULTANEOUS_OBJECTS; i++) {
          // zap extra junk off so we bound the memory usage.
          if (testers[i]->length() > MAX_OBJECT) {
            testers[i]->zap(MAX_OBJECT, testers[i]->length() - 1);
            testers[i]->shrink();
          }
        }
        break;
      }
      case do_snarf: {
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_snarf"));
#endif
        array<contents> flirpan = *testers[index];
//        int start = a_randomizer.inclusive(0, flirpan.length() - 1);
//        int end = a_randomizer.inclusive(0, flirpan.length() - 1);
//        flip_increasing(start, end);
//        flirpan = flirpan.subarray(start, end);
        int rand_index = a_randomizer.inclusive(0, MAX_SIMULTANEOUS_OBJECTS - 1);
        if (index == rand_index) continue;  // skip it; try again later.
        array<contents> nugwort = *testers[rand_index];
        // perform a swap between two of our arrays.
        array<contents> temp_hold(0, NIL, flags);
        temp_hold.snarf(*testers[index]);
        testers[index]->snarf(*testers[rand_index]);
        testers[rand_index]->snarf(temp_hold);
        // the copies should have flipped places now.  check them.
        ASSERT_EQUAL(flirpan.length(), testers[rand_index]->length(),
            "snarf needs to produce right length at A");
        for (int i = 0; i < flirpan.length(); i++)
          ASSERT_EQUAL(flirpan[i], testers[rand_index]->get(i),
              "snarf needs to produce right array at A");
        ASSERT_EQUAL(nugwort.length(), testers[index]->length(),
            "snarf needs to produce right length at B");
        for (int j = 0; j < nugwort.length(); j++)
          ASSERT_EQUAL(nugwort[j], testers[index]->get(j),
              "snarf needs to produce right array at B");
        break;
      }
      case do_flush: {
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_flush"));
#endif
        array<contents> flirpan = *testers[index];

///fix up it up in it.

///        flirpan.reset(


        break;
      }

      case do_insert: {
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_insert"));
#endif
        array<contents> hold = *testers[index];
        int where = a_randomizer.inclusive(0, hold.last());
        int how_many = a_randomizer.inclusive(0, 25);
        testers[index]->insert(where, how_many);
        for (int i = 0; i < where; i++)
          ASSERT_EQUAL(hold[i], testers[index]->get(i),
              "should have good contents on left after insert");
        for (int j = where + how_many; j < testers[index]->length(); j++)
          ASSERT_EQUAL(hold[j - how_many], testers[index]->get(j),
              "should have good contents on right after insert");
        break;
      }
      case do_overwrite: {
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_overwrite"));
#endif
        if (!testers[index]->length()) continue;
        array<contents> hold = *testers[index];
        int index2 = index;
        while (index2 == index)
          index2 = a_randomizer.inclusive(0, MAX_SIMULTANEOUS_OBJECTS - 1);
        array<contents> &hold2 = *testers[index2];
        if (!hold2.length()) continue;
        int write_indy = a_randomizer.inclusive(0, hold.last());
        int write_len = minimum(hold2.length(), (hold.length() - write_indy));
// LOG(a_sprintf("len1 = %d len2 = %d wrindy=%d wrlen=%d", hold.length(), hold2.length(), write_indy, write_len));
        outcome ret = testers[index]->overwrite(write_indy,
            *testers[index2], write_len);
        ASSERT_EQUAL(ret.value(), common::OKAY,
            astring("should not have had outcome=") + common::outcome_name(ret));
        for (int i = 0; i < write_indy; i++)
          ASSERT_EQUAL(hold[i], testers[index]->get(i),
              "should have good contents on left after overwrite");
        for (int j = write_indy; j < write_indy + write_len; j++)
          ASSERT_EQUAL(hold2[j - write_indy], testers[index]->get(j),
              "should have good contents in middle after overwrite");
        for (int k = write_indy + write_len; k < testers[index]->length(); k++)
          ASSERT_EQUAL(hold[k], testers[index]->get(k),
              "should have good contents on right after overwrite");
        break;
      }
      case do_stuff: {
#ifdef DEBUG_TEST_ARRAY
        LOG(a_sprintf("do_stuff"));
#endif
        array<contents> &hold = *testers[index];
        contents burgers[100];
        int stuff_len = a_randomizer.inclusive(0, hold.length());
        stuff_len = minimum(stuff_len, 100);
        outcome ret = hold.stuff(stuff_len, burgers);
        ASSERT_EQUAL(ret.value(), common::OKAY,
            astring("should not have had outcome=") + common::outcome_name(ret));
        for (int i = 0; i < stuff_len; i++)
          ASSERT_EQUAL(burgers[i], hold[i],
              "should have good contents after stuff");
        break;
      }
      default: {
        ASSERT_FALSE(true, "test cases should have no invalid choices!");
        break;
      }
    }
  }

  // clean up.
  delete [] junk_space;
  for (int d = 0; d < MAX_SIMULTANEOUS_OBJECTS; d++) delete testers[d];
}

//////////////

struct gerkin { int l; abyte *p; char *r; void *pffttt; };

gerkin borgia;

class foop
{
public:
  virtual ~foop() {}
  virtual gerkin *boorba() = 0;
};

class boop : public foop
{
public:
  virtual gerkin *boorba() { return &borgia; }
};

void test_array::test_iteration_speed()
{
  FUNCDEF("test_iteration_speed");
  const int MAX_CHUNK = 100000;
  const int MAX_REPS = 20;
  byte_array chunky(MAX_CHUNK);

  {
    time_stamp start;
    int sum = 0;
    for (int j = 0; j < MAX_REPS; j++) {
      for (int i = 0; i < MAX_CHUNK; i++) {
        sum += chunky[i];
      }
    }
    int duration = int(time_stamp().value() - start.value());

    a_sprintf message("iteration over %d elements took %d ms,\t"
        "or %f ms per 1000 iters.",
        MAX_CHUNK * MAX_REPS, duration,
        double(duration) / double(MAX_CHUNK * MAX_REPS) * 1000.0);
//base_logger &b = program_wide_logger::get();
    LOG(message);
  }

  {
    time_stamp start;
    int sum = 0;
    const abyte *head = chunky.observe();
    for (int j = 0; j < MAX_REPS; j++) {
      for (int i = 0; i < MAX_CHUNK; i++) {
        sum += head[i];
      }
    }
    int duration = int(time_stamp().value() - start.value());

    LOG(a_sprintf("less-safe iteration over %d elements took %d ms,\tor %f ms per 1000 iters.",
        MAX_CHUNK * MAX_REPS, duration,
        double(duration) / double(MAX_CHUNK * MAX_REPS) * 1000.0));
  }
  {
    time_stamp start;
    boop tester;
//    int sum = 0;
    for (int j = 0; j < MAX_REPS; j++) {
      for (int i = 0; i < MAX_CHUNK; i++) {
//        chunky.modus().sampler();
        tester.boorba();
      }
    }
    int duration = int(time_stamp().value() - start.value());

    LOG(a_sprintf("virtual-function-only over %d elements took %d ms,\tor %f ms per 1000 iters.",
        MAX_CHUNK * MAX_REPS, duration,
        double(duration) / double(MAX_CHUNK * MAX_REPS) * 1000.0));
  }
}

//////////////

int test_array::execute()
{
  FUNCDEF("execute");
#if 0
// if enabled for the abusive type tests, then allow this one...
  test_iteration_speed();
//  LOG(a_sprintf("did iteration test.")); 
#endif

  int_array checking_start;
  ASSERT_FALSE(checking_start.length(),
      "int_array should not have contents from empty constructor");

  double_plus bogus4 = float(12.32);
  array_tester<double_plus >(*this, bogus4,
      byte_array::SIMPLE_COPY | byte_array::EXPONE
          | byte_array::FLUSH_INVISIBLE);
//  LOG(a_sprintf("did float array test.")); 

  int bogus2 = 39;
  array_tester<int>(*this, bogus2,
      byte_array::SIMPLE_COPY | byte_array::EXPONE
          | byte_array::FLUSH_INVISIBLE);
//  LOG(a_sprintf("did int array test.")); 

  test_content bogus3(12);
  array_tester<test_content>(*this, bogus3, byte_array::EXPONE
      | byte_array::FLUSH_INVISIBLE);
//  LOG(a_sprintf("did test_content array test.")); 

  test_arrays_of_void_pointer();
//  LOG(a_sprintf("did void * array test.")); 

  abyte bogus1 = 'c';
  array_tester<abyte>(*this, bogus1,
      byte_array::SIMPLE_COPY | byte_array::EXPONE
          | byte_array::FLUSH_INVISIBLE);
//  LOG(a_sprintf("did byte array test.")); 

////  LOG("array:: works for those functions tested.");

  return final_report();
}

HOOPLE_MAIN(test_array, )

