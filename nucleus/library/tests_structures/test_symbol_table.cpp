/*
*  Name   : test_symbol_table
*  Author : Chris Koeritz
**
* Copyright (c) 1994-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <basis/byte_array.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <structures/matrix.h>
#include <structures/symbol_table.h>
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
using namespace mathematics;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

//#define DEBUG_SYMBOL_TABLE
  // uncomment for noisy version.

const int test_iterations = 4;

const int FIND_ITERATIONS = 10;
const int MAXIMUM_RANDOM_ADDS = 20;

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger::get(), astring(to_print))

//#define OLD_TEST
  // uncomment for the older version of symbol table.

double time_in_add = 0;
double time_in_dep_find = 0;
double time_in_new_find = 0;
double time_in_pack = 0;
double time_in_unpack = 0;
double time_in_copy = 0;

//////////////

class my_table_def : virtual public hoople_standard, virtual public symbol_table<byte_array>
///, virtual public equalizable
{
public:
  DEFINE_CLASS_NAME("my_table_def");

  virtual void text_form(base_string &state_fill) const {
    state_fill.assign(astring(class_name()) + ": uhhh not really implemented");
  }

  virtual bool equal_to(const equalizable &s2) const {
    const my_table_def *to_compare = dynamic_cast<const my_table_def *>(&s2);
    if (!to_compare) return false;
    if (symbols() != to_compare->symbols()) return false;
    for (int i = 0; i < symbols(); i++) {
      if (name(i) != to_compare->name(i)) return false;
      if (operator [](i) != (*to_compare)[i]) return false;
    }
    return true;
  }
};

//////////////

// jethro is a simple object for containment below.
class jethro
{
public:
  astring _truck;

  bool operator ==(const jethro &tc) const { return tc._truck == _truck; }
  bool operator !=(const jethro &tc) const { return !(*this == tc); }
};

//////////////

// the test_content object is an object with proper copy constructor
// and assignment operator that also has deep contents.
class test_content : public packable
{
public:
  int _q;
  astring _ted;
  astring _jed;
  int_array _ned;
  matrix<jethro> _med;

  test_content() : _q(9) {}

  test_content(const astring &ted, const astring &jed) : _q(4), _ted(ted),
          _jed(jed) {}

//hmmm: pack and unpack don't do everything.
  void pack(byte_array &packed_form) const {
    attach(packed_form, _q);
    _ted.pack(packed_form);
    _jed.pack(packed_form);
  }
  int packed_size() const {
    return sizeof(_q) + _ted.packed_size() + _jed.packed_size();
  }
  bool unpack(byte_array &packed_form) {
    if (!detach(packed_form, _q)) return false;
    if (!_ted.unpack(packed_form)) return false;
    if (!_jed.unpack(packed_form)) return false;
    return true;
  }

  bool operator ==(const test_content &tc) const {
    if (tc._q != _q) return false;
    if (tc._ted != _ted) return false;
    if (tc._jed != _jed) return false;
    if (tc._ned.length() != _ned.length()) return false;
    for (int i = 0; i < _ned.length(); i++)
      if (tc._ned[i] != _ned[i]) return false;
    
    if (tc._med.rows() != _med.rows()) return false;
    if (tc._med.columns() != _med.columns()) return false;
    for (int c = 0; c < _med.columns(); c++)
      for (int r = 0; r < _med.rows(); r++)
        if (tc._med.get(r, c) != _med.get(r, c)) return false;

    return true;
  }
  bool operator !=(const test_content &tc) const { return !operator ==(tc); }

  operator int() const { return _q; }
};

//////////////

class second_table_def : virtual public hoople_standard, virtual public symbol_table<test_content>
{
public:
  DEFINE_CLASS_NAME("second_table_def")

  virtual void text_form(base_string &state_fill) const {
    state_fill.assign(astring(class_name()) + ": uhhh not really implemented");
  }

  virtual bool equal_to(const equalizable &s2) const {
    const second_table_def *to_compare = dynamic_cast<const second_table_def *>(&s2);
    if (symbols() != to_compare->symbols()) return false;
    for (int i = 0; i < symbols(); i++) {
      if ((*this)[i] != (*to_compare)[i]) return false;
    }
    return true;
  }
};

//////////////

class test_symbol_table : public virtual application_shell, public virtual unit_base
{
public:
  test_symbol_table() {}
  DEFINE_CLASS_NAME("test_symbol_table");

  void test_1();

  virtual int execute();

  void ADD(my_table_def &syms, const astring &name, const astring &to_add);
  void FIND(const my_table_def &syms, const astring &name, const astring &to_find);

  void ADD2(second_table_def &syms, const astring &name, const test_content &to_add);

  void pack(byte_array &packed_form, const my_table_def &to_pack);
  bool unpack(byte_array &packed_form, my_table_def &to_unpack);

  void pack(byte_array &packed_form, const second_table_def &to_pack);
  bool unpack(byte_array &packed_form, second_table_def &to_unpack);

  void test_byte_table();
  void test_tc_table();
};

//////////////

void test_symbol_table::ADD(my_table_def &syms, const astring &name, const astring &to_add)
{
  FUNCDEF("ADD")
  byte_array to_stuff(to_add.length() + 1, (abyte *)to_add.s());
  time_stamp start;
  outcome added = syms.add(name, to_stuff);
  ASSERT_EQUAL(added.value(), common::IS_NEW, "should not already be in table");
  time_stamp end;
  time_in_add += end.value() - start.value();
  start.reset();
#ifdef OLD_TEST
  int indy = syms.find(name);
  ASSERT_FALSE(negative(indy), "should be in table after add");
  end.reset();
  time_in_dep_find += end.value() - start.value();
  const byte_array *found = &syms[indy];
#else
  byte_array *found = syms.find(name);
  ASSERT_TRUE(found, "really should be in table after add");
  end.reset();
  time_in_new_find += end.value() - start.value();
#endif
  ASSERT_EQUAL(*found, to_stuff, "value should be right in table after add");
}

void test_symbol_table::FIND(const my_table_def &syms, const astring &name, const astring &to_add)
{
  FUNCDEF("FIND")
  byte_array to_stuff(to_add.length() + 1, (abyte *)to_add.s());
  for (int i = 0; i < FIND_ITERATIONS; i++) {
    time_stamp start;
#ifdef OLD_TEST
    // double the calls so we roughly match the other test.
    int indy = syms.find(name);
    ASSERT_FALSE(negative(indy), "should locate item in table");
    indy = syms.find(name);
    ASSERT_FALSE(negative(indy), "second find should locate item in table");
    byte_array *found = &syms[indy];
    time_stamp end;
    time_in_dep_find += end.value() - start.value();
#else
    int indy = syms.dep_find(name);
    ASSERT_FALSE(negative(indy), "should locate item in table (dep_find)");
    time_stamp end;
    time_in_dep_find += end.value() - start.value();
    start.reset();
    byte_array *found = syms.find(name);
    ASSERT_TRUE(found, "second find should see item in table (new_find)");
    end.reset();
    time_in_new_find += end.value() - start.value();
#endif
  }
}

void test_symbol_table::pack(byte_array &packed_form, const my_table_def &to_pack)
{
  attach(packed_form, to_pack.symbols());
  astring name;
  byte_array content;
  for (int i = 0; i < to_pack.symbols(); i++) {
    to_pack.retrieve(i, name, content);
    name.pack(packed_form);
    attach(packed_form, content);
  }
}

bool test_symbol_table::unpack(byte_array &packed_form, my_table_def &to_unpack)
{
  to_unpack.reset();
  int syms = 0;
  if (!detach(packed_form, syms)) return false;
  astring name;
  byte_array chunk;
  for (int i = 0; i < syms; i++) {
    if (!name.unpack(packed_form)) return false;
    if (!detach(packed_form, chunk)) return false;
    ADD(to_unpack, name, (char *)chunk.observe());
  }
  return true;
}

void test_symbol_table::pack(byte_array &packed_form, const second_table_def &to_pack)
{
  attach(packed_form, to_pack.symbols());
  astring name;
  test_content content;
  for (int i = 0; i < to_pack.symbols(); i++) {
    to_pack.retrieve(i, name, content);
    name.pack(packed_form);
    content.pack(packed_form);
  }
}

bool test_symbol_table::unpack(byte_array &packed_form, second_table_def &to_unpack)
{
  to_unpack.reset();
  int syms = 0;
  if (!detach(packed_form, syms)) return false;
  astring name;
  test_content chunk;
  for (int i = 0; i < syms; i++) {
    if (!name.unpack(packed_form)) return false;
    if (!chunk.unpack(packed_form)) return false;
    to_unpack.add(name, chunk);
  }
  return true;
}

//////////////

my_table_def creatapose()
{
  my_table_def to_return;
  astring name;
  astring content;
  for (int y = 0; y < MAXIMUM_RANDOM_ADDS; y++) {
    name = string_manipulation::make_random_name(40, 108);
    content = string_manipulation::make_random_name(300, 1000);
    byte_array to_stuff(content.length() + 1, (abyte *)content.s());
    to_return.add(name, to_stuff);
  }
  return to_return;
}

//////////////

void test_symbol_table::test_byte_table()
{
  FUNCDEF("test_byte_table")
  my_table_def syms;
  my_table_def new_syms;
  my_table_def newer_syms;
  for (int qq = 0; qq < test_iterations; qq++) {
    syms.reset();  // still could be costly.
#ifdef DEBUG_SYMBOL_TABLE
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

    // test copying the table.
    time_stamp start;  // click, on.
    my_table_def copy1(syms);
    {
      my_table_def joe(copy1);
      my_table_def joe2 = joe;
      ASSERT_EQUAL(joe2, joe, "copy test A: symbol tables should be same");
      my_table_def joe3 = creatapose();  // on stack.
      my_table_def joe4 = joe3;
      my_table_def joe5 = joe4;
      ASSERT_EQUAL(joe5, joe3, "copy test A2: symbol tables should be same");
    }
    ASSERT_FALSE(! (syms == copy1), "copy test B: symbol tables should be same still");
    time_stamp end;
    time_in_copy += end.value() - start.value();

#ifdef DEBUG_SYMBOL_TABLE
////    LOG(astring(astring::SPRINTF,"This is the symbol table before any manipulation\n%s", syms.text_form()));
    LOG("now packing the symbol table...");
#endif

#ifdef DEBUG_SYMBOL_TABLE
    LOG("now unpacking from packed form");
#endif
    byte_array packed_form;
    pack(packed_form, syms);
    ASSERT_TRUE(unpack(packed_form, new_syms), "unpack test should not fail to unpack");

#ifdef DEBUG_SYMBOL_TABLE
///    LOG(astring(astring::SPRINTF, "unpacked form has:\n%s", new_syms->text_form().s()));
#endif
    ASSERT_FALSE(! (syms == new_syms), "unpacked test symbol tables must be equal");

#ifdef DEBUG_SYMBOL_TABLE
///    LOG(astring(astring::SPRINTF, "got the unpacked form, and dumping it:\n%s", new_syms->text_form().s()));
    LOG("packing the symbol table again...");
#endif
    byte_array packed_again(0);
    start.reset();  // click, on.
    pack(packed_again, new_syms);
    end.reset();  // click, off.
    time_in_pack += end.value() - start.value();
#ifdef DEBUG_SYMBOL_TABLE
    LOG("now unpacking from packed form again...");
#endif
    start = time_stamp();
    ASSERT_TRUE(unpack(packed_again, newer_syms), "newer unpacking should working be");
    end = time_stamp();
    time_in_unpack += end.value() - start.value();
#ifdef DEBUG_SYMBOL_TABLE
///    LOG(astring(astring::SPRINTF, "got the unpacked form, and dumping it:\n%s", newer_syms->text_form().s()));
#endif
    ASSERT_EQUAL(new_syms, newer_syms,
        "unpacked test these just aren't getting it but should be same");
  }
}

//////////////

void test_symbol_table::ADD2(second_table_def &syms, const astring &name,
    const test_content &to_add)
{ 
  FUNCDEF("ADD2")
  time_stamp start;
  outcome added = syms.add(name, to_add);
  ASSERT_EQUAL(added.value(), common::IS_NEW, "new item should not already be in table");
  time_stamp end;
  time_in_add += end.value() - start.value();
  start = time_stamp();  // reset start.
#ifdef OLD_TEST
  int indy = syms.find(name);
  ASSERT_FALSE(negative(indy), "item should be found after add");
  // refind to balance timing.
  indy = syms.find(name);
  ASSERT_FALSE(negative(indy), "item should be found after second add");
  end = time_stamp();  // reset end.
  time_in_dep_find += end.value() - start.value();
#else
  int indy = syms.dep_find(name);
  ASSERT_FALSE(negative(indy), "finding item after add should work");
  end = time_stamp();  // reset end.
  time_in_dep_find += end.value() - start.value();
  start = time_stamp();
  test_content *found = syms.find(name);
  ASSERT_TRUE(found, "item shouldn't be nil that we found");
  end = time_stamp();  // reset end.
  time_in_new_find += end.value() - start.value();
#endif
  astring name_out;
  test_content content_out;
  if (syms.retrieve(indy, name_out, content_out) != common::OKAY) {
    ASSERT_EQUAL(name_out, name, "name should be correct after retrieve");
    ASSERT_EQUAL(content_out, to_add, "content should be correct after retrieve");
  }
}

//////////////

void test_symbol_table::test_tc_table()
{
  FUNCDEF("test_tc_table")
  second_table_def syms;
  second_table_def new_syms;
  second_table_def newer_syms;
  for (int qq = 0; qq < test_iterations; qq++) {
    syms.reset();
#ifdef DEBUG_SYMBOL_TABLE
    LOG(astring(astring::SPRINTF, "index %d", qq));
#endif
    astring freudname("blurgh");
    test_content freud("Sigmund Freud was a very freaked dude.", "flutenorf");
    ADD2(syms, freudname, freud);
    astring borgname("borg");
    test_content borg("You will be assimilated.", "alabaster");
    ADD2(syms, borgname, borg);
    astring xname("X-Men");
    test_content x("The great unknown superhero cartoon.", "somnambulist");
    ADD2(syms, xname, x);
    astring aname("fleeny-brickle");
    test_content a("lallax menick publum.", "aglos bagnort pavlod");
    ADD2(syms, aname, a);
    astring axname("ax");
    test_content ax("Lizzy Borden has a very large hatchet.", "chop");
    ADD2(syms, axname, ax);
    astring bloinkname("urg.");
    test_content bloink("this is a short and stupid string", "not that short");
    ADD2(syms, bloinkname, bloink);
    astring faxname("fax");
    test_content fax("alligators in my teacup.", "lake placid");
    ADD2(syms, faxname, fax);
    astring zname("eagle ovaries");
    test_content z("malfeasors beware", "endangered");
    ADD2(syms, zname, z);

    // test copying the table.
    time_stamp start;
    second_table_def copy1(syms);
    {
      second_table_def joe(copy1);
      second_table_def joe2 = joe;
      ASSERT_EQUAL(joe2, joe, "copy test C: should have same symbol tables");
    }
    ASSERT_FALSE(! (syms == copy1), "copy test D: symbol tables shouldn't be different");
    time_stamp end;
    time_in_copy += end.value() - start.value();

#ifdef DEBUG_SYMBOL_TABLE
    astring texto;
    syms.text_form(texto);
    LOG(astring("This is the symbol table before any manipulation\n") + texto);
    LOG("now packing the symbol table...");
#endif

#ifdef DEBUG_SYMBOL_TABLE
    LOG("now unpacking from packed form");
#endif
    byte_array packed_form;
    pack(packed_form, syms);
    ASSERT_TRUE(unpack(packed_form, new_syms), "crikey all these unpacks should work");
#ifdef DEBUG_SYMBOL_TABLE
    new_syms.text_form(texto);
    LOG(astring("unpacked form has:\n") + texto);
#endif
    ASSERT_FALSE(! (syms == new_syms), "unpacked test symbol tables should be equivalent");

#ifdef DEBUG_SYMBOL_TABLE
    new_syms.text_form(texto);
    LOG(astring("got the unpacked form, and dumping it:\n") + texto);
    LOG("packing the symbol table again...");
#endif
    byte_array packed_again(0);
    pack(packed_again, new_syms);
#ifdef DEBUG_SYMBOL_TABLE
    LOG("now unpacking from packed form again...");
#endif
    ASSERT_TRUE(unpack(packed_again, newer_syms), "unpacking should get back the goods");
#ifdef DEBUG_SYMBOL_TABLE
    newer_syms.text_form(texto);
    LOG(astring("got the unpacked form, and dumping it:\n") + texto);
#endif
    ASSERT_FALSE(! (new_syms == newer_syms), "unpacked test symbol tables should stay same");
  }
}

//////////////

int test_symbol_table::execute()
{
#ifdef DEBUG_SYMBOL_TABLE
  LOG(astring("starting test 1: ") + time_stamp::notarize(false));
#endif
  test_byte_table();
#ifdef DEBUG_SYMBOL_TABLE
  LOG(astring("done test 1: ") + time_stamp::notarize(false));
  LOG(astring("starting test 2: ") + time_stamp::notarize(false));
#endif

  test_tc_table();
#ifdef DEBUG_SYMBOL_TABLE
  LOG(astring("done test 2: ") + time_stamp::notarize(false));
  LOG(astring(astring::SPRINTF, "time in add=%f", time_in_add));
  LOG(astring(astring::SPRINTF, "time in dep_find=%f", time_in_dep_find));
  LOG(astring(astring::SPRINTF, "time in new_find=%f", time_in_new_find));
  LOG(astring(astring::SPRINTF, "time in pack=%f", time_in_pack));
  LOG(astring(astring::SPRINTF, "time in unpack=%f", time_in_unpack));
#endif
  return final_report();
}

//////////////

HOOPLE_MAIN(test_symbol_table, )

