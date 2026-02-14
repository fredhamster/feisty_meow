/*****************************************************************************\
*                                                                             *
*  Name   : test_file_transfer_tentacle                                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Tests the file_transfer_tentacle without any networking involved.        *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/functions.h>
#include <structures/string_array.h>
#include <structures/static_memory_gremlin.h>
#include <loggers/console_logger.h>
#include <application/application_shell.h>
#include <tentacles/file_transfer_tentacle.h>
#include <tentacles/recursive_file_copy.h>

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

class test_file_transfer_tentacle : public application_shell
{
public:
  test_file_transfer_tentacle() : application_shell(static_class_name()) {}
  DEFINE_CLASS_NAME("test_dirtree_fcopy");
  int execute();
};

int test_file_transfer_tentacle::execute()
{
  FUNCDEF("execute");

  if (__argc < 3) {
    log("\
This program needs two parameters:\n\
a directory for the source root and one for the target root.\n\
Optionally, a third parameter may specify a starting point within the\n\
source root.\n\
Further, if fourth or more parameters are found, they are taken to be\n\
files to include; only they will be transferred.\n");
    return 23;
  }

  astring source_dir = __argv[1];
  astring target_dir = __argv[2];

  astring source_start = "";
  if (__argc >= 4) {
    source_start = __argv[3];
  }

  string_array includes;
  if (__argc >= 5) {
    for (int i = 4; i < __argc; i++) {
      includes += __argv[i];
    }
  }


  outcome returned = recursive_file_copy::copy_hierarchy
      (file_transfer_tentacle::COMPARE_SIZE_AND_TIME, source_dir,
      target_dir, includes, source_start);

/*
  astring source_root = "snootums";
  if (source_start.t()) {
    source_root += filename::default_separator() + source_start;
  }

  tcpip_stack stack;
  octopus ring_leader(stack.hostname(), 10 * MEGABYTE);
  file_transfer_tentacle *tran = new file_transfer_tentacle(MAX_CHUNK, false);
  ring_leader.add_tentacle(tran);

  outcome add_ret = tran->add_correspondence("snootums", source_dir,
      10 * MINUTE_ms);
  if (add_ret != tentacle::OKAY)
    deadly_error(class_name(), func, "failed to add the correspondence");

  file_transfer_infoton *initiate = new file_transfer_infoton;
  initiate->_request = true;
  initiate->_command = file_transfer_infoton::TREE_COMPARISON;
  initiate->_src_root = source_root;
  initiate->_dest_root = target_dir;
  directory_tree target_area(target_dir);
  target_area.calculate();
  initiate->package_tree_info(target_area, includes);

  octopus_entity ent = ring_leader.issue_identity();
  octopus_request_id req_id(ent, 1);
  outcome start_ret = ring_leader.evaluate(initiate, req_id);
  if (start_ret != tentacle::OKAY)
    deadly_error(class_name(), func, "failed to start the comparison");

  file_transfer_infoton *reply_from_init
      = (file_transfer_infoton *)ring_leader.acquire_specific_result(req_id);
  if (!reply_from_init)
    deadly_error(class_name(), func, "no response to tree compare start");

  filename_list diffs;
  byte_array pack_copy = reply_from_init->_packed_data;
  if (!diffs.unpack(pack_copy))
    deadly_error(class_name(), func, "could not unpack filename list!");
//  LOG(astring("got list of diffs:\n") + diffs.text_form());

  octopus client_spider(stack.hostname(), 10 * MEGABYTE);
  file_transfer_tentacle *tran2 = new file_transfer_tentacle(MAX_CHUNK, false);
  tran2->register_file_transfer(ent, source_root, target_dir, includes);
  client_spider.add_tentacle(tran2);

  octopus_request_id resp_id(ent, 2);
  outcome ini_resp_ret = client_spider.evaluate(reply_from_init, resp_id);
  if (ini_resp_ret != tentacle::OKAY)
    deadly_error(class_name(), func, "failed to process the start response!");

  infoton *junk = client_spider.acquire_specific_result(resp_id);
  if (junk)
    deadly_error(class_name(), func, "got a response we shouldn't have!");

  int iter = 0;
  while (true) {
LOG(a_sprintf("ongoing chunk %d", ++iter));

    // keep going until we find a broken reply.
    file_transfer_infoton *ongoing = new file_transfer_infoton;
    ongoing->_request = true;
    ongoing->_command = file_transfer_infoton::PLACE_FILE_CHUNKS;
    ongoing->_src_root = source_root;
    ongoing->_dest_root = target_dir;

    octopus_request_id chunk_id(ent, iter + 10);
    outcome place_ret = ring_leader.evaluate(ongoing, chunk_id);
    if (place_ret != tentacle::OKAY)
      deadly_error(class_name(), func, "failed to run ongoing transfer");
  
    file_transfer_infoton *reply = (file_transfer_infoton *)ring_leader
         .acquire_specific_result(chunk_id);
    if (!reply)
      deadly_error(class_name(), func, "failed to get ongoing transfer reply");

    if (!reply->_packed_data.length()) {
      LOG("hit termination condition: no data packed in for file chunks.");
      break;
    }

    byte_array copy = reply->_packed_data;
    while (copy.length()) {
      file_transfer_header head;
      if (!head.unpack(copy)) 
        deadly_error(class_name(), func, "failed to unpack header");
LOG(astring("header: ") + head.text_form());
LOG(a_sprintf("size in array: %d", copy.length()));
      if (copy.length() < head._length)
        deadly_error(class_name(), func, "not enough length in array");
      copy.zap(0, head._length - 1);
LOG(a_sprintf("size in array now: %d", copy.length()));
    }
    if (copy.length())
      deadly_error(class_name(), func, "still had data in array");

    octopus_request_id resp_id(ent, iter + 11);
    outcome resp_ret = client_spider.evaluate(reply, resp_id);
    if (resp_ret != tentacle::OKAY)
      deadly_error(class_name(), func, "failed to process the transfer reply!");
  
  }
*/

  if (returned == common::OKAY)
    guards::alert_message("file_transfer_tentacle:: works for those "
        "functions tested.");
  else
    guards::alert_message(astring("file_transfer_tentacle:: failed with "
        "outcome=") + recursive_file_copy::outcome_name(returned));
  return 0;
}

HOOPLE_MAIN(test_file_transfer_tentacle, )

