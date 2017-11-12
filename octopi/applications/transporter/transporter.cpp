/*****************************************************************************\
*                                                                             *
*  Name   : transporter                                                       *
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
#include <application/command_line.h>
#include <cromp/cromp_client.h>
#include <cromp/cromp_server.h>
#include <filesystem/directory_tree.h>
#include <filesystem/filename_list.h>
#include <loggers/combo_logger.h>
#include <octopus/entity_defs.h>
#include <octopus/tentacle.h>
#include <octopus/unhandled_request.h>
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

#define LOG(a) CLASS_EMERGENCY_LOG(program_wide_logger::get(), a)

const int REPORTING_INTERVAL = 28 * SECOND_ms;  // how often to squawk.

const int REFRESH_INTERVAL = 120 * MINUTE_ms;  // how often we check tree.

const int TRANSFER_PORT = 10808;
  // simple port grabbed randomly for the default.

const int MAX_CHUNK = 2 * MEGABYTE;
  // we will transfer fairly large chunks so we can get this done reasonably
  // quickly.  even at that size, it shouldn't cause most modern machines to
  // hiccup even slightly.

//////////////

class transporter : public application_shell
{
public:
  transporter();
  ~transporter();

  virtual int execute();

  DEFINE_CLASS_NAME("transporter");

  int push_client_download();
    // for a client side download, this prods the transfer process.

  int print_instructions();
    // shows the instructions for this application.

private:
  bool _saw_clients;  // true if we ever got a connection.
  cromp_server *_server_side;
    // provides connection and transmission services for servers.
  cromp_client *_client_side;  // client side connection.
  bool _leave_when_no_clients;  // true if we should just do one run.
  bool _encryption;  // true if we're encrypting.
  astring _source;  // the source path which a client will ask the server for.
  astring _target;  // the target path where files are stored for the client.
  bool _started_okay;  // true if we got past the command line checks.
};

//////////////

transporter::transporter()
: application_shell(),
  _saw_clients(false),
  _server_side(NULL_POINTER),
  _client_side(NULL_POINTER),
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
  int port = TRANSFER_PORT;
  if (args.get_value("port", port_text, false))
    port = port_text.convert(TRANSFER_PORT);
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
LOG("client side chosen");
    server = false;
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

    LOG("starting transfer server");
    _server_side = new cromp_server(cromp_server::any_address(port));
    file_transfer_tentacle *new_tent = new file_transfer_tentacle(MAX_CHUNK,
        (file_transfer_tentacle::transfer_modes)(file_transfer_tentacle::ONLY_REPORT_DIFFS
        | file_transfer_tentacle::COMPARE_SIZE_AND_TIME
        | file_transfer_tentacle::COMPARE_CONTENT_SAMPLE));

LOG(key + " => " + root);
    new_tent->add_correspondence(key, root, REFRESH_INTERVAL);
    _server_side->add_tentacle(new_tent);
    _server_side->enable_servers(_encryption);
  } else {
    LOG("starting transfer client");
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

transporter::~transporter()
{
  WHACK(_client_side);
  WHACK(_server_side);
}

int transporter::print_instructions()
{
  astring name = filename(_global_argv[0]).basename().raw();
  log(a_sprintf("%s usage:", name.s()));
  log(astring::empty_string());
  log(a_sprintf("\
This program can transfer directory trees across the network.  It will only\n\
copy the files missing on the client's side given what the server offers.\n\
The program can function as either the server side or the client side.\n\
The available flags are:\n\
\n\
%s --client --host srvname --port P --source key_path --target cli_dest\n\
\n\
The client side needs to know the server host (srvname) and the port where\n\
the server is listening for connections (P).  The client will compare its\n\
local path (cli_dest) with the server's keyed path (key_path) and copy the\n\
files that are missing on the client's side.  The key path will begin with\n\
whatever keyword the server is offering, plus optional additional path\n\
components to retrieve less than the whole tree being served.\n\
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

int transporter::push_client_download()
{
  FUNCDEF("push_client_download");
  // prepare a client request
  file_transfer_infoton initiate;
  initiate._request = true;
  initiate._command = file_transfer_infoton::BUILD_TARGET_TREE;
  initiate._src_root = _source;
  initiate._dest_root = _target;

  // make a directory snapshot with just directories, no files.
  directory_tree target_area_just_dirs(_target, "*", true);
  string_set includes;
  initiate.package_tree_info(target_area_just_dirs, includes);
  octopus_request_id cmd_id;
  outcome build_ret = _client_side->submit(initiate, cmd_id);
  if (build_ret != tentacle::OKAY)
    non_continuable_error(class_name(), func, astring("failed to build the "
        " target tree: ") + cromp_client::outcome_name(build_ret));

  // now get the full contents going on.
  initiate._command = file_transfer_infoton::TREE_COMPARISON;
  directory_tree target_area(_target);
  target_area.calculate(false);
  includes.reset();
  initiate.package_tree_info(target_area, includes);
  outcome start_ret = _client_side->submit(initiate, cmd_id);
  if (start_ret != tentacle::OKAY)
    non_continuable_error(class_name(), func, astring("failed to initiate "
        " the transfer: ") + cromp_client::outcome_name(start_ret));

  infoton *start_reply_tmp = NULL_POINTER;
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

//debugging start
  filename_list diffs;
  byte_array pack_copy = start_reply->_packed_data;
  if (!diffs.unpack(pack_copy))
    non_continuable_error(class_name(), func, "could not unpack filename list!");
  LOG(astring("got list of diffs:\n") + diffs.text_form());
//debugging end

  outcome eval_ret = _client_side->octo()->evaluate(start_reply, cmd_id, true);
  if (eval_ret != cromp_client::OKAY)
    non_continuable_error(class_name(), func, astring("failed to process the "
        "start response: ") + cromp_client::outcome_name(eval_ret));

  int iter = 0;

  while (true) {
LOG(a_sprintf("ongoing chunk %d", ++iter));
    // keep going until we find a broken reply.
    file_transfer_infoton ongoing;
    ongoing._request = true;
    ongoing._command = file_transfer_infoton::PLACE_FILE_CHUNKS;
    ongoing._src_root = _source;
    ongoing._dest_root = _target;

    octopus_request_id cmd_id;
    outcome place_ret = _client_side->submit(ongoing, cmd_id);
    if (place_ret != cromp_client::OKAY)
      non_continuable_error(class_name(), func, astring("failed to send ongoing "
          "chunk: ") + cromp_client::outcome_name(place_ret));

    infoton *place_reply_tmp = NULL_POINTER;
//hmmm: set timeout appropriate to the speed of the connection!
    outcome place_receipt = _client_side->acquire(place_reply_tmp, cmd_id);
    if (place_receipt != cromp_client::OKAY)
      non_continuable_error(class_name(), func, astring("failed to receive "
          "place response: ") + cromp_client::outcome_name(place_receipt));

    file_transfer_infoton *place_reply = dynamic_cast<file_transfer_infoton *>
        (place_reply_tmp);
    if (!place_reply) {
      if (dynamic_cast<unhandled_request *>(place_reply_tmp)) {
        log(astring("The server does not support file transfers."), ALWAYS_PRINT);
        WHACK(place_reply_tmp);
        break;
      }
      non_continuable_error(class_name(), func, "failed to cast storage reply infoton "
          "to proper type");
    }

    int reply_size = place_reply->_packed_data.length();

    outcome eval_ret2 = _client_side->octo()->evaluate(place_reply, cmd_id, true);
    if (eval_ret2 != tentacle::OKAY)
      non_continuable_error(class_name(), func, astring("failed to process the "
          "place response: ") + cromp_client::outcome_name(eval_ret2));

    if (!reply_size) {
      LOG("hit termination condition: no data packed in for file chunks.");
      break;
    }
  }
  return 0;
}

int transporter::execute()
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

    if (_client_side) return push_client_download();

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

HOOPLE_MAIN(transporter, )

