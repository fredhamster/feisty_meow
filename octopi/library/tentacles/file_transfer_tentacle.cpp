/*****************************************************************************\
*                                                                             *
*  Name   : file_transfer_tentacle                                            *
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

#include "file_transfer_tentacle.h"

#include <basis/mutex.h>
#include <filesystem/directory_tree.h>
#include <filesystem/filename.h>
#include <filesystem/filename_list.h>
#include <filesystem/heavy_file_ops.h>
#include <loggers/program_wide_logger.h>
#include <octopus/entity_defs.h>
#include <octopus/unhandled_request.h>
#include <processes/ethread.h>
#include <textual/parser_bits.h>

using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace octopi;
using namespace processes;
using namespace structures;
using namespace textual;
using namespace timely;

namespace octopi {

#undef AUTO_LOCK
#define AUTO_LOCK auto_synchronizer loc(*_lock);
  // protects our lists.

const int FTT_CLEANING_INTERVAL = 30 * SECOND_ms;
  // this is how frequently we clean up the list to remove outdated transfers.

const int TRANSFER_TIMEOUT = 10 * MINUTE_ms;
  // if it hasn't been touched in this long, it's out of there.

#define DEBUG_FILE_TRANSFER_TENTACLE
  // uncomment for noisier version.

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

//////////////

class file_transfer_record 
{
public:
  // valid for both transfers and correspondences.
  astring _src_root;  // where the info is on the data provider.
  time_stamp _last_active;  // when this was last used.

  // valid for file transfers only.
  octopus_entity _ent;  // the entity requesting this service.
  astring _dest_root;  // where the info is on the data sink.
  filename_list *_diffs;  // the differences to be transferred.
  file_transfer_header _last_sent;  // the last chunk that was sent.
  bool _done;  // true if the transfer is finished.
  string_array _includes;  // the set to include.

  // valid for correspondence records only.
  directory_tree *_local_dir;  // our local information about the transfer.
  astring _source_mapping;  // valid for a correspondence record.
  int _refresh_interval;  // the rate of refreshing the source tree.

  file_transfer_record() : _diffs(NIL), _last_sent(file_time()),
      _done(false), _local_dir(NIL)
  {}

  ~file_transfer_record() {
    WHACK(_local_dir);
    WHACK(_diffs);
  }

  astring text_form() const {
    astring to_return;
    to_return += astring("src=") + _src_root + astring(" last act=")
        + _last_active.text_form();
    if (_ent.blank()) to_return += astring(" ent=") + _ent.text_form();
    if (_dest_root.t()) {
      to_return += astring(" dest=") + _dest_root;
      to_return += astring(" last_sent=") + _last_sent.text_form();
    }
    return to_return;
  }
};

//////////////

// this implementation assumes that the same entity will never simultaneously
// transfer the same source to the same destination.  that assumption holds
// up fine for different clients, since they should have different entities.
// when there is a collision on the entity/src/dest, then the default action
// is to assume that the transfer is just being started over.

class file_transfer_status : public amorph<file_transfer_record>
{
public:
  // find a transfer record by the key fields.
  file_transfer_record *find(const octopus_entity &ent, const astring &src,
      const astring &dest) {
    for (int i = 0; i < elements(); i++) {
      const file_transfer_record *rec = get(i);
      if (rec && (rec->_ent == ent) && (rec->_src_root == src)
          && (rec->_dest_root == dest) ) {
        return borrow(i);
      }
    }
    return NIL;
  }

  virtual ~file_transfer_status() {}

  DEFINE_CLASS_NAME("file_transfer_status");

  // find a file correspondence record by the mapping name.
  file_transfer_record *find_mapping(const astring &source_mapping) {
    for (int i = 0; i < elements(); i++) {
      const file_transfer_record *rec = get(i);
      if (rec && (rec->_source_mapping == source_mapping) )
        return borrow(i);
    }
    return NIL;
  }

  // turns a source mapping into the location that it corresponds to.
  astring translate(const astring &source_path) const {
//    FUNCDEF("translate");
    string_array pieces;
    filename(source_path).separate(pieces);
    astring source_mapping = pieces[0];
    pieces.zap(0, 0);  // remove source part.

    for (int i = 0; i < elements(); i++) {
      const file_transfer_record *rec = get(i);
      if (rec && (rec->_source_mapping == source_mapping) ) {
        return rec->_src_root;
      }
    }
    return astring::empty_string();
  }

  // removes a file transfer record by the key fields.
  bool whack(const octopus_entity &ent, const astring &src,
      const astring &dest) {
    for (int i = 0; i < elements(); i++) {
      const file_transfer_record *rec = get(i);
      if (rec && (rec->_ent == ent) && (rec->_src_root == src)
          && (rec->_dest_root == dest) ) {
        zap(i, i);
        return true;
      }
    }
    return false;
  }

  // clean all records for the entity "ent".
  void whack_all(const octopus_entity &ent) {
    for (int i = elements() - 1; i >= 0; i--) {
      const file_transfer_record *rec = get(i);
      if (rec && (rec->_ent == ent) )
        zap(i, i);
    }
  }

  // removes a file transfer correspondence.
  bool whack_mapping(const astring &source_mapping) {
    for (int i = elements() - 1; i >= 0; i--) {
      const file_transfer_record *rec = get(i);
      if (rec && (rec->_source_mapping == source_mapping) ) {
        zap(i, i);
        return true;
      }
    }
    return false;
  }

  // returns a string dump of the fields in this list.
  astring text_form() const {
    astring to_return;
    for (int i = 0; i < elements(); i++) {
      const file_transfer_record *rec = get(i);
      if (rec)
        to_return += rec->text_form() + parser_bits::platform_eol_to_chars();
    }
    return to_return;
  }
};

//////////////

class file_transfer_cleaner : public ethread
{
public:
  file_transfer_cleaner(file_transfer_tentacle &parent)
      : ethread(FTT_CLEANING_INTERVAL, SLACK_INTERVAL), _parent(parent) {}

  virtual void perform_activity(void *formal(ptr)) { _parent.periodic_actions(); }

private:
  file_transfer_tentacle &_parent;
};

//////////////

file_transfer_tentacle::file_transfer_tentacle(int maximum_transfer,
    file_transfer_tentacle::transfer_modes mode_of_transfer)
: tentacle_helper<file_transfer_infoton>
      (file_transfer_infoton::file_transfer_classifier(), false),
  _maximum_transfer(maximum_transfer),
  _transfers(new file_transfer_status),
  _correspondences(new file_transfer_status),
  _lock(new mutex),
  _cleaner(new file_transfer_cleaner(*this)),
  _mode(mode_of_transfer)
{
  _cleaner->start(NIL);
}

file_transfer_tentacle::~file_transfer_tentacle()
{
  _cleaner->stop();
  WHACK(_transfers);
  WHACK(_correspondences);
  WHACK(_cleaner);
  WHACK(_lock);
}

astring file_transfer_tentacle::text_form() const
{
  AUTO_LOCK;
  return _transfers->text_form();
}

void file_transfer_tentacle::expunge(const octopus_entity &to_remove)
{
  AUTO_LOCK;
  _transfers->whack_all(to_remove);
}

outcome file_transfer_tentacle::add_correspondence
    (const astring &source_mapping, const astring &source_root,
     int refresh_interval)
{
#ifdef DEBUG_FILE_TRANSFER_TENTACLE
  FUNCDEF("add_correspondence");
#endif
  AUTO_LOCK;

  remove_correspondence(source_mapping);  // clean the old one out first.

  // create new file transfer record to hold this correspondence.
  file_transfer_record *new_record = new file_transfer_record;
  new_record->_source_mapping = source_mapping;
  new_record->_src_root = source_root;
  new_record->_refresh_interval = refresh_interval;
  new_record->_local_dir = new directory_tree(source_root);
//hmmm: doesn't say anything about a pattern.  do we need to worry about that?

  // check that the directory looked healthy.
  if (!new_record->_local_dir->good()) {
    WHACK(new_record);
    return common::ACCESS_DENIED;
  }
#ifdef DEBUG_FILE_TRANSFER_TENTACLE
  LOG(astring("adding tree for: ent=") + new_record->_ent.text_form()
      + " src=" + new_record->_src_root + " dest=" + new_record->_dest_root);
#endif
  // calculate size and checksum info for the directory.
  new_record->_local_dir->calculate( !(_mode & COMPARE_CONTENT_SAMPLE) );

#ifdef DEBUG_FILE_TRANSFER_TENTACLE
  LOG(astring("done adding tree for: ent=") + new_record->_ent.text_form()
      + " src=" + new_record->_src_root + " dest=" + new_record->_dest_root);
#endif

  _correspondences->append(new_record);

  return OKAY;
}

outcome file_transfer_tentacle::remove_correspondence
    (const astring &source_mapping)
{
  AUTO_LOCK;
  if (!_correspondences->whack_mapping(source_mapping))
    return NOT_FOUND;
  return OKAY;
}

bool file_transfer_tentacle::get_differences(const octopus_entity &ent,
    const astring &src, const astring &dest, filename_list &diffs)
{
//  FUNCDEF("get_differences");
  diffs.reset();
  AUTO_LOCK;
  file_transfer_record *the_rec = _transfers->find(ent, src, dest);
  if (!the_rec) return false;
  if (!the_rec->_diffs) return false;  // no diffs listed.
  diffs = *the_rec->_diffs;
  return true;
}

bool file_transfer_tentacle::status(const octopus_entity &ent,
    const astring &src, const astring &dest, double &total_size,
    int &total_files, double &current_size, int &current_files, bool &done,
    time_stamp &last_active)
{
//  FUNCDEF("status");
  total_size = 0;
  total_files = 0;
  current_files = 0;
  current_size = 0;
  AUTO_LOCK;
  file_transfer_record *the_rec = _transfers->find(ent, src, dest);
  if (!the_rec) return false;
  done = the_rec->_done;
  last_active = the_rec->_last_active;

  if (the_rec->_diffs) {
    the_rec->_diffs->calculate_progress(the_rec->_last_sent._filename,
        the_rec->_last_sent._byte_start + the_rec->_last_sent._length,
        current_files, current_size);
    total_files = the_rec->_diffs->total_files();
    total_size = the_rec->_diffs->total_size();
  }

  return true;
}

outcome file_transfer_tentacle::register_file_transfer
    (const octopus_entity &ent, const astring &src_root,
    const astring &dest_root, const string_array &includes)
{
//  FUNCDEF("register_file_transfer");
  AUTO_LOCK;
  // make sure that this isn't an existing transfer.  if so, we just update
  // the status.
  file_transfer_record *the_rec = _transfers->find(ent, src_root, dest_root);
  if (!the_rec) {
    the_rec = new file_transfer_record;
    the_rec->_src_root = src_root;
    the_rec->_dest_root = dest_root;
    the_rec->_ent = ent;
    the_rec->_includes = includes;
    _transfers->append(the_rec);  // add the new record.
  } else {
    the_rec->_done = false;
    the_rec->_includes = includes;
    the_rec->_last_active.reset();  // freshen up the last activity time.
  }
  return OKAY;
}

outcome file_transfer_tentacle::cancel_file_transfer(const octopus_entity &ent,
    const astring &src_root, const astring &dest_root)
{
  AUTO_LOCK;
  return _transfers->whack(ent, src_root, dest_root)?  OKAY : NOT_FOUND;
}

directory_tree *file_transfer_tentacle::lock_directory(const astring &key)
{
  _lock->lock();
  file_transfer_record *the_rec = _correspondences->find_mapping(key);
  if (!the_rec || !the_rec->_local_dir) {
    _lock->unlock();
    return NIL;  // unknown transfer.
  }
  return the_rec->_local_dir;
}

void file_transfer_tentacle::unlock_directory()
{
  _lock->unlock();
}

bool file_transfer_tentacle::add_path(const astring &key,
    const astring &new_path)
{
  AUTO_LOCK;
  file_transfer_record *the_rec = _correspondences->find_mapping(key);
  if (!the_rec) return false;  // unknown transfer.
  if (!the_rec->_local_dir) return false;  // not right type.
  return the_rec->_local_dir->add_path(new_path) == common::OKAY;
}

bool file_transfer_tentacle::remove_path(const astring &key,
    const astring &old_path)
{
  AUTO_LOCK;
  file_transfer_record *the_rec = _correspondences->find_mapping(key);
  if (!the_rec) return false;  // unknown transfer.
  if (!the_rec->_local_dir) return false;  // not right type.
  return the_rec->_local_dir->remove_path(old_path) == common::OKAY;
}

void file_transfer_tentacle::periodic_actions()
{
#ifdef DEBUG_FILE_TRANSFER_TENTACLE
  FUNCDEF("periodic_actions");
#endif
  AUTO_LOCK;

  // first, we'll clean out old transfers.
  time_stamp oldest_allowed(-TRANSFER_TIMEOUT);
    // nothing older than this should be kept.
  for (int i = _transfers->elements() - 1; i >= 0; i--) {
    const file_transfer_record *curr = _transfers->get(i);
    if (curr->_last_active < oldest_allowed) {
#ifdef DEBUG_FILE_TRANSFER_TENTACLE
      LOG(astring("cleaning record for: ent=") + curr->_ent.text_form()
          + " src=" + curr->_src_root + " dest=" + curr->_dest_root);
#endif
      _transfers->zap(i, i);
    }
  }

  // then we'll rescan any trees that are ready for it.
  for (int i = 0; i < _correspondences->elements(); i++) {
    file_transfer_record *curr = _correspondences->borrow(i);
    if (curr->_last_active < time_stamp(-curr->_refresh_interval)) {
      if (curr->_local_dir) {
#ifdef DEBUG_FILE_TRANSFER_TENTACLE
        LOG(astring("refreshing tree for: ent=") + curr->_ent.text_form()
            + " src=" + curr->_src_root + " dest=" + curr->_dest_root);
#endif
        WHACK(curr->_local_dir);
        curr->_local_dir = new directory_tree(curr->_src_root);
        curr->_local_dir->calculate( !(_mode & COMPARE_CONTENT_SAMPLE) );
#ifdef DEBUG_FILE_TRANSFER_TENTACLE
        LOG(astring("done refreshing tree for: ent=") + curr->_ent.text_form()
            + " src=" + curr->_src_root + " dest=" + curr->_dest_root);
#endif
      }
      curr->_last_active.reset();  // reset our action time.
    }
  }
}

outcome file_transfer_tentacle::reconstitute(const string_array &classifier,
    byte_array &packed_form, infoton * &reformed)
{
  // this method doesn't use the lists, so it doesn't need locking.
  if (classifier != file_transfer_infoton::file_transfer_classifier())
    return NO_HANDLER;
  return reconstituter(classifier, packed_form, reformed,
      (file_transfer_infoton *)NIL);
}

// the "handle_" methods are thread-safe because the mutex is locked before
// their invocations.

outcome file_transfer_tentacle::handle_tree_compare_request
    (file_transfer_infoton &req, const octopus_request_id &item_id)
{
  FUNCDEF("handle_tree_compare_request");

  // get the mapping from the specified location on this side.
  filename splitting(req._src_root);
  string_array pieces;
  splitting.separate(pieces);
  astring source_mapping = pieces[0];

  // patch the name up to find the sub_path for the source.
  filename source_start;
  pieces.zap(0, 0);
  source_start.join(pieces);

  // locate the allowed transfer depot for the mapping they provided.
  file_transfer_record *mapping_record
      = _correspondences->find_mapping(source_mapping);
  if (!mapping_record) {
    LOG(astring("could not find source mapping of ") + source_mapping);
    return NOT_FOUND;
  }

  // unpack the tree that they sent us which describes their local area.
  directory_tree *dest_tree = new directory_tree;
  if (!dest_tree->unpack(req._packed_data)) {
    LOG(astring("could not unpack requester's directory tree"));
    WHACK(dest_tree);
    return GARBAGE;
  }

  string_array requested_names;
  if (!requested_names.unpack(req._packed_data)) {
    LOG(astring("could not unpack requester's filename includes"));
    WHACK(dest_tree);
    return GARBAGE;
  }

  // look up to see if this is about something that has already been seen.
  // we don't want to add a new transfer record if they're already working on
  // this.  that also lets them do a new tree compare to restart the transfer.
  file_transfer_record *the_rec = _transfers->find(item_id._entity,
      req._src_root, req._dest_root);
  if (!the_rec) {
    // there was no existing record; we'll create a new one.
    the_rec = new file_transfer_record;
    the_rec->_ent = item_id._entity;
    the_rec->_src_root = req._src_root;
    the_rec->_dest_root = req._dest_root;
    _transfers->append(the_rec);
  } else {
    // record some activity on this record.
    the_rec->_done = false;
    the_rec->_last_active.reset();
  }

  the_rec->_diffs = new filename_list;

  int how_comp = file_info::EQUAL_NAME;  // the prize for doing nothing.
  if (_mode & COMPARE_SIZE_AND_TIME)
    how_comp |= file_info::EQUAL_FILESIZE | file_info::EQUAL_TIMESTAMP;
  if (_mode & COMPARE_CONTENT_SAMPLE)
    how_comp |= file_info::EQUAL_CHECKSUM;

  // compare the two trees of files.
  directory_tree::compare_trees(*mapping_record->_local_dir,
      source_start.raw(), *dest_tree, astring::empty_string(),
      *the_rec->_diffs, (file_info::file_similarity)how_comp);

//LOG(astring("filenames decided as different:\n") + the_rec->_diffs->text_form());

  // now prune the diffs to accord with what they claim they want.
  if (requested_names.length()) {
    for (int i = the_rec->_diffs->elements() - 1; i >= 0; i--) {
      filename diff_curr = *the_rec->_diffs->get(i);
      bool found = false;
      for (int j = 0; j < requested_names.length(); j++) {
        filename req_curr(requested_names[j]);
        if (req_curr.compare_suffix(diff_curr)) {
          found = true;
//LOG(astring("will use: ") + req_curr);
          break;
        }
      }
      if (!found) the_rec->_diffs->zap(i, i);
    }
  }

  req._packed_data.reset();  // clear out existing stuff before cloning.
  file_transfer_infoton *reply = dynamic_cast<file_transfer_infoton *>(req.clone());
  the_rec->_diffs->pack(reply->_packed_data);

//hmmm: does the other side really need the list of filenames?  i guess we
//      could check validity of what's transferred or check space available
//      before the client starts the transfer.

  reply->_request = false;  // it's a response now.
  store_product(reply, item_id);
    // send back the comparison list.

  return OKAY;
}

outcome file_transfer_tentacle::handle_tree_compare_response
    (file_transfer_infoton &resp, const octopus_request_id &item_id)
{
  FUNCDEF("handle_tree_compare_response");
  file_transfer_record *the_rec = _transfers->find(item_id._entity,
      resp._src_root, resp._dest_root);
  if (!the_rec) {
    LOG(astring("could not find the record for this transfer: item=")
        + item_id.text_form() + " src=" + resp._src_root + " dest="
        + resp._dest_root);
    return NOT_FOUND;  // not registered, so reject it.
  }

  the_rec->_last_active.reset();  // record some activity on this record.

  filename_list *flist = new filename_list;
  if (!flist->unpack(resp._packed_data)) {
    WHACK(flist);
    return GARBAGE;
  }

//hmmm: verify space on device?

  the_rec->_diffs = flist;  // set the list of differences.
  return OKAY;
}

outcome file_transfer_tentacle::handle_storage_request
    (file_transfer_infoton &req, const octopus_request_id &item_id)
{
  FUNCDEF("handle_storage_request");
  if (_mode & ONLY_REPORT_DIFFS) {
    // store an unhandled infoton.
    unhandled_request *deny = new unhandled_request(item_id, req.classifier(),
        NO_HANDLER);
    store_product(deny, item_id);
    return NO_HANDLER;
  }

  // look up the transfer record.
  file_transfer_record *the_rec = _transfers->find(item_id._entity,
      req._src_root, req._dest_root);
  if (!the_rec) {
    LOG(astring("could not find the record for this transfer: item=")
        + item_id.text_form() + " src=" + req._src_root + " dest="
        + req._dest_root);
    return NOT_FOUND;  // not registered, so reject it.
  }

  the_rec->_last_active.reset();  // mark it as still active.

  file_transfer_infoton *resp = dynamic_cast<file_transfer_infoton *>(req.clone());

  if (!the_rec->_diffs) return BAD_INPUT;  // wrong type of object.

  outcome bufret = heavy_file_operations::buffer_files
      (_correspondences->translate(the_rec->_src_root), *the_rec->_diffs,
      the_rec->_last_sent, resp->_packed_data, _maximum_transfer);
  if (bufret != OKAY) {
    // complain, but still send.
    LOG(astring("buffer files returned an error on item=")
        + item_id.text_form() + " src=" + req._src_root + " dest="
        + req._dest_root);
  }

  if ( (bufret == OKAY) && !resp->_packed_data.length() ) {
    // seems like the transfer is done.

    the_rec->_done = true;
//hmmm: mark the record and time out faster?
  }

  resp->_request = false;  // it's a response now.
  store_product(resp, item_id);
  return bufret;
}

outcome file_transfer_tentacle::handle_storage_response
    (file_transfer_infoton &resp, const octopus_request_id &item_id)
{
  FUNCDEF("handle_storage_response");
  if (_mode & ONLY_REPORT_DIFFS) {
    // not spoken here.
    return NO_HANDLER;
  }

  // look up the transfer record.
  file_transfer_record *the_rec = _transfers->find(item_id._entity,
      resp._src_root, resp._dest_root);
  if (!the_rec) return NOT_FOUND;  // not registered, so reject it.

  the_rec->_last_active.reset();  // mark it as still active.

  if (!resp._packed_data.length()) {
    // mark that we're done now.
    the_rec->_done = true;
  }

  // chew on all the things they sent us.
  while (resp._packed_data.length()) {
    file_time empty;
    file_transfer_header found(empty);
    if (!found.unpack(resp._packed_data)) {
      // bomb out now.
      LOG(astring("corruption seen on item=") + item_id.text_form()
          + " src=" + resp._src_root + " dest=" + resp._dest_root);
      return GARBAGE;
    }
    the_rec->_last_sent = found;

    if (found._length > resp._packed_data.length()) {
      // another case for leaving--not enough data left in the buffer.
      LOG(astring("data underflow seen on item=") + item_id.text_form()
          + " src=" + resp._src_root + " dest=" + resp._dest_root);
      return GARBAGE;
    }
    byte_array to_write = resp._packed_data.subarray(0, found._length - 1);
    resp._packed_data.zap(0, found._length - 1);

    if (!the_rec->_diffs) return BAD_INPUT;

    const file_info *recorded_info = the_rec->_diffs->find(found._filename);
    if (!recorded_info) {
      LOG(astring("unrequested file seen: ") + found._filename);
      continue;  // maybe there are others that aren't confused.
    }

    astring full_file = resp._dest_root + filename::default_separator()
        + recorded_info->secondary();

    outcome ret = heavy_file_operations::write_file_chunk(full_file,
        found._byte_start, to_write);
    if (ret != OKAY) {
      LOG(astring("failed to write file chunk: error=")
          + heavy_file_operations::outcome_name(ret) + " file=" + full_file
          + a_sprintf(" start=%d len=%d", found._byte_start, found._length));
    }
    found._time.set_time(full_file);
  }

  // there is no response product to store.
  return OKAY;
}

// this is the only method that is allowed to invoke the "handle_X" methods
// and it must lock the object beforehand.

outcome file_transfer_tentacle::consume(infoton &to_chow,
    const octopus_request_id &item_id, byte_array &transformed)
{
//  FUNCDEF("consume");
  transformed.reset();
  file_transfer_infoton *inf = dynamic_cast<file_transfer_infoton *>(&to_chow);
  if (!inf) return DISALLOWED;  // not for us.

  AUTO_LOCK;  // protect our lists while we're working on them.

  switch (inf->_command) {
    case file_transfer_infoton::TREE_COMPARISON: {
      if (inf->_request) return handle_tree_compare_request(*inf, item_id);
      else return handle_tree_compare_response(*inf, item_id);
    }
    case file_transfer_infoton::PLACE_FILE_CHUNKS: {
      if (inf->_request) return handle_storage_request(*inf, item_id);
      else return handle_storage_response(*inf, item_id);
    }
  }
  return BAD_INPUT;  // not a recognized command.
}

outcome file_transfer_tentacle::refresh_now(const astring &source_mapping)
{
#ifdef DEBUG_FILE_TRANSFER_TENTACLE
  FUNCDEF("refresh_now");
#endif
  AUTO_LOCK;
  for (int i = 0; i < _correspondences->elements(); i++) {
    file_transfer_record *curr = _correspondences->borrow(i);
    if (!curr) continue;
    if (curr->_source_mapping != source_mapping) continue;
    if (curr->_local_dir) {
#ifdef DEBUG_FILE_TRANSFER_TENTACLE
      LOG(astring("refreshing tree for: ent=") + curr->_ent.text_form()
          + " src=" + curr->_src_root + " dest=" + curr->_dest_root);
#endif
      WHACK(curr->_local_dir);
      curr->_local_dir = new directory_tree(curr->_src_root);
      curr->_local_dir->calculate( !(_mode & COMPARE_CONTENT_SAMPLE) );
#ifdef DEBUG_FILE_TRANSFER_TENTACLE
      LOG(astring("done refreshing tree for: ent=") + curr->_ent.text_form()
          + " src=" + curr->_src_root + " dest=" + curr->_dest_root);
#endif
    }
    curr->_last_active.reset();  // reset our action time.
    return OKAY;
  }
  return NOT_FOUND;
}

} //namespace.


