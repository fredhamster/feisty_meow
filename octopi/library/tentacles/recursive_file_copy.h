#ifndef RECURSIVE_FILE_COPY_CLASS
#define RECURSIVE_FILE_COPY_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : recursive file copy                                               *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <basis/contracts.h>
#include <filesystem/filename_list.h>

namespace octopi {

//! Copies entire hierarchies in the file system from one place to another.

class recursive_file_copy
{
public:
  virtual ~recursive_file_copy();

  DEFINE_CLASS_NAME("recursive_file_copy");

  enum outcomes {
    OKAY = basis::common::OKAY,
    BAD_INPUT = basis::common::BAD_INPUT,
    GARBAGE = basis::common::GARBAGE,
    NOT_FOUND = basis::common::NOT_FOUND,
    NONE_READY = basis::common::NONE_READY,
    FAILURE = basis::common::FAILURE
  };
  static const char *outcome_name(const basis::outcome &to_name);

//hmmm: need an option to specify the logging that we are currently doing in here.

  //! copies a directory hierarchy starting at "source_dir" into "target_dir".
  /*! the "transfer_mode" is a combination of mode values from the file_transfer_tentacle class,
  and dictates how the files to copy will be decided.  the "includes" is a list of files that will
  be included.  if that is an empty array, then all files will be included.  the "source_start"
  is a subdirectory within the source to start copying at; only items there and below will be
  included. */
  static basis::outcome copy_hierarchy(int transfer_mode, const basis::astring &source_dir,
      const basis::astring &target_dir, const structures::string_array &includes,
      const basis::astring &source_start = basis::astring::empty_string());
};

} //namespace.

#endif

