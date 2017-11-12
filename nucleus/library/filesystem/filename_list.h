#ifndef FILENAME_LIST_CLASS
#define FILENAME_LIST_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : filename_list                                                     *
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

//! Implements a list of filenames.
/*!
  This is based on an amorph so that adding to the list is efficient.
  The underlying type held is actually a file_info rather than a filename.
  This should not impose much extra overhead.

  Note: this is a heavyweight header; it shouldn't be used in other headers.
*/

#include "file_info.h"

#include <basis/contracts.h>
#include <structures/amorph.h>

namespace filesystem {

class filename_list
: public structures::amorph<file_info>, public virtual basis::packable
{
public:
  filename_list();

  filename_list &operator =(const filename_list &to_copy);

  int total_files() const;
    //!< returns the number of files currently held in the list.

  double total_size() const;
    //!< returns the full size of all files listed.

  bool calculate_progress(const filename &file, double current_offset,
      int &current_file, double &current_size);
    //!< given the last "file" and position, this returns current positioning.
    /*!< the "current_file" is set to the index of the "file" in the list
    (using 1-based numbering for a 1,2,3,... series), and the "current_size"
    is set to the total amount of bytes processed so far. */

  filename_list &operator = (const structures::string_array &to_copy);

  void fill(structures::string_array &to_fill);
    //!< stuffs the array "to_fill" with the filesnames from our list.

  const file_info *find(const filename &to_check) const;
    //!< locates the record of information for the filename "to_check".
    /*!< do not modify the returned object.  it contains the current state
    of the file in question.  if the file wasn't in the list, then NULL_POINTER is
    returned. */

  int locate(const filename &to_find) const;
    //! finds the index for "to_find" or returns a negative number.

  bool member(const filename &to_check) const;
    //!< returns true if "to_check" is listed here.

  bool member_with_state(const file_info &to_check, file_info::file_similarity comparison_method);
    //!< returns true if the file "to_check" exists in the list with appropriate equivalence.
    /*!< this will fail if the file name isn't present at all.  and then it will also not return
    true if the size is different (when EQUAL_FILESIZE is true), when the timestamp is different
    (when EQUAL_TIMESTAMP is true), and when the checksum is different (you get the idea). */

  //! max_lines is the maximum number of lines to print into the string.
  basis::astring text_form(int max_lines = MAXINT32) const;

  virtual int packed_size() const;

  virtual void pack(basis::byte_array &packed_form) const;

  virtual bool unpack(basis::byte_array &packed_form);
};

} //namespace.

#endif

