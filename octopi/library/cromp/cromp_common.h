#ifndef CROMP_COMMON_CLASS
#define CROMP_COMMON_CLASS

/*
*  Name   : cromp_common
*  Author : Chris Koeritz
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <crypto/rsa_crypto.h>
#include <octopus/octopus.h>
#include <octopus/entity_defs.h>
#include <octopus/entity_data_bin.h>
#include <octopus/infoton.h>
#include <sockets/internet_address.h>
#include <sockets/machine_uid.h>
#include <sockets/spocket.h>
#include <sockets/tcpip_stack.h>

namespace cromp {

const int DEFAULT_MAX_ENTITY_QUEUE = 14 * basis::MEGABYTE;
  //!< the default size we allow per each entity.
  /*!<  note that if there are many entities and they're all getting full, the total allowed
  will be this number multiplied by the number of entities. */

//! A few common features used by both CROMP clients and servers.

class cromp_common : public virtual basis::root_object
{
public:
  cromp_common(const basis::astring &host, int max_per_ent);
    // constructs a normal common object.  open_common() must be invoked before
    // this object becomes useful.  the "host" should be the actual TCPIP host
    // that this program is running on.  the "max_per_ent" is the maximum
    // allowed size (in bytes) of pending items per entity.

  cromp_common(sockets::spocket *preexisting, octopi::octopus *singleton);
    // uses a "preexisting" spocket object for the communications needed here.
    // the "singleton" octopus is used instead of our base class for restoring
    // any data.  note that "singleton" is not dealt with in the destructor;
    // it is considered owned externally.  however, the "preexisting" spocket
    // is considered to be owned by this class now.  also note that if a NULL_POINTER
    // "preexisting" socket is passed, then socket creation occurs by the
    // normal process.

  virtual ~cromp_common();

  static int default_port();
    // returns the default port used by cromp and octopus.  this is not
    // appropriate for most derived apps; they should use their own ports.

  DEFINE_CLASS_NAME("cromp_common");

  basis::outcome open_common(const sockets::internet_address &where);
    // opens the object to begin communication.  this is the first point at
    // which the socket is opened.

  basis::outcome close_common();
    // shuts down our presence on the network.

  sockets::spocket *spock() const;
    // allows external access to our socket object.  do not abuse this.
    // also keep in mind that in some stages of construction, this can return
    // NULL_POINTER.  do not assume it is non-null.

  sockets::internet_address other_side() const;
    // returns the location that we're connected to, if any.

  virtual basis::outcome add_tentacle(octopi::tentacle *to_add, bool filter = false);
    // allows customization of the processing that the cromp object performs.

  enum outcomes {
    OKAY = basis::common::OKAY,
    DISALLOWED = basis::common::DISALLOWED,
    BAD_INPUT = basis::common::BAD_INPUT,
    NOT_FOUND = basis::common::NOT_FOUND,
    TIMED_OUT = basis::common::TIMED_OUT,
    GARBAGE = basis::common::GARBAGE,
    NO_HANDLER = basis::common::NO_HANDLER,
    PARTIAL = basis::common::PARTIAL,
    ENCRYPTION_MISMATCH = basis::common::ENCRYPTION_MISMATCH,

    NO_SERVER = sockets::communication_commons::NO_SERVER,
    NO_CONNECTION = sockets::communication_commons::NO_CONNECTION,

    DEFINE_OUTCOME(TOO_FULL, -40, "The request cannot be processed yet")
  };

  static const char *outcome_name(const basis::outcome &to_name);

  static basis::astring chew_hostname(const sockets::internet_address &addr,
          sockets::internet_address *resolved = NULL_POINTER);
    // resolves the hostname in "addr" and returns the resolved hostname as
    // a machine_uid compact_form().  the "resolved" form of the address is
    // also stored if the pointer is non-null.

  basis::astring responses_text_form() const;
    // returns a textual form of the responses awaiting pickup.

  bool buffer_clog(int clog_point = 1 * basis::MEGABYTE) const;
    // returns true if the buffer is bigger than a certain "clog_point".

  basis::outcome pack_and_ship(const octopi::infoton &request,
          const octopi::octopus_request_id &item_id, int max_tries);
    // requests a transaction from the other side, specified by the "request".
    // the return value is OKAY if the request was successfully sent or it
    // will be another outcome that indicates a failure of transmission.
    // the "item_id" must be set ahead of time to identify the request and
    // sender.  the "max_tries" limits how many times the sending is
    // reattempted on failure.

  basis::outcome pack_and_ship(const octopi::infoton_list &requests, int max_tries);
    // sends a batch of "requests" in one blast.

