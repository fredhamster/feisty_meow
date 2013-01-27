/*****************************************************************************\
*                                                                             *
*  Name   : directory                                                         *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2001-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "directory.h"
#include "filename.h"

#include <algorithms/shell_sort.h>
#include <application/windoze_helper.h>
#include <basis/astring.h>
#include <basis/contracts.h>
#include <basis/functions.h>
#include <basis/utf_conversion.h>
#include <loggers/program_wide_logger.h>
#include <structures/string_array.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#ifdef __UNIX__
  #include <dirent.h>
  #include <fnmatch.h>
  #include <string.h>
  #include <unistd.h>
#endif
#ifdef __WIN32__
  #include <direct.h>
#endif

/*
#ifdef __WIN32__
  const int MAX_ABS_PATH = 2048;
#elif defined(__APPLE__)
  const int MAX_ABS_PATH = 2048;
#else
  const int MAX_ABS_PATH = MAX_ABS_PATH;
#endif
*/

//#define DEBUG_DIRECTORY
  // uncomment for noisier runs.

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

using namespace algorithms;
using namespace basis;
using namespace loggers;
using namespace structures;

namespace filesystem {

directory::directory(const astring &path, const char *pattern)
: _scanned_okay(false),
  _path(new astring),
  _files(new string_array),
  _folders(new string_array),
  _pattern(new astring(pattern))
{ reset(path, pattern); }

directory::directory(const directory &to_copy)
: _scanned_okay(false),
  _path(new astring),
  _files(new string_array),
  _folders(new string_array),
  _pattern(new astring)
{ reset(*to_copy._path, to_copy._pattern->observe()); }

directory::~directory()
{
  _scanned_okay = false;
  WHACK(_path);
  WHACK(_files);
  WHACK(_folders);
  WHACK(_pattern);
}

const astring &directory::path() const { return *_path; }

const astring &directory::pattern() const { return *_pattern; }

directory &directory::operator =(const directory &to_copy)
{
  if (this == &to_copy) return *this;  // oops.
  _scanned_okay = false;
  reset(*to_copy._path, to_copy._pattern->observe());
  return *this;
}

astring directory::absolute_path(const astring &rel_path)
{
  char abs_path[MAX_ABS_PATH + 1];
  abs_path[0] = '\0';
#ifdef __WIN32__
  if (!_fullpath(abs_path, rel_path.s(), MAX_ABS_PATH)) return "";
  return abs_path;
#else
  if (!realpath(rel_path.s(), abs_path)) return "";
  return abs_path;
#endif
}

astring directory::current()
{
  astring to_return(".");  // failure result.
#ifdef __WIN32__
  flexichar buffer[MAX_ABS_PATH + 1] = { '\0' };
  GetCurrentDirectory(MAX_ABS_PATH, buffer);
  to_return = from_unicode_temp(buffer);
#else
  char buffer[MAX_ABS_PATH + 1] = { '\0' };
  if (realpath(".", buffer)) to_return = buffer;
#endif
  return to_return;
}

bool directory::reset(const astring &path, const char *pattern)
{ *_path = path; *_pattern = pattern; return rescan(); }

bool directory::move_up(const char *pattern)
{
  astring currdir = current();
  return reset(currdir + "/..", pattern);
}

bool directory::move_down(const astring &subdir, const char *pattern)
{
  astring currdir = current();
  return reset(currdir + "/" + subdir, pattern);
}

const string_array &directory::files() const { return *_files; }

const string_array &directory::directories() const { return *_folders; }

bool directory::rescan()
{
  FUNCDEF("rescan");
  _scanned_okay = false;
  _files->reset();
  _folders->reset();
  astring cur_dir = ".";
  astring par_dir = "..";
#ifdef __WIN32__
  // start reading the directory.
  WIN32_FIND_DATA wfd;
  astring real_path_spec = *_path + "/" + *_pattern;
  HANDLE search_handle = FindFirstFile(to_unicode_temp(real_path_spec), &wfd);
  if (search_handle == INVALID_HANDLE_VALUE) return false;  // bad path.
  do {
    // ignore the two standard directory entries.
    astring filename_transcoded(from_unicode_temp(wfd.cFileName));
    if (!strcmp(filename_transcoded.s(), cur_dir.s())) continue;
    if (!strcmp(filename_transcoded.s(), par_dir.s())) continue;

#ifdef UNICODE
  #ifdef DEBUG_DIRECTORY
    to_unicode_persist(kludgemart, filename_transcoded);
    if (memcmp((wchar_t*)kludgemart, wfd.cFileName, wcslen(wfd.cFileName)*2))
      printf("failed to compare the string before and after transcoding\n");
  #endif
#endif

//wprintf(to_unicode_temp("file is %ls\n"), (wchar_t*)to_unicode_temp(filename_transcoded));
    
    filename temp_name(*_path, filename_transcoded.s());

    // add this to the appropriate list.
    if (temp_name.is_directory()) {
      _folders->concatenate(filename_transcoded);
    } else {
      _files->concatenate(filename_transcoded);

#ifdef UNICODE
  #ifdef DEBUG_DIRECTORY
      to_unicode_persist(kludgemart2, temp_name.raw());
      FILE *fpjunk = _wfopen(kludgemart2, to_unicode_temp("rb"));
      if (!fpjunk)
        LOG(astring("failed to open the file for testing: ") + temp_name.raw() + "\n");
      if (fpjunk) fclose(fpjunk);
  #endif
#endif

	}
  } while (FindNextFile(search_handle, &wfd));
  FindClose(search_handle);
#endif
#ifdef __UNIX__
  DIR *dir = opendir(_path->s());
//hmmm: could check errno to determine what caused the problem.
  if (!dir) return false;
  dirent *entry = readdir(dir);
  while (entry) {
    char *file = entry->d_name;
    bool add_it = true;
    if (!strcmp(file, cur_dir.s())) add_it = false;
    if (!strcmp(file, par_dir.s())) add_it = false;
    // make sure that the filename matches the pattern also.
    if (add_it && !fnmatch(_pattern->s(), file, 0)) {
      filename temp_name(*_path, file);
      if (!temp_name.is_normal()) {
//#ifdef DEBUG_DIRECTORY
        LOG(astring("skipping abnormal file:  ") + temp_name);
//#endif
        entry = readdir(dir);
        continue;  // cannot be adding goofy named pipes etc; cannot manage those.
      }
      // add this to the appropriate list.
      if (temp_name.is_directory())
        _folders->concatenate(file);
      else 
        _files->concatenate(file);
    }
    entry = readdir(dir);
  }
  closedir(dir);
#endif
  shell_sort(_files->access(), _files->length());
  shell_sort(_folders->access(), _folders->length());

  _scanned_okay = true;
  return true;
}

bool directory::make_directory(const astring &path)
{
#ifdef __UNIX__
  int mk_ret = mkdir(path.s(), 0777);
#endif
#ifdef __WIN32__
  int mk_ret = mkdir(path.s());
#endif
  return !mk_ret;
}

bool directory::remove_directory(const astring &path)
{
#ifdef __UNIX__
  int rm_ret = rmdir(path.s());
#endif
#ifdef __WIN32__
  int rm_ret = rmdir(path.s());
#endif
  return !rm_ret;
}

bool directory::recursive_create(const astring &directory_name)
{
  FUNCDEF("recursive_create");
  filename dir(directory_name);
  string_array pieces;
  bool rooted;
  dir.separate(rooted, pieces);
  for (int i = 0; i < pieces.length(); i++) {
    // check each location along the way.
    string_array partial = pieces.subarray(0, i);
    filename curr;
    curr.join(rooted, partial);  // this is our current location.
    // make sure, if we see a drive letter component, that we call it
    // a proper directory name.
    if (curr.raw()[curr.raw().end()] == ':')
      curr = curr.raw() + "/";
    if (curr.exists()) {
      if (curr.is_directory()) {
        continue;  // that's good.
      }
      return false;  // if it's an existing file, we're hosed.
    }
    // the directory at this place doesn't exist yet.  let's create it.
    if (!directory::make_directory(curr.raw())) return false;
  }
  return true;
}

} // namespace.
