/*****************************************************************************\
*                                                                             *
*  Name   : rendezvous                                                        *
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

//note: after repeated investigation, it seems that if we unlink the rendezvous
//      file on destruction, then this hoses up any locks attempted ever after.
//      instead of waiting for a lock, new attempts think they can go ahead,
//      even though someone else might also have been given the lock.  it seems
//      we cannot remove the files without destroying the semantics.

#include "rendezvous.h"

#include <application/windoze_helper.h>
#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/utf_conversion.h>
#include <filesystem/filename.h>

#ifdef __UNIX__
  #include <stdio.h>
  #include <sys/stat.h>
  #include <unistd.h>
#endif

using namespace basis;
using namespace filesystem;

namespace processes {

//#define DEBUG_RENDEZVOUS
  // uncomment for a noisier file.

#undef LOG
#define LOG(tp) printf("%s%s\n", time_stamp::notarize(true).s(), astring(tp).s())
  // we can only use simple logging here since the rendezvous is relied on
  // at very low levels and use of a log_base object would cause infinite
  // loops.

// used for the name of a mutex or part of the unix lock filename.
astring general_lock_name(const astring &root_part)
{ return root_part + "_app_lock"; }

#ifdef __UNIX__
// the name of the locking file used in unix.
astring unix_rendez_file(const astring &lock_name)
{
  astring clean_name = lock_name;
  // remove troublesome characters from the name.
  filename::detooth_filename(clean_name);
  // make sure our target directory exists.

  // use a system-wide location for rendezvous state files.
  astring tmp_dir = "/tmp/rendezvous";

  mkdir(tmp_dir.observe(), 0777);
  return tmp_dir + "/ren_" + clean_name;
}
#endif

rendezvous::rendezvous(const astring &root_name)
: _handle(NULL_POINTER),
  _locked(false),
  _root_name(new astring(root_name))
{
#ifdef DEBUG_RENDEZVOUS
  FUNCDEF("constructor");
#endif
  astring lock_name = general_lock_name(root_name);
#ifdef __UNIX__
  astring real_name = unix_rendez_file(lock_name);
  FILE *locking_file = fopen(real_name.s(), "wb");
  if (!locking_file) {
#ifdef DEBUG_RENDEZVOUS
    LOG(astring("failure to create locking file ") + real_name
        + ": " + critical_events::system_error_text(critical_events::system_error()) );
#endif
    return;
  }
  // success now.
  _handle = locking_file;
#endif
#ifdef __WIN32__
  _handle = CreateMutex(NULL_POINTER, false, to_unicode_temp(lock_name));
  if (!_handle) return;
#endif
}

rendezvous::~rendezvous()
{
#ifdef DEBUG_RENDEZVOUS
  FUNCDEF("destructor");
  LOG("okay, into destructor for rendezvous.");
#endif
#ifdef __UNIX__
  if (_handle) {
    if (_locked) {
      int ret = lockf(fileno((FILE *)_handle), F_ULOCK, sizeof(int));
      if (ret) {
#ifdef DEBUG_RENDEZVOUS
        LOG("failure to get lock on file.");
#endif
      }
      _locked = false;  // clear our locked status since we no longer have one.

//note: after repeated investigation, it seems that if we unlink the rendezvous
//      file on destruction, then this hoses up any locks attempted ever after.
//      instead of waiting for a lock, new attempts think they can go ahead,
//      even though someone else might also have been given the lock.  it seems
//      we cannot remove the files without destroying the semantics.
    }

    fclose((FILE *)_handle);
    _handle = NULL_POINTER;
  }
#endif
#ifdef __WIN32__
  if (_handle) CloseHandle((HANDLE)_handle);
#endif
  WHACK(_root_name);
}

void rendezvous::establish_lock() { lock(); }

void rendezvous::repeal_lock() { unlock(); }

bool rendezvous::healthy() const
{
  return !!_handle;
}

bool rendezvous::lock(locking_methods how)
{
#ifdef DEBUG_RENDEZVOUS
  FUNCDEF("lock");
#endif
  if (how == NO_LOCKING) return false;
  if (!healthy()) return false;
#ifdef __UNIX__
  int command = F_TLOCK;
  if (how == ENDLESS_WAIT) command = F_LOCK;
  int ret = lockf(fileno((FILE *)_handle), command, sizeof(int));
  if (ret) {
#ifdef DEBUG_RENDEZVOUS
    LOG("failure to get lock on file.");
#endif
    return false;
  }
#ifdef DEBUG_RENDEZVOUS
  LOG("okay, got lock on shared mem.");
#endif
  _locked = true;
  return true;
#endif
#ifdef __WIN32__
  int timing = 0;  // immediate return.
  if (how == ENDLESS_WAIT) timing = INFINITE;
  int ret = WaitForSingleObject((HANDLE)_handle, timing);
  if ( (ret == WAIT_ABANDONED) || (ret == WAIT_TIMEOUT) ) return false;
  else if (ret != WAIT_OBJECT_0) {
#ifdef DEBUG_RENDEZVOUS
    LOG("got an unanticipated error from waiting for the mutex.");
#endif
    return false;
  }
  _locked = true;
  return true;
#endif
  return false;
}

void rendezvous::unlock()
{
#ifdef DEBUG_RENDEZVOUS
  FUNCDEF("unlock");
#endif
  if (!healthy()) return;
  if (_locked) {
#ifdef __UNIX__
    int ret = lockf(fileno((FILE *)_handle), F_ULOCK, sizeof(int));
    if (ret) {
#ifdef DEBUG_RENDEZVOUS
      LOG("failure to get lock on file.");
#endif
    }
#endif
#ifdef __WIN32__
    ReleaseMutex((HANDLE)_handle);
#endif
    _locked = false;
  } else {
#ifdef DEBUG_RENDEZVOUS
    LOG("okay, rendezvous wasn't locked.");
#endif
  }
}

} //namespace.

