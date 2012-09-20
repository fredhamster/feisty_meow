/*****************************************************************************\
*                                                                             *
*  Name   : huge_file                                                         *
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

#include "byte_filer.h"
#include "huge_file.h"

#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/guards.h>

#include <stdio.h>

#undef LOG
#define LOG(to_print) printf("%s::%s: %s\n", static_class_name(), func, astring(to_print).s())

//#define DEBUG_HUGE_FILE
  // uncomment for noisy version.

using namespace basis;

namespace filesystem {

huge_file::huge_file(const astring &filename, const astring &permissions)
: _real_file(new byte_filer(filename, permissions)),
  _file_pointer(0)
{
}

huge_file::~huge_file()
{
  WHACK(_real_file);
}

void huge_file::flush() { _real_file->flush(); }

bool huge_file::truncate() { return _real_file->truncate(); }

double huge_file::length()
{
  FUNCDEF("length");

//trying to read to see if we're past endpoint.
//  if this approach works, length may want to close and reopen file for
//  reading, since we can't add any bytes to it for writing just to find
//  the length out.


  double save_posn = _file_pointer;
  // skip to the beginning of the file so we can try to find the end.
  _file_pointer = 0;
  _real_file->seek(0, byte_filer::FROM_START);
  size_t naive_size = _real_file->length();
  if (naive_size < _real_file->file_size_limit()) {
    // lucked out; we are within normal file size limitations.
    seek(save_posn, byte_filer::FROM_START);
    return double(naive_size);
  }
  
  double best_highest = 0.0;  // the maximum we've safely seeked to.

  size_t big_jump = byte_filer::file_size_limit();
    // try with the largest possible seek at first.

  while (true) {
#ifdef DEBUG_HUGE_FILE
    LOG(a_sprintf("best highest=%.0f", best_highest));
#endif
    // iterate until we reach our exit condition, which seems like it must
    // always occur eventually unless the file is being monkeyed with.
    bool seek_ret = _real_file->seek(int(big_jump), byte_filer::FROM_CURRENT);
#ifdef DEBUG_HUGE_FILE
    LOG(a_sprintf("  seek ret=%d", int(seek_ret)));
#endif
    byte_array temp_bytes;
    int bytes_read = _real_file->read(temp_bytes, 1);
    if (bytes_read < 1)
      seek_ret = false;
#ifdef DEBUG_HUGE_FILE
    LOG(a_sprintf("  read %d bytes", bytes_read));
#endif
    bool at_eof = _real_file->eof();
#ifdef DEBUG_HUGE_FILE
    LOG(a_sprintf("  at_eof=%d", int(at_eof)));
#endif
    if (seek_ret && !at_eof) {
#ifdef DEBUG_HUGE_FILE
      LOG("seek worked, incrementing best highest and trying same jump again");
#endif
      // the seek worked, so we'll just jump forward again.
      best_highest += double(big_jump);
      _file_pointer += double(big_jump);
      continue;
    } else if (seek_ret && at_eof) {
#ifdef DEBUG_HUGE_FILE
      LOG("seek worked but found eof exactly.");
#endif
      // the seek did worked, but apparently we've also found the end point.
      best_highest += double(big_jump);
      _file_pointer += double(big_jump);
      break;
    } else {
      // that seek was too large, so we need to back down and try a smaller
      // seek size.
#ifdef DEBUG_HUGE_FILE
      LOG("seek failed, going back to best highest and trying same jump again");
#endif
      _file_pointer = 0;
      _real_file->seek(0, byte_filer::FROM_START); 
      outcome worked = seek(best_highest, byte_filer::FROM_START);
        // this uses our version to position at large sizes.
      if (worked != OKAY) {
        // this is a bad failure; it says that the file size changed or
        // something malfunctioned.  we should always be able to get back to
        // the last good size we found if the file is static.
        LOG(a_sprintf("failed to seek back to best highest %.0f on ",
            best_highest) + _real_file->filename());
        // try to repair our ideas about the file by starting the process
        // over.
//hmmm: count the number of times restarted and bail after N.
        seek_ret = _real_file->seek(0, byte_filer::FROM_START);
        _file_pointer = 0;
        if (!seek_ret) {
          // the heck with this.  we can't even go back to the start.  this
          // file seems to be screwed up now.
          LOG(astring("failed to seek back to start of file!  on ")
              + _real_file->filename());
          return 0;
        }
        // reset the rest of the positions for our failed attempt to return
        // to what we already thought was good.
        _file_pointer = 0;
        big_jump = byte_filer::file_size_limit();
        best_highest = 0;
        continue;
      }
      // okay, nothing bad happened when we went back to our last good point.
      if (big_jump <= 0) {
        // success in finding the smallest place that we can't seek between.
#ifdef DEBUG_HUGE_FILE
        LOG("got down to smallest big jump, 0!");
#endif
        break;
      }
      // formula expects that the maximum file size is a power of 2.
      big_jump /= 2;
#ifdef DEBUG_HUGE_FILE
      LOG(a_sprintf("restraining big jump down to %u.", big_jump));
#endif
      continue;
    }
  }

  // go back to where we started out.
  seek(0, byte_filer::FROM_START);
  seek(save_posn, byte_filer::FROM_CURRENT);
#ifdef DEBUG_HUGE_FILE
  LOG(a_sprintf("saying file len is %.0f.", best_highest + 1.0));
#endif
  return best_highest + 1.0;
}

bool huge_file::good() const { return _real_file->good(); }

bool huge_file::eof() const { return _real_file->eof(); }

outcome huge_file::move_to(double absolute_posn)
{
#ifdef DEBUG_HUGE_FILE
  FUNCDEF("move_to");
#endif
  double difference = absolute_posn - _file_pointer;
    // calculate the size we want to offset.
#ifdef DEBUG_HUGE_FILE
  LOG(a_sprintf("abs_pos=%.0f difference=%.0f old_filepoint=%.0f",
      absolute_posn, difference, _file_pointer));
#endif
  // if we're at the same place, we don't have to do anything.
  if (difference < 0.000001) {
#ifdef DEBUG_HUGE_FILE
    LOG("difference was minimal, saying we're done.");
#endif
    return OKAY;
  }
  while (absolute_value(difference) > 0.000001) {
    double seek_size = minimum(double(byte_filer::file_size_limit() - 1),
        absolute_value(difference));
    if (difference < 0)
      seek_size *= -1.0;  // flip sign of seek.
#ifdef DEBUG_HUGE_FILE
    LOG(a_sprintf("  seeksize=%d", int(seek_size)));
#endif
    bool seek_ret = _real_file->seek(int(seek_size),
        byte_filer::FROM_CURRENT);
    if (!seek_ret) {
#ifdef DEBUG_HUGE_FILE
      LOG(a_sprintf("failed to seek %d from current", int(seek_size)));
#endif
      return FAILURE;  // seek failed somehow.
    }
    _file_pointer += seek_size;
#ifdef DEBUG_HUGE_FILE
    LOG(a_sprintf("  now_filepoint=%.0f", _file_pointer));
#endif
    difference = absolute_posn - _file_pointer;
#ifdef DEBUG_HUGE_FILE
    LOG(a_sprintf("  now_difference=%.0f", difference));
#endif
  }
  return OKAY;
}

outcome huge_file::seek(double new_position, byte_filer::origins origin)
{
#ifdef DEBUG_HUGE_FILE
  FUNCDEF("seek");
#endif
  if (origin == byte_filer::FROM_CURRENT) {
    return move_to(_file_pointer + new_position);
  } else if (origin == byte_filer::FROM_START) {
    _file_pointer = 0;
    if (!_real_file->seek(0, byte_filer::FROM_START))
      return FAILURE;
    return move_to(new_position);
  } else if (origin == byte_filer::FROM_END) {
#ifdef DEBUG_HUGE_FILE
    LOG("into precarious FROM_END case.");
#endif
    double file_len = length();  // could take a scary long time possibly.
#ifdef DEBUG_HUGE_FILE
    LOG(a_sprintf("  FROM_END got len %.0f.", file_len));
#endif
    _file_pointer = file_len;
      // it's safe, although not efficient, for us to call the length()
      // method here.  our current version of length() uses the byte_filer's
      // seek method directly and only FROM_CURRENT and FROM_START from this
      // class's seek method.
    _real_file->seek(0, byte_filer::FROM_END);
    return move_to(_file_pointer - new_position);
  }
  // unknown origin.
  return BAD_INPUT;
}

outcome huge_file::read(byte_array &to_fill, int desired_size, int &size_read)
{
  FUNCDEF("read");
  size_read = 0;
  int ret = _real_file->read(to_fill, desired_size);
  if (ret < 0)
    return FAILURE;  // couldn't read the bytes.
  _file_pointer += double(size_read);
  size_read = ret;
  return OKAY; 
}

outcome huge_file::write(const byte_array &to_write, int &size_written)
{
  FUNCDEF("write");
  size_written = 0;
  int ret = _real_file->write(to_write);
  if (ret < 0)
    return FAILURE;  // couldn't write the bytes.
  _file_pointer += double(size_written);
  size_written = ret;
  return OKAY;
}

} //namespace.

