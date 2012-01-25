#ifndef MEMORY_CHECKER_CLASS
#define MEMORY_CHECKER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : memory_checker                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1998-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "definitions.h"

#ifdef ENABLE_MEMORY_HOOK

#include "build_configuration.h"
uhh

// forward.
class allocation_memories;
class memory_checker;

//////////////

memory_checker BASIS_EXTERN &program_wide_memories();
  //!< a global version of the memory checker to access memory tracking.
  /*!< this accesses the singleton object that tracks all memory allocations
  occuring in the program via calls to new and delete.  it should be used
  rather than creating a memory_checker anywhere else. */

//////////////

//! Debugging assistance tool that checks for memory leaks.
/*!
  Provides allocation checking for heap memory for C++.  This is used as a
  replacement for the standard new and delete operations.  No object should
  really need to deal with this class directly; it is hooked in automatically
  unless the ENABLE_MEMORY_HOOK macro is defined.  Generally, release builds
  should not have this enabled, since it will slow things down tremendously.
  NOTE: this object can absolutely not be hooked into callstack tracker, since
  this object implements all c++ memory, including the tracker's.  This object
  will use the backtrace information if it's available.
*/

class memory_checker
{
public:
  void construct();  //!< faux constructor for mallocing.

  void destruct();  //!< faux destructor shuts down object.

  //! turn off memory checking.
  void disable() { _enabled = false; }
  //! turn memory checking back on.
  void enable() { _enabled = true; }
  //! reports on whether the memory checker is currently in service or not.
  bool enabled() const { return _enabled; }

  void *provide_memory(size_t size, char *file, int line);
    //!< returns a chunk of memory with the "size" specified.
    /*!< this is the replacement method for the new operator.  we will be
    calling this instead of the compiler provided new.  the "file" string
    should be a record of the location where this is invoked, such as is
    provided by the __FILE__ macro.  the "line" should be set to the line
    number within the "file", if applicable (use __LINE__). */

  int release_memory(void *ptr);
    //!< drops our record for the memory at "ptr".
    /*!< this is the only way to remove an entry from our listings so that
    it will not be reported as a leak.  we do not currently gather any info
    about where the release is invoked.  if there was a problem, the returned
    outcome will not be OKAY. */

  char *text_form(bool show_outstanding);
    //!< returns a newly allocated string with the stats for this object.
    /*!< if "show_outstanding" is true, then all outstanding allocations are
    displayed in the string also.  the invoker *must* free the returned
    pointer. */

private:
  allocation_memories *_mems;  //!< internal object tracks all allocations.
  bool _unusable;  //!< true after destruct is called.
  bool _enabled;  //!< true if the object is okay to use.
};

#else // enable memory hook.
  // this section disables the memory checker entirely.
  #define program_wide_memories()
    // define a do nothing macro for the global memory tracker.
#endif // enable memory hook.

#endif // outer guard.

