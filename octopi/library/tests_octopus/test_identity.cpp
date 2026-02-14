/*****************************************************************************\
*                                                                             *
*  Name   : octopus identity test                                             *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Checks out the client identification methods in octopus.                 *
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
#include <octopus/entity_defs.h>
#include <octopus/identity_infoton.h>
#include <octopus/infoton.h>
#include <octopus/octopus.h>
#include <octopus/tentacle.h>
#include <application/application_shell.h>
#include <loggers/console_logger.h>
#include <structures/static_memory_gremlin.h>

//////////////

class test_octopus_identity : public application_shell
{
public:
  test_octopus_identity() : application_shell(class_name()) {}
  DEFINE_CLASS_NAME("test_octopus_identity");
  virtual int execute();
};

int test_octopus_identity::execute()
{
  octopus logos("local", 18 * MEGABYTE);

  identity_infoton *ide = new identity_infoton;
  octopus_request_id junk_id = octopus_request_id::randomized_id();
    // bogus right now.

  byte_array packed;
  ide->pack(packed);
  if (ide->packed_size() != packed.length())
    deadly_error(class_name(), "packing test",
        astring("the packed size was different than expected."));

  outcome ret = logos.evaluate(ide, junk_id);
  if (ret != tentacle::OKAY)
    deadly_error(class_name(), "evaluate test",
        astring("the evaluation failed with an error ")
        + tentacle::outcome_name(ret));
log("point a");

  octopus_request_id response_id;  // based on bogus from before.
  infoton *response = logos.acquire_result(junk_id._entity, response_id);
  if (!response)
    deadly_error(class_name(), "acquire test",
        astring("the acquire_result failed to produce a result."));

  identity_infoton *new_id = dynamic_cast<identity_infoton *>(response);
  if (!new_id)
    deadly_error(class_name(), "casting",
        astring("the returned infoton is not the right type."));

  octopus_entity my_ide = new_id->_new_name;

log(astring("new id is: ") + my_ide.text_form());

  if (my_ide.blank())
    deadly_error(class_name(), "retrieving id",
        astring("the new entity id is blank."));


  log("octopus:: identity works for those functions tested.");

  return 0;
}

HOOPLE_MAIN(test_octopus_identity, )

