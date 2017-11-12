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

#include "critical_events.h"
#include "file_logger.h"
#include "logging_filters.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/mutex.h>
#include <configuration/application_configuration.h>
#include <filesystem/byte_filer.h>
#include <filesystem/directory.h>
#include <filesystem/filename.h>
#include <mathematics/chaos.h>
#include <structures/static_memory_gremlin.h>
#include <textual/byte_formatter.h>

#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
  #include <unistd.h>
#else
  #include <io.h>
#endif
#include <stdio.h>

using namespace basis;
using namespace configuration;
using namespace filesystem;
using namespace mathematics;
using namespace structures;
using namespace textual;

namespace loggers {

const int REDUCE_FACTOR = 5;
  // we whack this portion of the file every time we truncate.  if it's set
  // to 14, for example, then a 14th of the file is whacked every time whacking
  // is needed.

const int MAXIMUM_BUFFER_SIZE = 140000;
  // the maximum allowed chunk that can be copied from the old logfile
  // to the current one.

int static_chaos() {
  static chaos __hidden_chaos;
  return __hidden_chaos.inclusive(0, 1280004);
}

file_logger::file_logger()
: _filename(new astring()),
  _file_limit(DEFAULT_LOG_FILE_SIZE),
  _outfile(NULL_POINTER),
  _flock(new mutex)
{
  name("");
}

file_logger::file_logger(const astring &initial_filename, int limit)
: _filename(new astring()),
  _file_limit(limit),
  _outfile(NULL_POINTER),
  _flock(new mutex)
{
  name(initial_filename); 
  // we don't open the file right away because we don't know they'll ever
  // use the thing.
}

file_logger::~file_logger()
{
  close_file();
  WHACK(_filename);
  WHACK(_flock);
}

basis::astring file_logger::log_file_for_app_name()
{
  filename prog = application_configuration::application_name();
  return application_configuration::make_logfile_name(prog.rootname() + ".log");
}

bool file_logger::reopen()
{
  auto_synchronizer l(*_flock);
  name(*_filename);
  return open_file();
}

void file_logger::close_file()
{
  auto_synchronizer l(*_flock);
  if (_outfile) _outfile->flush();
    // dump anything that hasn't gone out yet.
  WHACK(_outfile);
}

void file_logger::name(const astring &new_name)
{
  auto_synchronizer l(*_flock);
  close_file();
  *_filename = new_name; 
}

int file_logger::size_reduction() const
{
  auto_synchronizer l(*_flock);
  return int(_file_limit / REDUCE_FACTOR);
}

bool file_logger::good() const
{
  auto_synchronizer l(*_flock);
  if (!_outfile && !_file_limit) return true;
  if (!_outfile) return false;
  return _outfile->good();
}

astring file_logger::name() const
{
  auto_synchronizer l(*_flock);
  return *_filename;
}

void file_logger::flush()
{
  auto_synchronizer l(*_flock);
  if (!_outfile) open_file();
  if (_outfile) _outfile->flush();
}

bool file_logger::open_file()
{
  auto_synchronizer l(*_flock);
  close_file();  // close any existing log file.

  if (!_file_limit) {
    // if there's a limit of zero, we'll never open the file.
    return true;
  }

  // make sure we've got a name.
  if (!*_filename) {
    // if the name is empty, they don't want to save to a normal log file.
    _outfile = new byte_filer;
    return true;
  }

  // canonicalize the name so that we use the same tag for synchronization.
  // this might still fail if there are some jokers using different relative
  // paths to the file.  but if it's an absolute name, it should work.
  for (int i = 0; i < _filename->length(); i++)
    if ((*_filename)[i] == '\\') (*_filename)[i] = '/';

  // make sure the directory containing the log file exists, if we can.
  filename temp_file(*_filename);
  filename temp_dir(temp_file.dirname());
  if (!temp_dir.good() || !temp_dir.is_directory()) {
    directory::recursive_create(temp_dir);
  }

  // if this opening doesn't work, then we just can't log.
  _outfile = new byte_filer(*_filename, "a+b");
  return _outfile->good();
}

outcome file_logger::log(const base_string &to_show, int filter)
{
  if (!_file_limit) return common::OKAY;

  size_t current_size = 0;
  {
    auto_synchronizer l(*_flock);
    if (!member(filter)) return common::OKAY;
    if (!_outfile) open_file();
    if (!_outfile) return common::BAD_INPUT;  // file opening failed.
    if (!_outfile->good()) return common::BAD_INPUT;
      // there is no log file currently.
  
    // dump the string out.
    if (to_show.length())
      _outfile->write((abyte *)to_show.observe(), to_show.length());
//hmmm: need eol feature again.
//    if (eol() != NO_ENDING) {
//      astring end = get_ending();
astring end = parser_bits::platform_eol_to_chars();
      _outfile->write((abyte *)end.s(), end.length());
//    }
    current_size = _outfile->tell();
    flush();
  }

  // check if we need to truncate yet.
  if (current_size > _file_limit) truncate(_file_limit - size_reduction());
  return common::OKAY;
}

outcome file_logger::log_bytes(const byte_array &to_log, int filter)
{
  if (!_file_limit) return common::OKAY;

  size_t current_size = 0;
  {
    auto_synchronizer l(*_flock);
    if (!member(filter)) return common::OKAY;
    if (!_outfile) open_file();
    if (!_outfile) return common::BAD_INPUT;  // file opening failed.
    if (!_outfile->good()) return common::BAD_INPUT;
      // there is no log file currently.
  
    // dump the contents out.
    if (to_log.length())
      _outfile->write(to_log.observe(), to_log.length());
    current_size = _outfile->tell();
    flush();
  }

  // check if we need to truncate yet.
  if (current_size > _file_limit)
    truncate(_file_limit - size_reduction());
  return common::OKAY;
}

outcome file_logger::format_bytes(const byte_array &to_log, int filter)
{
  if (!_file_limit) return common::OKAY;

  {
    auto_synchronizer l(*_flock);
    if (!member(filter)) return common::OKAY;
    if (!_outfile) open_file();
    if (!_outfile) return common::BAD_INPUT;  // file opening failed.
    if (!_outfile->good()) return common::BAD_INPUT;
      // there is no log file currently.
  }

  // dump the contents out.
  if (to_log.length()) {
    astring dumped_form;
    byte_formatter::text_dump(dumped_form, to_log);
    log(dumped_form);
  }

  // check if we need to truncate yet.
//  int current_size = _outfile->tell();
//  flush();
//  if (current_size > _file_limit) truncate(_file_limit - size_reduction());

  return common::OKAY;
}

//hmmm: should move the truncation functionality into a function on
//      the file object.

void file_logger::truncate(size_t new_size)
{
  auto_synchronizer l(*_flock);
  if (!_outfile) open_file();
  if (!_outfile) return;  // file opening failed.
  if (!_outfile->good()) return;
    // there is no log file currently.

  size_t current_size = 0;

///  // our synchronization scheme allows us to use this inter-application
///  // lock; the logger's own lock is always acquired first.  no one else can
///  // grab the "file_lock", so no deadlocks.
///
///  rendezvous file_lock(*_filename + "_trunclock");
///  if (!file_lock.healthy()) {
///    critical_events::write_to_critical_events((astring("could not create "
///       "lock for ") + *_filename).s());
///    return;
///  }
///  // waiting forever until the file lock succeeds.  as long as there are
///  // no deadlocks permitted, then this shouldn't be too dangerous...
///  bool got_lock = file_lock.lock(rendezvous::ENDLESS_WAIT);
///  if (!got_lock) {
///    critical_events::write_to_critical_events((astring("could not acquire "
///        "lock for ") + *_filename).s());
///    return;
///  }

  // make sure we weren't second in line to clean the file.  if someone else
  // already did this, we don't need to do it again.
  _outfile->seek(0, byte_filer::FROM_END);
  current_size = _outfile->tell();
  if (current_size <= new_size) {
    // release the lock and leave since we don't need to change the file.
///    file_lock.unlock();
    return;
  }
  // create a bogus temporary name.
  astring new_file(astring::SPRINTF, "%s.tmp.%d", name().s(),
      static_chaos());

  // unlink the temp file, if it exists.
  unlink(new_file.s());

  // grab the current size before we close our file.
  current_size = _outfile->tell();

  // redo the main stream for reading also.
  WHACK(_outfile);
  _outfile = new byte_filer(*_filename, "rb");

  // open the temp file as blank for writing.
  byte_filer *hold_stream = new byte_filer(new_file, "w+b");

  int start_of_keep = int(current_size - new_size);

  // position the old file where it will be about the right size.
  _outfile->seek(start_of_keep, byte_filer::FROM_START);

  astring buff(' ', MAXIMUM_BUFFER_SIZE + 1);

  // we only read as long as the file end isn't hit and we're not past the
  // original end of the file.  if the file got bigger during the truncation,
  // that's definitely not our doing and should not be coddled.  we've seen
  // a situation where we never thought we'd hit the end of the file yet before
  // adding this size check.
  size_t bytes_written = 0;  // how many bytes have gone out already.
//hmmm: loop could be extracted to some kind of dump from file one into file
//      two operation that starts at a particular address and has a particular
//      size or range.
  while (!_outfile->eof() && (bytes_written <= new_size)) {
    // grab a line from the input file.
    buff[0] = '\0';  // set it to be an empty string.
    int bytes_read = _outfile->read((abyte *)buff.s(), MAXIMUM_BUFFER_SIZE);
    if (!bytes_read)
      break;
    bytes_written += bytes_read;
    // write the line and a CR to the output file.
    if (!_outfile->eof() || bytes_read)
      hold_stream->write((abyte *)buff.s(), bytes_read);
  }
  WHACK(_outfile);
  _outfile = new byte_filer(*_filename, "w+b");

  // we think the new stream is ready for writing.
  size_t hold_size = hold_stream->tell();
    // get the current length of the clipped chunk.
  bytes_written = 0;  // reset our counter.

  // jump back to the beginning of the temp file.
  hold_stream->seek(0, byte_filer::FROM_START);
  while (!hold_stream->eof() && (bytes_written <= hold_size) ) {
    // scoot all the old data back into our file.
    int bytes_read = hold_stream->read((abyte *)buff.s(), MAXIMUM_BUFFER_SIZE);
    if (!bytes_read)
      break;
        // something funky happened; we shouldn't be at the end of the file yet.
    bytes_written += bytes_read;
    if (!hold_stream->eof() || bytes_read)
      _outfile->write((abyte *)buff.s(), bytes_read);
  }
  WHACK(hold_stream);
  unlink(new_file.s());  // trash the temp file.
///  file_lock.unlock();  // repeal the process-wide lock.
  name(*_filename);  // re-open the regular file with append semantics.
}

} //namespace.

