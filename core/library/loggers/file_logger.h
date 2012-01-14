#ifndef FILE_LOGGER_CLASS
#define FILE_LOGGER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : file_logger                                                       *
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

//! Enables the printing of information to a log file.
/*!
  The information can be conditionally printed using the filter support.
  The log file will automatically be truncated when it passes the size limit.
*/

#include "console_logger.h"
#include "eol_aware.h"
#include "filter_set.h"

#include <basis/astring.h>
#include <basis/contracts.h>
#include <basis/functions.h>
#include <basis/mutex.h>
#include <filesystem/byte_filer.h>
#include <textual/parser_bits.h>

namespace loggers {

class file_logger : public virtual standard_log_base
{
public:
  file_logger();
    //!< creates a logger without a log file and with the default size limit.
    /*!< the log file name can be changed using filename(). */

  file_logger(const basis::astring &filename, int limit = DEFAULT_LOG_FILE_SIZE);
    //!< constructs a logger using the "filename" for output.
    /*!< there will be no logging if the "filename" is empty.  the "limit"
    specifies how large the log file can be (in bytes). */

  virtual ~file_logger();

  DEFINE_CLASS_NAME("file_logger");

  enum limits {
    //! this just defines the default for the log file size.
    DEFAULT_LOG_FILE_SIZE = 0x10F00D
  };

  bool good() const;
    //!< returns true if the logger appears correctly hooked up to a file.
    /*!< note that we don't open the file when file_logger is constructed;
    it is only opened once the first logging is attempted. */

  bool reopen();
    //!< closes the current file and attempts to reopen it.
    /*!< this is handy if the original opening of the file failed. */

  basis::outcome log(const basis::base_string &info, int filter = basis::ALWAYS_PRINT);
    //!< writes information to the log file (if the filename is valid).
    /*!< the "filter" value is checked to see if it is in the current set
    of allowed filters.  a value of zero is always printed.  if the filename()
    has not been set, then the information is lost. */

  basis::outcome log_bytes(const basis::byte_array &to_log, int filter = basis::ALWAYS_PRINT);
    //!< sends a stream of bytes "to_log" without interpretation into the log.
    /*!< if the "filter" is not enabled, then the info is just tossed out. */

  basis::outcome format_bytes(const basis::byte_array &to_log, int filter = basis::ALWAYS_PRINT);
    //!< fancifully formats a stream of bytes "to_log" and sends them into log.

  basis::astring name() const;
    //!< observes the filename where logged information is written.
  void name(const basis::astring &new_name);
    //!< modifies the filename where logged information will be written.
    /*!< if "new_name" is blank, then the logged information will not
    be saved. */

  int limit() const { return int(_file_limit); }
    //!< observes the allowable size of the log file.
  void limit(int new_limit) { _file_limit = new_limit; }
    //!< modifies the allowable size of the log file.

  void flush();
    //!< causes any pending writes to be sent to the output file.

  void truncate(size_t new_size);
    //!< chops the file to ensure it doesn't go much over the file size limit.
    /*!< this can be used externally also, but be careful with it. */

  //! returns a log file name for file_logger based on the program name.
  /*! for a program named myapp.exe, this will be in the form:
    {logging_dir}/myapp.log
  */
  static basis::astring log_file_for_app_name();

private:
  basis::astring *_filename;  //!< debugging output file.
  size_t _file_limit;  //!< maximum length of file before truncation.
  filesystem::byte_filer *_outfile;  //!< the object that points at our output file.
  basis::mutex *_flock;  //!< protects the file and other parameters.

  int size_reduction() const;
    //!< returns the size of the chunk to truncate from the file.
    /*!< this is a percentage of the maximum size allowed. */

  bool open_file();
    //!< if the opening of the file is successful, then true is returned.
    /*!< also, the _outfile member variable is non-zero. */

  void close_file();
    //!< shuts down the file, if any, we had opened for logging.

  // unavailable.
  file_logger(const file_logger &);
  file_logger &operator =(const file_logger &);
};

//////////////

//! a macro that retasks the program-wide logger as a file_logger.
#define SETUP_FILE_LOGGER { \
  loggers::standard_log_base *old_log = loggers::program_wide_logger::set \
    (new loggers::file_logger(loggers::file_logger::log_file_for_app_name())); \
  WHACK(old_log); \
}

} //namespace.

#endif

