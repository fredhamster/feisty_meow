//////////////
// Name   : test_packable_tree
// Author : Chris Koeritz
//////////////
// Copyright (c) 1992-$now By Author.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation:
//     http://www.gnu.org/licenses/gpl.html
// or under the terms of the GNU Library license:
//     http://www.gnu.org/licenses/lgpl.html
// at your preference.  Those licenses describe your legal rights to this
// software, and no other rights or warranties apply.
// Please send updates for this code to: fred@gruntose.com -- Thanks, fred.
//////////////

//! tests some critical properties for the packable tree.

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/enhance_cpp.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <filesystem/file_info.h>
#include <loggers/critical_events.h>
#include <loggers/file_logger.h>
#include <mathematics/chaos.h>
#include <nodes/packable_tree.h>
#include <structures/object_packers.h>
#include <structures/string_array.h>
#include <structures/static_memory_gremlin.h>
#include <textual/byte_formatter.h>
#include <textual/string_manipulation.h>
#include <timely/earth_time.h>
#include <timely/time_stamp.h>
#include <unit_test/unit_base.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace mathematics;
using namespace nodes;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

//#define DEBUG_PACKABLE_TREE
  // set this to enable debugging features of the string class.

//HOOPLE_STARTUP_CODE;

//#define DEBUG_PACKABLE_TREE_TEST
  // uncomment for testing version.

#define LOG(s) EMERGENCY_LOG(program_wide_logger::get(), s)

#define WHERE __WHERE__.s()

#define FUNKIT(str) basis::a_sprintf("%s: %s", func, basis::astring(str).s())

// test: reports an error if the condition evaluates to non-zero.
int compnum = 0;
const float TEST_RUNTIME_DEFAULT = .02 * MINUTE_ms;
  // the test, by default, will run for this long.

//////////////

class test_packable_tree : public application_shell, public unit_base
{
public:
  test_packable_tree() {}
  ~test_packable_tree() {}

  DEFINE_CLASS_NAME("test_packable_tree");

  virtual int execute() {
    run_test();
    return final_report();
  }

  void run_test();
};

HOOPLE_MAIN(test_packable_tree, )

//////////////

//! it's not the one tree (c).  this is just a derived packable_tree we can test with.

class many_tree : public packable_tree
{
public:
  many_tree(const file_info &inf) : c_inf(new file_info(inf)) {}

  virtual ~many_tree() { WHACK(c_inf); }

  file_info get_info() const { return *c_inf; }

  virtual int packed_size() const {
    return c_inf->packed_size();
  }

  virtual void pack(basis::byte_array &packed_form) const {
    c_inf->pack(packed_form);
  }

  virtual bool unpack(basis::byte_array &packed_form) {
    if (!c_inf->unpack(packed_form)) return false;
//other pieces?
    return true;
  }

private:
  file_info *c_inf;
};

//////////////

//! the factory that creates our special type of tree.

class tree_defacto : public packable_tree_factory
{
public:
  packable_tree *create() { return new many_tree(file_info()); }
};

//////////////

void test_packable_tree::run_test()
{
  FUNCDEF("run_test");

  const file_info farfle(filename("arf"), 2010);
  const file_info empty;
  const file_info snood(filename("wookie"), 8888);

  {
    // simple creation, packing, unpacking, destruction tests on a blank object.
    many_tree gruntcake(farfle);
    byte_array packed_form;
    int pack_guess = gruntcake.packed_size();
    gruntcake.pack(packed_form);
    ASSERT_EQUAL(pack_guess, packed_form.length(), FUNKIT("packed length is incorrect"));
    many_tree untbake_target(empty);
    ASSERT_TRUE(untbake_target.unpack(packed_form), FUNKIT("unpack operation failed"));
    ASSERT_EQUAL(untbake_target.get_info(), gruntcake.get_info(),
        FUNKIT("unpack had wrong contents"));
  }

  {
    // recursive packing tests...
    // first layer.
    many_tree *spork = new many_tree(farfle);
    many_tree *limpet = new many_tree(empty);
    many_tree *congo = new many_tree(snood);
    many_tree *dworkin = new many_tree(empty);
    many_tree *greep = new many_tree(farfle);
    // second layer.
    many_tree *flep = new many_tree(snood);
    many_tree *glug = new many_tree(empty);
    many_tree *aptitoot = new many_tree(farfle);
    // third layer.
    many_tree *grog = new many_tree(snood);
    // connect first to second.
    flep->attach(spork);
    flep->attach(limpet);
    glug->attach(congo);
    aptitoot->attach(dworkin);
    aptitoot->attach(greep);
    // connect second to third.
    grog->attach(flep);
    grog->attach(glug);
    grog->attach(aptitoot);

    // now recursively pack that bad boy three level tree.
    byte_array packed;
    int size_guess = grog->recursive_packed_size();
    grog->recursive_pack(packed);
    ASSERT_EQUAL(size_guess, packed.length(), "recursive_packed_size failed");
    tree_defacto factotum;
    packable_tree *unpacked = many_tree::recursive_unpack(packed, factotum);
    ASSERT_TRUE(unpacked, "recursive_unpack failed");
    ASSERT_TRUE(dynamic_cast<many_tree *>(unpacked), "recursive_unpack has wrong type");
    many_tree *survivor = dynamic_cast<many_tree *>(unpacked);

if (survivor) {
}

//compare trees?

  }

}

//////////////

