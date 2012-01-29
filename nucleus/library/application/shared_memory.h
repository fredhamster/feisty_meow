#ifndef SHARED_MEMORY_CLASS
#define SHARED_MEMORY_CLASS

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

#include <basis/contracts.h>
#include <filesystem/byte_filer.h>
#include <processes/rendezvous.h>

namespace application {

//! Implements storage for memory that can be shared between threads.
/*!
  Provides a means to create shared memory chunks and access them from
  anywhere in a program or from cooperating programs.
*/

class shared_memory : public virtual basis::root_object
{
public:
  shared_memory(int size, const char *identity);
    //!< a shared chunk of the "size" specified will be created.
    /*!< it is named by the "identity" string.  that "identity" uniquely
    points to one shared chunk on this machine.  note that if this object is
    the first to create the chunk of memory, then all of the contents are
    initialized to zero.  this can be used to determine if the chunk needs
    higher level, application-specific initialization. */

  virtual ~shared_memory();
    //!< cleans up the shared bit of memory as far as we're concerned.
    /*!< if some other instance still has it opened, then it isn't
    destroyed for real yet. */

  DEFINE_CLASS_NAME("shared_memory");

  bool valid() const { return _valid; }
    //!< this must be true for the shared_memory to be usable.
    /*!< if it's false, then the memory chunk was never created. */

  int size() const { return _size; }
    //!< returns the size of the shared chunk of memory.

  const basis::astring &identity() const;
    //!< provides a peek at the name that this chunk was constructed with.

  bool first_usage(basis::abyte *locked_memory, int max_compare);
    //!< returns true if the "locked_memory" was just created.
    /*!< that is assuming that a user of the shared memory will set the first
    "max_compare" bytes to something other than all zeros.  this is really
    just a test of whether bytes zero through bytes "max_compare" - 1 are
    currently zero, causing a return of true.  seeing anything besides zero
    causes a false return. */

  basis::abyte *lock();
    //!< locks the shared memory and returns a pointer to the storage.
    /*!< the synchronization supported is only within this program; this type
    of shared memory is not intended for access from multiple processes,
    just for access from multiple threads in the same app. */

  void unlock(basis::abyte * &to_unlock);
    //!< returns control of the shared memory so others can access it.
    /*!< calls to lock() must be paired up with calls to unlock(). */

  static basis::astring unique_shared_mem_identifier(int sequencer);
    //!< returns a unique identifier for a shared memory chunk.
    /*!< the id returned is unique on this host for this particular process
    and application, given a "sequencer" number that is up to the application
    to keep track of uniquely.  the values from -100 through -1 are reserved
    for hoople library internals. */

private:
  processes::rendezvous *_locking;  //!< protects our shared memory.
#ifdef __UNIX__
  int _the_memory;  //!< OS index of the memory.
#elif defined(__WIN32__)
  void *_the_memory;  //!< OS pointer to the memory.
#endif
  bool _valid;  //!< true if the memory creation succeeded.
  basis::astring *_identity;  //!< holds the name we were created with.
  int _size;  //!< size of memory chunk.

  // these do the actual work of getting the memory.
  basis::abyte *locked_grab_memory();
  void locked_release_memory(basis::abyte * &to_unlock);

  static basis::astring special_filename(const basis::astring &identity);
    //!< provides the name for our shared memory file, if needed.

  // forbidden.
  shared_memory(const shared_memory &);
  shared_memory &operator =(const shared_memory &);
};

} //namespace.


#endif  // outer guard.

