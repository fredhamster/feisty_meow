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

#include "directory.h"
#include "filename.h"
#include "filename_list.h"
#include "heavy_file_ops.h"
#include "huge_file.h"

#include <basis/functions.h>
#include <basis/guards.h>
#include <structures/object_packers.h>

using namespace basis;
using namespace structures;

namespace filesystem {

//#define DEBUG_HEAVY_FILE_OPS
  // uncomment for noisier debugging.

#undef LOG
#include <stdio.h>
#define LOG(to_print) printf("%s::%s: %s\n", static_class_name(), func, astring(to_print).s())

//////////////

// the smallest we let the packing area's available space get before we stop filling it.
const int MINIMUM_ARRAY_SIZE = 1024;

//////////////

file_transfer_header::file_transfer_header(const file_time &time_stamp)
: _filename(),
  _byte_start(0),
  _length(0),
  _time(time_stamp)
{
}

astring file_transfer_header::text_form() const
{
  astring time_text;
  _time.text_form(time_text);
  return astring("file=") + _filename
      + a_sprintf(" start=%d len=%d stamp=", _byte_start, _length)
      + time_text;
}

astring file_transfer_header::readable_text_form() const
{
  astring time_text;
  _time.readable_text_form(time_text);
  return _filename
      + a_sprintf(" [%d bytes, mod ", _length)
      + time_text + "]";
}

void file_transfer_header::pack(byte_array &packed_form) const
{
  _filename.pack(packed_form);
  attach(packed_form, _byte_start);
  attach(packed_form, _length);
  _time.pack(packed_form);
}

bool file_transfer_header::unpack(byte_array &packed_form)
{
  if (!_filename.unpack(packed_form)) return false;
  if (!detach(packed_form, _byte_start)) return false;
  if (!detach(packed_form, _length)) return false;
  if (!_time.unpack(packed_form)) return false;
  return true;
}

int file_transfer_header::packed_size() const
{
byte_array temp;
attach(temp, _byte_start);
//hmmm: really ugly above; we should get a more exact way to know the size of
//      packed doubles.
  return _filename.length() + 1
      + temp.length()
      + sizeof(int)
      + _time.packed_size();
}

//////////////

const size_t heavy_file_operations::COPY_CHUNK_FACTOR = 1 * MEGABYTE;

size_t heavy_file_operations::copy_chunk_factor()
{ return COPY_CHUNK_FACTOR; }

heavy_file_operations::~heavy_file_operations() {}
  // we only need this due to our use of the root_object class_name support.

const char *heavy_file_operations::outcome_name(const outcome &to_name)
{
  switch (to_name.value()) {
    case SOURCE_MISSING: return "SOURCE_MISSING";
    case TARGET_ACCESS_ERROR: return "TARGET_ACCESS_ERROR";
    case TARGET_DIR_ERROR: return "TARGET_DIR_ERROR";
    default: return common::outcome_name(to_name);
  }
}

outcome heavy_file_operations::copy_file(const astring &source,
    const astring &destination, int copy_chunk_factor)
{
#ifdef DEBUG_HEAVY_FILE_OPS
  FUNCDEF("copy_file");
#endif
  // check that the source exists...
  filename source_path(source);
  if (!source_path.exists()) return SOURCE_MISSING;
  file_time source_time(source_path);  // get the time on the source.

  // make sure the target directory exists...
  filename target_path(destination);
  filename targ_dir = target_path.dirname();
  if (!directory::recursive_create(targ_dir.raw())) return TARGET_DIR_ERROR;

  // open the source for reading.
  huge_file source_file(source, "rb");
  if (!source_file.good()) return SOURCE_MISSING;
//hmmm: could be source is not accessible instead.

  // open target file for writing.
  huge_file target_file(destination, "wb");
  if (!target_file.good()) return TARGET_ACCESS_ERROR;

  byte_array chunk;
  int bytes_read = 0;
  outcome ret;
  while ( (ret = source_file.read(chunk, copy_chunk_factor, bytes_read))
      == huge_file::OKAY) {
    int bytes_stored;
    ret = target_file.write(chunk, bytes_stored);
    if (bytes_stored != bytes_read) return TARGET_ACCESS_ERROR;
    if (source_file.eof()) break;  // time to escape.
  }

  // set the time on the target file from the source's time.
  source_time.set_time(target_path);

#ifdef DEBUG_HEAVY_FILE_OPS
  astring time;
  source_time.text_form(time);
  LOG(astring("setting file time for ") + source + " to " + time);
#endif

  return OKAY;
}

outcome heavy_file_operations::write_file_chunk(const astring &target,
    double byte_start, const byte_array &chunk, bool truncate,
    int copy_chunk_factor)
{
  FUNCDEF("write_file_chunk");
  if (byte_start < 0) return BAD_INPUT;

  filename targ_name(target);
  astring targ_dir = targ_name.dirname().raw();
#ifdef DEBUG_HEAVY_FILE_OPS
  LOG(astring("creating target's directory: ") + targ_name.dirname().raw());
#endif
  if (!directory::recursive_create(targ_dir)) {
    LOG(astring("failed to create directory: ") + targ_name.dirname().raw());
    return TARGET_DIR_ERROR;
  }

  if (!targ_name.exists()) {
    huge_file target_file(target, "w");
  }

  huge_file target_file(target, "r+b");
    // open the file for updating (either read or write).
  if (!target_file.good()) return TARGET_ACCESS_ERROR;
  double curr_len = target_file.length();

  if (curr_len < byte_start) {
    byte_array new_chunk;
    while (curr_len < byte_start) {
      target_file.seek(0, byte_filer::FROM_END);  // go to the end of the file.
      new_chunk.reset(minimum(copy_chunk_factor,
          int(curr_len - byte_start + 1)));
      int written;
      outcome ret = target_file.write(new_chunk, written);
      if (written < new_chunk.length()) return TARGET_ACCESS_ERROR;
      curr_len = target_file.length();
    }
  }
  target_file.seek(byte_start, byte_filer::FROM_START);
    // jump to the proper location in the file.
  int wrote;
  outcome ret = target_file.write(chunk, wrote);
  if (wrote != chunk.length()) return TARGET_ACCESS_ERROR;
  if (truncate) {
    target_file.truncate();
  }
  return OKAY;
}

basis::outcome heavy_file_operations::advance(const filename_list &to_transfer,
    file_transfer_header &last_action)
{
  FUNCDEF("advance");
  int indy = to_transfer.locate(last_action._filename);
  if (negative(indy)) return BAD_INPUT;  // error, file not found in list.
  if (indy >= to_transfer.elements() - 1) return FINISHED;  // done.
  const file_info *currfile = to_transfer.get(indy + 1);
  last_action._filename = currfile->raw();
  last_action._time = currfile->_time;

#ifdef DEBUG_HEAVY_FILE_OPS
  if (currfile->_time == file_time(time_t(0)))
    LOG(astring("failed for ") + currfile->raw() + " -- has zero file time");
#endif

  last_action._byte_start = 0;
  last_action._length = 0;
  return OKAY;
}

outcome heavy_file_operations::buffer_files(const astring &source_root,
    const filename_list &to_transfer, file_transfer_header &last_action,
    byte_array &storage, int maximum_bytes)
{
  FUNCDEF("buffer_files");
  storage.reset();  // clear out the current contents.

  if (!to_transfer.elements()) {
    // we seem to be done.
    return FINISHED;
  }

  outcome to_return = OKAY;

  // start filling the array with bytes from the files.
  while (storage.length() < maximum_bytes) {
    double remaining_in_array = maximum_bytes - storage.length()
        - last_action.packed_size();
    if (remaining_in_array < MINIMUM_ARRAY_SIZE) {
      // ensure that we at least have a reasonable amount of space left
      // for storing into the array.
      break;
    }

    // find the current file we're at, as provided in record.
    if (!last_action._filename) {
      // no filename yet.  assume this is the first thing we've done.
      const file_info *currfile = to_transfer.get(0);
      last_action._filename = currfile->raw();
      last_action._time = currfile->_time;
      last_action._byte_start = 0;
      last_action._length = 0;
    }

    const file_info *found = to_transfer.find(last_action._filename);
    if (!found) {
      // they have referenced a file that we don't have.  that's bad news.
      return BAD_INPUT;
    }

    astring full_file = source_root + "/" + last_action._filename;
    huge_file current(full_file, "rb");
    if (!current.good()) {
      // we need to skip this file.
      LOG(astring("skipping bad file: ") + full_file);
      to_return = advance(to_transfer, last_action);
      if (to_return != OKAY) break;
      continue;
    }

    if (last_action._byte_start + last_action._length >= current.length()) {
      // this file is done now.  go to the next one.
#ifdef DEBUG_HEAVY_FILE_OPS
      LOG(astring("finished stuffing file: ") + full_file);
#endif
      to_return = advance(to_transfer, last_action);
      if (to_return != OKAY) break;
      continue;
    }

    // calculate the largest piece remaining of that file that will fit in the
    // allotted space.
    double new_start = last_action._byte_start + last_action._length;
    double remaining_in_file = current.length() - new_start;
    if (remaining_in_file < 0) remaining_in_file = 0;
    double new_len = minimum(remaining_in_file, remaining_in_array);
    
    // pack this new piece of the file.
    current.seek(new_start, byte_filer::FROM_START);
    byte_array new_chunk;
    int bytes_read = 0;
    outcome ret = current.read(new_chunk, int(new_len), bytes_read);
    if (bytes_read != new_len) {
      if (!bytes_read) {
        // some kind of problem reading the file.
        to_return = advance(to_transfer, last_action);
        if (to_return != OKAY) break;
        continue;
      }
//why would this happen?  just complain, i guess.
    }

    // update the record since it seems we're successful here.
    last_action._byte_start = new_start;
    last_action._length = int(new_len);

    // add in this next new chunk of file.
    last_action.pack(storage);  // add the header.
    storage += new_chunk;  // add the new stuff.

    if (!current.length()) {
      // ensure we don't get stuck redoing zero length files, which we allowed
      // to go past their end above (since otherwise we'd never see them).
      to_return = advance(to_transfer, last_action);
      if (to_return != OKAY) break;
      continue;
    }
    
    // just keep going, if there's space...
  }

  return to_return;
}

} //namespace.

