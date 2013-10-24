/*****************************************************************************\
*                                                                             *
*  Name   : octopus_entity tester                                             *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Checks that the octopus_entity class is behaving as expected.            *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/byte_array.h>
#include <mathematics/chaos.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <octopus/entity_defs.h>
#include <application/application_shell.h>
#include <loggers/console_logger.h>
#include <loggers/file_logger.h>
#include <structures/static_memory_gremlin.h>
#include <sockets/tcpip_stack.h>
#include <textual/string_manipulation.h>

#ifdef __WIN32__
  #include <process.h>
#else
  #include <unistd.h>
#endif

const int ITERATE_EACH_TEST = 1000;
  // the number of times to repeat each test operation.

class test_entity : public application_shell
{
public:
  test_entity() : application_shell(class_name()) {}
  DEFINE_CLASS_NAME("test_entity");
  virtual int execute();
};

int test_entity::execute()
{
  chaos rando;
  SET_DEFAULT_COMBO_LOGGER;
  tcpip_stack stack;

  octopus_entity blankie;
  if (!blankie.blank())
    deadly_error(class_name(), "emptiness test",
        "the blank entity was not seen as empty.");
  octopus_entity fullish("gurp", 28, 39, 4);
  if (fullish.blank())
    deadly_error(class_name(), "emptiness test",
        "the non-blank entity was seen as empty.");

  for (int i = 0; i < ITERATE_EACH_TEST; i++) {
    // test the basic filling of the values in an entity.
    octopus_entity blank_ent;
    int sequencer = rando.inclusive(1, MAXINT - 10);
    int add_in = rando.inclusive(0, MAXINT - 10);
    octopus_entity filled_ent(stack.hostname(), application_configuration::process_id(), sequencer,
        add_in);
    blank_ent = octopus_entity(stack.hostname(), application_configuration::process_id(), sequencer,
        add_in);
    if (blank_ent != filled_ent)
      deadly_error(class_name(), "simple reset test",
          "failed to resolve to same id");
    astring text1 = filled_ent.to_text();
    astring text2 = blank_ent.to_text();
    if (text1 != text2)
      deadly_error(class_name(), "to_text test", "strings are different");
///log(text1);
    octopus_entity georgio = octopus_entity::from_text(text2);
///log(georgio.to_text());
    if (georgio != filled_ent)
      deadly_error(class_name(), "from_text test",
          "entity is different after from_text");

    octopus_request_id fudnix(filled_ent, 8232390);
    astring text3 = fudnix.to_text();
    octopus_request_id resur = octopus_request_id::from_text(text3);
    if (resur != fudnix)
      deadly_error(class_name(), "from_text test",
          "request id is different after from_text");
    
    blank_ent = octopus_entity();  // reset it again forcefully.
    blank_ent = octopus_entity(filled_ent.hostname(), filled_ent.process_id(),
        filled_ent.sequencer(), filled_ent.add_in());
    if (blank_ent != filled_ent)
      deadly_error(class_name(), "reset from attribs test",
          "failed to resolve to same id");
//    log(a_sprintf("%d: ", i + 1) + filled_ent.mangled_form());

    byte_array chunk1;
    filled_ent.pack(chunk1);
    octopus_entity unpacked1;
    unpacked1.unpack(chunk1);
    if (unpacked1 != filled_ent)
      deadly_error(class_name(), "pack/unpack test",
          "failed to return same values");

    // test of entity packing and size calculation.
    octopus_entity ent(string_manipulation::make_random_name(1, 428),
            randomizer().inclusive(0, MAXINT/2),
            randomizer().inclusive(0, MAXINT/2),
            randomizer().inclusive(0, MAXINT/2));
    octopus_request_id bobo(ent, randomizer().inclusive(0, MAXINT/2));
    int packed_estimate = bobo.packed_size();
    byte_array packed_bobo;
    bobo.pack(packed_bobo);
    if (packed_bobo.length() != packed_estimate)
      deadly_error(class_name(), "entity packed_size test",
          "calculated incorrect packed size");
  }


  log("octopus_entity:: works for those functions tested.");
  return 0;
}

//hmmm: tests the octopus entity object,
//      can do exact text check if want but that's not guaranteed to be useful
//      in the future.

HOOPLE_MAIN(test_entity, )

