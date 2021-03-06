/*****************************************************************************\
*                                                                             *
*  Name   : recursive_file_copy                                               *
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

#include "file_transfer_infoton.h"
#include "file_transfer_tentacle.h"
#include "recursive_file_copy.h"

#include <application/application_shell.h>
#include <basis/guards.h>
#include <filesystem/directory.h>
#include <filesystem/directory_tree.h>
#include <filesystem/filename.h>
#include <filesystem/filename_list.h>
#include <filesystem/heavy_file_ops.h>
#include <filesystem/huge_file.h>
#include <loggers/program_wide_logger.h>
#include <octopus/entity_defs.h>
#include <octopus/entity_data_bin.h>
#include <octopus/octopus.h>
#include <structures/static_memory_gremlin.h>
#include <textual/string_manipulation.h>
#include <timely/time_control.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;
using namespace timely;

namespace octopi {

#define DEBUG_RECURSIVE_FILE_COPY
  // uncomment for noisier debugging.

#define FAKE_HOSTNAME "internal_fake_host"

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)
#undef BASE_LOG
#define BASE_LOG(s) EMERGENCY_LOG(program_wide_logger::get(), s)

#define RETURN_ERROR_RFC(msg, err) { \
  LOG(msg); \
  return err; \
}

const int MAX_CHUNK_RFC_COPY_HIER = 1 * MEGABYTE;
  // maximum size for each transfer chunk.

const int EXPECTED_MAXIMUM_TRANSFER_TIME = 10 * HOUR_ms;
  // how long do we allow the scanned file lists to stay relevant.
  // we allow it a long time, since this is a copy and not an active
  // synchronization.

recursive_file_copy::~recursive_file_copy() {}

const char *recursive_file_copy::outcome_name(const outcome &to_name)
{ return common::outcome_name(to_name); }

outcome recursive_file_copy::copy_hierarchy(int transfer_mode,
  const astring &source_dir, const astring &target_dir,
  const string_array &includes, const astring &source_start)
{
  FUNCDEF("copy_hierarchy");

/*
  string_array includes;
  if (_global_argc >= 5) {
    for (int i = 4; i < _global_argc; i++) {
      includes += _global_argv[i];
    }
  }
*/

  const astring transfer_name = "snootums";

  astring source_root = transfer_name;
  if (source_start.t()) {
    source_root += filename::default_separator() + source_start;
  }

  octopus ring_leader(FAKE_HOSTNAME, 10 * MEGABYTE);
  file_transfer_tentacle *tran = new file_transfer_tentacle
      (MAX_CHUNK_RFC_COPY_HIER, (file_transfer_tentacle::transfer_modes)transfer_mode);
  ring_leader.add_tentacle(tran);

  outcome add_ret = tran->add_correspondence(transfer_name, source_dir,
      EXPECTED_MAXIMUM_TRANSFER_TIME);
  if (add_ret != tentacle::OKAY)
    RETURN_ERROR_RFC("failed to add the correspondence", NOT_FOUND);

//hmmm: this kind of object creation should be packaged in helper functions.
  file_transfer_infoton *initiate = new file_transfer_infoton;
  initiate->_request = true;
  initiate->_command = file_transfer_infoton::BUILD_TARGET_TREE;
  initiate->_src_root = source_root;
  initiate->_dest_root = target_dir;

  // make a directory snapshot with just directories, no files.
  directory_tree target_area_just_dirs(source_dir, "*", true);
  initiate->package_tree_info(target_area_just_dirs, includes);

  octopus_entity ent = ring_leader.issue_identity();
  octopus_request_id req_id(ent, 1);
  outcome start_ret = ring_leader.evaluate(initiate, req_id);
  if (start_ret != tentacle::OKAY)
    RETURN_ERROR_RFC("failed to build target tree", NONE_READY);

  file_transfer_infoton *reply_from_init
      = (file_transfer_infoton *)ring_leader.acquire_specific_result(req_id);
  if (!reply_from_init) {
LOG("spewing list of what IS there...");
LOG(ring_leader.responses().text_form());
    RETURN_ERROR_RFC("no response to request to build target tree", NONE_READY);
  }

  if (reply_from_init->_success != OKAY) {
    RETURN_ERROR_RFC("failed to get good result from building target tree", reply_from_init->_success);
  }

//now repeating a lot of above to get tree compare going.

