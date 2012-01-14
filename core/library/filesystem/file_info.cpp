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

#include "file_info.h"
#include "huge_file.h"

#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/contracts.h>
#include <basis/functions.h>
#include <structures/checksums.h>
#include <structures/object_packers.h>

#include <stdio.h>

#define DEBUG_FILE_INFO
  // uncomment for noisy version.

#undef LOG
#define LOG(to_print) printf("%s::%s: %s\n", static_class_name(), func, astring(to_print).s())

using namespace basis;
using namespace structures;

namespace filesystem {

file_info::file_info()
: filename(astring::empty_string()),
  _file_size(0),
  _time(),
  _checksum(),
  c_secondary(),
  c_attachment()
{}

file_info::file_info(const filename &to_copy, double file_size,
    const file_time &time, int checksum)
: filename(to_copy),
  _file_size(file_size),
  _time(time),
  _checksum(checksum),
  c_secondary(),
  c_attachment()
{}

file_info::file_info(const file_info &to_copy)
: filename(to_copy),
  _file_size(to_copy._file_size),
  _time(to_copy._time),
  _checksum(to_copy._checksum),
  c_secondary(to_copy.c_secondary),
  c_attachment(to_copy.c_attachment)
{
}

file_info::~file_info() {}

const byte_array &file_info::attachment() const { return c_attachment; }

void file_info::attachment(const byte_array &new_attachment)
{ c_attachment = new_attachment; }

const astring &file_info::secondary() const { return c_secondary; }

void file_info::secondary(const astring &new_sec) { c_secondary = new_sec; }

astring file_info::text_form() const
{
  astring to_return = raw()
      + a_sprintf(", size=%0.f, chksum=%d", _file_size, _checksum);
  if (c_secondary.t())
    to_return += astring(", 2ndary=") + c_secondary;
  return to_return;
}

bool file_info::calculate(const astring &prefix, bool just_size, int checksum_edge)
{
  FUNCDEF("calculate");
  filename full;
  if (prefix.t()) full = prefix + "/" + *this;
  else full = *this;
  if (!full.exists()) {
#ifdef DEBUG_FILE_INFO
    LOG(astring("failed to find file: ") + full.raw());
#endif
    return false;
  }
  // get time again.
  _time = file_time(full);

//#ifdef DEBUG_FILE_INFO
//  astring temptext;
//  _time.text_form(temptext);
//  LOG(astring("file calculate on ") + full.raw() + " time=" + temptext);
//#endif

  // open the file for reading.
  huge_file to_read(full.raw(), "rb");
  if (!to_read.good()) {
#ifdef DEBUG_FILE_INFO
    LOG(astring("file has non-good status: ") + full.raw());
#endif
    return false;  // why did that happen?
  }
  // set the size appropriately.
  _file_size = to_read.length();
  if (just_size)
    return true;  // done for that case.

  // now read the file and compute a checksum.
  uint16 curr_sum = 0;  // the current checksum being computed.
  byte_array chunk;  // temporary chunk of data from file.

//hmmm: make this optimization (hack) optional!

  // this algorithm takes a chunk on each end of the file for checksums.
  // this saves us from reading a huge amount of data, although it will be
  // fooled if a huge binary file is changed only in the middle and has the
  // same size as before.  for most purposes, this is not a problem, although
  // databases that are fixed size might fool us.  if records are written in
  // the middle without updating the head or tail sections, then we're hosed.

  bool skip_tail = false;  // true if we don't need the tail piece.
  double head_start = 0, head_end = 0, tail_start = 0,
      tail_end = _file_size - 1;
  if (_file_size <= double(2 * checksum_edge)) {
    // we're applying a rule for when the file is too small compared to
    // the chunk factor doubled; we'll just read the whole file.
    head_end = _file_size - 1;
    skip_tail = true;
  } else {
    // here we compute the ending of the head piece and the beginning of
    // the tail piece.  each will be about checksum_edge in size.
    head_end = minimum(_file_size / 2, double(checksum_edge)) - 1;
    tail_start = _file_size - minimum(_file_size / 2, double(checksum_edge));
  }

  // read the head end of the file.
  int size_read = 0;
  outcome ret = to_read.read(chunk, int(head_end - head_start + 1), size_read);
  if (ret != huge_file::OKAY) {
#ifdef DEBUG_FILE_INFO
    LOG(astring("reading file failed: ") + full.raw());
#endif
    return false;  // failed to read.
  }
  curr_sum = checksums::rolling_fletcher_checksum(curr_sum, chunk.observe(),
      chunk.length());

  // read the tail end of the file.
  if (!skip_tail) {
    to_read.seek(tail_start, byte_filer::FROM_START);
    ret = to_read.read(chunk, int(tail_end - tail_start + 1), size_read);
    if (ret != huge_file::OKAY) {
#ifdef DEBUG_FILE_INFO
      LOG(astring("reading tail of file failed: ") + full.raw());
#endif
      return false;  // failed to read.
    }
    curr_sum = checksums::rolling_fletcher_checksum(curr_sum, chunk.observe(),
        chunk.length());
  }

  _checksum = curr_sum;
  return true;
}

int file_info::packed_size() const
{
  return filename::packed_size()
      + structures::packed_size(_file_size)
      + _time.packed_size()
      + PACKED_SIZE_INT32
      + c_secondary.packed_size()
      + structures::packed_size(c_attachment);
}

void file_info::pack(byte_array &packed_form) const
{
  filename::pack(packed_form);
  attach(packed_form, _file_size);
  _time.pack(packed_form);
  attach(packed_form, _checksum);
  c_secondary.pack(packed_form);
  attach(packed_form, c_attachment);
}

bool file_info::unpack(byte_array &packed_form)
{
  if (!filename::unpack(packed_form))
    return false;
  if (!detach(packed_form, _file_size))
    return false;
  if (!_time.unpack(packed_form))
    return false;
  if (!detach(packed_form, _checksum))
    return false;
  if (!c_secondary.unpack(packed_form))
    return false;
  if (!detach(packed_form, c_attachment))
    return false;
  return true;
}

file_info &file_info::operator = (const file_info &to_copy)
{
  if (this == &to_copy)
    return *this;
  (filename &)(*this) = (filename &)to_copy;
  c_attachment = to_copy.c_attachment;
  _time = to_copy._time;
  _file_size = to_copy._file_size;
  c_secondary = to_copy.c_secondary;
  _checksum = to_copy._checksum;
  return *this;
}

} //namespace.

