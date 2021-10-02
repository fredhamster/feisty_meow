#ifndef HEAVY_FILE_OPERATIONS_CLASS
#define HEAVY_FILE_OPERATIONS_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : heavy file operations                                             *
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

#include "filename_list.h"

#include <basis/astring.h>
#include <basis/contracts.h>

namespace filesystem {

//! describes one portion of an ongoing file transfer.
/*! this is just a header describing an attached byte package.  it is expected
that the bytes follow this in the communication stream. */

class file_transfer_header : public basis::packable
{
public:
  basis::astring _filename;  //!< the name of the file being transferred.
//hmmm: consider adding full length here so we know it.
  double _byte_start;  //!< the starting location in the file being sent.
  int _length;  //!< the length of the transferred piece.
  file_time _time;  //!< the timestamp on the file.

  DEFINE_CLASS_NAME("file_transfer_header");

  file_transfer_header(const file_time &time_stamp);
    //!< refactored to force addition of the time_stamp.

  virtual void pack(basis::byte_array &packed_form) const;
  virtual bool unpack(basis::byte_array &packed_form);

  virtual int packed_size() const;

  basis::astring text_form() const;

//hmmm: this could live in lots of other places.  file_info for one.
  basis::astring readable_text_form() const;
    //!< a nicer formatting of the information.
};

//////////////

//! Provides serious file operations, such as copy and partial writing.

class heavy_file_operations : public virtual basis::root_object
{
public:
  virtual ~heavy_file_operations();

  enum outcomes {
    OKAY = basis::common::OKAY,
    BAD_INPUT = basis::common::BAD_INPUT,
    FINISHED = basis::common::IS_EMPTY,  // nothing left to pack.
    DEFINE_OUTCOME(SOURCE_MISSING, -43, "The source file is not accessible"),
    DEFINE_OUTCOME(TARGET_DIR_ERROR, -44, "The target's directory could not "
        "be created"),
    DEFINE_OUTCOME(TARGET_ACCESS_ERROR, -45, "The target file could not be "
        "created")
  };
  static const char *outcome_name(const basis::outcome &to_name);

  DEFINE_CLASS_NAME("heavy_file_operations");

  static const size_t COPY_CHUNK_FACTOR;
    //!< the default copy chunk size for the file copy method.
  static size_t copy_chunk_factor();
    //!< method can be exported for use by shared libs.

  static basis::outcome copy_file(const basis::astring &source, const basis::astring &destination,
          int copy_chunk_factor = heavy_file_operations::copy_chunk_factor());
    //!< copies a file from the "source" location to the "destination".
    /*!< the outcomes could be from this class or from common::outcomes.
    the "copy_chunk_factor" is the read buffer size to use while copying. */

  static basis::outcome write_file_chunk(const basis::astring &target, double byte_start,
          const basis::byte_array &chunk, bool truncate = true,
          int copy_chunk_factor = heavy_file_operations::copy_chunk_factor());
    //!< stores a chunk of bytes into the "target" file.
    /*!< writes the content stored in "chunk" into the file "target" at the
    position "byte_start".  the entire "chunk" will be used, which means the
    file will either be that much larger or have the space between byte_start
    and (byte_start + chunk.length() - 1) replaced.  if the file is not yet as
    large as "byte_start", then it will be expanded appropriately.  if
    "truncate" is true, then any contents past the new chunk are dropped from
    the file. */

  static basis::outcome buffer_files(const basis::astring &source_root,
          const filename_list &to_transfer, file_transfer_header &last_action,
          basis::byte_array &storage, int maximum_bytes);
    //!< reads files in "to_transfer" and packs them into a "storage" buffer.
    /*!< the maximum size allowed in storage is "maximum_bytes".  the record
    of the last file piece stored in "last_action" allows the next chunk
    to be sent in subsequent calls.  note that the buffer "storage" is cleared
    out before bytes are stored into it; this is not an additive operation. */

private:
  static basis::outcome advance(const filename_list &to_transfer,
          file_transfer_header &last_action);
    //!< advances to the next file in the transfer list "to_transfer".
};

//////////////

} //namespace.

#endif

