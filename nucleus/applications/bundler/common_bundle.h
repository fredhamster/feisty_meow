#ifndef COMMON_BUNDLER_DEFS
#define COMMON_BUNDLER_DEFS

/*****************************************************************************\
*                                                                             *
*  Name   : common bundler definitions                                        *
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

//! Contains some definitions used by both the bundle creator and unpacker.
/*!
  Note that this is a heavyweight header and should not be pulled into
  other headers.
*/

#include <basis/astring.h>
#include <basis/contracts.h>
#include <filesystem/byte_filer.h>
#include <structures/set.h>

////////////////////////////////////////////////////////////////////////////

//! flags that control special attributes of the packed files.
enum special_bundling_flags {
  SOURCE_EXECUTE = 0x2,  //!< the file should be executed before bundling.
  TARGET_EXECUTE = 0x4,  //!< the file should be executed on unbundling.
  RECURSIVE_SRC = 0x8,  //!< source is a recursive folder.
  OMIT_PACKING = 0x10,  //!< for a source side exe, do not pack the file.
  SET_VARIABLE = 0x20,  //!< this item just has a variable assignment.
  IGNORE_ERRORS = 0x40,  //!< if set, errors in an item will not stop program.
  NO_OVERWRITE = 0x80,  //!< target file will not be overwritten if exists.
  QUIET_FAILURE = 0x100,  //!< when errors happen, no popup message happens.
  MAKE_BACKUP_FILE = 0x200,  //!< save a copy if original file already exists.
  TEST_VARIABLE_DEFINED = 0x400  //!< check for required variable's presence.
};

////////////////////////////////////////////////////////////////////////////

//! we will read the manifest pieces out of our own exe image.
/*!
  the manifest chunks provide us with enough information to unpack the
  data chunks that come afterward.
*/

struct manifest_chunk : public basis::text_formable
{
  basis::un_int _size;  //!< the size of the packed file.
  basis::astring _payload;  //!< guts of the chunk, such as location for file on target or a variable definition.
  basis::un_int _flags;  //!< uses the special_bundling_flags.
  basis::astring _parms;  //!< the parameters to pass on the command line.
  structures::string_set _keywords;  //!< keywords applicable to this item.
  basis::byte_array c_filetime;  //!< more than enough room for unix file time.

  // note: when flags has SET_VARIABLE, the _payload is the variable
  // name to be set and the _parms is the value to use.

  static int packed_filetime_size();

  //! the chunk is the unit found in the packing manifest in the bundle.
  manifest_chunk(int size, const basis::astring &target, int flags,
      const basis::astring &parms, const structures::string_set &keywords)
      : _size(size), _payload(target), _flags(flags), _parms(parms),
        _keywords(keywords), c_filetime(packed_filetime_size()) {
    for (int i = 0; i < packed_filetime_size(); i++) c_filetime[i] = 0;
  }

  manifest_chunk() : _size(0), _flags(0), c_filetime(packed_filetime_size()) {
    //!< default constructor.
    for (int i = 0; i < packed_filetime_size(); i++) c_filetime[i] = 0;
  }

  virtual ~manifest_chunk();

  virtual void text_form(basis::base_string &state_fill) const {
    state_fill.assign(basis::astring(class_name())
        + basis::a_sprintf(": size=%d payload=%s flags=%x parms=%s",
              _size, _payload.s(), _flags, _parms.s()));
  }

  DEFINE_CLASS_NAME("manifest_chunk");

  void pack(basis::byte_array &target) const;  //!< streams out into the "target".
  bool unpack(basis::byte_array &source);  //!< streams in from the "source".

  static bool read_manifest(filesystem::byte_filer &bundle, manifest_chunk &to_fill);
    //!< reads a chunk out of the "bundle" and stores it in "to_fill".
    /*!< false is returned if the read failed. */

  static basis::astring read_a_string(filesystem::byte_filer &bundle);
    //!< reads a string from the "bundle" file, one byte at a time.

  static bool read_an_int(filesystem::byte_filer &bundle, basis::un_int &found);
    //!< reads an integer (4 bytes) from the file into "found".

  static bool read_an_obscured_int(filesystem::byte_filer &bundle, basis::un_int &found);
    //!< reads in our obscured packing format for an int, which takes 8 bytes.

  static bool read_a_filetime(filesystem::byte_filer &bundle, basis::byte_array &found);
    //!< retrieves packed_filetime_size() byte timestamp from the "bundle".
};

////////////////////////////////////////////////////////////////////////////

#endif

