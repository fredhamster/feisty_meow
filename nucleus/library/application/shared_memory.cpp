/*****************************************************************************\
*                                                                             *
*  Name   : shared_memory                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "shared_memory.h"

#include <application/windoze_helper.h>
#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/utf_conversion.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <configuration/application_configuration.h>
#include <loggers/critical_events.h>
#include <filesystem/byte_filer.h>
#include <filesystem/filename.h>
#include <structures/string_hasher.h>

#include <stdio.h>
#include <stdlib.h>
#ifdef __UNIX__
  #include <fcntl.h>
  #include <sys/ipc.h>
  #include <sys/mman.h>
  #include <sys/shm.h>
  #include <sys/types.h>
  #include <unistd.h>
#endif

using namespace basis;
using namespace configuration;
using namespace filesystem;
using namespace loggers;
using namespace processes;
using namespace structures;

namespace application {

shared_memory::shared_memory(int size, const char *identity)
: _locking(new rendezvous(identity)),
  _the_memory(NIL),
  _valid(false),
  _identity(new astring(identity)),
  _size(size)
{
  FUNCDEF("constructor");
  bool first_use = false;  // assume already existing until told otherwise.
  _locking->lock();  // force all others to wait on our finishing creation.
#ifdef __UNIX__
  int flag = O_RDWR | O_CREAT | O_EXCL;
    // try to create the object if it doesn't exist yet, but fail if it does.
  int mode = 0700;  // rwx------ for just us.
  _the_memory = shm_open(special_filename(identity).s(), flag, mode);
    // create the shared memory object but fail if it already exists.
  if (negative(_the_memory)) {
    // failed to create the shared segment.  try without forcing exclusivity.
    flag = O_RDWR | O_CREAT;
    _the_memory = shm_open(special_filename(identity).s(), flag, mode);
    basis::un_int err = critical_events::system_error();  // get last error.
    if (negative(_the_memory)) {
      // definitely a failure now...
      printf("error allocating shared segment for %s, was told %s.\n",
          special_filename(identity).s(), critical_events::system_error_text(err).s());
      _the_memory = 0;
      _locking->unlock();  // release lock before return.
      return;
    }
    // getting to here means the memory was already allocated.  so we're fine.
  } else {
    // the shared memory segment was just created this time.
    int ret = ftruncate(_the_memory, size);
    basis::un_int err = critical_events::system_error();  // get last error.
    if (ret) {
      printf("error truncating shared segment for %s, was told %s.",
          special_filename(identity).s(), critical_events::system_error_text(err).s());
    }
    first_use = true;
  }
  _valid = true;
#elif defined(__WIN32__)
  _the_memory = ::CreateFileMapping((HANDLE)-1, NULL, PAGE_READWRITE,
      0, size, to_unicode_temp(identity));
  basis::un_int err = critical_events::system_error();  // get last error.
  first_use = (err != ERROR_ALREADY_EXISTS);
  if (!_the_memory) {
    _locking->unlock();
    return;  // not healthy.
  }
  _valid = true;
#else
//this is junk; simulates shared memory poorly.
  #pragma message("simulating shared memory since unknown for this platform.")
  if (!_bogus_shared_space().length()) {
    _bogus_shared_space().reset(size);
    first_use = true;
  }
  _the_memory = _bogus_shared_space().access();
  _valid = true;
#endif
  if (first_use) {
    // initialize the new memory to all zeros.
    abyte *contents = locked_grab_memory();
    if (!contents) {
      _valid = false;
       _locking->unlock();
       return;
    }
    memset(contents, 0, size);
    locked_release_memory(contents);  // release the memory for now.
  }
  _locking->unlock();
}

shared_memory::~shared_memory()
{
#ifdef __UNIX__
  if (_the_memory) {
    close(int(_the_memory));
    shm_unlink(special_filename(identity()).s());
  }
#elif defined(__WIN32__)
  ::CloseHandle(_the_memory);
#else
  //hmmm: fix.
  _the_memory = NIL;
#endif
  WHACK(_locking);
  WHACK(_identity);
  _valid = false;
}

const astring &shared_memory::identity() const { return *_identity; }

astring shared_memory::special_filename(const astring &identity)
{
  astring shared_file = identity;
  filename::detooth_filename(shared_file);
  shared_file = astring("/tmp_") + "sharmem_" + shared_file;
  return shared_file;
}

astring shared_memory::unique_shared_mem_identifier(int sequencer)
{
  astring to_return("SMID-");
  to_return += a_sprintf("%d-%d-", application_configuration::process_id(), sequencer);
  to_return += application_configuration::application_name();
  // replace file delimiters in the name with a safer character.
  filename::detooth_filename(to_return);
  return to_return;
}

bool shared_memory::first_usage(abyte *contents, int max_compare)
{
  for (int i = 0; i < max_compare; i++)
    if (contents[i] != 0) return false;
  return true;
}

abyte *shared_memory::lock()
{
  _locking->lock();
  return locked_grab_memory();
}

void shared_memory::unlock(abyte * &to_unlock)
{
  locked_release_memory(to_unlock);
  _locking->unlock();
}

abyte *shared_memory::locked_grab_memory()
{
  FUNCDEF("locked_grab_memory")
  abyte *to_return = NIL;
  if (!_the_memory) return to_return;
#ifdef __UNIX__
  to_return = (abyte *)mmap(NIL, _size, PROT_READ | PROT_WRITE,
      MAP_SHARED, int(_the_memory), 0);
#elif defined(__WIN32__)
  to_return = (abyte *)::MapViewOfFile((HANDLE)_the_memory, FILE_MAP_ALL_ACCESS,
      0, 0, 0);
#else
  to_return = (abyte *)_the_memory;
#endif
  if (!to_return) {
//    FUNCTION(func);
//not working yet.  callstack tracker or whatever is hosed up.
    throw(astring(astring(class_name()) + "::" + func + ": no data was accessible in shared space."));
  }
  return to_return;
}

void shared_memory::locked_release_memory(abyte * &to_unlock)
{
  if (!_the_memory || !to_unlock) return;
#ifdef __UNIX__
  munmap(to_unlock, _size);
#elif defined(__WIN32__)
  ::UnmapViewOfFile(to_unlock);
#else
//uhh.
#endif
}

} //namespace.

