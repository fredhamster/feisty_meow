/*****************************************************************************\
*                                                                             *
*  Name   : unpacking octopus test                                            *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    A test of octopuses used for unpacking flat structures.                  *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <structures/static_memory_gremlin.h>
#include <octopus/entity_defs.h>
#include <octopus/infoton.h>
#include <octopus/octopus.h>
#include <octopus/tentacle_helper.h>
#include <application/application_shell.h>
#include <loggers/console_logger.h>
#include <loggers/file_logger.h>
#include <structures/static_memory_gremlin.h>
#include <sockets/internet_address.h>

//hmmm: provide equality ops to be able to check that same stuff
//      came back out that went in.

class test_unpacker : public application_shell
{
public:
  test_unpacker() : application_shell(class_name()) {}
  DEFINE_CLASS_NAME("test_unpacker");
  virtual int execute();
  void test_unpacking();
};

//////////////

// the infotons here have a three level classifier.  the outer level is
// for the benefit of the handler_arm tentacle that just checks that the
// group name is correct before passing off the request to its internal
// octopus.  then the second level specifies which class of infotons are
// being managed.  the third level specifies the leaf type of the infoton--
// the specific type of data being wrapped.

const char *base_list[] = { "gruntiak" };

SAFE_STATIC_CONST(string_array, base_classifier, (1, base_list))

const char *math_list[] = { "math" };

SAFE_STATIC_CONST(string_array, math_classifier, (base_classifier()
    + string_array(1, math_list)))

const char *addr_list[] = { "address" };

SAFE_STATIC_CONST(string_array, addr_classifier, (base_classifier()
    + string_array(1, addr_list)))

class address_ton : public infoton, public network_address
{
public:
  address_ton() : infoton(addr_classifier() + "leaf") {}

  virtual void pack(byte_array &packed_form) const {
    network_address::pack(packed_form);
  }

  virtual bool unpack(byte_array &packed_form) {
    return network_address::unpack(packed_form);
  }

  virtual int packed_size() const {
    return 5 * sizeof(int) + 128 /*address estimate*/;
  }

  virtual clonable *clone() const {
    return new address_ton(*this);
  }
};

//some floating point nums.
class float_ton : public infoton
{
public:
  float f1;
  double d1;

  float_ton() : infoton(math_classifier() + "float") {}

  virtual void pack(byte_array &packed_form) const {
    structures::attach(packed_form, f1);
    structures::attach(packed_form, d1);
  }

  virtual int packed_size() const {
    return sizeof(double) + sizeof(float);
  }

  virtual bool unpack(byte_array &packed_form) {
    double hold;
    if (!structures::detach(packed_form, hold)) return false;
    f1 = float(hold);
    if (!structures::detach(packed_form, d1)) return false;
    return true;
  }

  virtual clonable *clone() const {
    return new float_ton(*this);
  }
};

//an integer set.
class int_set_ton : public infoton
{
public:
  int_set nums;

  int_set_ton() : infoton(math_classifier() + "intset") {}

  virtual void pack(byte_array &packed_form) const {
    structures::attach(packed_form, nums.elements());
    for (int i = 0; i < nums.elements(); i++)
      structures::attach(packed_form, nums[i]);
  }

  virtual int packed_size() const {
    return sizeof(int) + nums.elements() * sizeof(int);
  }

  virtual bool unpack(byte_array &packed_form) {
    int len = 0;
    nums.reset();
    if (!structures::detach(packed_form, len)) return false;
    for (int i = 0; i < len; i++) {
      int got = 0;
      if (!structures::detach(packed_form, got)) return false;
      nums += got;
    }
    return true;
  }

  virtual clonable *clone() const {
    return new int_set_ton(*this);
  }
};

//////////////

// handles network addresses.
class address_chomper : public tentacle_helper<address_ton>
{
public:
  address_chomper()
  : tentacle_helper<address_ton>(addr_classifier().subarray(1, 1), true) {}
};

// handles floats and int_sets.
class numerical_chomper : public tentacle
{
public:
  numerical_chomper() : tentacle(math_classifier().subarray(1, 1), true) {}

