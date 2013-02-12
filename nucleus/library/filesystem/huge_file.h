#ifndef HUGE_FILE_CLASS
#define HUGE_FILE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : huge_file                                                         *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2007-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "byte_filer.h"

#include <basis/astring.h>
#include <basis/outcome.h>
#include <basis/common_outcomes.h>
#include <basis/enhance_cpp.h>

namespace filesystem {

//! Supports reading and writing to very large files, > 4 gigabytes.
/*!
  The standard file I/O functions only handle files up to 4 gigabytes.  This
  class extends the range to essentially unlimited sizes, as long as the
  operating system can accurately do relative seeks and can read/write to
  files of the size needed.
*/

class huge_file
{
public:
  huge_file(const basis::astring &filename, const basis::astring &permissions);
    //!< opens "filename" for access, where it presumably is a very large file.
    /*!< see byte filer for a description of the permissions.
    */
  virtual ~huge_file();

  DEFINE_CLASS_NAME("huge_file");

  enum outcomes {
    OKAY = basis::common::OKAY,
    FAILURE = basis::common::FAILURE,
    ACCESS_DENIED = basis::common::ACCESS_DENIED,
    BAD_INPUT = basis::common::BAD_INPUT
  };

  const basis::astring &name() const;
    //!< returns the name of the file this operates on.

  bool good() const;
    //!< reports if the file was opened successfully.

  bool eof() const;
    //!< reports when the file pointer has reached the end of the file.

  double length();
    //!< expensive operation accesses the file to find length.

  double file_pointer() const { return _file_pointer; }
    //!< returns where we currently are in the file.

  basis::outcome seek(double new_position,
          byte_filer::origins origin = byte_filer::FROM_CURRENT);
    //!< move the file pointer to "new_position" if possible.
    /*!< the relative seek is the easiest type of seek to accomplish with a
    huge file.  the other types are also supported, but take a bit more to
    implement. */

  basis::outcome move_to(double absolute_posn);
    //!< simpler seek just goes from current location to "absolute_posn".

  basis::outcome read(basis::byte_array &to_fill, int desired_size, int &size_read);
    //!< reads "desired_size" into "to_fill" if possible.
    /*!< "size_read" reports how many bytes were actually read. */

  basis::outcome write(const basis::byte_array &to_write, int &size_written);
    //!< stores the array "to_write" into the file.
    /*!< "size_written" reports how many bytes got written. */

  bool truncate();
    //!< truncates the file after the current position.

  basis::outcome touch();
    //<! creates the file if it doesn't exist, or updates the time on the file.

  void flush();
    //!< forces any pending writes to actually be saved to the file.

private:
  byte_filer *_real_file;  //!< supports us but is subject to 4g limit.
  double _file_pointer;  //!< position in the file if OS is tracking us.
};

} //namespace.

#endif

