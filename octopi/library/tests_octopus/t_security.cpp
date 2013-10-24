/*****************************************************************************\
*                                                                             *
*  Name   : octopus security test                                             *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Checks out the login support for octopus.  This just exercises the base  *
*  support which doesn't perform any extra verification on the user.          *
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
#include <basis/mutex.h>
#include <structures/static_memory_gremlin.h>
#include <octopus/entity_defs.h>
#include <octopus/infoton.h>
#include <octopus/octopus.h>
#include <octopus/tentacle.h>
#include <application/application_shell.h>
#include <loggers/console_logger.h>
#include <structures/static_memory_gremlin.h>
#include <sockets/internet_address.h>
#include <tentacles/login_tentacle.h>
#include <tentacles/simple_entity_registry.h>

//////////////

astring base_list[] = { "cli", "simp" };

SAFE_STATIC_CONST(string_array, simp_classifier, (2, base_list))

class simple_infoton : public infoton
{
public:
  astring futzle;

  simple_infoton() : infoton(simp_classifier()) {}

  virtual void pack(byte_array &packed_form) const {
    futzle.pack(packed_form);
  }
  virtual bool unpack(byte_array &packed_form) {
    if (!futzle.unpack(packed_form)) return false;
    return true;
  }
  virtual int packed_size() const { return futzle.length() + 1; }
  virtual clonable *clone() const { return new simple_infoton(*this); }

private:
};

//////////////

// provides a simple service to allow us to test whether the security is
// working or not.

class simple_tentacle : public tentacle
{
public:
  simple_tentacle() : tentacle(simp_classifier(), true) {}

  virtual outcome reconstitute(const string_array &classifier,
          byte_array &packed_form, infoton * &reformed) {
    reformed = NIL;
    if (classifier != simp_classifier()) return NO_HANDLER;
    reformed = new simple_infoton;
    if (!reformed->unpack(packed_form)) {
      WHACK(reformed);
      return GARBAGE;
    }
    return OKAY;
  }

  virtual outcome consume(infoton &to_chow,
          const octopus_request_id &formal(item_id), byte_array &transformed) {
    transformed.reset();
    if (to_chow.classifier() != simp_classifier()) return NO_HANDLER;
    // consume without doing anything.
    return OKAY;
  }

  virtual void expunge(const octopus_entity &formal(to_zap)) {}
};

//////////////

//hmmm: this test should do a sample login octopus and do a login, reside for
//      a while, log out, do another one, let it time out, try to access
//      something with dead id hoping to be rejected, etc.

class test_octopus_security : public application_shell
{
public:
  test_octopus_security() : application_shell(class_name()) {}
  DEFINE_CLASS_NAME("test_octopus_security");
  virtual int execute();
};

int test_octopus_security::execute()
{
  octopus logos("local", 18 * MEGABYTE);
  simple_tentacle *tenty = new simple_tentacle;
  logos.add_tentacle(tenty);
  tenty = NIL;  // octopus has charge of this now.

  // turn on security in logos.
  simple_entity_registry *guardian = new simple_entity_registry;
  logos.add_tentacle(new login_tentacle(*guardian), true);

  // create an entity to work with.
  octopus_entity jimbo("localhost", application_configuration::process_id(), 128, 982938);
  octopus_request_id req1(jimbo, 1);

  // add the user jimbo.
  guardian->add_entity(jimbo, byte_array());

  // create a piece of data to try running on tentacle.
  simple_infoton testose;
  simple_infoton *testose_copy = new simple_infoton(testose);

  // test that the simple tentacle allows the op.
  outcome ret = logos.evaluate(testose_copy, req1);
  if (ret != tentacle::OKAY)
    deadly_error(class_name(), "first test",
        astring("the operation failed with an error ")
            + tentacle::outcome_name(ret));

  // create another entity to work with.
  octopus_entity burfo("localhost", application_configuration::process_id(), 372, 2989);
  octopus_request_id req2(burfo, 1);

  // try with an unlicensed user burfo...
  testose_copy = new simple_infoton(testose);
  ret = logos.evaluate(testose_copy, req2);
  if (ret == tentacle::OKAY)
    deadly_error(class_name(), "second test",
        astring("the operation didn't fail when it should have."));
  else if (ret != tentacle::DISALLOWED)
    deadly_error(class_name(), "second test",
        astring("the operation didn't provide the proper outcome, it gave: ")
            + tentacle::outcome_name(ret));

  // remove the user jimbo.
  guardian->zap_entity(jimbo);

  // test that jimbo fails too now.
  testose_copy = new simple_infoton(testose);
  ret = logos.evaluate(testose_copy, req1);
  if (ret == tentacle::OKAY)
    deadly_error(class_name(), "third test",
        astring("the operation didn't fail when it should have."));
  else if (ret != tentacle::DISALLOWED)
    deadly_error(class_name(), "third test",
        astring("the operation didn't provide the proper outcome, it gave: ")
            + tentacle::outcome_name(ret));

  // add the user burfo in now instead.
  guardian->add_entity(burfo, byte_array());

  // test that burfo works.
  testose_copy = new simple_infoton(testose);
  ret = logos.evaluate(testose_copy, req2);
  if (ret != tentacle::OKAY)
    deadly_error(class_name(), "fourth test",
        astring("the operation failed with an error ")
        + tentacle::outcome_name(ret));

  log("octopus:: security works for those functions tested.");

  WHACK(guardian); 

  return 0;
}

HOOPLE_MAIN(test_octopus_security, )

