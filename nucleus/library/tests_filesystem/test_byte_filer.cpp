/*****************************************************************************\
*                                                                             *
*  Name   : test_byte_filer                                                   *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/byte_array.h>
#include <mathematics/chaos.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/astring.h>

#include <application/hoople_main.h>
#include <configuration/application_configuration.h>
#include <filesystem/byte_filer.h>
#include <filesystem/directory.h>
#include <filesystem/filename.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <mathematics/chaos.h>
#include <structures/checksums.h>
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

#define DEBUG_BYTE_FILER
  // uncomment for noisy test run.

class test_byte_filer : virtual public unit_base, virtual public application_shell
{
public:
  test_byte_filer() : application_shell() {}
  DEFINE_CLASS_NAME("test_byte_filer");
  int run_simple_test();
  int run_file_scan();
  virtual int execute();
};

const astring &TEST_FILE()
{
  const char *TEST_FILE_BASE = "/zz_garbage";
  const char *TEST_FILE_SUFFIX = ".txt";
  static astring __hidden_filename;
  if (!__hidden_filename) {
    __hidden_filename = environment::get("TMP");
    if (!__hidden_filename) __hidden_filename = "/tmp";
    __hidden_filename += TEST_FILE_BASE;
    __hidden_filename += a_sprintf("%d", chaos().inclusive(0, 65535));
    __hidden_filename += TEST_FILE_SUFFIX;
  }
  return __hidden_filename;
}

int test_byte_filer::run_simple_test()
{
  FUNCDEF("run_simple_test");
#ifdef DEBUG_BYTE_FILER
  LOG("ahoy, beginning file test...");
  LOG(astring("test file is ") + TEST_FILE());
#endif

  chaos randomizer;

//hmmm: move to t_filename.
  // test filename's exist operation.
  byte_filer garbage(TEST_FILE().s(), "wb");
  garbage.write("oy.\n");
  garbage.close();
  filename test1(TEST_FILE());
  ASSERT_TRUE(test1.exists(), "exists test file should exist");
  filename test2("c:\\this_file_shouldNt_exist_ever.txt");
  ASSERT_FALSE(test2.exists(), "weird file should not existed");
  // test again to make sure it didn't create it.
  ASSERT_FALSE(test2.exists(), "weird file should still not exist");
  test1.unlink();

  int block_size = randomizer.inclusive(3000, 30000);
#ifdef DEBUG_BYTE_FILER
  LOG(a_sprintf("block size=%d", block_size));
#endif
  abyte *original_block = new abyte[block_size];
  for (int i = 0; i < block_size; i++)
    original_block[i] = abyte(randomizer.inclusive(32, 126));
  unsigned int original_checksum
      = checksums::bizarre_checksum((abyte *)original_block, block_size);
  if (original_checksum) {} // compiler quieting.
#ifdef DEBUG_BYTE_FILER
  LOG(a_sprintf("random block checksum=%d", original_checksum));
#endif
  {
    byte_array to_stuff_in_file(block_size, original_block);
    delete [] original_block;
    byte_filer fred(TEST_FILE(), "w+");
    fred.write(to_stuff_in_file);
  }
#ifdef DEBUG_BYTE_FILER
  LOG(astring("about to compare file to checksum"));
#endif
  {
    abyte *temp_array = new abyte[21309];
    byte_array to_fake_stuff(21309, temp_array);
    delete [] temp_array;
    byte_filer fred(TEST_FILE(), "r");
#ifdef DEBUG_BYTE_FILER
    LOG(astring("about to try writing to file"));
#endif
    int should_be_failure = fred.write(to_fake_stuff);
    ASSERT_EQUAL(should_be_failure, 0, "write on read only, should not succeed");

///    int fredsize = int(fred.size());
///    fred.chunk_factor(fredsize);

#ifdef DEBUG_BYTE_FILER
    LOG(a_sprintf("about to try reading from file %d bytes", fredsize));
#endif
    byte_array file_contents;
    int bytes_read = fred.read(file_contents, block_size * 2);
    ASSERT_EQUAL(bytes_read, block_size, "reading entire file should get proper size");
    un_int check_2 = checksums::bizarre_checksum((abyte *)file_contents.access(), file_contents.length());
    ASSERT_EQUAL((int)check_2, (int)original_checksum, "should read correct contents for checksum");
  }

#define FACTOR 1354

  {
    int numpacs = number_of_packets(block_size, FACTOR);
    byte_filer fred(TEST_FILE(), "rb");
///file::READ_ONLY);
///    fred.chunk_factor(FACTOR);
    int whole_size = 0;
    for (int i = 0; i < numpacs; i++) {
      byte_array blob_i;
      int bytes_in = fred.read(blob_i, FACTOR);
      ASSERT_FALSE(bytes_in > FACTOR, "we should never somehow read in more than we asked for");
      whole_size += blob_i.length();
    }
    ASSERT_EQUAL(whole_size, fred.length(), "chunking comparison should see sizes as same");
  }

// test writing out a copy and comparing them... there's no == on files!

  ASSERT_TRUE(filename(TEST_FILE()).unlink(), "cleanup should be able to remove temporary file");

  // it seems everything worked during our tests.
  return 0;
}

int test_byte_filer::run_file_scan()
{
  FUNCDEF("run_file_scan");
  chaos randomizer;

  string_array files(_global_argc, (const char **)_global_argv);
  files.zap(0, 0);  // toss the first element since that's our app filename.

  if (!files.length()) {
    // pretend they gave us the list of files in the TMP directory.  some of
    // these might fail if they're locked up.
//    astring tmpdir = environment::get("TMP");
    astring tmpdir = application_configuration::current_directory();
    directory dir(tmpdir);
    for (int i = 0; i < dir.files().length(); i++) {
      // skip text files since we use those right here.
      if ( (dir.files()[i].ends(".txt")) || (dir.files()[i].ends(".txt")) )
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
    byte_filer test(curr, "rb");
    if (!test.good()) {
      LOG(astring("good check: ") + curr + " cannot be opened.  is this bad?");
      continue;
    }

    // check that we get the expected position report from scooting to the
    // end of a file.
    test.seek(0, byte_filer::FROM_END);
    ASSERT_EQUAL((int)test.tell(), (int)test.length(), "seek check should get to end as expected");
    test.seek(0, byte_filer::FROM_START);

    size_t len = test.length();
//log(a_sprintf("file len is %.0f", double(len)));
    size_t posn = 0;
    while ( (posn < len) && !test.eof() ) {
      size_t readlen = randomizer.inclusive(1, 256 * KILOBYTE);
//log(a_sprintf("read %u bytes, posn now %d bytes", readlen, posn));
      int bytes_read = int(test.read(data_found, int(readlen)));
      ASSERT_TRUE(bytes_read >= 0, "reading should not fail to read some bytes");
      if (bytes_read > 0) {
        posn += bytes_read;
      }
    }
    ASSERT_TRUE(test.eof(), "eof check should see us at eof");
    ASSERT_EQUAL((int)posn, (int)len, "eof check should be at right position");
//    log(astring("successfully read ") + curr);
  }

  return 0;
}

int test_byte_filer::execute()
{
//  FUNCDEF("execute");
  int ret = run_simple_test();
  if (ret) return ret;  // failed.
  ret = run_file_scan();
  if (ret) return ret;  // failed here.

  return final_report();
}

HOOPLE_MAIN(test_byte_filer, )

