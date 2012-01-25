/*
*  Name   : test_directory
*  Author : Chris Koeritz
*  Purpose:
*    Tests the directory object out to see if it scans properly.
**
* Copyright (c) 2001-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <basis/functions.h>
#include <basis/guards.h>
#include <structures/string_array.h>
#include <application/hoople_main.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <filesystem/directory.h>
#include <filesystem/filename.h>
#include <structures/static_memory_gremlin.h>
#include <textual/string_manipulation.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace mathematics;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

//////////////

class test_directory : public virtual unit_base, public virtual application_shell
{
public:
  test_directory() : application_shell() {}
  DEFINE_CLASS_NAME("test_directory");
  int execute();
};

//////////////

int test_directory::execute()
{
  FUNCDEF("execute");
  {
    astring path = "/tmp";  // default path.
#ifdef __WIN32__
    path = "c:/";  // default path for windoze.
#endif
    if (application::_global_argc >= 2)
      path = application::_global_argv[1];

    astring pattern = "*";
    if (application::_global_argc >= 3)
      pattern = application::_global_argv[2];

//    log(astring("Scanning directory named \"") + path + "\"");
//    log(astring("Using pattern-match \"") + pattern + "\"");

    directory dir(path, pattern.s());
    ASSERT_TRUE(dir.good(), "the current directory should be readable");
//    log(path + " contained these files:");
    astring names;
    for (int i = 0; i < dir.files().length(); i++) {
      names += dir.files()[i] + " ";
    }
    astring split;
    string_manipulation::split_lines(names, split, 4);
//    log(split);
//    log(path + " contained these directories:");
    names = "";
    for (int i = 0; i < dir.directories().length(); i++) {
      names += dir.directories()[i] + " ";
    }
    string_manipulation::split_lines(names, split, 4);
//    log(split);
  }
//hmmm: the above test proves zilch.
//      it needs to do this differently.
//      instead of relying on someone else's folder, pick and make our own.
//      then fill it with some known stuff.
//      verify then that the read form is identical!



//more tests!

  return final_report();
}

HOOPLE_MAIN(test_directory, )