//hmmm: this kind of object creation should be packaged in helper functions.
  file_transfer_infoton *comparison_req = new file_transfer_infoton;
  comparison_req->_request = true;
  comparison_req->_command = file_transfer_infoton::TREE_COMPARISON;
  comparison_req->_src_root = source_root;
  comparison_req->_dest_root = target_dir;
  // make a directory snapshot with just directories, no files.
  directory_tree target_area(target_dir);

//hmmm: simple asset counting debugging in calculate would be nice too.
  target_area.calculate( !(transfer_mode & file_transfer_tentacle::COMPARE_CONTENT_SAMPLE) );

  comparison_req->package_tree_info(target_area, includes);

  ent = ring_leader.issue_identity();
  req_id = octopus_request_id(ent, 1);
  start_ret = ring_leader.evaluate(comparison_req, req_id);
  if (start_ret != tentacle::OKAY)
    RETURN_ERROR_RFC("failed to build target tree", NONE_READY);

  reply_from_init = (file_transfer_infoton *)ring_leader.acquire_specific_result(req_id);
  if (!reply_from_init) {
LOG("spewing list of what IS there...");
LOG(ring_leader.responses().text_form());
    RETURN_ERROR_RFC("no response to request to build target tree", NONE_READY);
  }


//resuming


  filename_list diffs;
  byte_array pack_copy = reply_from_init->_packed_data;
  if (!diffs.unpack(pack_copy)) {
    RETURN_ERROR_RFC("could not unpack filename list!", GARBAGE);
  }
//  LOG(astring("got list of diffs:\n") + diffs.text_form());

  octopus client_spider(FAKE_HOSTNAME, 10 * MEGABYTE);
  file_transfer_tentacle *tran2 = new file_transfer_tentacle
      (MAX_CHUNK_RFC_COPY_HIER, (file_transfer_tentacle::transfer_modes)transfer_mode);
  tran2->register_file_transfer(ent, source_root, target_dir, includes);
  client_spider.add_tentacle(tran2);

  octopus_request_id resp_id(ent, 2);
  outcome ini_resp_ret = client_spider.evaluate(reply_from_init, resp_id);
  if (ini_resp_ret != tentacle::OKAY)
    RETURN_ERROR_RFC("failed to process the start response!", FAILURE);

  infoton *junk = client_spider.acquire_specific_result(resp_id);
  if (junk)
    RETURN_ERROR_RFC("got a response we shouldn't have!", FAILURE);

  astring current_file;  // what file is in progress right now?

  int iter = 0;
  while (true) {
#ifdef DEBUG_RECURSIVE_FILE_COPY
    LOG(a_sprintf("ongoing chunk %d", ++iter));
#endif

    // keep going until we find a broken reply.
    file_transfer_infoton *ongoing = new file_transfer_infoton;
    ongoing->_request = true;
    ongoing->_command = file_transfer_infoton::PLACE_FILE_CHUNKS;
    ongoing->_src_root = source_root;
    ongoing->_dest_root = target_dir;

    octopus_request_id chunk_id(ent, iter + 10);
    outcome place_ret = ring_leader.evaluate(ongoing, chunk_id);
    if (place_ret != tentacle::OKAY)
      RETURN_ERROR_RFC("failed to run ongoing transfer", FAILURE);
  
    file_transfer_infoton *reply = (file_transfer_infoton *)ring_leader
         .acquire_specific_result(chunk_id);
    if (!reply)
      RETURN_ERROR_RFC("failed to get ongoing transfer reply", NONE_READY);

    if (reply->_command == file_transfer_infoton::CONCLUDE_TRANSFER_MARKER) {
      BASE_LOG(astring("finished transfer from \"") + source_dir
          + "\" to \"" + target_dir + "\"");
      break;
    }

    byte_array copy = reply->_packed_data;
    while (copy.length()) {
      file_time empty;
      file_transfer_header head(empty);
      if (!head.unpack(copy)) 
        RETURN_ERROR_RFC("failed to unpack header", GARBAGE);
      if (copy.length() < head._length)
        RETURN_ERROR_RFC("not enough length in array", GARBAGE);
      if (head._length > 0)
        copy.zap(0, head._length - 1);

//hmmm: this needs better formatting, and should not repeat the same file name even
//      if it's in multiple chunks.
      BASE_LOG(head.readable_text_form());
    }
    if (copy.length())
      RETURN_ERROR_RFC("still had data in array", GARBAGE);

    octopus_request_id resp_id(ent, iter + 11);
    outcome resp_ret = client_spider.evaluate(reply, resp_id);
    if (resp_ret != tentacle::OKAY)
      RETURN_ERROR_RFC("failed to process the transfer reply!", FAILURE);
  }

  return OKAY;
}

} //namespace.


