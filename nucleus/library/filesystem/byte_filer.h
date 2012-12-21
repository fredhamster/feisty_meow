#ifndef BYTE_FILER_CLASS
#define BYTE_FILER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : byte_filer                                                        *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/definitions.h>

#include "filename.h"

namespace filesystem {

// forward declarations.
class file_hider;

//! Provides file managment services using the standard I/O support.

class byte_filer
{
public:
  byte_filer();
    //!< constructs an object that doesn't access a file yet.
    /*!< use open() to make the object valid. */

  byte_filer(const basis::astring &fname, const basis::astring &permissions);
    //!< opens a file "fname" as specified in "permissions".
    /*!< these are identical to the standard I/O permissions:

    - "r"  - opens text file for reading.
    - "w"  - opens text file for writing and discards any previous contents.
    - "a"  - opens text file for writing at end; appends to contents.
    - "r+" - opens text file for update (both reading and writing).
    - "w+" - creates a text file for update; any previous contents are lost.
    - "a+" - opens or creates a text file for update, appending at end.

    a "b" can be added to the end of these to indicate a binary file should
    be used instead of a text file. */

  byte_filer(const char *fname, const char *permissions);
    //!< synonym for above but takes char pointers.

  byte_filer(bool auto_close, void *opened);
    //!< uses a previously "opened" stdio FILE handle.  be careful!
    /*!< the "opened" object must be a valid FILE pointer; void * is used to
    avoid pulling in the stdio header.  this method will not close the file
    handle if "auto_close" is false. */

  ~byte_filer();

  static size_t file_size_limit();
    //!< returns the maximum size that seek and length can support.
    /*!< use the huge_file class if you need to exceed the stdio limits. */

  bool open(const basis::astring &fname, const basis::astring &permissions);
    //!< opens a file with "fname" and "permissions" as in the constructor.
    /*!< if a different file had already been opened, it is closed. */

  void close();
    //!< shuts down the open file, if any.
    /*!< open() will have to be invoked before this object can be used again. */

  basis::astring name() const;
    //!< returns the file name that the object is operating on.

  bool good();
    //!< returns true if the file seems to be in the appropriate desired state.

  size_t length();
    //!< returns the file's total length, in bytes.
    /*!< this cannot accurately report a file length if it is file_size_limit()
    or greater. */

  size_t tell();
    //!< returns the current position within the file, in terms of bytes.
    /*!< this is also limited to file_size_limit(). */

  void flush();
    //!< forces any pending writes to actually be saved to the file.

  enum origins {
    FROM_START,    //!< offset is from the beginning of the file.
    FROM_END,      //!< offset is from the end of the file.
    FROM_CURRENT   //!< offset is from current cursor position.
  };

  bool seek(int where, origins origin = FROM_START);
    //!< places the cursor in the file at "where", based on the "origin".
    /*!< note that if the origin is FROM_END, then the offset "where" should
    be a negative number if you're trying to access the interior of the file;
    positive offsets indicate places after the actual end of the file. */

  bool eof();
    //!< returns true if the cursor is at (or after) the end of the file.

  int read(basis::abyte *buffer, int buffer_size);
    //!< reads "buffer_size" bytes from the file into "buffer".
    /*!< for all of the read and write operations, the number of bytes that
    is actually processed for the file is returned. */
  int write(const basis::abyte *buffer, int buffer_size);
    //!< writes "buffer_size" bytes into the file from "buffer".

  int read(basis::byte_array &buffer, int desired_size);
    //!< reads "buffer_size" bytes from the file into "buffer".
  int write(const basis::byte_array &buffer);
    //!< writes the "buffer" into the file.

  int read(basis::astring &buffer, int desired_size);
    //!< read() pulls up to "desired_size" bytes from the file into "buffer".
    /*!< since the read() will grab as much data as is available given that it
    fits in "desired_size".  null characters embedded in the file are a bad
    issue here; some other method must be used to read the file instead (such
    as the byte_array read above).  the "buffer" is shrunk to fit the zero
    terminator that we automatically add. */

  int write(const basis::astring &buffer, bool add_null = false);
    //!< stores the string in "buffer" into the file at the current position.
    /*!< if "add_null" is true, then write() adds a zero terminator to what
    is written into the file.  otherwise just the string's non-null contents
    are written. */

  int getline(basis::abyte *buffer, int desired_size);
    //!< reads a line of text (terminated by a return) into the "buffer".
  int getline(basis::byte_array &buffer, int desired_size);
    //!< reads a line of text (terminated by a return) into the "buffer".
  int getline(basis::astring &buffer, int desired_size);
    //!< reads a line of text (terminated by a return) into the "buffer".

  bool truncate();
    //!< truncates the file after the current position.

  void *file_handle();
    //!< provides a hook to get at the operating system's file handle.
    /*!< this is of the type FILE *, as defined by <stdio.h>. */

private:
  file_hider *_handle;  //!< the standard I/O support that we rely upon.
  filename *_filename;  //!< holds onto our current filename.
  bool _auto_close;  //!< true if the object should close the file.

  // not to be called.
  byte_filer(const byte_filer &);
  byte_filer &operator =(const byte_filer &);
};

} //namespace.

#endif