  basis::outcome retrieve_and_restore(octopi::infoton * &item,
          const octopi::octopus_request_id &req_id, int timeout);
    // attempts to pull down data from our spocket and process it back into
    // an infoton in "item".   only the specific object for the "req_id"
    // will be provided.  if the "timeout" is non-zero, then data will be
    // awaited that long.  if "timeout" is zero, the method will return
    // immediately if the data is not already available.

  basis::outcome retrieve_and_restore_any(octopi::infoton * &item, octopi::octopus_request_id &req_id,
          int timeout);
    // returns the first infoton that becomes available.  the "req_id" is set
    // to the identifier from the transaction.  this is useful more for the
    // server side than for the client side.

  basis::outcome push_outgoing(int max_tries);
    // composes calls to grab_anything and send_buffer into an attempt to
    // get all of the data out that is waiting while not ignoring the incoming
    // data from the other side.

  void grab_anything(bool wait);
    // attempts to locate any data waiting for our socket.  any infotons
    // found get stuffed into the requests bin.  this is never needed for
    // cromp_clients; the retrieve methods will automatically invoke this
    // as appropriate.  if "wait" is true, then a data pause will occur
    // on the socket to await data.

  basis::outcome send_buffer();
    // pushes out any pending data waiting for the other side.  the returns
    // can range the gamut, but OKAY and PARTIAL are both successful outcomes.
    // OKAY means that everything was sent successfully (or there was nothing
    // to send).  PARTIAL means that not all the data was sent, but some got
    // out successfully.  any other errors probably indicate a major problem.

  // these adjust the storage sizes allowed internally.
  int max_bytes_per_entity() const;
  void max_bytes_per_entity(int max_bytes_per_entity);

  // provides information on the overall number of bytes encountered by all
  // cromp clients in this program.  these are only intended for testing
  // purposes and might in fact roll over for a busy client app.
  static double total_bytes_sent() { return _bytes_sent_total; }
  static double total_bytes_received() { return _bytes_received_total; }

  static const int HOSTCHOP;
    // this is the number of characters from the host's name that go into the
    // octopus identity.  the portion after that many characters is a compacted
    // form of the machine_uid.

  static bool decode_host(const basis::astring &coded_host, basis::astring &hostname,
           sockets::machine_uid &machine);
    // takes the "coded_host" from an entity and returns the "hostname" and
    // "machine" information that was encoded in it.  those will be gibberish
    // unless true is returned.

  octopi::octopus *octo() const { return _octopus; }
    // returns our octopus support object.  this should always exist when this
    // object is constructed properly.

  static const crypto::rsa_crypto &localhost_only_key();
    // this key should *only* be used for speeding up encryption on the local
    // host.  it is generated when the first caller needs it but then is
    // a constant key during the program's runtime.  this object can be used
    // for initializing services when it is _known_ that they are connecting
    // only on the localhost; that's the only place it should be used because
    // re-using the key on the network provides less security than using
    // the randomized encryption startup in cromp.

  int pending_sends() const;
    //!< returns the number of bytes still unsent.

  int accumulated_bytes() const;
    //!< returns the number of bytes pending processing from the other side.

protected:
  octopi::octopus *singleton() const { return _singleton; }
    // returns the singleton octopus passed to the constructor earlier.
    // this will return NULL_POINTER if it was not constructed that way.

private:
  sockets::spocket *_commlink;  // transceiver for data.
  octopi::octopus *_octopus;  // main octopus; might be same as singleton.
  octopi::octopus *_singleton;  // used for dependent cromp server objects.
  octopi::entity_data_bin *_requests;  // where the incoming requests are stored.
  basis::mutex *_accum_lock;  // protects the accumulator and other data below.
  timely::time_stamp *_last_data_seen;  // last time we got anything on socket.
  basis::byte_array *_accumulator;  // accumulates data for this object.
  basis::byte_array *_sendings;  // accumulates outgoing sends when socket is full.
  basis::byte_array *_receive_buffer;  // temporary buffer.
  basis::byte_array *_still_flat;  // another temporary buffer.
  timely::time_stamp *_last_cleanup;  // when we last cleaned out our bin.

  // helps for testing; not used for operation.
  static double _bytes_sent_total;
  static double _bytes_received_total;

  void snarf_from_socket(bool wait);
    // retrieves data waiting on the socket and adds to the accumulator.
    // if "wait" is true, then the presence of data is awaited first.

  void process_accumulator();
    // chews on the accumulated data to seek any commands that are present.

  void conditional_cleaning();
    // flushes out any old items if they haven't been cleaned in a bit.

  basis::outcome retrieve_and_restore_root(bool get_anything, octopi::infoton * &item,
          octopi::octopus_request_id &req_id, int timeout);
    // used for both types of retrieval; if "get_anything" is true, then
    // any old item will be returned.  if "get_anything" is false, then only
    // the item with "req_id" is returned.
};

} //namespace.

#endif