  outcome reconstitute(const string_array &classifier, byte_array &packed_form,
      infoton * &reformed)
  {
    reformed = NULL_POINTER;
    if (classifier.length() < 2) return BAD_INPUT;
    astring key = classifier[1];
    if (key == "float") {
      float_ton *to_return = new float_ton;
      if (!to_return->unpack(packed_form)) {
        WHACK(to_return);
        return NULL_POINTER;
      }
      reformed = to_return;
      return OKAY;
    } else if (key == "intset") {
      int_set_ton *to_return = new int_set_ton;
      if (!to_return->unpack(packed_form)) {
        WHACK(to_return);
        return NULL_POINTER;
      }
      reformed = to_return;
      return OKAY;
    } else
      return NO_HANDLER;
  }

  outcome consume(infoton &formal(to_chow), const octopus_request_id &formal(item_id),
          byte_array &transformed)
  { transformed.reset(); return tentacle::BAD_INPUT; }

  virtual void expunge(const octopus_entity &formal(zapola)) {}
};

//////////////

// delegates the unpacking to an internal tentacle.  it peels off a level
// of classifier to find the real handler.
class outer_arm : public tentacle
{
public:
  outer_arm()
  : tentacle(base_classifier(), true),
    _unpackers("local", 10 * MEGABYTE),
    _numer(new numerical_chomper),
    _addron(new address_chomper)
  {
    // register the two tentacles.
    outcome ret = _unpackers.add_tentacle(_numer);
    if (ret != tentacle::OKAY)
      deadly_error(class_name(), "adding numerical tentacle",
          astring("failed to add: ") + tentacle::outcome_name(ret));
    ret = _unpackers.add_tentacle(_addron);
    if (ret != tentacle::OKAY)
      deadly_error(class_name(), "adding address tentacle",
          astring("failed to add: ") + tentacle::outcome_name(ret));
  }

  ~outer_arm() {
    // just reset the two tentacles, since the _unpackers octopus should
    // clean them up.
    _numer = NULL_POINTER;
    _addron = NULL_POINTER;
  }

  outcome reconstitute(const string_array &classifier, byte_array &packed_form,
      infoton * &reformed)
  {
    // strip first word of classifier.
    string_array real_class = classifier;
    real_class.zap(0, 0);
    // route to octopus.
    return _unpackers.restore(real_class, packed_form, reformed);
  }

  outcome consume(infoton &to_chow, const octopus_request_id &item_id,
          byte_array &transformed)
  {
    transformed.reset();
    // strip first word of classifier.
    string_array real_class = to_chow.classifier();
    real_class.zap(0, 0);
    to_chow.set_classifier(real_class);
    // route to octopus.
    return _unpackers.evaluate((infoton *)to_chow.clone(), item_id);
  }

  void expunge(const octopus_entity &formal(whackola)) {}

private:
  octopus _unpackers;
  numerical_chomper *_numer;
  address_chomper *_addron;
};

//////////////

