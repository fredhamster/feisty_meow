/*****************************************************************************\
*                                                                             *
*  Name   : test_many_cromp                                                   *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Sees how a cromp server does with a large number of connections.         *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "crompish_pax.h"

#include <mathematics/chaos.h>
#include <basis/astring.h>

#include <application/application_shell.h>
#include <application/command_line.h>
#include <application/hoople_main.h>
#include <structures/amorph.h>
#include <cromp/cromp_client.h>
#include <octopus/entity_defs.h>
#include <octopus/infoton.h>
#include <loggers/console_logger.h>
#include <loggers/critical_events.h>
#include <loggers/file_logger.h>
#include <loggers/program_wide_logger.h>
#include <sockets/internet_address.h>
#include <structures/static_memory_gremlin.h>
#include <timely/time_control.h>
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

#define DEBUG_TESTER
  // uncomment for noisier version.

const int REPORTING_INTERVAL = 20 * SECOND_ms;
  // how frequently we tell about bad crompers.

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), astring(s))
#define BASE_LOG(s) EMERGENCY_LOG(program_wide_logger::get(), astring(s))

class many_cromp_tester : virtual public unit_base, virtual public application_shell
{
public:
  many_cromp_tester();
  ~many_cromp_tester();

  virtual int execute();

  DEFINE_CLASS_NAME("many_cromp_tester");

private:
  amorph<cromp_client> _uplinks;  // a list of cromp clients.
  bool _encryption;  // true if we're encrypting.
  int _count;  // number of cromps.
};

//////////////

many_cromp_tester::many_cromp_tester()
: application_shell(),
///"many_cromp_tester"),
  _uplinks(),
  _encryption(false),
  _count(1)
{
  FUNCDEF("constructor");
  LOG("");
  LOG("");

  internet_address server_loc;

  command_line args(application::_global_argc, application::_global_argv);
//LOG(a_sprintf("argc is %d and first is %s", application::_global_argc, application::_global_argv[0]));

  // check for a port on the command line.
  astring port_text;
  int port = 5678;
  if (args.get_value("port", port_text, false)) {
    LOG(astring("using port: ") + port_text);
    port = port_text.convert(5678);
  }
  server_loc.port = port;

  astring count_text;
  if (args.get_value("count", count_text, false)) {
    LOG(astring("using count: ") + count_text);
    _count = count_text.convert(_count);
  }

//hmmm: normalize host so this can take either name or IP.

  int indy = 0;
  if (args.find("encrypt", indy, false)
      || (args.find('e', indy, false)) ) {
    // they're saying that we should encrypt the communication.
    _encryption = true;
  }

  // check for a hostname on the command line.
  astring hostname("local");
  astring host_temp;
  if (args.get_value("host", host_temp, false)) {
    LOG(astring("using host: ") + host_temp);
    hostname = host_temp;
  }
LOG(astring("using host: ") + hostname);
  strcpy(server_loc.hostname, hostname.s());

LOG(astring("opening at ") + server_loc.text_form());

LOG(a_sprintf("count of %d cromps will be created.", _count));

  for (int i = 0; i < _count; i++) {
LOG(a_sprintf("%d. A", i));
    cromp_client *uplink = new cromp_client(server_loc);
LOG(a_sprintf("%d. B", i));
    uplink->add_tentacle(new bubbles_tentacle(false));
LOG(a_sprintf("%d. C", i));
    _uplinks.append(uplink);
  }

}

many_cromp_tester::~many_cromp_tester()
{
}

int many_cromp_tester::execute()
{
  FUNCDEF("execute");

  if (_encryption) {
    for (int i = 0; i < _uplinks.elements(); i++) {
      _uplinks.borrow(i)->enable_encryption();
    }
  }

  for (int i = 0; i < _uplinks.elements(); i++) {
    outcome ret = _uplinks.borrow(i)->connect();
    if (ret != cromp_client::OKAY) {
      deadly_error(class_name(), func, astring("connection failed with error: ")
          + cromp_client::outcome_name(ret));
    }
  }

time_stamp when_to_leave(10 * HOUR_ms);

  time_stamp next_report(REPORTING_INTERVAL);

  while (time_stamp() < when_to_leave) {
    int unconnected = 0;
    for (int i = 0; i < _uplinks.elements(); i++) {
      if (!_uplinks.borrow(i)->connected())
        unconnected++;
    }

    if (time_stamp() > next_report) {
      int connected = _uplinks.elements() - unconnected;
      LOG(a_sprintf("[ %d connected and %d did not ]", connected, unconnected));
      next_report.reset(REPORTING_INTERVAL);
    }

//do something with uplinks.

    time_control::sleep_ms(100);
  }


  BASE_LOG("many cromp_client:: works for those functions tested.");

  return 0;
}

//////////////

HOOPLE_MAIN(many_cromp_tester, )

