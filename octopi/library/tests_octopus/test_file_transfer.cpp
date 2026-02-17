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

#include <application/application_shell.h>
#include <application/hoople_main.h>
#include <basis/functions.h>
#include <filesystem/directory.h>
#include <filesystem/filename.h>
#include <loggers/console_logger.h>
#include <loggers/critical_events.h>
#include <mathematics/chaos.h>
#include <processes/launch_process.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_array.h>
#include <tentacles/file_transfer_tentacle.h>
#include <tentacles/recursive_file_copy.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace mathematics;
using namespace octopi;
using namespace processes;
using namespace structures;
using namespace textual;
using namespace unit_test;

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), astring(s))

class test_file_transfer_tentacle : virtual public unit_base, virtual public application_shell
{
public:
  test_file_transfer_tentacle() : application_shell() {
    DEFAULT_SOURCE_DIRECTORY = environment::get("FEISTY_MEOW_SCRIPTS");
    DEFAULT_TARGET_DIRECTORY = environment::get("HOME") + a_sprintf("/tftt_junkdir_%d", randomizer().inclusive(1, 65535));
  }
  DEFINE_CLASS_NAME("test_dirtree_fcopy");
  int execute();
  void print_instructions();

private:
  astring DEFAULT_SOURCE_DIRECTORY;
  astring DEFAULT_TARGET_DIRECTORY;
};

void test_file_transfer_tentacle::print_instructions()
{
  FUNCDEF("print_instructions");
  LOG("\
This program needs two parameters:\n\
a directory for the source root and one for the target root.\n\
Optionally, a third parameter may specify a starting point within the\n\
source root.\n\
Further, if fourth or more parameters are found, they are taken to be\n\
files to include; only they will be transferred.\n");

}

int test_file_transfer_tentacle::execute()
{
  FUNCDEF("execute");

///  if (application::_global_argc < 3) {
///    print_instructions();
///    return 23;
///  }

  astring source_dir = "";
  if (application::_global_argc > 2) source_dir = application::_global_argv[1];
  if (source_dir.empty()) {
    LOG(astring("using default source directory: ") + DEFAULT_SOURCE_DIRECTORY);
    source_dir = DEFAULT_SOURCE_DIRECTORY;
  }
  astring target_dir = "";
  if (application::_global_argc > 3) target_dir = application::_global_argv[2];
  if (target_dir.empty()) {
    LOG(astring("using default target directory: ") + DEFAULT_TARGET_DIRECTORY);
    target_dir = DEFAULT_TARGET_DIRECTORY;
    bool made_it = directory::make_directory(target_dir);
    if (!made_it) {
      deadly_error(class_name(), func, astring("failed to create target directory for copying: ") + target_dir);
    }
  }

  // do some verification of what we were handed as directories.
  filename source(source_dir);
  if (!source.exists() || !source.is_directory()) {
    print_instructions();
    return 23;
  }
  filename target(target_dir);
  if (!target.exists() || !target.is_directory()) {
    print_instructions();
    return 23;
  }

  astring source_start = "";
  if (application::_global_argc >= 4) {
    source_start = application::_global_argv[3];
  }

  string_array includes;
  if (application::_global_argc >= 5) {
    for (int i = 4; i < application::_global_argc; i++) {
      includes += application::_global_argv[i];
    }
  }

  outcome returned = recursive_file_copy::copy_hierarchy
      (file_transfer_tentacle::COMPARE_SIZE_AND_TIME, source_dir,
      target_dir, includes, source_start);


  int retval = 0;  // dress for success (then re-clothe after wardrobe malfunctions).

  if (returned == common::OKAY) {
//hmmm: now check the contents with differ or something independent of our code.
//returned = more results from that
  }

  // delete the temporary directory.
  launch_process launcher;
  un_int kid_id = 0;
  un_int rm_result = launcher.run("rm", astring("-rf ") + target_dir, launch_process::AWAIT_APP_EXIT, kid_id);
  if (rm_result != 0) {
    critical_events::alert_message(astring(class_name()) + ":: failed to remove temporary directory '" + target_dir + a_sprintf("' with OS exit value of %d", rm_result));
    // this is just a guess at what failure occurred in terms of our outcomes; the log should show more details.
    returned = common::ACCESS_DENIED;
  }
  
  if (returned == common::OKAY)
    critical_events::alert_message(astring(class_name()) + ":: works for those "
        "functions tested.");
  else
    critical_events::alert_message(astring(class_name()) + "file_transfer_tentacle:: failed with "
        "outcome=" + recursive_file_copy::outcome_name(returned));

  return 0;
}

HOOPLE_MAIN(test_file_transfer_tentacle, )

