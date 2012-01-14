#ifndef SEQUENCE_TRACKER_CLASS
#define SEQUENCE_TRACKER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : sequence_tracker                                                  *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Tracks sequence numbers coming from a collection of hosts.  These are    *
*  presumably attached to network packets.  The intention is to record if     *
*  we've already seen a packet or not.  When hosts have not communicated for  *
*  a while, they are removed from tracking.  Also, when enough time has       *
*  elapsed for a sequence number, we consider that we've heard everything     *
*  we're going to before that sequence number; hence, any prior sequence      *
*  numbers are considered already received.                                   *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/contracts.h>
#include <basis/mutex.h>
#include <timely/time_stamp.h>

namespace sockets {

class host_history;
class machine_uid;

//! this will keep track of sequencing for a communication process on a per host basis.

class sequence_tracker : public virtual basis::root_object
{
public:
  sequence_tracker(int coalesce_time, int silence_time);
    // tracks the sequence numbers from a set of hosts.  the "coalesce_time" is
    // the interval that we wait before considering all prior sequence numbers
    // to have been received.  the "silence_time" is the time interval a host
    // is allowed to be silent before being eliminated.  all measurements are
    // in milliseconds.

  ~sequence_tracker();

  DEFINE_CLASS_NAME("sequence_tracker");

  void add_pair(const machine_uid &host, int sequence);
    // adds a hostname/sequence# pair as being received "now".

  bool have_seen(const machine_uid &host, int sequence);
    // returns true if the "host" and "sequence" have already been seen in
    // a previous transmission.

  void clean_up();
    // this must be invoked periodically to allow the clearing of outdated
    // information.  once a second seems frequent enough.

  basis::astring text_form(bool verbose = false) const;
    // provides a dump of the information held in the tracker.

private:
  int _coalesce_time;  // sequences older than this get coalesced.
  int _silence_time;  // hosts silent for longer than this get canned.
  host_history *_hosts;  // the overall record of sequence activity per host.
  basis::mutex *_lock;  // protects from multi-threaded access.
};

} //namespace.

#endif

