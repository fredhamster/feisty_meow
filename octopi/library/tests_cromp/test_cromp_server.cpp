/*****************************************************************************\
*                                                                             *
*  Name   : test_cromp_server                                                 *
*  Author : Chris Koeritz                                                     *
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

#include <basis/byte_array.h>
#include <basis/function.h>
#include <basis/istring.h>
#include <basis/log_base.h>
#include <basis/portable.h>
#include <cromp/cromp_server.h>
#include <mechanisms/time_stamp.h>
#include <octopus/tentacle.h>
#include <opsystem/application_shell.h>
#include <opsystem/command_line.h>
#include <loggers/console_logger.h>
#include <loggers/file_logger.h>
#include <data_struct/static_memory_gremlin.h>
#include <sockets/address.h>
#include <sockets/machine_uid.h>
#include <sockets/tcpip_stack.h>

#define LOG(a) CLASS_EMERGENCY_LOG(program_wide_logger(), a)

const int REPORTING_INTERVAL = 28 * SECOND_ms;  // how often to squawk.

//const int ACCEPTING_THREADS = 5;
const int ACCEPTING_THREADS = 1;

// specifies whether to support lazy evaluation by tentacles.
//const bool SUPPORT_BACKGROUNDING = true;
const bool SUPPORT_BACKGROUNDING = false;

// determines whether the octopus will do immediate evaluation or not.
//const bool IMMEDIATE_EVALUATION = true;
const bool IMMEDIATE_EVALUATION = false;

////////////////////////////////////////////////////////////////////////////

// forward.
class cromp_server_tester;

class our_cromp_server : public cromp_server
{
public:
  our_cromp_server(cromp_server_tester &parent, const internet_address &addr)
  : cromp_server(addr, ACCEPTING_THREADS, IMMEDIATE_EVALUATION),
    _parent(parent) {}

  ~our_cromp_server() {}

  IMPLEMENT_CLASS_NAME("our_cromp_server");

private:
  cromp_server_tester &_parent;
};

////////////////////////////////////////////////////////////////////////////

class cromp_server_tester : public application_shell
{
public:
  bool _saw_clients;  // true if we ever got a connection.

  cromp_server_tester();
  ~cromp_server_tester();

  virtual int execute();

  IMPLEMENT_CLASS_NAME("cromp_server_tester");

private:
  our_cromp_server *_uplink;
    // provides the connection and transmission services.
  bool _leave_when_no_clients;  // true if we should just do one run.
  bool _encryption;  // true if we should do an encrypted connection.
  internet_address c_address;
};

////////////////////////////////////////////////////////////////////////////

class real_bubbles_tentacle : public bubbles_tentacle
{
public:
  real_bubbles_tentacle(cromp_server_tester &parent, bool backgrounding)
      : bubbles_tentacle(backgrounding), _parent(parent) {}

  virtual outcome consume(infoton &to_chow, const octopus_request_id &item_id,
      byte_array &transformed)
  {
    transformed.reset();
    _parent._saw_clients = true;  // we saw someone.
//LOG("got to our cromp's bubble tentacle.");
    bubble *inf = dynamic_cast<bubble *>(&to_chow);
    if (!inf) return NO_HANDLER;
//LOG("caching product!  success getting unpacked etc.");
    bubble *junk = (bubble *)inf->clone();
    store_product(junk, item_id);
    return OKAY;
  }

private:
  cromp_server_tester &_parent;
};

////////////////////////////////////////////////////////////////////////////

cromp_server_tester::cromp_server_tester()
: application_shell("cromp_server_tester"),
  _saw_clients(false),
  _uplink(NIL),
  _leave_when_no_clients(false),
  _encryption(false)
{
  FUNCDEF("constructor");
  SET_DEFAULT_COMBO_LOGGER;
  LOG("");
  LOG("");

  command_line args(__argc, __argv);
  // check for a port on the command line.
  istring port_text;
  int port = 5678;
  if (args.get_value("port", port_text, false)) {
    LOG(istring("using port: ") + port_text);
    port = port_text.convert(5678);
  }
  int posn = 0;
  if (args.find("exit", posn)) {
    LOG("seeing the 'exit without clients' flag set.");
    _leave_when_no_clients = true;
  }

//hmmm:normalize host so this can take either name or IP.

  // check for a hostname on the command line.
  istring hostname("local");
  istring host_temp;
  if (args.get_value("host", host_temp, false)) {
    LOG(istring("using host: ") + host_temp);
    hostname = host_temp;
  }
  strcpy(c_address.hostname, hostname.s());

//LOG(istring("here's the command line:") + log_base::platform_ending() + args.text_form());

  int indy = 0;
  if (args.find("encrypt", indy) || (args.find('e', indy)) ) {
    // they're saying that we should encrypt the communication.
    LOG("turning on encryption.");
    _encryption = true;
  }

//  tcpip_stack stack;
//  machine_uid host = stack.this_host(machine_uid::TCPIP_LOCATION);
  c_address.port = port;
//  host.native().stuff(internet_address::ADDRESS_SIZE, c_address.ip_address);
  LOG("starting cromp_server");
  _uplink = new our_cromp_server(*this, c_address);
  _uplink->add_tentacle(new real_bubbles_tentacle(*this, SUPPORT_BACKGROUNDING));
  _uplink->enable_servers(_encryption);
}

cromp_server_tester::~cromp_server_tester()
{
  WHACK(_uplink);
}

int cromp_server_tester::execute()
{
  FUNCDEF("execute");

  time_stamp next_report(REPORTING_INTERVAL);

  while (true) {
    // make sure we didn't see our exit condition.
    if (!_uplink->clients() && _leave_when_no_clients && _saw_clients) {
      LOG("exiting now");
      break;
    }

    if (time_stamp() > next_report) {
      int client_count = _uplink->clients();
      const char *verb = "are";
      if (client_count == 1) verb = "is";
      const char *ending = "s";
      if (client_count == 1) ending = "";
      LOG(isprintf("There %s %d client%s.", verb, client_count, ending));
      next_report.reset(REPORTING_INTERVAL);
    }

// new test case; rip out server while still possibly connected.
if (randomizer().inclusive(1, 200) == 32) {
LOG("tearing down entire server and re-creating.");
WHACK(_uplink);
_uplink = new our_cromp_server(*this, c_address);
_uplink->add_tentacle(new real_bubbles_tentacle(*this, SUPPORT_BACKGROUNDING));
_uplink->enable_servers(_encryption);
}

    portable::sleep_ms(100); 
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////

HOOPLE_MAIN(cromp_server_tester, )

