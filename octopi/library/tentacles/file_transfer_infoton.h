#ifndef FILE_TRANSFER_INFOTON_CLASS
#define FILE_TRANSFER_INFOTON_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : file_transfer_infoton                                             *
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

#include <basis/byte_array.h>
#include <filesystem/directory_tree.h>
#include <octopus/infoton.h>

namespace octopi {

//! Base objects used by the file transfer tentacle to schedule transfers.
/*!
  Note: this is a fairly heavy-weight header.
*/

class file_transfer_infoton : public infoton
{
public:
  //! the commands specify what this package is intended to do.
  enum commands {
    BUILD_TARGET_TREE = 4,
      //!< asks the target side to build the directory tree from the source.
      /*!< this is a new first step for the transfer.  we want to make sure
      the target will look right for what we're going to transfer over. */
    TREE_COMPARISON = 1,
      //!< the destination root will be compared with the source root.
      /*!< the packed data in the request holds a packed symbol tree
      describing the destination hierarchy.  the packed data in the response
      is the packed filename list that represents the differences between the
      two hierarchies.  this is considered to start a file transfer based on
      those differences. */
    PLACE_FILE_CHUNKS = 2,
      //!< the destination side requests a new set of chunks.
      /*!< this is based on the source's memory of where the transfer is at.
      this will only perform properly when the file transfer was requested to
      be started by the client using a TREE_COMPARISON request.  the request
      has an empty data chunk, but the response consists of an arbitrary
      number of pairs of @code
      [ file_transfer_header + file chunk described in header ]
      @endcode */
    CONCLUDE_TRANSFER_MARKER = 3,
      //!< this infoton marks the end of the transfer process.
      /*!< we've added this type of transfer infoton to handle the finish
      of the transfer.  previously this was marked by a null data packet,
      which turns out to be a really bad idea. */
  };

  basis::outcome _success;  //!< reports what kind of result occurred.
  bool _request;  //!< if it's not a request, then it's a response.
  basis::abyte _command;  //!< one of the commands above.
  basis::astring _src_root;  //!< the top-level directory of the source.
  basis::astring _dest_root;  //!< the top-level directory of the destination.
  basis::byte_array _packed_data;  //!< the packed headers and file chunks.

  file_transfer_infoton();

  file_transfer_infoton(const basis::outcome &success, bool request, commands command,
          const basis::astring &source, const basis::astring &destination,
          const basis::byte_array &packed_data);

  virtual ~file_transfer_infoton();

  virtual void pack(basis::byte_array &packed_form) const;
  virtual bool unpack(basis::byte_array &packed_form);

  void package_tree_info(const filesystem::directory_tree &tree,
          const structures::string_array &includes);
    //!< prepares the packed data from the "tree" and "includes" list.

  virtual basis::clonable *clone() const { return cloner<file_transfer_infoton>(*this); }

  virtual void text_form(basis::base_string &fill) const;

  virtual int packed_size() const;

  static const structures::string_array &file_transfer_classifier();
    //!< returns the classifier for this type of infoton.
};

} //namespace.

#endif

