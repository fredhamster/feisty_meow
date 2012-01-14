/*
*  Name   : test_byte_array_amorph
*  Author : Chris Koeritz
*  Purpose:
*    Puts the amorph object through its paces.
**
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include "bogon.h"

#include <application/hoople_main.h>
#include <mathematics/chaos.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <structures/amorph.h>
#include <timely/time_stamp.h>
#include <loggers/file_logger.h>
#include <loggers/console_logger.h>
#include <loggers/combo_logger.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

#include <memory.h>
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

#define DEBUG_ARRAY
  // uncomment to enable array debugging.

#define DEBUG_AMORPH
  // uncomment to enable amorph debugging.

//#define DEBUG_TEST_AMORPH
  // uncomment for this program to be noisier.

#ifdef DEBUG_TEST_AMORPH
  #define LOG(to_print) EMERGENCY_LOG(program_wide_logger::get(), to_print)
#else
  #define LOG(to_print) {}
#endif

//////////////

class t_amorph : virtual public unit_base, virtual public application_shell
{
public:
  t_amorph() : unit_base() {}
  DEFINE_CLASS_NAME("t_amorph");
  int test_bogon_amorph();
  int test_byte_array_amorph();
  byte_array fake_pack(amorph<byte_array> &me);
  int compare(amorph<byte_array> &one, amorph<byte_array> &two);
  amorph<byte_array> *fake_amorph_unpack(byte_array &packed_amorph);
  int compare(const amorph<bogon> &one, const amorph<bogon> &two);

  struct blob_hold { int size; int offset; };

  virtual int execute();
};

#define PACK_BLOB_SIZE(max_limbs) (max_limbs * sizeof(blob_hold))

HOOPLE_MAIN(t_amorph, );

//////////////

const int default_test_iterations = 2;

const int MAX_LIMBS = 200;
  // the highest number of items stored in the amorphs here.

const int MIN_CHUBBY = 60;
  // the smallest chunk to allocate for storing text strings...  all strings
  // must therefore be shorter than this length.
const int MAX_RANDO = 275;
  // the maximum amount of space to add when allocating a randomly sized chunk.

#define PROGRAM_NAME astring("test_amorph")

int t_amorph::compare(amorph<byte_array> &one, amorph<byte_array> &two)
{
  FUNCDEF("compare amorph<byte_array>");
  ASSERT_EQUAL(one.elements(), two.elements(), "elements comparison");
  if (one.elements() != two.elements()) return false;
  ASSERT_EQUAL(one.valid_fields(), two.valid_fields(), "valid fields comparison");
  if (one.valid_fields() != two.valid_fields()) return false;
  for (int i = 0; i < one.elements(); i++) {
    if (!one.get(i) && !two.get(i)) continue;
    ASSERT_FALSE(!one.get(i) || !two.get(i), "inequal emptiness");
    ASSERT_EQUAL(one.get(i)->length(), two.get(i)->length(), "inequal sizes");
    if (one.get(i)->length() > 0) {
      ASSERT_INEQUAL(one[i]->observe(), two[i]->observe(), "pointer in use twice");
      ASSERT_FALSE(memcmp(one[i]->observe(), two[i]->observe(), one[i]->length()),
            "inequal contents");
    }
  }
  return true;
}

byte_array t_amorph::fake_pack(amorph<byte_array> &me)
{
  FUNCDEF("fake_pack");
  // snagged from the packable_amorph pack function!
  // count the whole size needed to store the amorph.
  int amo_size = 0;
  amorph<byte_array> hold_packed_bits(me.elements());

  for (int i = 0; i < me.elements(); i++)
    if (me.get(i) && me.get(i)->length()) {
      byte_array packed_item;
      attach(packed_item, *me[i]);
      byte_array *to_stuff = new byte_array(packed_item);
      hold_packed_bits.put(i, to_stuff);
      amo_size += packed_item.length();
    }
  int len = amo_size + sizeof(int) + PACK_BLOB_SIZE(me.elements());

  // allocate a storage area for the packed form.
  byte_array to_return(len);
  int temp = me.elements();
  memcpy((int *)to_return.access(), &temp, sizeof(int));
    // size of package is stored at the beginning of the memory.

  int current_offset = sizeof(int);
  // the indices into the packed form are located after the amorph header.
  blob_hold *blob_array = (blob_hold *)(to_return.access() + current_offset);
  current_offset += PACK_BLOB_SIZE(me.elements());

  // the entire amorph is replicated into the new buffer.
  for (int j = 0; j < me.elements(); j++) {
    // the offset of this limb in the packed area is saved in the hold.
    blob_array[j].size
      = (hold_packed_bits[j]? hold_packed_bits[j]->length() : 0);
    blob_array[j].offset = current_offset;
    if (hold_packed_bits[j] && hold_packed_bits[j]->length()) {
      // the actual data is copied....
      memcpy(to_return.access() + current_offset,
            (abyte *)hold_packed_bits[j]->observe(),
            hold_packed_bits[j]->length());
      // and the "address" is updated.
      current_offset += hold_packed_bits[j]->length();
    }
  }
  ASSERT_EQUAL(current_offset, len, "offset is incorrect after packing");
  return to_return;
}

amorph<byte_array> *t_amorph::fake_amorph_unpack(byte_array &packed_amorph)
{
  // snagged from the packable_amorph unpack function!
  int max_limbs;
  memcpy(&max_limbs, (int *)packed_amorph.access(), sizeof(max_limbs));
  amorph<byte_array> *to_return = new amorph<byte_array>(max_limbs);

  blob_hold *blob_array = new blob_hold[max_limbs];
  memcpy(blob_array, (blob_hold *)(packed_amorph.access()
     + sizeof(int)), PACK_BLOB_SIZE(max_limbs));
  for (int i = 0; i < to_return->elements(); i++)
    if (blob_array[i].size) {
      abyte *source = packed_amorph.access() + blob_array[i].offset;
      byte_array packed_byte_array(blob_array[i].size, source);
      byte_array *unpacked = new byte_array;
      detach(packed_byte_array, *unpacked);
      to_return->put(i, unpacked);
    }
  delete [] blob_array;
  return to_return;
}

int t_amorph::test_byte_array_amorph()
{
  FUNCDEF("test_byte_array_amorph");
  LOG("start of amorph of abyte array test");
  for (int qq = 0; qq < default_test_iterations; qq++) {
    LOG(astring(astring::SPRINTF, "index %d", qq));
    {
      // some simple creation and stuffing tests....
      amorph<byte_array> fred(20);
      amorph<byte_array> gen(10);
      for (int i=0; i < 10; i++)  {
        byte_array *gens = new byte_array(8, (abyte *)"goodbye");
        gen.put(i, gens);
      }
      for (int j = 0; j < 20; j++)  {
        byte_array *freds = new byte_array(6, (abyte *)"hello");
         fred.put(j, freds);
      }
      amorph_assign(gen, fred);
      LOG("done with fred & gen");
    }

    LOG("before fred creation");
    chaos randomizer;
    amorph<byte_array> fred(MAX_LIMBS - 1);
    fred.append(NIL);  // add one to make it max limbs big.
    LOG("after append nil");
    {
      for (int i = 0; i < fred.elements(); i++) {
        int size = MIN_CHUBBY + randomizer.inclusive(0, MAX_RANDO);
        astring text("bogus burfonium nuggets");
        astring burph(astring::SPRINTF, " ung %d ", i);
        text += burph;
        abyte *temp = new abyte[size];
        text.stuff((char *)temp, text.length()+1);
        byte_array *to_stuff = new byte_array(size, temp);
        fred.put(i, to_stuff);
        delete [] temp;
      }
    }
    LOG("after first loop");
    {
      amorph<byte_array> bungee3;
      amorph_assign(bungee3, fred);
      amorph<byte_array> burglar2;
      amorph_assign(burglar2, bungee3);
      amorph<byte_array> trunklid;
      amorph_assign(trunklid, burglar2);
      ASSERT_INEQUAL(trunklid.elements(), 0, "const constructor test - no elements!");
    }
    LOG("after copies performed");
    {
      astring text;
      text = "hello this is part one.";
      LOG(astring(astring::SPRINTF, "len is %d, content is %s",
          text.length(), text.observe()));
      char *tadr = text.access();
      abyte *badr = (abyte *)tadr;
      byte_array *to_stuff = new byte_array(text.length() + 1, badr);
      fred.put(183, to_stuff);
      text = "wonky tuniea bellowbop";
      byte_array *to_stuff1 = new byte_array(text.length()+1, (abyte *)text.s());
      fred.put(90, to_stuff1);

      text = "frunkwioioio";
      byte_array *to_stuff2 = new byte_array(text.length()+1, (abyte *)text.s());
      fred.put(12, to_stuff2);

      fred.clear(98); fred.clear(122); fred.clear(123);
      fred.clear(256);
      fred.clear(129);
      fred.zap(82, 90);
      fred.zap(93, 107);
    }
    LOG("after second loop");
    {
      byte_array packed = fake_pack(fred);
      LOG(astring(astring::SPRINTF, "done packing in %s, pack has %d "
          "elems.", class_name(), packed.length()));
      amorph<byte_array> *new_fred = fake_amorph_unpack(packed);
      LOG("done unpacking in test_amorph");
      ASSERT_TRUE(compare(fred, *new_fred), "first pack test, amorphs not the same");
      abyte *cont1
        = (new_fred->get(14)? (*new_fred)[14]->access() : (abyte *)"NIL");
      abyte *cont2
        = (new_fred->get(20)? (*new_fred)[20]->access() : (abyte *)"NIL");
      abyte *cont3
        = (new_fred->get(36)? (*new_fred)[36]->access() : (abyte *)"NIL");

      if (cont1) LOG(astring(astring::SPRINTF, "14: %s", cont1));
      if (cont2) LOG(astring(astring::SPRINTF, "20: %s", cont2));
      if (cont3) LOG(astring(astring::SPRINTF, "36: %s", cont3));
      LOG("fields all compare identically after pack and unpack");
      byte_array packed_second = fake_pack(*new_fred);
      delete new_fred;
      amorph<byte_array> *newer_fred = fake_amorph_unpack(packed_second);
      ASSERT_TRUE(compare(*newer_fred, fred), "second pack test, amorphs not the same");
      delete newer_fred;
    }

    {
      amorph<byte_array> fred(randomizer.inclusive(20, 30));
      int size = MIN_CHUBBY + randomizer.inclusive(0, MAX_RANDO);
      astring text("bogus burfonium nuggets");
      astring burph(astring::SPRINTF, " ung %d ", 2314);
      text += burph;
      byte_array intermed(size);

      for (int i = 0; i < fred.elements(); i += 5) {
        byte_array *to_stuff = new byte_array(size, intermed.access());
        memcpy(intermed.access(), (abyte *)text.s(), text.length() + 1);
        fred.put(i, to_stuff);
      }
      fred.clear_all();
      for (int j = 0; j < fred.elements(); j += 5) {
        byte_array *to_stuff = new byte_array(size, intermed.access());
        memcpy(intermed.access(), (abyte *)text.s(), text.length() + 1);
        fred.put(j, to_stuff);
      }
      text = "frunkwioioio";
      byte_array *to_stuff = new byte_array(text.length()+1, (abyte *)text.s());
      fred.put(12, to_stuff);
      fred.clear_all();
    }
    LOG("survived the clear_alls");
    {
      amorph<byte_array> *ted = new amorph<byte_array>(0);
      amorph_assign(*ted, fred);
      ASSERT_TRUE(compare(*ted, fred), "ted and fred aren't the same");
      {
        amorph<byte_array> *george = new amorph<byte_array>(0);
        amorph_assign(*george, fred);
        ASSERT_TRUE(compare(*george, fred), "fred and george aren't the same");
        ted->zap(3, 20);
        george->zap(3, 10);
        george->zap(3, 12);
        ASSERT_TRUE(compare(*ted, *george), "after zap, ted and george aren't the same");
        ted->adjust(ted->elements() - 20);
        george->adjust(george->elements() - 5);
        george->adjust(george->elements() - 5);
        george->adjust(george->elements() - 5);
        george->adjust(george->elements() - 5);
        ASSERT_TRUE(compare(*ted, *george), "after adjust, ted and george aren't the same");
        delete george;
      }
      delete ted;
    }
  }
  return 0;
}

int t_amorph::compare(const amorph<bogon> &one, const amorph<bogon> &two)
{
  FUNCDEF("compare amorph<bogon>");
  if (one.elements() != two.elements()) return false;
  for (int i = 0; i < one.elements(); i++) {
    if (!one.get(i) && !two.get(i)) continue;
    ASSERT_FALSE(!one.get(i) || !two.get(i), "both should be non-nil");
    ASSERT_EQUAL(one.get(i)->size(), two.get(i)->size(), "sizes should be equal");
    if (one.get(i)->size() > 0) {
      ASSERT_INEQUAL(one.get(i)->held(), two.get(i)->held(), "pointer should not be in use twice");
      ASSERT_FALSE(memcmp(one.get(i)->held(), two.get(i)->held(), one.get(i)->size()),
          "contents should be equal");
    }
  }
  return true;
}

int t_amorph::test_bogon_amorph()
{
  FUNCDEF("test_bogon_amorph");
  LOG("start of amorph of bogon test");
  for (int qq = 0; qq < default_test_iterations; qq++) {
    LOG(astring(astring::SPRINTF, "index %d", qq));
    {
      // some simple creation and stuffing tests....
      amorph<bogon> fred(20);
      amorph<bogon> gen(10);
      for (int i = 0; i < 10; i++)  {
        bogon *gens = new bogon((abyte *)"goodbye");
        gen.put(i, gens);
      }
      for (int j = 0; j < 20; j++)  {
        bogon *freds = new bogon((abyte *)"hello");
        fred.put(j, freds);
      }
      ASSERT_FALSE(compare(fred, gen), "fred and gen ARE the same");
      amorph_assign(gen, fred);
      ASSERT_TRUE(compare(fred, gen), "fred and gen aren't the same");
    }

    chaos randomizer;

    amorph<bogon> fred(MAX_LIMBS);

    LOG("after append nil");
    {
      for (int i = 0; i < fred.elements(); i++) {
        int size = MIN_CHUBBY + randomizer.inclusive(0, MAX_RANDO);
        astring text("bogus burfonium nuggets");
        astring burph(astring::SPRINTF, " ung %d ", i);
        text += burph;
        abyte *temp = new abyte[size];
        text.stuff((char *)temp, text.length()+1);
        bogon *to_stuff = new bogon(temp);
        fred.put(i, to_stuff);
        delete [] temp;
      }
    }

    LOG("after first loop");
    {
      amorph<bogon> bungee3;
      amorph_assign(bungee3, fred);
      amorph<bogon> burglar2;
      amorph_assign(burglar2, bungee3);
      amorph_assign(burglar2, bungee3);
      amorph<bogon> trunklid;
      amorph_assign(trunklid, burglar2);
      ASSERT_TRUE(trunklid.elements(), "const constructor test: no elements!");
    }
    {
      astring text;
      text = "hello this is part one.";
      bogon *to_stuff = new bogon((abyte *)text.s());
      fred.put(32, to_stuff);

      text = "wonky tuniea bellowbop";
      bogon *to_stuff1 = new bogon((abyte *)text.s());
      fred.put(84, to_stuff1);

      text = "frunkwioioio";
      bogon *to_stuff2 = new bogon((abyte *)text.s());
      fred.put(27, to_stuff2);

      fred.clear(98); fred.clear(122); fred.clear(123);
      fred.clear(256);
      fred.clear(129);
      fred.zap(82, 90);
      fred.zap(93, 107);
    }
    LOG("after second loop");
    {
      amorph<bogon> fred(randomizer.inclusive(20, 30));
      astring text("bogus burfonium nuggets");
      astring burph(astring::SPRINTF, " ung %d ", 2314);
      text += burph;

      for (int i = 0; i < fred.elements(); i += 5) {
        bogon *to_stuff = new bogon((abyte *)text.s());
        fred.put(i, to_stuff);
      }
      fred.clear_all();
      for (int j = 0; j < fred.elements(); j += 5) {
        bogon *to_stuff = new bogon((abyte *)text.s());
        fred.put(j, to_stuff);
      }
      text = "frunkwioioio";
      bogon *to_stuff = new bogon((abyte *)text.s());
      fred.put(6, to_stuff);
      fred.clear_all();
    }
    LOG("survived the clear_alls");
    {
      amorph<bogon> *ted = new amorph<bogon>();
      amorph_assign(*ted, fred);
      ASSERT_TRUE(compare(*ted, fred), "after assign, ted and fred aren't the same");
      {
        amorph<bogon> *george = new amorph<bogon>();
        amorph_assign(*george, fred);
        ASSERT_TRUE(compare(*george, fred), "pre-zap, george and fred aren't the same");
        ted->zap(3, 20);
        george->zap(3, 10);
        george->zap(3, 12);
        ASSERT_TRUE(compare(*ted, *george), "after zap, ted and george aren't the same");
        ted->adjust(ted->elements()-20);
        george->adjust(george->elements()-5);
        george->adjust(george->elements()-5);
        george->adjust(george->elements()-5);
        george->adjust(george->elements()-5);
        ASSERT_TRUE(compare(*ted, *george), "after more zaps, ted and george aren't the same");
        delete george;
      }
      delete ted;
    }
  }
  return 0;
}

const int MAX_TEST_DURATION = 1 * MINUTE_ms;
  // each of the tests calling on the templated tester will take this long.

const int MAX_SIMULTANEOUS_OBJECTS = 42;  // the maximum length tested.

//hmmm: this test_amorph_of is not completed.

template <class contents>
int test_amorph_of(const contents &bogus)
{
  chaos rando;

  // these are the actions we try on the amorph during the test.
  // the first and last elements must be identical to the first and last
  // tests to perform.
  enum actions { first, do_zap = first, do_adjust, do_assign,


      do_borrow, last = do_borrow};

  time_stamp exit_time(::MAX_TEST_DURATION);
  while (time_stamp() < exit_time) {
    int index = rando.inclusive(0, ::MAX_SIMULTANEOUS_OBJECTS - 1);
    int choice = rando.inclusive(first, last);
    switch (choice) {
      case do_zap: {

        break;
      }
      case do_adjust: {

        break;
      }
      case do_assign: {

        break;
      }
      case do_borrow: {

        break;
      }
    }
  }
}

int t_amorph::execute()
{
  SETUP_COMBO_LOGGER;
  int errs = 0;
  int retval = test_byte_array_amorph();
  if (retval != 0) errs += retval;
  retval = test_bogon_amorph();
  if (retval != 0) errs += retval;

//incorporate these errors somehow also.

//  if (retval == 0)
//    critical_events::alert_message("amorph:: works for those functions tested.");
//  else
//    critical_events::alert_message("amorph:: there were errors!");
  return final_report();
}

