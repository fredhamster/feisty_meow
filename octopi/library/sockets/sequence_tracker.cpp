/*****************************************************************************\
*                                                                             *
*  Name   : sequence_tracker                                                  *
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

#include "machine_uid.h"
#include "sequence_tracker.h"

#include <basis/functions.h>
#include <basis/astring.h>

#include <basis/mutex.h>
#include <loggers/program_wide_logger.h>
#include <structures/amorph.h>
#include <structures/int_hash.h>
#include <textual/parser_bits.h>
#include <timely/time_stamp.h>

using namespace basis;
using namespace loggers;
using namespace structures;
using namespace textual;
using namespace timely;

namespace sockets {

const int MAX_BITS_FOR_SEQ_HASH = 10;
  // the number of bits in the hash table of sequences, allowing 2^max buckets.

const int CLEANING_SPAN = 20000;
  // if the sequence number is this far off from the one received, we will
  // clean up the span list.

const int MAX_ITEMS = 200;
  // maximum number of items in tracker.  this is quite low since we don't
  // want to be lugging around thousands of indices.  for connection oriented,
  // it will never be much of an issue, although for a broadcast style bus it
  // could be kind of an issue if we do retransmissions with a lot of lag.

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

//hmmm: we need to address when a host has:
//        a) rolled over in sequences (not for 4 years at least?)
//        b) changed its address where another host now has old address (which
//           is also a weirdo sequence jump, maybe backwards, maybe not).
//      would our timing help to guarantee that any oddness introduced is
//      swamped out in a few minutes?  the worst thing would be a lost packet
//      we dumped because we thought we'd seen it but hadn't.
//      probably we will see a sequence that's really old seeming; should that
//      be enough to trigger flushing the whole host?

class sequence_record
{
public:
  int _sequence;  // the sequence number in question.
  time_stamp _when;  // when we received this sequence.

  sequence_record(int seq = 0) : _sequence(seq) {}

  astring text_form() const {
    return a_sprintf("seq=%d, time=", _sequence) + _when.text_form();
  }
};

//////////////

class host_record
{
public:
  int _received_to;  // highest sequence we've got full reception prior to.
  machine_uid _host;  // the host we're concerned with.
  int_hash<sequence_record> _sequences;  // record of active sequences.
  time_stamp _last_active;  // the last time we heard from this host.
    // we could piece this together from the sequences but we prefer not to.

  host_record(const machine_uid &host)
  : _received_to(0), _host(host), _sequences(MAX_BITS_FOR_SEQ_HASH),
    _last_active()
  {}

  void clean_up(int coalesce_time) {
    // check sequences for being right next to the highest received sequence.
    // if they're one up, they can be collapsed without waiting for the aging
    // process.
    int_set ids;
    _sequences.ids(ids);

    // we restrict the size of the array with this block.
    if (ids.elements() > MAX_ITEMS) {
      int zap_point = ids.elements() - MAX_ITEMS;
        // we want to remove anything before this index.
      for (int s0 = 0; s0 < zap_point; s0++) {
        int seq = ids[s0];
        _sequences.zap(seq);
        // set our received_to value from the current element.
        if (_received_to < seq)
          _received_to = seq;
      }
      // now clean the list of our ids since they're gone.
      ids.zap(0, zap_point - 1);
    }

    for (int s1 = 0; s1 < ids.elements(); s1++) {
      sequence_record *seq;
      int id = ids[s1];
      if (!_sequences.find(id, seq)) continue;  // bad mistake going on.
      if (_received_to + 1 == seq->_sequence) {
        // we've hit one that can be collapsed.
        _received_to++;
        _sequences.zap(id);
        ids.zap(s1, s1);
        s1--;  // skip back before deleted item.
      }
    }

    // check sequence ages.  coalesce any older ones.
    for (int s2 = 0; s2 < ids.elements(); s2++) {
      sequence_record *seq;
      int id = ids[s2];
      if (!_sequences.find(id, seq)) continue;  // bad mistake going on.
      if (seq->_when < time_stamp(-coalesce_time)) {
        // this sequence number has floated too long; crush it now.
        if (_received_to < seq->_sequence)
          _received_to = seq->_sequence;  // update highest received seq.
        _sequences.zap(id);
      }
    }
  }

  astring text_form(bool verbose) {
    astring to_return;
    to_return += astring("host=") + _host.text_form()
        + a_sprintf(", rec_to=%d", _received_to)
        + ", active=" + _last_active.text_form();
    if (verbose) {
      int_set ids;
      _sequences.ids(ids);
      for (int i = 0; i < ids.elements(); i++) {
        sequence_record *found;
        if (!_sequences.find(ids[i], found))
          continue;  // that's a bad thing.
        to_return += astring(parser_bits::platform_eol_to_chars()) + "\t"
            + found->text_form();
      }
    } else {
      to_return += a_sprintf(", sequences held=%d", _sequences.elements());
    }
    return to_return;
  }

};

//////////////

//hmmm: should this be a hash table?

class host_history : public amorph<host_record>
{
public:
  virtual ~host_history() {}

  DEFINE_CLASS_NAME("host_history");

  int find_host(const machine_uid &to_find) {
    for (int i = 0; i < elements(); i++) {
      if (borrow(i)->_host == to_find) return i;
    }
    return common::NOT_FOUND;
  }

  bool whack_host(const machine_uid &to_find) {
    int indy = find_host(to_find);
    if (negative(indy)) return false;
    zap(indy, indy);
    return true;
  }

  void clean_up(int silence_time, int coalesce_time) {
    for (int h = 0; h < elements(); h++) {
      host_record &rec = *borrow(h);
      // check host liveliness.
      if (rec._last_active < time_stamp(-silence_time)) {
        // this host got too stale; whack it now.
        zap(h, h);
        h--;  // skip back to prior element.
        continue;
      }
      rec.clean_up(coalesce_time);
    }
  }

  bool add_sequence(const machine_uid &to_find, int sequence,
      int silence_time, int coalesce_time) {
    FUNCDEF("add_sequence");
    int indy = find_host(to_find);
    if (negative(indy)) {
      host_record *rec = new host_record(to_find);
      append(rec);
      indy = find_host(to_find);
      if (negative(indy)) {
        LOG(astring("*** failure to add a host to the tracker: ")
            + to_find.text_form());
        return false;  // serious error.
      }
    }
    host_record &rec = *borrow(indy);
    if (borrow(indy)->_received_to + 1 == sequence) {
      // this is just one up from our last received guy, so optimize it out.
      rec._received_to = sequence;
    } else if (sequence - borrow(indy)->_received_to > CLEANING_SPAN) {
      // if the number is wildly different, assume we haven't dealt with this
      // for too long.
      rec._received_to = sequence;
#ifdef DEBUG_SEQUENCE_TRACKER
      LOG("sequence is wildly different, cleaning.");
#endif
      clean_up(silence_time, coalesce_time);
    } else {
      // standard treatment, add it to the list.
      rec._sequences.add(sequence, new sequence_record(sequence));
      if (rec._sequences.elements() > MAX_ITEMS) {
        // too many sequences floating around now.  clean them up.
        clean_up(silence_time, coalesce_time);
      }
    }
    rec._last_active = time_stamp();
    return true;
  }

  astring text_form(bool verbose) {
    astring to_return;
    for (int i = 0; i < elements(); i++) {
      to_return += borrow(i)->text_form(verbose);
      if (i < elements() - 1)
        to_return += parser_bits::platform_eol_to_chars();
    }
    return to_return;
  }

};

//////////////

sequence_tracker::sequence_tracker(int coalesce_time, int silence_time)
: _coalesce_time(coalesce_time),
  _silence_time(silence_time),
  _hosts(new host_history),
  _lock(new mutex)
{
}

sequence_tracker::~sequence_tracker()
{
  WHACK(_lock);
  WHACK(_hosts);
}

astring sequence_tracker::text_form(bool verbose) const
{
  auto_synchronizer l(*_lock);
  return _hosts->text_form(verbose);
}

void sequence_tracker::add_pair(const machine_uid &host, int sequence)
{
  auto_synchronizer l(*_lock);
  if (!_hosts->add_sequence(host, sequence, _silence_time, _coalesce_time)) {
//complain? 
    return;
  }
}

bool sequence_tracker::have_seen(const machine_uid &host, int sequence)
{
  auto_synchronizer l(*_lock);
  int indy = _hosts->find_host(host);
  if (negative(indy)) return false;
  host_record &rec = *_hosts->borrow(indy);
  if (sequence <= rec._received_to) return true;
  sequence_record *found;
  return !!rec._sequences.find(sequence, found);
}

void sequence_tracker::clean_up()
{
  auto_synchronizer l(*_lock);
  _hosts->clean_up(_silence_time, _coalesce_time);
}

} //namespace.


