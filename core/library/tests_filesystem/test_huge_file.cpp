/*
*  Name   : test_huge_file
*  Author : Chris Koeritz
**
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <application/hoople_main.h>
#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <configuration/application_configuration.h>
#include <filesystem/directory.h>
#include <filesystem/filename.h>
#include <filesystem/huge_file.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <mathematics/chaos.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_array.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace configuration;
using namespace mathematics;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

class test_huge_file : public virtual unit_base, virtual public application_shell
{
public:
  test_huge_file() : application_shell() {}
  DEFINE_CLASS_NAME("test_huge_file");
  void run_file_scan();
  virtual int execute();
};

void test_huge_file::run_file_scan()
{
  FUNCDEF("run_file_scan");
  chaos randomizer;

  string_array files(application::_global_argc, (const char **)application::_global_argv);
  files.zap(0, 0);  // toss the first element since that's our app filename.

  if (!files.length()) {
    // pretend they gave us the list of files in the TMP directory.  some of
    // these might fail if they're locked up.
//    astring tmpdir = environment::get("TMP");
    astring tmpdir = application_configuration::current_directory();
    directory dir(tmpdir);
    for (int i = 0; i < dir.files().length(); i++) {
      // skip text files since we use those right here.
      if (dir.files()[i].ends(".txt"))
        continue;
      astring chewed_string = tmpdir + "/" + dir.files()[i];
      files += chewed_string;
    }
//LOG(astring("added files since no cmd args: ") + files.text_form());
  }

  byte_array data_found;
  for (int i = 0; i < files.length(); i++) {
    astring curr = files[i];
//    LOG(a_sprintf("file %d: ", i) + curr);
    huge_file test(curr, "rb");
    ASSERT_TRUE(test.good(), "good check should say yes, it's good.");
    double len = test.length();
//log(a_sprintf("file len is %.0f", len));
    double posn = 0;
    while ( (posn < len) && !test.eof() ) {
      int readlen = randomizer.inclusive(1, 256 * KILOBYTE);
//log(a_sprintf("read %.0f bytes, posn now %.0f bytes", double(readlen), posn));
      int bytes_read = 0;
      outcome ret = test.read(data_found, readlen, bytes_read);
      ASSERT_EQUAL(ret.value(), huge_file::OKAY, "should be able to read file");
      if (ret == huge_file::OKAY) {
        posn += bytes_read;
      }
    }
    ASSERT_TRUE(test.eof(), "eof check should be at eof.");
    if (posn != len) 
      log(a_sprintf("failed check, want %.0f, got %.0f", double(len), double(posn)));
    ASSERT_EQUAL(posn, len, "eof check should be at right position: ");
//    log(astring("successfully read ") + curr);
  }
}

int test_huge_file::execute()
{
  FUNCDEF("execute");
  run_file_scan();
  return final_report();
}

HOOPLE_MAIN(test_huge_file, )

