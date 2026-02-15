/*****************************************************************************\
*                                                                             *
*  Name   : cromp_decoder app                                                 *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/application_shell.h>
#include <application/hoople_main.h>
#include <basis/astring.h>
#include <cromp/cromp_common.h>
#include <loggers/program_wide_logger.h>
#include <loggers/console_logger.h>
#include <loggers/file_logger.h>
#include <octopus/entity_defs.h>
#include <sockets/machine_uid.h>
#include <structures/static_memory_gremlin.h>
#include <textual/byte_formatter.h>
#include <unit_test/unit_base.h>

#include <stdio.h>

using namespace application;
using namespace basis;
using namespace configuration;
using namespace cromp;
using namespace mathematics;
using namespace filesystem;
using namespace loggers;
using namespace octopi;
using namespace processes;
using namespace sockets;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), astring(s))
#define BASE_LOG(s) EMERGENCY_LOG(program_wide_logger::get(), astring(s))

const int MAX_LINE = 2048;
  // the longest line we'll bother to try to process.

class cromp_decoder : virtual public unit_base, virtual public application_shell
{
public:
  cromp_decoder();
  ~cromp_decoder();

  virtual int execute();

  DEFINE_CLASS_NAME("cromp_decoder");
};

//////////////

cromp_decoder::cromp_decoder() : application_shell() {}

cromp_decoder::~cromp_decoder() {}

int cromp_decoder::execute()
{
  FUNCDEF("execute");

  BASE_LOG("\
This application will decode a cromp entity and report the different values");
  BASE_LOG("\
that are encoded into it.");

  astring buffer;  // we'll read input from the user into this.

  while (true) {
    BASE_LOG("Please enter the entity (or hit just enter to exit).")
    
    buffer = astring('\0', MAX_LINE + 10);  // reset the buffer.
    char *buf2 = fgets(buffer.s(), MAX_LINE, stdin);
    if (buf2 != buffer.s()) {
      deadly_error(class_name(), func,
          "memory was allocated when we didn't want it to be.");
    }

    buffer.shrink();
    buffer.strip("\r\n", astring::FROM_END);
    if (!buffer.length()) break;

    // make sure they didn't actually give us a request id.
    int indy = buffer.find('/');
    if (non_negative(indy)) buffer.zap(indy, buffer.end());

    octopus_entity ent = octopus_entity::from_text(buffer);
    if (ent.blank()) {
      BASE_LOG("That entity was blank or invalid.");
      BASE_LOG("");
      continue;
    }

    BASE_LOG("");
    BASE_LOG("");
    BASE_LOG("Entity contains:");
    BASE_LOG("");
    BASE_LOG(a_sprintf("\tProcess ID=%d", ent.process_id()));
    BASE_LOG(a_sprintf("\tSequencer=%d", ent.sequencer()));
    BASE_LOG(a_sprintf("\tChaotic Addin=%d", ent.add_in()));

    astring host;
    machine_uid machine;
    bool worked = cromp_common::decode_host(ent.hostname(), host, machine);
    if (!worked) {
      BASE_LOG("Failed to decode the hostname!  Was it a valid entity?");
      continue;
    }
    BASE_LOG(astring("\tPartial Hostname=") + host);
    BASE_LOG(astring("\tMachine UID=") + machine.text_form());
    BASE_LOG("");
  }

  return 0;
}

//////////////

HOOPLE_MAIN(cromp_decoder, )

