/*****************************************************************************\
*                                                                             *
*  Name   : find_missing                                                      *
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

#include <basis/byte_array.h>
#include <basis/astring.h>

#include <application/hoople_main.h>
//#include <application/command_line.h>
#include <basis/astring.h>
#include <cromp/cromp_client.h>
#include <cromp/cromp_server.h>
#include <filesystem/directory_tree.h>
#include <filesystem/filename_list.h>
#include <loggers/critical_events.h>
#include <loggers/combo_logger.h>
//#include <loggers/program_wide_logger.h>
#include <octopus/entity_defs.h>
#include <octopus/tentacle.h>
#include <sockets/internet_address.h>
#include <sockets/machine_uid.h>
#include <sockets/tcpip_stack.h>
#include <structures/static_memory_gremlin.h>
#include <tentacles/file_transfer_tentacle.h>
#include <timely/time_control.h>
#include <timely/time_stamp.h>

using namespace application;
using namespace basis;
using namespace cromp;
using namespace filesystem;
using namespace loggers;
using namespace octopi;
using namespace sockets;
using namespace structures;
using namespace textual;
using namespace timely;

#undef BASE_LOG
#define BASE_LOG(a) EMERGENCY_LOG(program_wide_logger::get(), astring(a))
#undef LOG
#define LOG(a) CLASS_EMERGENCY_LOG(program_wide_logger::get(), astring(a))

const int REPORTING_INTERVAL = 28 * SECOND_ms;  // how often to squawk.

const int REFRESH_INTERVAL = 20 * MINUTE_ms;  // how often we check tree.

const int COMPARATOR_PORT = 10809;
  // simple port grabbed randomly for the default.

const int MAX_CHUNK = 16 * KILOBYTE;
  // chunk size doesn't matter here; not transferring.

//////////////

class find_missing : public application_shell
{
public:
  find_missing();
  ~find_missing();

  virtual int execute();

  DEFINE_CLASS_NAME("find_missing");

  int retrieve_info_from_server();
    // for a client side comparison, this finds out which files are
    // different and reports them.

  int print_instructions();
    // shows the instructions for this program.

private:
  bool _saw_clients;  // true if we ever got a connection.
  cromp_server *_server_side;
    // provides connection and transmission services for servers.
  cromp_client *_client_side;  // client side connection.
  bool _leave_when_no_clients;  // true if we should just do one run.
  bool _encryption;  // true if we're encrypting.
  astring _source;  // the source path which a client will ask the server for.
  astring _target;  // the target path where files are stored for the client.
  bool _started_okay;  // got through the command line checking.
};

//////////////

find_missing::find_missing()
: application_shell(),
  _saw_clients(false),
  _server_side(NIL),
  _client_side(NIL),
  _leave_when_no_clients(false),
  _encryption(false),
  _started_okay(false)
{
  FUNCDEF("constructor");
  SETUP_COMBO_LOGGER;
  LOG("");
  LOG("");

  command_line args(_global_argc, _global_argv);
  // check for a port on the command line.
  astring port_text;
  int port = COMPARATOR_PORT;
  if (args.get_value("port", port_text, false))
    port = port_text.convert(COMPARATOR_PORT);
  int posn = 0;
  if (args.find("exit", posn)) {
    LOG("seeing the 'exit without clients' flag set.");
    _leave_when_no_clients = true;
  }

  int indy = 0;
  if (args.find("encrypt", indy, false)
      || (args.find('e', indy, false)) ) {
LOG("enabling encryption!");
    // they're saying that we should encrypt the communication.
    _encryption = true;
  }

  bool server = true;
  indy = 0;
  if (args.find("client", indy, false)) {
LOG("client role chosen.");
    server = false;
  } else {
LOG("server role chosen.");
  }

  internet_address addr;
  addr.port = port;

  // check for a hostname on the command line.
  astring hostname("local");
  astring host_temp;
  if (args.get_value("host", host_temp, false)) {
    LOG(astring("using host: ") + host_temp);
    hostname = host_temp;
  } else LOG(astring("using host: ") + hostname);
  strcpy(addr.hostname, hostname.s());

  if (server) {
    astring key;
    if (!args.get_value("key", key, false)) {
      print_instructions();
      LOG("No keyword specified on command line.");
      return;
    }
    astring root;
    if (!args.get_value("root", root, false)) {
      print_instructions();
      LOG("No transfer root was specified on the command line.");
      return;
    }

    LOG("starting comparison server");
    _server_side = new cromp_server(cromp_server::any_address(port));
    file_transfer_tentacle *new_tent = new file_transfer_tentacle(MAX_CHUNK,
        (file_transfer_tentacle::transfer_modes)(file_transfer_tentacle::ONLY_REPORT_DIFFS
        | file_transfer_tentacle::COMPARE_SIZE_AND_TIME
        | file_transfer_tentacle::COMPARE_CONTENT_SAMPLE));
    new_tent->add_correspondence(key, root, REFRESH_INTERVAL);
    _server_side->add_tentacle(new_tent);
    _server_side->enable_servers(_encryption);
  } else {
    LOG("starting comparison client");
    _client_side = new cromp_client(addr);
    if (_encryption) _client_side->enable_encryption();

    outcome ret = _client_side->connect();
    if (ret != cromp_client::OKAY)
      non_continuable_error(class_name(), func, astring("failed to connect to "
          "the server: ") + cromp_client::outcome_name(ret));

    file_transfer_tentacle *new_tent = new file_transfer_tentacle(MAX_CHUNK,
        (file_transfer_tentacle::transfer_modes)(file_transfer_tentacle::ONLY_REPORT_DIFFS
        | file_transfer_tentacle::COMPARE_SIZE_AND_TIME
        | file_transfer_tentacle::COMPARE_CONTENT_SAMPLE));
    if (!args.get_value("source", _source, false)) {
      print_instructions();
      LOG("No source path was specified on the command line.");
      return;
    }
    if (!args.get_value("target", _target, false)) {
      print_instructions();
      LOG("No target path was specified on the command line.");
      return;
    }

    string_array includes;
    outcome regis = new_tent->register_file_transfer
        (_client_side->entity(), _source, _target, includes);
    if (regis != cromp_client::OKAY)
      non_continuable_error(class_name(), func, "failed to register transfer");

    _client_side->add_tentacle(new_tent);
  }

  _started_okay = true;

}

find_missing::~find_missing()
{
  WHACK(_client_side);
  WHACK(_server_side);
}

int find_missing::print_instructions()
{
  astring name = filename(_global_argv[0]).basename().raw();
  BASE_LOG(a_sprintf("%s usage:", name.s()));
  BASE_LOG("");
  BASE_LOG(a_sprintf("\
This program can compare directory trees and report the files that are\n\
missing on the client's side compared to what the server is offering.\n\
The program can function as either the server side or the client side.\n\
The available flags are:\n\
\n\
%s --client --host srvname --port P --source key_path --target cli_dest\n\
\n\
The client side needs to know the server host (srvname) and the port where\n\
the server is listening for connections (P).  The client will compare its\n\
local path (cli_dest) with the server's keyed path (key_path).  The key\n\
path will begin with whatever keyword the server is offering, plus optional\n\
additional path components to retrieve less than the whole tree being\n\
served.\n\
\n\
\n\
%s --server --host srvname --port P --key keyname --root srv_path\n\
\n\
The server side needs to know what address and port to listen on (srvname\n\
and P).  It will open a server there that provides a directory hierarchy\n\
starting at the root specified (srv_path).  The directory tree will be known\n\
to clients as the key word (keyname), thus freeing the clients from needing\n\
to know absolute paths on the server.\n\
\n\
", name.s(), name.s()));

  return 23;
}

int find_missing::retrieve_info_from_server()
{
  FUNCDEF("retrieve_info_from_server");
  // prepare a client request
  file_transfer_infoton initiate;
  initiate._request = true;
  initiate._command = file_transfer_infoton::TREE_COMPARISON;
  initiate._src_root = _source;
  initiate._dest_root = _target;
  directory_tree target_area(_target);
  target_area.calculate(false);
  string_set includes;
  initiate.package_tree_info(target_area, includes);
  octopus_request_id cmd_id;
  outcome start_ret = _client_side->submit(initiate, cmd_id);
  if (start_ret != tentacle::OKAY)
    non_continuable_error(class_name(), func, astring("failed to initiate "
        " the transfer: ") + cromp_client::outcome_name(start_ret));

  infoton *start_reply_tmp = NIL;
//hmmm: set timeout appropriate to the speed of the connection!
  outcome first_receipt = _client_side->acquire(start_reply_tmp, cmd_id);
  if (first_receipt != cromp_client::OKAY)
    non_continuable_error(class_name(), func, astring("failed to receive response: ")
        + cromp_client::outcome_name(start_ret));
  file_transfer_infoton *start_reply = dynamic_cast<file_transfer_infoton *>
      (start_reply_tmp);
  if (!start_reply)
    non_continuable_error(class_name(), func, "failed to cast starting infoton to "
        "proper type");

  filename_list diffs;
  byte_array pack_copy = start_reply->_packed_data;
  if (!diffs.unpack(pack_copy))
    non_continuable_error(class_name(), func, "could not unpack filename list!");
  BASE_LOG("Differences found between local target and server's tree:");
///  BASE_LOG(diffs.text_form());
  for (int i = 0; i < diffs.elements(); i++) {
    BASE_LOG(a_sprintf("%d: %s", i + 1, diffs[i]->raw().s()));
  }

  return 0;
}

int find_missing::execute()
{
  FUNCDEF("execute");

  if (!_started_okay) return 32;

  time_stamp next_report(REPORTING_INTERVAL);

  while (true) {
    // make sure we didn't see our exit condition.

    if (_server_side && !_server_side->clients() && _leave_when_no_clients
        && _saw_clients) {
      LOG("exiting now");
      break;
    }

    if (_client_side) return retrieve_info_from_server();

    if (time_stamp() > next_report) {
      if (_server_side)
        LOG(a_sprintf("There are %d clients.", _server_side->clients()));
//report about client side also.
      next_report.reset(REPORTING_INTERVAL);
    }

    time_control::sleep_ms(100); 
  }
  return 0;
}

//////////////

HOOPLE_MAIN(find_missing, )