void test_unpacker::test_unpacking()
{
  octopus unpacky("local", 10 * MEGABYTE);
  outer_arm *outer = new outer_arm;
  outcome ret = unpacky.add_tentacle(outer);
  if (ret != tentacle::OKAY)
    deadly_error(class_name(), "adding outer tentacle",
        astring("failed to add: ") + tentacle::outcome_name(ret));

  // test infoton fast packing.
  int_set_ton jubjub;
  jubjub.nums.add(299);
  jubjub.nums.add(39274);
  jubjub.nums.add(25182);
  byte_array packed(10388);  // have data in there to start.
  infoton::fast_pack(packed, jubjub);
  if (jubjub.packed_size() + infoton::fast_pack_overhead(jubjub.classifier())
      != packed.length() - 10388)
    deadly_error(class_name(), "packing test",
        astring("erroneous size calculated for first fast_pack"));
  string_array shirley_class;
  byte_array shirley_data;
  packed.zap(0, 10387);  // remove the original data.

  // testing the overhead calculation.
  byte_array junk_jub;
  jubjub.pack(junk_jub);
  if (packed.length() != junk_jub.length()
      + infoton::fast_pack_overhead(jubjub.classifier()))
    deadly_error(class_name(), "test fast pack overhead",
        "sizes differed from calculated");

  if (!infoton::fast_unpack(packed, shirley_class, shirley_data))
    deadly_error(class_name(), "test infoton fast pack",
        "failed shirley unpack");
  if (packed.length() != 0)
    deadly_error(class_name(), "test infoton fast pack",
        "shirley didn't consume all");
  if (shirley_class != jubjub.classifier())
    deadly_error(class_name(), "test infoton fast pack",
        "inequal orig classifier");
  int_set_ton scroop;
  if (!scroop.unpack(shirley_data))
    deadly_error(class_name(), "test infoton fast pack",
        "failed scroop unpack");
  if (shirley_data.length())
    deadly_error(class_name(), "test infoton fast pack",
        "scroop didn't consume all");
  if (scroop.nums.length() != 3)
    deadly_error(class_name(), "test infoton fast pack",
        "wrong length in scroop");
  if ( (scroop.nums[0] != jubjub.nums[0]) || (scroop.nums[1] != jubjub.nums[1])
      || (scroop.nums[2] != jubjub.nums[2]) )
    deadly_error(class_name(), "test infoton fast pack",
        "erroneous information");

  byte_array fasting;
  infoton::fast_pack(fasting, jubjub);
  if (jubjub.packed_size() + infoton::fast_pack_overhead(jubjub.classifier())
      != fasting.length())
    deadly_error(class_name(), "packing test",
        astring("erroneous size calculated for second fast_pack"));

  // another test of the overhead calculator.
  byte_array junk_fast;
  jubjub.pack(junk_fast);
  if (fasting.length() != junk_fast.length()
      + infoton::fast_pack_overhead(jubjub.classifier()))
    deadly_error(class_name(), "test fast pack overhead 2",
        "sizes differed from calculated");

  string_array nudge_class;
  byte_array nudge_data;
  if (!infoton::fast_unpack(fasting, nudge_class, nudge_data))
    deadly_error(class_name(), "test infoton fast pack", "fast pack failed to unpack");
  if (fasting.length())
    deadly_error(class_name(), "test infoton fast pack", "fast pack didn't consume all");
  int_set_ton croup;
  if (!croup.unpack(nudge_data))
    deadly_error(class_name(), "test infoton fast pack", "croup wouldn't unpack");
  if ( (croup.nums[0] != jubjub.nums[0]) || (croup.nums[1] != jubjub.nums[1])
      || (croup.nums[2] != jubjub.nums[2]) )
    deadly_error(class_name(), "test infoton fast pack", "croup has errors");
  byte_array chunkmo;
  chunkmo += 0x23;
  chunkmo += 0xf8;
  chunkmo += 0x37;
  chunkmo += 0x65;
  address_ton norf;
  (network_address &)norf = network_address(internet_address
      (chunkmo, "urp", 23841));
  chunkmo.reset();
  infoton::fast_pack(chunkmo, norf);
  string_array clarfiator;
  byte_array pacula;
  if (!infoton::fast_unpack(chunkmo, clarfiator, pacula))
    deadly_error(class_name(), "test fast_unpack", "chunkmo has errors");
  infoton *scrung = NULL_POINTER;
//log(astring("classif is ") + clarfiator.text_form());

  outcome scrung_ret = unpacky.restore(clarfiator, pacula, scrung);
  if (scrung_ret != tentacle::OKAY)
    deadly_error(class_name(), "test fast_unpack",
        a_sprintf("can't restore scrung: %s",
            tentacle::outcome_name(scrung_ret)));
  address_ton *rescrung = dynamic_cast<address_ton *>(scrung);
  if (!rescrung)
    deadly_error(class_name(), "test fast_unpack", "wrong dynamic type for scrung");
  address_ton &prescrung = *rescrung;
  if ((network_address &)prescrung != (network_address &)norf)
    deadly_error(class_name(), "test fast_unpack", "wrong network address restored");
  WHACK(scrung);
}

const int MAXIMUM_TESTS = 10;
  // was added to check for memory leaks.

int test_unpacker::execute()
{
  int iters = 0;
  while (iters++ < MAXIMUM_TESTS) {
//log(a_sprintf("iter #%d", iters));
    test_unpacking();
  }
  log("unpacking octopus:: works for all functions tested.");
//time_control::sleep_ms(30000);
  return 0;
}

HOOPLE_MAIN(test_unpacker, )

