/*
*  Name   : test_file_time
*  Author : Chris Koeritz
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/hoople_main.h>
#include <basis/enhance_cpp.h>
#include <basis/functions.h>
#include <basis/astring.h>
#include <filesystem/file_time.h>
#include <loggers/program_wide_logger.h>
#include <loggers/critical_events.h>
#include <mathematics/chaos.h>
#include <timely/time_stamp.h>
#include <unit_test/unit_base.h>

#include <stdio.h>
#include <sys/stat.h>
#ifdef __UNIX__
  #include <unistd.h>
#endif

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace mathematics;
using namespace timely;
using namespace unit_test;

//#define DEBUG_TEST_FILE_INFO
  // uncomment for noisy version.

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

static chaos a_randomizer;

//////////////

class test_file_time : public application_shell, public unit_base
{
public:
  test_file_time() : application_shell(), unit_base() {}

  DEFINE_CLASS_NAME("test_file_time");

  virtual int execute();
};

//////////////

int test_file_time::execute()
{
  FUNCDEF("execute");

#ifdef __UNIX__
  // just open the root directory on unix; always works.
  astring toppy("/");
#endif
#ifdef __WIN32__
  // windows cannot fopen a directory.  this blows.  so we pick a file
  // that should work for most windowses.
  astring toppy("c:/Windows/notepad.exe");
#endif

  // test storing info via the constructor.
  file_time absurdity_time(toppy);
  FILE *topdir = fopen(toppy.s(), "r");
  ASSERT_INEQUAL(topdir, NULL_POINTER, "opening topdir for testing");
  if (topdir == NULL_POINTER) {
    return 1;
  }
  file_time nutty_time(topdir);

  int filenum = fileno(topdir);
  struct stat sbuffer;
  int stat_okay = fstat(filenum, &sbuffer);
  ASSERT_FALSE(stat_okay, "failure to read filetime");
  file_time goofy_time(sbuffer.st_mtime); 
  fclose(topdir);
  file_time testing(goofy_time);  // copy ctor.
  // test that they all got the same idea from the file.
  ASSERT_EQUAL(absurdity_time, nutty_time, "filename vs. FILE ctor");
  ASSERT_EQUAL(absurdity_time, goofy_time, "filename vs. time_t ctor");
  ASSERT_EQUAL(absurdity_time, testing, "filename vs. copy ctor");
  ASSERT_EQUAL(nutty_time, goofy_time, "FILE vs. time_t ctor");
  ASSERT_EQUAL(nutty_time, testing, "FILE vs. copy ctor");
  // one reversed direction check.
  ASSERT_EQUAL(goofy_time, absurdity_time, "time_t vs. filename ctor");

  // test packing the object and packed_size.
  byte_array packed;
  int size = testing.packed_size();
  testing.pack(packed);
  ASSERT_EQUAL(size, packed.length(), "packed size accuracy");
  file_time unstuffy;
  ASSERT_TRUE(unstuffy.unpack(packed), "unpacking");
  ASSERT_EQUAL((double)testing.raw(), (double)unstuffy.raw(), "unpacked contents should be equal to prior");

  // test the text_form method.
  astring text;
  testing.text_form(text);
  ASSERT_INEQUAL(text.length(), 0, "text_form produces text");

  // test validity after unpacking.
  ASSERT_EQUAL(unstuffy, goofy_time, "constructor file size");

  return final_report();
}

HOOPLE_MAIN(test_file_time, )

