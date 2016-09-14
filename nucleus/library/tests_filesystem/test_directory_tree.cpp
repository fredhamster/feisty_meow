/*
*  Name   : test_directory_tree
*  Author : Chris Koeritz
*  Purpose:
*    Tests the directory_tree object on some well-known directories.
**
* Copyright (c) 2001-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <application/hoople_main.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <filesystem/directory_tree.h>
#include <filesystem/filename.h>
#include <filesystem/filename_list.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <processes/launch_process.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_array.h>
#include <textual/string_manipulation.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace mathematics;
using namespace filesystem;
using namespace loggers;
using namespace processes;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

const bool JUST_SIZES = false;
  // determines if we'll only compare file size and time.

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

class test_directory_tree : public virtual unit_base, virtual public application_shell
{
public:
  test_directory_tree() : application_shell() {}
  DEFINE_CLASS_NAME("test_directory_tree");
  int execute();
};

int test_directory_tree::execute()
{
  FUNCDEF("execute");

  astring path = "/usr/include";
#ifdef __WIN32__
  // default path for windoze uses an area that should always exist.
  path = environment::get("COMMONPROGRAMFILES");
#endif

  // process the command line parameters, which are optionally a directory name and
  // a pattern to use when scanning.
  if (_global_argc >= 2)
    path = _global_argv[1];

  astring pattern = "*";
  if (_global_argc >= 3)
    pattern = _global_argv[2];

  {
//    log(astring("Scanning directory tree at \"") + path + "\"");
//    log(astring("Using pattern-match \"") + pattern + "\"");

    directory_tree dir(path, pattern.s());
    ASSERT_TRUE(dir.good(), "directory_tree construction should succeed and be readable.");

    dir_tree_iterator *ted = dir.start(directory_tree::prefix);
      // create our iterator to do a prefix traversal.

    int depth;  // current depth in tree.
    filename curr;  // the current path the iterator is at.
    string_array files;  // the filenames held at the iterator.

    while (directory_tree::current(*ted, curr, files)) {
      // we have a good directory to show.
      directory_tree::depth(*ted, depth);
//      log(string_manipulation::indentation(depth * 2) + astring("[")
//          + curr.raw() + "]");
      astring names;
      for (int i = 0; i < files.length(); i++) names += files[i] + " ";
      if (names.length()) {
        astring split;
        string_manipulation::split_lines(names, split, depth * 2 + 2);
//        log(split);
      }

      // go to the next place.
      directory_tree::next(*ted);
    }

    directory_tree::throw_out(ted);
  }

  {
    // second test group.  seek operation.
//scan the directory, create some temporary directories and junk filenames
//therein, then seek to that location.

  }

  {
    // third test group.  tree comparison operation.
//    log(astring("Self-comparing directory tree at \"") + path + "\"");
//    log(astring("Using pattern-match \"") + pattern + "\"");

//    LOG("reading tree 1.");
    directory_tree dir(path, pattern.s());
    ASSERT_TRUE(dir.good(), "the directory should be readable for self-compare");

    // now read a copy of the tree also.
//    LOG("reading tree 2.");
    directory_tree dir2(path, pattern.s());
    ASSERT_TRUE(dir2.good(), "the directory should read the second time fine too");

    LOG("comparing the two trees.");
    filename_list diffs;
    directory_tree::compare_trees(dir, dir2, diffs, file_info::EQUAL_CHECKSUM_TIMESTAMP_FILESIZE);
LOG(diffs.text_form());

    ASSERT_FALSE(diffs.elements(), "there should be no differences comparing identical dirs");
  }

  {
    // fourth test: see if the calculate function works.
//    log(astring("Calculating sums for tree at \"") + path + "\"");
//    log(astring("Using pattern-match \"") + pattern + "\"");

//    LOG("reading tree 1.");
    directory_tree dir(path, pattern.s());
    ASSERT_TRUE(dir.good(), "the directory should be readable for checksums");

    // now read a copy of the tree also.
//    LOG("reading tree 2.");
    directory_tree dir2(path, pattern.s());
    ASSERT_TRUE(dir2.good(), "checksummer should be able to read second time also");

//    LOG("calculating checksums for tree 1.");
    ASSERT_TRUE(dir.calculate(JUST_SIZES), "the first checksummer tree can be calculated");

//    LOG("calculating checksums for tree 2.");
    ASSERT_TRUE(dir2.calculate(JUST_SIZES), "the second checksummer tree can be calculated");

//    LOG("comparing the two trees.");
    filename_list diffs;
    directory_tree::compare_trees(dir, dir2, diffs, file_info::EQUAL_CHECKSUM_TIMESTAMP_FILESIZE);
//LOG(diffs.text_form());

    ASSERT_FALSE(diffs.elements(), "no checksummer differences should be seen for identical directories");
  }

  {
    // fifth test: see if the packing works.
//    log(astring("Reading tree for packing at \"") + path + "\"");
//    log(astring("Using pattern-match \"") + pattern + "\"");

//    LOG("reading tree.");
    directory_tree dir(path, pattern.s());
    ASSERT_TRUE(dir.good(), "packer directory should be read");

//    LOG("calculating checksums for tree.");
    ASSERT_TRUE(dir.calculate(JUST_SIZES), "the first packer tree can be calculated");

    byte_array packed_form;
    int size_packed = dir.packed_size();
    dir.pack(packed_form);
//LOG(a_sprintf("tree became %d abyte array", packed_form.length()));
    ASSERT_EQUAL(size_packed, packed_form.length(), "packed size should be right");

    directory_tree dir2;
    ASSERT_TRUE(dir2.unpack(packed_form), "second tree can be unpacked from the first");

//    LOG("comparing the two trees.");
    filename_list diffs;
    directory_tree::compare_trees(dir, dir2, diffs, file_info::EQUAL_CHECKSUM_TIMESTAMP_FILESIZE);
//LOG(diffs.text_form());

    ASSERT_FALSE(diffs.elements(), "identical directories should stay same after packing");

    directory_tree::compare_trees(dir2, dir, diffs, file_info::EQUAL_CHECKSUM_TIMESTAMP_FILESIZE);
    ASSERT_FALSE(diffs.elements(), "no differences for reverse compare identical dirs");
  }

  {
    // sixth test: see if the make_directories function works.
LOG("reading tree to recreate");
    directory_tree dir(path, pattern.s());
    ASSERT_TRUE(dir.good(), "makedirs test directory reading");
    filename tmpdir(environment::get("GENERATED_DIR") + "/zz_balfazzaral");
    LOG(astring("will write to tmp in ") + tmpdir);
    basis::outcome result = dir.make_directories(tmpdir.raw());
    ASSERT_EQUAL(result.value(), common::OKAY, "makedirs should succeed");
    

LOG("what happened with that?  did it work?");

//hmmm: compare the directories with what we expect to be made;
//      do a dirtree iterator on the path, and make sure each of those exists in the target place.


    // clean up the output directory.
//this won't do it; it's a directory!
//    bool worked = tmpdir.recursive_unlink();    
//    ASSERT_TRUE(worked, "removing temporary files after test");

//hmmm: plug in real recursive delete here instead.
basis::un_int kid;
launch_process::run("rm", astring("-rf ") + tmpdir.raw(), launch_process::AWAIT_APP_EXIT, kid);
ASSERT_FALSE(kid, "removing temporary files after test");

  }


// nth test:
// combine the results of the second test with a comparison like in the
// third test.  delete all of those temporary files that were added.
// rescan tree. make sure that a tree containing the temporaries
// when compared with the current post-deletion tree produces a list
// that contains all the temporary files and directories.


//hmmm: more tests!

  return final_report();
}

HOOPLE_MAIN(test_directory_tree, )

