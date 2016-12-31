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

#include "byte_filer.h"

#include <application/windoze_helper.h>
#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/utf_conversion.h>

#include <stdio.h>
#include <string.h>
#ifdef __UNIX__
  #include <unistd.h>
#endif
#ifdef __WIN32__
  #include <io.h>
#endif

#define DEBUG_BYTE_FILER
  // uncomment for noisy version of class.

using namespace basis;

namespace filesystem {

const size_t BTFL_FILE_TELL_LIMIT = size_t(2) * size_t(GIGABYTE);
  // the largest a long integer can represent in the tell system call.

class file_hider
{
public:
  FILE *fp;  // the real file pointer.

  file_hider() : fp(NULL_POINTER) {}
};

//////////////

byte_filer::byte_filer()
: _handle(new file_hider),
  _filename(new filename),
  _auto_close(true)
{}

byte_filer::byte_filer(const astring &fname, const astring &perms)
: _handle(new file_hider),
  _filename(new filename),
  _auto_close(true)
{ open(fname, perms); }

byte_filer::byte_filer(const char *fname, const char *perms)
: _handle(new file_hider),
  _filename(new filename),
  _auto_close(true)
{ open(fname, perms); }

byte_filer::byte_filer(bool auto_close, void *handle)
: _handle(new file_hider),
  _filename(new filename),
  _auto_close(auto_close)
{
  if (handle) {
    _handle->fp = (FILE *)handle;
  }
}

byte_filer::~byte_filer() { close(); WHACK(_handle); WHACK(_filename); }

const astring &byte_filer::name() const { return _filename->raw(); }

size_t byte_filer::file_size_limit() { return BTFL_FILE_TELL_LIMIT; }

bool byte_filer::open(const astring &fname, const astring &perms)
{
  close();
  _auto_close = true;  // reset since we know we're opening this.
  _filename->reset(fname);
  _handle->fp = _filename->raw().t()? fopen(_filename->raw().s(), perms.s()) : NULL_POINTER;
  if (_handle->fp == NULL_POINTER) return false;
  return good();
}

void byte_filer::close()
{
  _filename->reset("");
  if (_auto_close && _handle->fp) fclose(_handle->fp);
  _handle->fp = NULL_POINTER;
}

bool byte_filer::good() { return !!_handle->fp; }

size_t byte_filer::tell()
{
  if (!_handle->fp) return 0;
  long to_return = ::ftell(_handle->fp);
  if (to_return == -1) {
    // if we couldn't get the size, either the file isn't there or the size
    // is too big for our OS to report.
///printf(a_sprintf("failed to tell size, calling it %.0f, and one plus that is %.0f\n", double(BTFL_FILE_TELL_LIMIT), double(long(long(BTFL_FILE_TELL_LIMIT) + 1))).s());
    if (good()) return BTFL_FILE_TELL_LIMIT;
    else return 0;
  }
  return size_t(to_return);
}

void *byte_filer::file_handle() { return _handle->fp; }

bool byte_filer::eof() { return !_handle->fp ? true : !!feof(_handle->fp); }

int byte_filer::read(abyte *buff, int size)
{ return !_handle->fp ? 0 : int(::fread((char *)buff, 1, size, _handle->fp)); }

int byte_filer::write(const abyte *buff, int size)
{ return !_handle->fp ? 0 : int(::fwrite((char *)buff, 1, size, _handle->fp)); }

int byte_filer::read(byte_array &buff, int desired_size)
{
  buff.reset(desired_size);
  int to_return = read(buff.access(), desired_size); 
  buff.zap(to_return, buff.length() - 1);
  return to_return;
}

int byte_filer::write(const byte_array &buff)
{ return write(buff.observe(), buff.length()); }

size_t byte_filer::length()
{
  size_t current_posn = tell();
  seek(0, FROM_END);  // jump to end of file.
  size_t file_size = tell();  // get position.
  seek(int(current_posn), FROM_START);  // jump back to previous place.
  return file_size;
}

int byte_filer::read(astring &s, int desired_size)
{
  s.pad(desired_size + 2);
  int found = read((abyte *)s.observe(), desired_size);
  if (non_negative(found)) s[found] = '\0';
  s.shrink();
  return found;
}

int byte_filer::write(const astring &s, bool add_null)
{
  int len = s.length();
  if (add_null) len++;
  return write((abyte *)s.observe(), len);
}

void byte_filer::flush()
{
  if (!_handle->fp) return;
  ::fflush(_handle->fp);
}

bool byte_filer::truncate()
{
  flush();
  int fnum = fileno(_handle->fp);
#ifdef __WIN32__
  return SetEndOfFile((HANDLE)_get_osfhandle(fnum));
#else
  size_t posn = tell();
  // if we're at the highest point we can be, we no longer trust our
  // ability to truncate properly.
  if (posn >= file_size_limit())
    return false;
  return !ftruncate(fnum, posn);
#endif
}

bool byte_filer::seek(int where, origins origin)
{
  if (!_handle->fp) return false;
  int real_origin;
  switch (origin) {
    case FROM_START: real_origin = SEEK_SET; break;
    case FROM_END: real_origin = SEEK_END; break;
    case FROM_CURRENT: real_origin = SEEK_CUR; break;
    default: return false;  // not a valid choice.
  }
  int ret = ::fseek(_handle->fp, where, real_origin);
  return !ret;
}

int byte_filer::getline(abyte *buff, int desired_size)
{
  if (!_handle->fp) return 0;
  char *ret = ::fgets((char *)buff, desired_size, _handle->fp);
  return !ret? 0 : int(strlen((char *)buff)) + 1;
}

int byte_filer::getline(byte_array &buff, int desired_size)
{
  buff.reset(desired_size + 1);
  return getline(buff.access(), desired_size);
}

int byte_filer::getline(astring &buff, int desired_size)
{
  buff.pad(desired_size + 1);
  int to_return = getline((abyte *)buff.access(), desired_size);
  if (non_negative(to_return)) buff[to_return] = '\0';
  buff.shrink();
  return to_return;
}

} //namespace.


