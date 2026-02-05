/*****************************************************************************\
*                                                                             *
*  Name   : test_fcopy_file_transfer                                          *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Tests the directory_tree object as a file copy prompter.                 *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/function.h>
#include <basis/guards.h>
#include <opsystem/application_shell.h>
#include <loggers/console_logger.h>
#include <opsystem/directory.h>
#include <opsystem/directory_tree.h>
#include <opsystem/filename.h>
#include <opsystem/filename_list.h>
#include <opsystem/heavy_file_ops.h>
#include <data_struct/static_memory_gremlin.h>
#include <textual/string_manipulation.h>

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger(), s)

class test_fcopy : public application_shell
{
public:
  test_fcopy() : application_shell(static_class_name()) {}
  IMPLEMENT_CLASS_NAME("test_dirtree_fcopy");
  int execute();
};

int test_fcopy::execute()
{
  FUNCDEF("execute");

  if (__argc < 3)
    non_continuable_error(class_name(), "command line", "this program needs two "
          "parameters:\na directory for the source and one for the target.");

  istring source_dir = __argv[1];
  istring target_dir = __argv[2];

  // read the source location.
  log(istring("Scanning source tree at \"") + source_dir + "\"");
  directory_tree source(source_dir, "*");
  if (!source.good())
    non_continuable_error(class_name(), "directory_tree construction",
          "the source directory could not be read");

  // read the stuff that exists at the target.
  log(istring("Scanning target tree at \"") + target_dir + "\"");
  directory_tree target(target_dir, "*");
  if (!target.good())
    non_continuable_error(class_name(), "directory_tree construction",
        "the target directory could not be read");

  LOG("calculating checksums for source.");
  if (!source.calculate())
    non_continuable_error(class_name(), "directory_tree calculation",
        "the source tree could not be calculated");

  LOG("calculating checksums for target.");
  if (!target.calculate())
    non_continuable_error(class_name(), "directory_tree calculation",
        "the target tree could not be calculated");

  istring source_start;
  istring target_start;
  if (__argc > 3)
    source_start = __argv[3];
  if (__argc > 4)
    target_start = __argv[4];

  LOG("comparing the two trees.");
  filename_list diffs;
  directory_tree::compare_trees(source, source_start, target, target_start,
      diffs);
//LOG("missing files:");
//LOG(diffs.text_form());

  byte_array packed_form;
  diffs.pack(packed_form);
  filename_list regen;
  if (!regen.unpack(packed_form))
    non_continuable_error(class_name(), "filename_list packing",
        "could not unpack the list of differences");

//LOG("after packing and restoring:");
//LOG(regen.text_form());

  if (regen.elements() != diffs.elements())
    non_continuable_error(class_name(), "filename_list packing",
        "there were a different number of elements in unpacked form");
  for (int i = 0; i < diffs.elements(); i++) {
    if (!regen.member(*diffs[i])) {
      istring name = diffs[i]->raw();
      non_continuable_error(class_name(), "filename_list packing",
          istring("name from original set was missing in regenerated: ")
              + name);
    }
  }

  for (int i = 0; i < diffs.elements(); i++) {
    file_info *curr = diffs[i];
    filename source_file = source.path() + filename::default_separator()
        + curr->raw();
    filename target_file = target.path() + filename::default_separator()
        + curr->secondary();
//LOG(istring("cp source: ") + source_file.raw());
//LOG(istring("-> target: ") + target_file.raw());
    filename targ_dir = target_file.dirname();
    if (!targ_dir.is_directory()) {
      bool worked = directory::recursive_create(targ_dir);
      if (!worked)
        non_continuable_error(class_name(), "recursive mkdir",
            istring("failed to create the target directory ") + targ_dir);
    }

    outcome ret = heavy_file_operations::copy_file(source_file, target_file);
    if (ret != heavy_file_operations::OKAY)
      non_continuable_error(class_name(), "copying file", istring("there was an error ")
          + heavy_file_operations::outcome_name(ret)
          + " when copying the file.");
  }

//do the copy by going across the entire set of files and making sure
//they get copied to target.
//
//reread the target location.
//
//compare with source tree read before.

  guards::alert_message("directory_tree file transfer:: works for those functions tested.");
  return 0;
}

HOOPLE_MAIN(test_fcopy, )

