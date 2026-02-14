/*****************************************************************************\
*                                                                             *
*  Name   : test_cromp_client                                                 *
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

#include <mathematics/chaos.h>
#include <basis/astring.h>

#include <structures/set.h>
#include <cromp/cromp_client.h>
#include <processes/ethread.h>
#include <processes/thread_cabinet.h>
#include <sockets/throughput_counter.h>
#include <octopus/entity_data_bin.h>
#include <octopus/entity_defs.h>
#include <octopus/infoton.h>
#include <application/application_shell.h>
#include <application/command_line.h>
#include <loggers/console_logger.h>
#include <loggers/file_logger.h>
#include <filesystem/filename.h>
#include <processes/rendezvous.h>
#include <structures/static_memory_gremlin.h>
#include <sockets/internet_address.h>

#include <stdlib.h>

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

#define DEBUG_TESTER
  // uncomment for noisier version.

// the number of transactions to send during a test.  if timing connection
// duration, then use a maximum of 1.  if timing speed of operation once
// connected, use a large number.
//const int MAXIMUM_SENDS = 10008;
const int MAXIMUM_SENDS = 100008;
//const int MAXIMUM_SENDS = 10000008;
  // have had success with up to 10000000 sends using small data segments.

const int NUMBER_OF_THREADS = 1;
//const int NUMBER_OF_THREADS = 10;
//const int NUMBER_OF_THREADS = 20;
  // the number of simultaneous actors on the single cromp_client.

//const int GRABBER_THREADS = 5;
const int GRABBER_THREADS = 0;
  // the number of threads that just pluck at the cromp_client trying to
  // interfere with the testing threads.

//const int MAX_SEND_TRIES = 0;  // don't pause.
const int MAX_SEND_TRIES = 1;  // try to get stuff out but don't wait long.
//const int MAX_SEND_TRIES = 5;  // wait a reasonable amount of times to send.
//const int MAX_SEND_TRIES = 10000;  // force it to get out, hopefully.
  // the number of times we try to push the sends out.  zero means never
  // try to push anything, just add it to the buffer.  1 or more is that
  // many tries to push the send.

//const int CHECKPOINT_SIZE = 1000;
//const int CHECKPOINT_SIZE = 100;
const int CHECKPOINT_SIZE = 100;
  // prints a counter out when we reach a multiple of this many sends.

//const int DATA_SEGMENT_SIZE = 0;
const int DATA_SEGMENT_SIZE = 64;
//const int DATA_SEGMENT_SIZE = 128 * KILOBYTE;
//const int DATA_SEGMENT_SIZE = 1 * MEGABYTE;
  // the chunk size that we attach.

const int REPORTING_INTERVAL = 10 * SECOND_ms;
  // this is the period between reports on how the test is going.

//***this is where we are in testing the faux dual cpu problem.
//***the longer delay shows the problem more easily.  the shorter delay
//***is being used for a long test.
const int MAXIMUM_ACQUISITION_DELAY = 8 * SECOND_ms;
//const int MAXIMUM_ACQUISITION_DELAY = int(0.5 * SECOND_ms);
  // the longest we'll snooze off waiting for pending receptions to occur.

//const int MAXIMUM_PENDING_REQUESTS = 1;
const int MAXIMUM_PENDING_REQUESTS = 108;
  // this is a threshold for the number of requests; once hit, we start
  // awaiting the responses.

//const int PENDING_REQUESTS_FORCED = 3;
//const int PENDING_REQUESTS_FORCED = 80;
const int PENDING_REQUESTS_FORCED = MAXIMUM_PENDING_REQUESTS;
  // when we've been forced to gather some pending responses to previous
  // requests, this is how many we'll try to get at once.  numbers closer
  // to the MAXIMUM_PENDING_REQUESTS will force more synchrony.

const int CHANCE_OF_RECONSTRUCT = 14;
  // how frequently a bus reconstruction occurs, in 1000.

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)
#define BASE_LOG(s) EMERGENCY_LOG(program_wide_logger::get(), s)

class cromp_client_tester : public application_shell
{
public:
  cromp_client_tester();
  ~cromp_client_tester();

  virtual int execute();

  DEFINE_CLASS_NAME("cromp_client_tester");

  void bite_server(structures::set<octopus_request_id> &ids,
          structures::set<octopus_request_id> &delinquents, void *originator);
    // performs the big chunk of testing.  the "ids" are the history of the
    // sends that were made and they're managed by this method.  the
    // "originator" is a tag we can use to generate unique print outs.

  int print_instructions();
    //!< shows how to use the command line options for this tester.

  void grab_items();
    //!< tries to retrieve things from the socket.

  void cause_object_reconstruction();
    //!< makes the cromp client object be zapped and redone.

  void increment_thread_count() {
    FUNCDEF("increment_thread_count");
    auto_synchronizer l(*_lock);
    _threads_active++;
//LOG(a_sprintf("count now %d", _threads_active));
  }

  void decrement_thread_count() {
    FUNCDEF("decrement_thread_count");
    auto_synchronizer l(*_lock);
    _threads_active--;
//LOG(a_sprintf("count now %d", _threads_active));
  }

  void report(const time_stamp &start_time, double bytes_transmitted,
          double conversations);
    // describes how the test is going.

private:
  cromp_client *_uplink;  // provides the connection and transmission services.

  mutex *_lock;  // protects the objects below.
  int _threads_active;  // the number of transmitter threads running.
  time_stamp _last_report;  // when we last reported on progress.
  double _finished_loops;  // counts number of loops we've achieved.
  bool _encryption;  // true if we're encrypting.
  int _send_count;  //!< number of sends in this test.
  int _thread_count;  //!< number of threads in test.
  int _grabber_count;  //!< number of threads interfering with retrieve.
  int _send_tries;  //!< how many times to try send if not all data got out.
  int _checkpoint_count;  //!< items seen between printing of counter.
  int _dataseg_size;  //!< size of bytes added to test packets.
  int _report_interval;  //!< how frequently to show the report.
  int _snooze_duration;  //!< time wasted between each send.
  bool _rpc_style;  //!< true if emulating RPC and waiting for each item.
  bool _reconstruct_object;  //!< true if we periodically tear down object.
  internet_address _server_loc;  //!< holds onto the requested address.

  void look_for_receipts(int count, structures::set<octopus_request_id> &ids,
          structures::set<octopus_request_id> &delinquents, bool wait = false);
    // attempts to get "count" items from the list of "ids".
};

//////////////

class bitey_thread : public ethread
{
public:
  bitey_thread(cromp_client_tester &parent)
      : ethread(), _parent(parent) {}

  void perform_activity(void *formal(ptr)) {
    FUNCDEF("perform_activity");
    _parent.increment_thread_count();
    _parent.bite_server(_ids, _delinquents, this);
    _parent.decrement_thread_count();
  }

private:
  cromp_client_tester &_parent;
  structures::set<octopus_request_id> _ids;  // the ids for commands we've sent.
  structures::set<octopus_request_id> _delinquents;  // missing ids during rcv.
};

//////////////

//hmmm: next stop; inject the types of items they're expecting in grab_items.

class grabby_thread : public ethread
{
public:
  grabby_thread(cromp_client_tester &parent)
  : ethread(), _parent(parent) {}

  void perform_activity(void *formal(ptr)) {
    while (!should_stop()) {
      _parent.grab_items();
      if (_rando.inclusive(0, 100) > 10)
        time_control::sleep_ms(_rando.inclusive(5, 38));
    }
  }

private:
  cromp_client_tester &_parent;
  chaos _rando;
};

//////////////

cromp_client_tester::cromp_client_tester()
: application_shell("cromp_client_tester"),
  _uplink(NIL),
  _lock(new mutex),
  _threads_active(0),
  _finished_loops(0.0),
  _encryption(false),
  _send_count(0),
  _thread_count(0),
  _grabber_count(0),
  _send_tries(0),
  _checkpoint_count(0),
  _dataseg_size(0),
  _report_interval(0),
  _snooze_duration(0),
  _rpc_style(false),
  _reconstruct_object(false),
  _server_loc()
{
  FUNCDEF("constructor");
  LOG("");
  LOG("");

  command_line args(application::_global_argc, application::_global_argv);
//LOG(a_sprintf("argc is %d and first is %s", application::_global_argc, application::_global_argv[0]));

  int indy = 0;
  if (args.find("help", indy, false)
      || (args.find("?", indy, false))
      || (args.find('?', indy, false)) ) {
    print_instructions();
    exit(0);
  }

  // check for a port on the command line.
  astring port_text;
  int port = 5678;
  if (args.get_value("port", port_text, false)) {
    LOG(astring("using port: ") + port_text);
    port = port_text.convert(5678);
  }
  _server_loc.port = port;

//hmmm:normalize host so this can take either name or IP.

  indy = 0;
  if (args.find("encrypt", indy, false) || (args.find('e', indy, true)) ) {
    // they're saying that we should encrypt the communication.
    LOG("turning on encryption.");
    _encryption = true;
  }

  indy = 0;
  if (args.find("rpc", indy, false) || (args.find('R', indy, true)) ) {
    // this is telling us to turn on RPC mode.  we will make each request and
    // reply pair synchronous, i.e., each reply will be awaited for when a
    // request has been made.
    LOG("turning on RPC style requests.");
    _rpc_style = true;
  }

  // check for a hostname on the command line.
  astring hostname("local");
  astring host_temp;
  if (args.get_value("host", host_temp, false)) {
    LOG(astring("using host: ") + host_temp);
    hostname = host_temp;
  }
LOG(astring("using host: ") + hostname);
  strcpy(_server_loc.hostname, hostname.s());

  astring send_temp;
  int send_count = MAXIMUM_SENDS;
  if (args.get_value("sends", send_temp, false)) {
    LOG(astring("using send count: ") + send_temp);
    send_count = send_temp.convert(send_count);
    if (send_count <= 0) send_count = 1;
  }
  _send_count = send_count;

  astring thread_temp;
  int thread_count = NUMBER_OF_THREADS;
  if (args.get_value("threads", thread_temp, false)) {
    LOG(astring("using thread count: ") + thread_temp);
    thread_count = thread_temp.convert(thread_count);
    if (thread_count <= 0) thread_count = 1;
  }
  _thread_count = thread_count;

  astring grabber_temp;
  int grabber_count = GRABBER_THREADS;
  if (args.get_value("grab", grabber_temp, false)) {
    LOG(astring("using grabber count: ") + grabber_temp);
    grabber_count = grabber_temp.convert(grabber_count);
    if (grabber_count < 0) grabber_count = 0;
  }
  _grabber_count = grabber_count;

  astring send_tries_temp;
  int send_tries = MAX_SEND_TRIES;
  if (args.get_value("trysend", send_tries_temp, false)) {
    LOG(astring("using send tries: ") + send_tries_temp);
    send_tries = send_tries_temp.convert(send_tries);
    if (send_tries < 0) send_tries = 0;
  }
  _send_tries = send_tries;

//hmmm: how tiresome.  how about a macro here?  could help in general
//      with command_line also.

  astring checkpoint_temp;
  int checkpoint_count = CHECKPOINT_SIZE;
  if (args.get_value("print", checkpoint_temp, false)) {
    LOG(astring("using checkpoint count: ") + checkpoint_temp);
    checkpoint_count = checkpoint_temp.convert(checkpoint_count);
    if (checkpoint_count <= 0) checkpoint_count = 1;
  }
  _checkpoint_count = checkpoint_count;

  astring dataseg_temp;
  int dataseg_size = DATA_SEGMENT_SIZE;
  if (args.get_value("dataseg", dataseg_temp, false)) {
    LOG(astring("using dataseg size: ") + dataseg_temp);
    dataseg_size = dataseg_temp.convert(dataseg_size);
    if (dataseg_size < 0) dataseg_size = 0;
  }
  _dataseg_size = dataseg_size;

  astring report_temp;
  int report_interval = REPORTING_INTERVAL;
  if (args.get_value("report", report_temp, false)) {
    LOG(astring("using report interval: ") + report_temp);
    report_interval = report_temp.convert(report_interval);
    if (report_interval <= 0) report_interval = 1;
    report_interval *= SECOND_ms;  // convert to milliseconds.
  }
  _report_interval = report_interval;

  astring snooze_temp;
  int snooze_duration = 0;  // no snooze by default.
  if (args.get_value("snooze", snooze_temp, false)) {
    LOG(astring("using snooze duration: ") + snooze_temp);
    snooze_duration = snooze_temp.convert(snooze_duration);
    if (snooze_duration < 0) snooze_duration = 0;
  }
  _snooze_duration = snooze_duration;

  if (args.find("reconstruct", indy, false)) {
    LOG("saw reconstruct flag; will periodically tear down object.");
    _reconstruct_object = true;
  }

LOG(astring("opening at ") + _server_loc.text_form());
  _uplink = new cromp_client(_server_loc);

  _uplink->add_tentacle(new bubbles_tentacle(false));
//we don't need backgrounding right now.
}

cromp_client_tester::~cromp_client_tester()
{
  WHACK(_lock);
  WHACK(_uplink);
}

int cromp_client_tester::print_instructions()
{
  astring name = filename(application::_global_argv[0]).basename().raw();
  log(a_sprintf("%s usage:", name.s()));
  log("");
  log(a_sprintf("\
This program connects to a cromp test server and exchanges packets to test\n\
the performance of the cromp protocol.  All command line flags are optional\n\
but can be added to specify how the test should be performed.  Currently,\n\
the valid options are:\n\
   --help\tShow this set of command-line help.\n\
   -?\t\tditto.\n\
   --port N\tConnect to the server on the port specified.\n\
   --host X\tConnect to server at IP address or hostname X.\n\
   --encrypt\tEncrypt the connection.  Server must do this also.\n\
   -e\t\tditto.\n\
   --sends N\tThe number of sends to perform.\n\
   --threads N\tNumber of threads competing for single cromp link.\n\
   --grab N\tNumber of additional threads stressing retrievals.\n\
   --trysend N\tCount of tries for sending if not all data went out.\n\
   --print N\tItems handled in between showing send counter.\n\
   --dataseg N\tSize of extra data packed in each test packet.\n\
   --report N\tDuration of time between reports, in seconds.\n\
   --snooze N\tSleep N ms between each send; this invalidates timing info.\n\
   --rpc\tEmulate Remote Procedure Call by awaiting each response.\n\
   -R\t\tditto\n\
"));
  return -3;
}

void cromp_client_tester::look_for_receipts(int count,
    structures::set<octopus_request_id> &ids,
    structures::set<octopus_request_id> &delinquents, bool wait)
{
  FUNCDEF("look_for_receipts");
  infoton *received = NIL;
  while (count--) {
    if (!ids.length()) break;  // nothing to check on.
    octopus_request_id the_id = ids[0];
    ids.zap(0, 0);  // take out the one we're inspecting right now.

    time_stamp start_acquire;
    int delay = MAXIMUM_ACQUISITION_DELAY;
    if (wait) delay = 2 * MINUTE_ms;  // force a long delay.
    outcome ret = _uplink->acquire(received, the_id, delay);
    int acquire_duration = int(time_stamp().value() - start_acquire.value());
    if (acquire_duration >= MAXIMUM_ACQUISITION_DELAY - 1) {
      LOG("passed time limit for acquire!  this is the faux dual-cpu bug!");
      LOG(a_sprintf("there were %d items left to acquire.", count));
      LOG(a_sprintf("pending %d bytes to send, %d bytes accumulated.",
          _uplink->pending_sends(), _uplink->accumulated_bytes()));
      LOG(a_sprintf("the data bin had %d items awaiting pickup.",
          _uplink->octo()->responses().items_held()));
      if (ret != cromp_client::TIMED_OUT) {
        LOG("cromp client lied about outcome??  didn't call this timed out!!");
      }
    }

    if (ret != cromp_client::OKAY) {
      if (ret != cromp_client::TIMED_OUT) {
        LOG(astring("failed to acquire the response--got error ")
                + cromp_client::outcome_name(ret));
        // give it another chance later.
        ids += the_id;
LOG(a_sprintf("moved %s back to main id queue.", the_id.text_form().s()));
      } else {
        if (delinquents.member(the_id))
          continuable_error(class_name(), func,
              astring("a delinquent response is still missing: ")
              + the_id.text_form());
        // if we hadn't already seen it, we'll watch for it next time.
        delinquents += the_id;
LOG(a_sprintf("added %s to delinquents.", the_id.text_form().s()));
      }
      return;
    }

if (!received) {
deadly_error(class_name(), func,
"received packet was NIL even though outcome was OKAY!");
}

    // check that the right type is coming back to us.
    bubble *cast = dynamic_cast<bubble *>(received);
    if (!cast) {
      continuable_error(class_name(), func, astring("got the wrong type "
          "of response: ") + received->classifier().text_form());
    }
 
    // if we had a problem with this item earlier, we remove it since it
    // succeeded this time.
    if (delinquents.member(the_id))
      delinquents.remove(the_id);
    WHACK(received);
  }
}

void cromp_client_tester::bite_server(structures::set<octopus_request_id> &ids,
    structures::set<octopus_request_id> &delinquents,
    void *originator)
{
  FUNCDEF("bite_server");
  octopus_request_id cmd_id;

///  LOG(timestamp(true, true) + " starting...");

  outcome ret;

  double overall_sent = 0;

  // this computes the size of the exchange object with no extra data attached.
  byte_array temp;
  bubble test_size(_dataseg_size, screen_rectangle(0, 120, 220, 280),
      238843);
  test_size.data().reset();
    // set the data segment to zero length.
  test_size.pack(temp);
  int base_length = temp.length();
    // this is the base packed length of the bubble object.

  int failure_count = 0;

  time_stamp start;  // record when our testing started.

  for (int sends = 1; sends <= _send_count; sends++) {
    bubble to_send(_dataseg_size, screen_rectangle(0, 120, 220, 280),
        238843);
    int curr_sending = to_send.data_length() + base_length * 2;
    overall_sent += curr_sending;
      // we compute the overall sent by what's sent in the request (which is
      // of the base length plus the attached array size) and the reply (which
      // is the base length only since the server resets the data attachment).
      // we go ahead and count it as sent before the send, since we're going
      // to bomb out if the send doesn't work.
    ret = _uplink->submit(to_send, cmd_id, _send_tries);
    switch (ret.value()) {
      case cromp_client::OKAY: {
        // complete success in sending that chunk out.
        ids.add(cmd_id);  // record it.
        if (_rpc_style) {
          // this call is used to force single requests and replies RPC style.
          look_for_receipts(1, ids, delinquents, true);
        }
        // sleep if we were asked to.
        if (_snooze_duration) {
          _uplink->keep_alive_pause(_snooze_duration, 60);
          look_for_receipts(1, ids, delinquents);
        }
        break;
      }
      case cromp_client::TOO_FULL: {
//treating as failure right now.
BASE_LOG("got too full outcome!");
        sends--;
        overall_sent -= curr_sending;
        continue;
        break;
      }
      case cromp_client::TIMED_OUT: {
//treating as failure right now.
BASE_LOG("got timed out outcome!");
        sends--;
        overall_sent -= curr_sending;
        continue;
        break;
      }
      default: {
        // a failure case that we have no other handling for.
        if (failure_count++ < 20) {
          sends--;  // skip back for the failed one.
          overall_sent -= curr_sending;  // remove unsent portion.
          LOG(astring("got failure outcome ") + cromp_client::outcome_name(ret)
              + " from attempt to submit request.");
          if (_snooze_duration) {
            _uplink->keep_alive_pause(_snooze_duration, 60);
          }
          continue;  // try again.
        }
        continuable_error(class_name(), func,
            astring("failed to submit the request--got error ")
                + cromp_client::outcome_name(ret));
        break;
      }
    }

    _finished_loops += 1.0;

    if (ids.elements() > MAXIMUM_PENDING_REQUESTS) {
      // grab some of the items waiting.  hopefully they're back by now.
      look_for_receipts(PENDING_REQUESTS_FORCED, ids, delinquents);
    }

    if (! (sends % _checkpoint_count)) {
      BASE_LOG(a_sprintf("%x send #%d", originator, sends));
    }
  }
  BASE_LOG(a_sprintf("%x final send #%d", originator, _send_count));

///  LOG(timestamp(true, true) + " done.");
///  LOG(a_sprintf("sent %d items.", _send_count));

  look_for_receipts(ids.elements(), ids, delinquents);

  LOG(a_sprintf("concluded %d test requests and responses.", _send_count));
}

void cromp_client_tester::grab_items()
{
  FUNCDEF("grab_items");
  octopus_request_id id(_uplink->entity(), -12);
    // look for an id we don't expect to have any thing waiting for.
  infoton *found = NIL;
  outcome ret = _uplink->retrieve_and_restore(found, id, 0);
  WHACK(found);
}

void cromp_client_tester::report(const time_stamp &start_time,
    double bytes_transmitted, double conversations)
{
  FUNCDEF("report");
  throughput_counter bandwidth;  // calculator for communication speed.
  double duration = time_stamp().value() - start_time.value();
    // the elapsed duration so far.
  bandwidth.add_run(bytes_transmitted, duration, conversations * 2);
    // create a portrait of how the run has progressed.  we multiply the
    // conversations by two since we are counting both the request and the
    // response (send and receive) as a transfer.

  // calculate the number of bytes per item for real as it plays out in
  // cromp sending.
  double bytes_per_item = bandwidth.bytes_sent() / bandwidth.number_of_sends();

  bubble my_bubble(_dataseg_size);  // an exemplar for our sends.

  // calculate how much space bubble's naming takes up.
  byte_array packed_classifier;
  structures::pack_array(packed_classifier, my_bubble.classifier());
  double classifier_size = packed_classifier.length() - sizeof(int);
    // that's how much space is used by our goofy classifier name.  there are
    // a few bytes extra overhead for packing a string array and we remove
    // them from consideration; we only want credit for the name, since that
    // is not truly overhead, given that the bubble infoton chose it.
  double payload_portion = my_bubble.packed_size() + classifier_size;
    // calculate the portion of our transmissions that are solely the
    // result of what we are putting into the package.
  double overhead = bytes_per_item - payload_portion;
    // okay, this is how many bytes per item is cromp noise, rather than
    // something the user is responsible for.
  double percent_overhead = overhead / bytes_per_item;

// change 0 to 1 to enable this section of information.
#if 0
  // get additional facts about how much of a packed infoton is wasted.
  byte_array packed_infote;
  infoton::fast_pack(packed_infote, my_bubble);
  log(a_sprintf("sane? -- overhead for just packed infoton is %d bytes.",
      packed_infote.length() - payload_portion));
  octopus_request_id example_request(_uplink->entity(), 23982);
  byte_array packed_req_id;
  example_request.pack(packed_req_id);
  log(a_sprintf("  -- overhead for octo request id is %d bytes.",
      packed_req_id.length()));
  byte_array packed_transa;
  cromp_transaction::flatten(packed_transa, my_bubble,
      octopus_request_id(_uplink->entity(), 23982));
  log(a_sprintf("  -- overhead for cromp transation is %d bytes.",
      packed_transa.length() - payload_portion));
#endif

  BASE_LOG(a_sprintf("sent %.0f items, %.0f bytes, %.0f bytes per item,%s"
      "payload %.0f bytes, overhead %.0f bytes, percent overhead %.1f%%,%s"
      "in %.2f seconds is %f ms/item%s"
      "at %.2f %cb/sec & %.2f items/sec.",
      bandwidth.number_of_sends(), bandwidth.bytes_sent(),
      bytes_per_item,
      parser_bits::platform_eol_to_chars(),
      payload_portion, overhead, percent_overhead * 100.0,
      parser_bits::platform_eol_to_chars(),
      bandwidth.total_time() / SECOND_ms,
      bandwidth.total_time() / bandwidth.number_of_sends(),
      parser_bits::platform_eol_to_chars(),
      (bandwidth.kilobytes_per_second() < 1024.0? 
        bandwidth.kilobytes_per_second()
        : bandwidth.megabytes_per_second()),
      (bandwidth.kilobytes_per_second() < 1024.0? 'K' : 'M'),
      bandwidth.number_of_sends() / (bandwidth.total_time() / SECOND_ms)));
}

void cromp_client_tester::cause_object_reconstruction()
{
  FUNCDEF("cause_object_reconstruction");
  int rando = chaos().inclusive(1, 100);
  if (rando > CHANCE_OF_RECONSTRUCT) return;  // not doing it this time.
  LOG(astring("reconstructing client at ") + _server_loc.text_form());
//below is not good when multiple threads are allowed to romp on client.
////  WHACK(_uplink);
////  _uplink = new cromp_client(_server_loc);
  
  _uplink->disconnect();
  outcome ret = common::INVALID;
  int counter = 100;  // allowed this many times to try to reconnect.
  while ( (ret != common::OKAY) && (counter-- >= 0) ) {
    ret = _uplink->connect();
    if (ret != cromp_client::OKAY) {
      LOG(astring("couldn't reconnect this time: ")
          + cromp_client::outcome_name(ret));
      time_control::sleep_ms(420);
    }
  }
}

int cromp_client_tester::execute()
{
  FUNCDEF("execute");

  // testing that crompish pax are done right.
  bubble fud(randomizer().inclusive(12, 2829));
  byte_array packed_fud;
  fud.pack(packed_fud);
  if (packed_fud.length() != fud.packed_size())
    deadly_error(class_name(), func, "bubble's packed size method is wrong.");

  if (_encryption) _uplink->enable_encryption();

  outcome ret = _uplink->connect();
  if (ret != cromp_client::OKAY) {
    deadly_error(class_name(), func, astring("connection failed with error: ")
        + cromp_client::outcome_name(ret));
  }

  thread_cabinet cab;  // we store a bunch of threads here.

  LOG(a_sprintf("adding %d grabber threads to test.", _grabber_count));

  // create the extra grabber threads.
  for (int i = 0; i < _grabber_count; i++) {
    grabby_thread *to_add = new grabby_thread(*this);
    cab.add_thread(to_add, false, NIL);
  }

  LOG(a_sprintf("adding %d transmitter threads to test.", _thread_count));

  // create the specified number of threads.
  for (int j = 0; j < _thread_count; j++) {
    bitey_thread *to_add = new bitey_thread(*this);
    cab.add_thread(to_add, false, NIL);
  }

//LOG("starting all threads...");
  time_stamp start;
  cab.start_all(NIL);
//LOG("done starting threads...");

  time_control::sleep_ms(400);  // wait until a few get cranked up.

//LOG("did our initial sleep...");

  while (cab.any_running()) {
    time_control::sleep_ms(30);
    if (!_threads_active) {
      break;
    }
//LOG("main loop...");
    if (time_stamp(-_report_interval) > _last_report) {
      report(start, cromp_common::total_bytes_sent()
          + cromp_common::total_bytes_received(),
          _finished_loops);
      _last_report.reset();
    }
    if (_reconstruct_object) {
      cause_object_reconstruction();
    }
    if (!_uplink->connected()) {
      LOG("connection dropped.  trying to connect again.");
      outcome ret = _uplink->connect();
      if (ret != cromp_client::OKAY) {
        // snooze a bit so as not to drive server crazy or log too much noise.
        time_control::sleep_ms(10 * SECOND_ms);
      }
    }
  }

  LOG("- done testing -");

  if (_finished_loops != double(_thread_count) * _send_count)
    LOG(a_sprintf("number of loops was calculated differently: wanted %d, "
        "got %d", _thread_count * _send_count, _finished_loops));

  report(start, cromp_common::total_bytes_sent()
      + cromp_common::total_bytes_received(),
      _thread_count * _send_count);

//LOG("stopping all threads...");
  cab.stop_all();
  LOG("all threads exited.");

#ifdef DEBUG_TESTER
///  LOG("hit a key to continue...");
///  int char_read = fgetc(stdin);
#endif

  BASE_LOG("cromp_client:: works for those functions tested.");

  return 0;
}

//////////////

HOOPLE_MAIN(cromp_client_tester, )

