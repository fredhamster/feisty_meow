/*
*  Name   : test_file_info
*  Author : Chris Koeritz
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/hoople_main.h>
#include <basis/enhance_cpp.h>
#include <basis/functions.h>
#include <basis/astring.h>
#include <filesystem/file_info.h>
#include <loggers/program_wide_logger.h>
#include <loggers/critical_events.h>
#include <mathematics/chaos.h>
#include <timely/time_stamp.h>
#include <unit_test/unit_base.h>

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

class test_file_info : public application_shell, public unit_base
{
public:
  test_file_info() : application_shell(), unit_base() {}

  DEFINE_CLASS_NAME("test_file_info");

  virtual int execute();
};

//////////////

//hmmm: stolen from ssl_init.
byte_array random_bytes(int length)
{
  byte_array seed;
  for (int i = 0; i < length; i++)
    seed += abyte(chaos().inclusive(0, 255));
  return seed;
}

int test_file_info::execute()
{
  FUNCDEF("execute");
#ifdef __UNIX__
  file_time absurdity_time("/");
#endif
#ifdef __WIN32__
  file_time absurdity_time("c:/");
#endif

  // test storing info via the constructor.
  file_info testing(filename("/usr/schrodingers/dog/got/away"), 7298238);
  testing._time = absurdity_time;
  testing._checksum = 1283412;
  ASSERT_EQUAL((int)testing._file_size, (int)7298238, "constructor file size");
  ASSERT_EQUAL(testing._time, absurdity_time, "constructor file time");
  ASSERT_EQUAL(testing._checksum, 1283412, "constructor checksum");
  ASSERT_EQUAL((filename &)testing, filename("/usr/schrodingers/dog/got/away"),
      "constructor filename");

  // test packing the object and packed_size.
  byte_array packed;
  int size = testing.packed_size();
  testing.pack(packed);
  ASSERT_EQUAL(size, packed.length(), "basic packed size accuracy");
  file_info unstuffy;
  ASSERT_TRUE(unstuffy.unpack(packed), "basic unpacking");

  // test validity after unpacking.
  ASSERT_EQUAL((int)unstuffy._file_size, (int)7298238, "constructor file size");
  ASSERT_EQUAL(unstuffy._time, absurdity_time, "constructor file time");
  ASSERT_EQUAL(unstuffy._checksum, 1283412, "constructor checksum");
  ASSERT_EQUAL((filename &)unstuffy, filename("/usr/schrodingers/dog/got/away"),
      "constructor filename");

  // test the extra bits, the attachment and secondary name.
  astring seconame = "glorabahotep";
  testing.secondary(seconame );
  const byte_array randobytes = random_bytes(chaos().inclusive(37, 4128));
  testing.attachment(randobytes);
  packed.reset();
  size = testing.packed_size();
  testing.pack(packed);
  ASSERT_EQUAL(size, packed.length(), "secondary packed size accuracy");
  ASSERT_TRUE(unstuffy.unpack(packed), "secondary unpacking");
  // test that the secondary name and attachment came back.
  ASSERT_EQUAL(seconame, unstuffy.secondary(), "secondary name incorrect");
  ASSERT_EQUAL(randobytes, unstuffy.attachment(), "secondary attachment inaccurate");

  return final_report();
}

HOOPLE_MAIN(test_file_info, )

