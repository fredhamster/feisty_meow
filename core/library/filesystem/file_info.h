#ifndef FILE_INFO_CLASS
#define FILE_INFO_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : file_info                                                         *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1993-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "filename.h"
#include "file_time.h"

#include <basis/definitions.h>
#include <basis/enhance_cpp.h>

namespace filesystem {

//! Encapsulates some measures and calculations based on a file's contents.

class file_info : public filename 
{
public:
  //! this enum encapsulates how files may be compared.
  enum file_similarity {
    EQUAL_NAME = 0,         // we assume name equality is pre-eminent and always required.
    EQUAL_CHECKSUM = 0x1,   // the files have the same checksum, however computed.
    EQUAL_TIMESTAMP = 0x2,  // the files have exactly equal timestamps.
    EQUAL_FILESIZE = 0x4,   // the files have the same sizes.
    EQUAL_CHECKSUM_TIMESTAMP_FILESIZE = EQUAL_CHECKSUM & EQUAL_TIMESTAMP & EQUAL_FILESIZE
  };

  double _file_size;  //!< the size of the file.
  file_time _time;  //!< the file's access time.
  int _checksum;  //!< the checksum for the file.

  file_info();  //!< blank constructor.

  file_info(const filename &to_copy, double file_size, 
         const file_time &time = file_time(), int checksum = 0);
    //!< to get the real file size, timestamp and checksum, invoke the calculate method.

  file_info(const file_info &to_copy);

  virtual ~file_info();

  DEFINE_CLASS_NAME("file_info");

  file_info &operator = (const file_info &to_copy);

  basis::astring text_form() const;

  bool calculate(const basis::astring &prefix, bool just_size_n_time,
        int checksum_edge = 1 * basis::KILOBYTE);
    //!< fills in the correct file size and checksum information for this file.
    /*!< note that the file must exist for this to work.  if "just_size_n_time"
    is true, then the checksum is not calculated.  if the "prefix" is not
    empty, then it is used as a directory path that needs to be added to
    the filename to make it completely valid.  this is common if the
    filename is stored relative to a path.  the "checksum_edge" is used for
    checksum calculations; only 2 * checksum_edge bytes will be factored in,
    each part from the head and tail ends of the file. */

  const basis::astring &secondary() const;
    //!< observes the alternate form of the name.
  void secondary(const basis::astring &new_sec);
    //!< accesses the alternate form of the name.

  const basis::byte_array &attachment() const;
    //!< returns the chunk of data optionally attached to the file's info.
    /*!< this supports extending the file's record with extra data that might
    be needed during processing. */
  void attachment(const basis::byte_array &new_attachment);
    //!< sets the optional chunk of data hooked up to the file's info.

  // standard streaming operations.
  virtual int packed_size() const;
  virtual void pack(basis::byte_array &packed_form) const;
  virtual bool unpack(basis::byte_array &packed_form);

private:
  basis::astring c_secondary;  //!< alternate filename for the main one.
  basis::byte_array c_attachment;  //!< extra information, if needed.
};

} //namespace.

#endif

