#ifndef FILE_TIME_CLASS
#define FILE_TIME_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : file_time                                                         *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

//! A platform independent way to obtain the timestamp of a file.

#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/contracts.h>

#include <stdio.h>
#include <time.h>

namespace filesystem {

class file_time
: public virtual basis::hoople_standard,
  public virtual basis::orderable
{
public:
  file_time();  //!< sets up a bogus file_time object.

  file_time(FILE *the_FILE);
    //!< sets up the file_time information given a the file stream of interest.
    /*!< If the stream is NULL_POINTER, then the file_time is set up with an invalid
    time. */

  file_time(const basis::astring &filename);
    //!< this constructor operates on a file's name rather than a FILE stream.

  file_time(const time_t &init);
    //!< starts the file_time with a particular "init" time.

//hmmm: need a converter that sucks in an earth_time time_locus object.

  virtual ~file_time();

  DEFINE_CLASS_NAME("file_time");

  virtual void text_form(basis::base_string &time_string) const;
    //!< returns a definitive but sorta ugly version of the file's time.

  virtual void readable_text_form(basis::base_string &time_string) const;
    //!< sets "time_string" to a human readable form of the file's time.

  void reset(FILE *the_FILE);
    //!< reacquires the time from a different FILE than constructed with.
    /*!< this also can connect a FILE to the file_time object after using the
    empty constructor.  further, it can also be used to refresh a file's time
    to account for changes in its timestamp. */

  void reset(const basis::astring &filename);
    //!< parallel version of reset() takes a file name instead of a stream.

  void reset(const time_t &init);
    //!< parallel version of reset() takes a time_t instead of a stream.

  time_t raw() const { return _when; }
    //!< provides the OS version of the file's timestamp.

  bool set_time(const basis::astring &filename);
    //!< sets the time for the the "filename" to the currently held time.

  // Standard comparison operators between this file time and the file time
  // "ft2".  These are meaningless if either time is invalid.
  virtual bool less_than(const basis::orderable &ft2) const;
  virtual bool equal_to(const basis::equalizable &ft2) const;

  // supports streaming the time into and out of a byte array.
  virtual int packed_size() const;
  virtual void pack(basis::byte_array &packed_form) const;
  virtual bool unpack(basis::byte_array &packed_form);

private:
  time_t _when;  //!< our record of the file's timestamp.

  int compare(const file_time &ft2) const;
    //!< root comparison function for all the operators.
};

} //namespace.

#endif

