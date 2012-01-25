#ifndef PROCESS_CONTROL_CLASS
#define PROCESS_CONTROL_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : process_control                                                   *
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

#include "process_entry.h"

#include <basis/contracts.h>
#include <mathematics/chaos.h>
#include <structures/set.h>

namespace processes {

// forward.
class process_entry_array;
class process_implementation_hider;

//! Provides a bridge to the operating system for information on processes.
/*!
  This object can query the operating system for the current set of processes
  or zap a particular process of interest.
*/

class process_control : public virtual basis::nameable
{
public:
  process_control();
  virtual ~process_control();

  DEFINE_CLASS_NAME("process_control");

  bool healthy() const { return _healthy; }
    //!< returns true if this object should be functional.
    /*!< if it failed to construct properly, this returns false.  usually a
    failure indicates that a required dynamic library is missing, such as
    "psapi.dll" on win32. */

  process_entry query_process(basis::un_int to_query);
    //!< returns the information for just one process.

  bool query_processes(process_entry_array &to_fill);
    //!< finds the processes that are running and drops them into "to_fill".

  bool zap_process(basis::un_int to_zap);
    //!< preemptively zaps the process "to_zap".
    /*!< this does not invoke any friendly graceful shut down process, but
    merely terminates it if possible. */

  static bool find_process_in_list(const process_entry_array &processes,
          const basis::astring &app_name, structures::int_set &pids);
    //!< uses a pre-existing list of "processes" to search for the "app_name".
    /*!< if the process is found, true is returned and the "pids" are set to
    all entries matching the process name.  note that this is an approximate
    match for some OSes that have a brain damaged process lister (such as
    ms-windows); they have programs listed under incomplete names in some
    cases. */

  void sort_by_name(process_entry_array &to_sort);
    // sorts the list by process name.
  void sort_by_pid(process_entry_array &to_sort);
    // sorts the list by process id.

private:
  process_implementation_hider *_ptrs;  //!< our OS baggage.
#ifdef __UNIX__
  mathematics::chaos *_rando;  //!< used for process list.
#endif
#ifdef __WIN32__
  bool _use_psapi;  //!< true if we should be using the PSAPI on NT and family.
#endif
  bool _healthy;  //!< true if construction succeeded.

#ifdef __UNIX__
  bool get_processes_with_ps(process_entry_array &to_fill);
    //!< asks the ps program what processes exist.
#endif
#ifdef __WIN32__
  // fill in our function pointers to access the kernel functions appropriate
  // for either NT (psapi) or 9x (toolhelp).
  bool initialize_psapi_support();
  bool initialize_toolhelp_support();

  bool get_processes_with_psapi(process_entry_array &to_fill);
    //!< uses the PSAPI support for windows NT 4 (or earlier?).
  bool get_processes_with_toolhelp(process_entry_array &to_fill);
    //!< uses the toolhelp support for windows 9x, ME, 2000.
#endif
};

} //namespace.

#endif // outer guard.

