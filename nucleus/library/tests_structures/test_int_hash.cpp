/*
*  Name   : test_int_hash
*  Author : Chris Koeritz
*  Purpose:
*    Tests out hash_table specialization for integers.
**
* Copyright (c) 2001-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <application/hoople_main.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <loggers/console_logger.h>
#include <loggers/file_logger.h>
#include <mathematics/chaos.h>
#include <structures/int_hash.h>
#include <structures/static_memory_gremlin.h>
#include <textual/string_manipulation.h>
#include <timely/time_stamp.h>
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

//#define DEBUG_INT_HASH
  // uncomment for noisier run.

#define EXTREME_CHECKING
  // causes extra checks in the library code.

const double TEST_DURATION = 0.5 * SECOND_ms;

const int MAX_DEFAULT_BITS = 2;
  // we start low since we will rehash occasionally.

//////////////

enum test_actions {
  FIRST_TEST = 38,  // place-holder.
  ADD = FIRST_TEST,
    // adds an item that is probably new.
  ADD_ADD,
    // adds an item that is probably new, followed by another item under the
    // same key id.  this ensures the overwriting gets tested.
  ZAP,
    // finds an item we know is in the list and whacks it.
  ADD_ZAP,
    // adds a new item and immediately finds and zaps it.
  ZAP_ADD,
    // zaps an item that we know about and then adds a new item with the same
    // identifier.
  FIND,
    // locates an item in the list which we know should exist.
  ACQUIRE,
    // grabs an item out of the list (and tosses it).
  FIND_ZAP_ADD,
    // finds an item we know should exist, zaps it out of the list, then adds
    // a new item with the same id.
  ACQUIRE_ADD_ZAP,
    // removes an item from the list that we know should be there, adds it back
    // in, and then whacks it.
  FIND_ADD_FIND,
    // find an item with a particular id (one that we know should be in the
    // list) and then adds a different item using the same id.  the new item
    // is then sought.
  RESET,
    // tosses all data out of the hash table.  not done very often.
  CHECK_SANITY,
    // look for any problems or irregularities; print the contents of the list
    // if any are found.
  REHASH,
    // resizes the hash table.
  COPY,
    // copies a hash table to another hash table.
  LAST_TEST = COPY // place-holder; must equal test just prior.
};

//////////////

// a simple object that is used as the contents of the hash_table.

class data_shuttle
{
public:
  int food_bar;
  astring snacky_string;
  bool hungry;
  byte_array chunk;
  chaos chao;

  data_shuttle()
  : snacky_string(string_manipulation::make_random_name()),
    chunk(chao.inclusive(100, 10000)), food_bar(0), hungry(false) {}
};

//////////////

class test_int_hash : public virtual unit_base, virtual public application_shell
{
public:
  test_int_hash();

  int execute();
    //!< the main startup for the test.

  bool perform_a_test(test_actions test_type);
    //!< carries out the specifics of the "test_type".

  bool pick_a_test();
    //!< randomly picks one of the test types and performs it.

  DEFINE_CLASS_NAME("test_int_hash");

  static bool equivalence_applier(const int &key, data_shuttle &item, void *dlink);

  static const char *test_name(test_actions test_type);

  int raw_random_id();  //!< returns an unvetted random number.
  int unused_random_id();  //!< returns an unused (so far) random number.

  // these functions each perform one type of test, which their names indicate.
  bool test_add();
  bool test_add_add();
  bool test_zap();
  bool test_add_zap();
  bool test_zap_add();
  bool test_find();
  bool test_acquire();
  bool test_find_zap_add();
  bool test_acquire_add_zap();
  bool test_find_add_find();
  bool test_reset(bool always_do_it = false);
  bool test_check_sanity();
  bool test_copy();
  bool test_rehash();

private:
  int_set _keys_in_use;  // keys that we think are stored in the table.
  int_hash<data_shuttle> _the_table;  // our table under test.
  int _hits[LAST_TEST - FIRST_TEST + 1];  // tracks our testing activities.
  int _tested;  // simple counter of number of test calls.
};

//////////////

typedef int_hash<data_shuttle> our_hash;  // cleans up somewhat.

//////////////

test_int_hash::test_int_hash()
: application_shell(),
  _the_table(MAX_DEFAULT_BITS),
  _tested(0)
{
  for (int i = FIRST_TEST; i <= LAST_TEST; i++)
    _hits[i - FIRST_TEST] = 0;
}

int test_int_hash::execute()
{
  time_stamp exit_time((int)TEST_DURATION);
//log(astring("before starting tests"));
  while (time_stamp() < exit_time) {
    pick_a_test();
  }
  test_reset(true);  // make sure we do this at least once.
#ifdef DEBUG_INT_HASH
  log(a_sprintf("did %d tests.\n", _tested));
  log(astring("Test Activity:"));
  for (int i = 0; i < LAST_TEST - FIRST_TEST + 1; i++)
    log(astring(astring::SPRINTF, "%d (%s): %d hits", i + FIRST_TEST,
        test_name(test_actions(i + FIRST_TEST)), _hits[i]));
  log(a_sprintf("note that test %d will seldom be executed.", RESET));
#endif
  return final_report();
}

const char *test_int_hash::test_name(test_actions test_type)
{
  switch (test_type) {
    case ADD: return "ADD";
    case ADD_ADD: return "ADD_ADD";
    case ZAP: return "ZAP";
    case ADD_ZAP: return "ADD_ZAP";
    case ZAP_ADD: return "ZAP_ADD";
    case FIND: return "FIND";
    case ACQUIRE: return "ACQUIRE";
    case FIND_ZAP_ADD: return "FIND_ZAP_ADD";
    case ACQUIRE_ADD_ZAP: return "ACQUIRE_ADD_ZAP";
    case FIND_ADD_FIND: return "FIND_ADD_FIND";
    case RESET: return "RESET";
    case COPY: return "COPY";
    case REHASH: return "REHASH";
    case CHECK_SANITY: return "CHECK_SANITY";
    default: return "UnknownTest";
  }
}

bool test_int_hash::perform_a_test(test_actions test_type)
{
  FUNCDEF("perform_a_test");

//  log(astring(test_name(test_type)) + " ");

  switch (test_type) {
    case ADD: return test_add();
    case ADD_ADD: return test_add_add();
    case ZAP: return test_zap();
    case ADD_ZAP: return test_add_zap();
    case ZAP_ADD: return test_zap_add();
    case FIND: return test_find();
    case ACQUIRE: return test_acquire();
    case FIND_ZAP_ADD: return test_find_zap_add();
    case ACQUIRE_ADD_ZAP: return test_acquire_add_zap();
    case FIND_ADD_FIND: return test_find_add_find();
    case RESET: return test_reset();
    case COPY: return test_copy();
    case REHASH: return test_rehash();
    case CHECK_SANITY: return test_check_sanity();
    default:
      ASSERT_TRUE(false, "there should be no missing case seen!");
      return false;  // never gets here.
  }
}

bool test_int_hash::pick_a_test()
{
  _tested++;
  return perform_a_test(test_actions(randomizer().inclusive(FIRST_TEST, LAST_TEST)));
}

int test_int_hash::raw_random_id()
{
  return randomizer().inclusive(-MAXINT32 / 4, MAXINT32 / 4);
}

int test_int_hash::unused_random_id()
{
  while (true) {
    int checking = raw_random_id();
    if (!_keys_in_use.member(checking)) return checking;  // got one.
  } // keep going until we find unused id.
  return -1;
}

bool test_int_hash::test_add()
{
  FUNCDEF("test_add");
  _hits[ADD - FIRST_TEST]++;
  int random_id = raw_random_id();
  data_shuttle *to_add = new data_shuttle;
  to_add->snacky_string = string_manipulation::make_random_name();
  to_add->food_bar = random_id;
  outcome wanting = common::IS_NEW;
  if (_keys_in_use.member(random_id)) wanting = common::EXISTING;
  ASSERT_EQUAL(_the_table.add(random_id, to_add).value(), wanting.value(),
      "adding key should work with right expectation");
  if (_keys_in_use.member(random_id))
    return true;  // already was there so we replaced.
  _keys_in_use.add(random_id);
  return true;
}

//////////////

#undef UNIT_BASE_THIS_OBJECT
#define UNIT_BASE_THIS_OBJECT (*dynamic_cast<unit_base *>(application_shell::single_instance()))

int_hash<data_shuttle> *_hang_on = NULL_POINTER;
  // must be set before calling the apply method.

bool test_int_hash::equivalence_applier(const int &key, data_shuttle &item, void *dlink)
{
  FUNCDEF("equivalence_applier");
  ASSERT_TRUE(dlink, "should have been given name");
  if (!dlink) return false;  // fail.
  astring test_name = (char *)dlink;

//application_shell::single_instance()->log(astring("after name check"));

  data_shuttle *found = _hang_on->find(key);
  ASSERT_TRUE(found, test_name + ": should find equivalent entry in second list");
  if (!found) return false;  // bail or we'll crash.

//application_shell::single_instance()->log(astring("after finding"));

  ASSERT_EQUAL(item.food_bar, found->food_bar, test_name + ": food_bar should not differ");
  ASSERT_EQUAL(item.snacky_string, found->snacky_string, test_name + ": snacky_string should not differ");
  ASSERT_EQUAL(item.hungry, found->hungry, test_name + ": hungry should not differ");
  return true;
}

#undef UNIT_BASE_THIS_OBJECT
#define UNIT_BASE_THIS_OBJECT (*this)

//////////////

bool test_int_hash::test_rehash()
{
  FUNCDEF("test_rehash");
  // we don't want to rehash too often; it is expensive.
//  int maybe = randomizer().inclusive(1, 50);
//  if (maybe < 32) return true;  // not this time.

  _hang_on = &_the_table;  // must do this first.

  _hits[REHASH - FIRST_TEST]++;

  int_hash<data_shuttle> table_copy(_the_table.estimated_elements());

  _the_table.apply(equivalence_applier, (void *)func);
  astring second_test_name(func);
  second_test_name += " try 2";
  table_copy.apply(equivalence_applier, (void *)second_test_name.s());

  if (!_the_table.elements()) return true;  // nothing to do right now for comparisons.

//log("copying table...");
  copy_hash_table(table_copy, _the_table);
    // make a copy of the table.
  
  ASSERT_INEQUAL(0, table_copy.elements(), "copy shouldn't have unexpected absence of contents");

//log("rehashing table...");
  _the_table.rehash(randomizer().inclusive(1, 20));

//hmmm: need test of dehash function that reduces elements estimated.

//log("comparing table...");
  astring third_test_name(func);
  third_test_name += " try 3";
  table_copy.apply(equivalence_applier, (void *)third_test_name.s());
//log("done copy and compare.");

  return true;
}

bool test_int_hash::test_copy()
{
  FUNCDEF("test_copy");
  // we don't want to copy too often.  it's a heavy operation.
//  int maybe = randomizer().inclusive(1, 50);
//  if (maybe > 16) return true;  // not this time.

  _hang_on = &_the_table;  // must do this first.

  _hits[COPY - FIRST_TEST]++;

  int_hash<data_shuttle> table_copy(MAX_DEFAULT_BITS);

//log("copying table...");
  copy_hash_table(table_copy, _the_table);
    // make a copy of the table.
  
//log("comparing table...");
  table_copy.apply(equivalence_applier, (void *)func);
//log("done copy and compare.");

  return true;
}

//////////////

bool test_int_hash::test_add_add()
{
  FUNCDEF("test_add_add");
  _hits[ADD_ADD - FIRST_TEST]++;
  int random_id = unused_random_id();
  data_shuttle *to_add = new data_shuttle;
  to_add->snacky_string = string_manipulation::make_random_name();
  to_add->food_bar = random_id;
  ASSERT_EQUAL(_the_table.add(random_id, to_add).value(), common::IS_NEW, "key should be new");
  _keys_in_use.add(random_id);

  // second add on same id.
  data_shuttle *next_add = new data_shuttle;
  next_add->snacky_string = string_manipulation::make_random_name();
  next_add->food_bar = random_id;
  ASSERT_EQUAL(_the_table.add(random_id, next_add).value(), our_hash::EXISTING,
      "second add should not say first failed");

  return true;
}

bool test_int_hash::test_zap()
{
  FUNCDEF("test_zap");
  int maybe = randomizer().inclusive(1, 1000);
  if (maybe > 500) return true;
  if (!_keys_in_use.elements()) return false;  // can't do it yet.
  _hits[ZAP - FIRST_TEST]++;
  int rand_indy = randomizer().inclusive(0, _keys_in_use.elements() - 1);
  int dead_key = _keys_in_use[rand_indy];
  _keys_in_use.remove(dead_key);  // remove the record of that key.
  ASSERT_TRUE(_the_table.zap(dead_key), "zap should work on key");
  return true;
}

bool test_int_hash::test_add_zap()
{
  FUNCDEF("test_add_zap");
  // add.
  _hits[ADD_ZAP - FIRST_TEST]++;
  int random_id = unused_random_id();
  data_shuttle *to_add = new data_shuttle;
  to_add->snacky_string = string_manipulation::make_random_name();
  to_add->food_bar = random_id;
  ASSERT_EQUAL(_the_table.add(random_id, to_add).value(), common::IS_NEW, "key is new before zap");
  // zap.
  ASSERT_TRUE(_the_table.zap(random_id), "add then zap should remove the key");
  return true;
}

bool test_int_hash::test_zap_add()
{
  FUNCDEF("test_zap_add");
  if (!_keys_in_use.elements()) return false;  // can't do it yet.
  _hits[ZAP_ADD - FIRST_TEST]++;
  int rand_indy = randomizer().inclusive(0, _keys_in_use.elements() - 1);
  // in the end, the key list state won't be changed unless the test fails.
  int dead_key = _keys_in_use[rand_indy];
  ASSERT_TRUE(_the_table.zap(dead_key), "key should be there for zapping");

  data_shuttle *to_add = new data_shuttle;
  to_add->snacky_string = string_manipulation::make_random_name();
  to_add->food_bar = dead_key;
  outcome ret = _the_table.add(dead_key, to_add);
  ASSERT_EQUAL(ret.value(), our_hash::IS_NEW, "key should not already be present somehow");
  return true;
}

int functional_return(int to_pass) {
  return to_pass;
}

bool test_int_hash::test_find()
{
  FUNCDEF("test_find");
  if (!_keys_in_use.elements()) return false;  // can't do it yet.
  _hits[FIND - FIRST_TEST]++;
  int rand_indy = randomizer().inclusive(0, _keys_in_use.elements() - 1);
  int find_key = _keys_in_use[rand_indy];
  data_shuttle *found = NULL_POINTER;
  ASSERT_TRUE(_the_table.find(functional_return(find_key), found),
      "key should be there when we look");
  ASSERT_TRUE(_the_table.find(functional_return(find_key)), "find2: key be there when checked");
  ASSERT_TRUE(found, "when key is found contents should not be null");
  ASSERT_EQUAL(found->food_bar, find_key, "stored key should be same as real key");
  ASSERT_TRUE(found->snacky_string.length(), "stored string should have some length");
  return true;
}

bool test_int_hash::test_acquire()
{
  FUNCDEF("test_acquire");
  int maybe = randomizer().inclusive(1, 1000);
  if (maybe > 750) return true;
  if (!_keys_in_use.elements()) return false;  // can't do it yet.
  _hits[ACQUIRE - FIRST_TEST]++;
  int rand_indy = randomizer().inclusive(0, _keys_in_use.elements() - 1);
  int find_key = _keys_in_use[rand_indy];
  _keys_in_use.remove(find_key);  // remove the record of that key.
  data_shuttle *found = _the_table.acquire(find_key);
  ASSERT_TRUE(found, "key should be there like clockwork");
  ASSERT_EQUAL(found->food_bar, find_key, "stored key should be same as real key");
  ASSERT_TRUE(found->snacky_string.length(), "stored string should have some length");
  WHACK(found);
  found = _the_table.acquire(find_key);
  ASSERT_FALSE(found, "key should be missing after zap");
  return true;
}

bool test_int_hash::test_find_zap_add()
{
  FUNCDEF("test_find_zap_add");
  // find.
  if (!_keys_in_use.elements()) return false;  // can't do it yet.
  _hits[FIND_ZAP_ADD - FIRST_TEST]++;
  int rand_indy = randomizer().inclusive(0, _keys_in_use.elements() - 1);
  // this is another key list invariant function, if it works.
  int find_key = _keys_in_use[rand_indy];
  data_shuttle *found = NULL_POINTER;
  ASSERT_TRUE(_the_table.find(find_key, found), "key should be there when sought");
  ASSERT_TRUE(_the_table.find(find_key), "find2: key should be there for us to find");
  ASSERT_TRUE(found, "found key should have non-null contents");
  ASSERT_EQUAL(found->food_bar, find_key, "stored key should have no differences from real key");
  ASSERT_TRUE(found->snacky_string.length(), "stored string should have non-zero length");
  // zap.
  ASSERT_TRUE(_the_table.zap(find_key), "should be able to zap the item we had found");
  // add.
  data_shuttle *to_add = new data_shuttle;
  to_add->snacky_string = string_manipulation::make_random_name();
  to_add->food_bar = find_key;
  ASSERT_EQUAL(_the_table.add(find_key, to_add).value(), our_hash::IS_NEW,
      "the item we zapped should not still be there");
  return true;
}

bool test_int_hash::test_reset(bool always_do_it)
{
  FUNCDEF("test_reset");
  if (!always_do_it) {
    int maybe = randomizer().inclusive(1, 1000);
    // this is hardly ever hit, but it loses all contents too often otherwise.
    if ( (maybe > 372) || (maybe < 368) ) return true;
  }

  // we hit the big time; we will reset now.
  _hits[RESET - FIRST_TEST]++;
  _the_table.reset();
  for (int i = _keys_in_use.elements() - 1; i >= 0; i--) {
    int dead_key = _keys_in_use[i];
    ASSERT_FALSE(_the_table.acquire(dead_key), "after reset, we should not find an item");
    _keys_in_use.remove(dead_key);
  }
  return true;
}

//hmmm: implement these tests!

bool test_int_hash::test_acquire_add_zap()
{
  _hits[ACQUIRE_ADD_ZAP - FIRST_TEST]++;
return false;
}

bool test_int_hash::test_find_add_find()
{
  _hits[FIND_ADD_FIND - FIRST_TEST]++;
return false;
}

bool test_int_hash::test_check_sanity()
{
  _hits[CHECK_SANITY - FIRST_TEST]++;
return false;
}

//////////////

HOOPLE_MAIN(test_int_hash, )

