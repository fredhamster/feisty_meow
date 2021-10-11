/*
*  Name   : synch_files
*  Author : Chris Koeritz
*  Purpose:                                                                   *
*    Provides a file transfer utility using the file_transfer_tentacle.       *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com
*/

#include <application/hoople_main.h>
#include <basis/functions.h>
#include <loggers/console_logger.h>
#include <loggers/critical_events.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_array.h>
#include <tentacles/file_transfer_tentacle.h>
#include <tentacles/recursive_file_copy.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace octopi;
using namespace structures;
using namespace textual;
using namespace timely;

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

#define DEBUG_SYNCH_FILES
//hmmm: not used yet, but should be.

class synch_files_tentacle : public application_shell
{
public:
  synch_files_tentacle() : application_shell() {}
  DEFINE_CLASS_NAME("test_dirtree_fcopy");
  int execute();
};

int synch_files_tentacle::execute()
{
  FUNCDEF("execute");

  if (_global_argc < 3) {
    log(astring("\
This program needs two parameters:\n\
a directory for the source root and one for the target root.\n\
Optionally, a third parameter may specify a starting point within the\n\
source root.\n\
Further, if fourth or more parameters are found, they are taken to be\n\
files to include; only they will be transferred.\n"), ALWAYS_PRINT);
    return 23;
  }

  astring source_dir = _global_argv[1];
  astring target_dir = _global_argv[2];

  astring source_start = "";
  if (_global_argc >= 4) {
    source_start = _global_argv[3];
  }

  string_array includes;
  if (_global_argc >= 5) {
    for (int i = 4; i < _global_argc; i++) {
      includes += _global_argv[i];
    }
  }

//hmmm: make comparing the file chunks an option too!
  outcome returned = recursive_file_copy::copy_hierarchy
      (file_transfer_tentacle::COMPARE_SIZE_AND_TIME, source_dir,
      target_dir, includes, source_start);

  if (returned != common::OKAY) {
    critical_events::alert_message(astring("copy failure with outcome=")
        + recursive_file_copy::outcome_name(returned));
    return 1;
  } else return 0;
}

HOOPLE_MAIN(synch_files_tentacle, )

