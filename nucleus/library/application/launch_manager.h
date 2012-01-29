
// current bad issues:
//
// this class does not really provide a notion of the process name AND process id as being
// a pair one can operate on.  mostly it assumes a bunch of singletons or that it's okay
// to whack all of them.
// that could be fixed by making the procname+procid pair into a unit in all functions,
// and make kill_all be a specialization of that.


#ifndef LAUNCH_MANAGER_CLASS
#define LAUNCH_MANAGER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : launch_manager
*  Author : Chris Koeritz
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <basis/mutex.h>
#include <basis/outcome.h>
#include <basis/contracts.h>
#include <processes/configured_applications.h>
#include <processes/process_entry.h>
#include <processes/process_control.h>
#include <structures/set.h>
#include <timely/time_stamp.h>

namespace application {

class graceful_array;
class launch_manager_thread;

//! Provides methods for starting, stopping and checking on processes.
/*!
  This includes support for graceful shutdowns and background handling of
  exiting processes.
*/

class launch_manager : public virtual basis::root_object
{
public:
  launch_manager(processes::configured_applications &config);
    //!< the launch_manager needs a configuration set to work with.

  virtual ~launch_manager();

  DEFINE_CLASS_NAME("launch_manager");

  enum outcomes {
    OKAY = basis::common::OKAY,
    EXISTING = basis::common::EXISTING,
      //!< the entry already exists and overwriting is disallowed.
    ACCESS_DENIED = basis::common::ACCESS_DENIED,
      //!< the requested operation was not permitted.

    DEFINE_API_OUTCOME(FILE_NOT_FOUND, -53, "The file specified for the "
        "application doesn't exist, as far as we can tell"),
    DEFINE_API_OUTCOME(NO_PRODUCT, -54, "The product specified does not exist"),
    DEFINE_API_OUTCOME(NO_APPLICATION, -55, "The application is not listed for "
        "the product"),
    DEFINE_API_OUTCOME(NOT_RUNNING, -56, "The program is not currently active, "
        "according to the OS process list"),
    DEFINE_API_OUTCOME(BAD_PROGRAM, -57, "The file existed but doesn't appear "
        "to be a valid program image"),
    DEFINE_API_OUTCOME(NO_ANCHOR, -58, "This occurs when the graceful shutdown "
        "process cannot find the special anchor window that implements the "
        "client side of a graceful shutdown"),
    DEFINE_API_OUTCOME(LAUNCH_FAILED, -59, "The program existed and seemed "
        "valid but its launch failed for some reason"),
    DEFINE_API_OUTCOME(FROZEN, -60, "The application is broken somehow; the "
        "system reports it as non-responsive"),
    // note that these values are reversed, since the ordering is negative.
    FIRST_OUTCOME = FROZEN,  // hold onto start of range.
    LAST_OUTCOME = FILE_NOT_FOUND // hold onto end of range.
  };

  static const char *outcome_name(const basis::outcome &to_name);
    //!< returns the text associated with "to_name".

  basis::outcome launch_now(const basis::astring &product, const basis::astring &app_name,
          const basis::astring &parameters);
    //!< starts the application "app_name" now.
    /*!< causes the program with "app_name" that's listed for the "product" to
    be started with the "parameters".  this can fail if the application
    isn't listed or if the program can't be found or if the process can't
    be created. */

  basis::outcome launch_at_startup(const basis::astring &product, const basis::astring &app_name,
          const basis::astring &parameters, int one_shot);
    //!< records an entry for the "app_name" to be launched at startup.
    /*!< this does not launch the application now.  if "one_shot" is true, the
    application will only be started once and then removed from the list. */

  basis::outcome remove_from_startup(const basis::astring &product, const basis::astring &app_name);
    //!< takes the "app_name" out of the startup list.

  basis::outcome query_application(const basis::astring &product, const basis::astring &app_name);
    //!< retrieves the current state of the program with "app_name".

  basis::outcome zap_process(const basis::astring &product, const basis::astring &app_name,
          bool graceful);
    //!< zaps the process named "app_name".
    /*!< if "graceful" is true, then the clean shutdown process is attempted.
    otherwise the application is harshly terminated. */

  void add_gag_exclusion(const basis::astring &exclusion);
    //!< add an application that isn't subject to gagging.
    /*!< if the launch_manager is gagged, the excluded applications can
    still be started. */
  void add_tracking_exclusion(const basis::astring &exclusion);
    //!< apps that aren't tracked when running.
    /*!< if a zap is attempted on one of these applications, then
    the process is not shut down. */

  basis::outcome shut_down_launching_services(const basis::astring &secret_word);
    //!< closes down the ability of clients to launch applications.
    /*!< the checking and shut down functions continue to operate.  this is
    intended for use prior to a restart of the application controller, in
    order to ensure that no remote clients can start new servers on this
    machine. */

  basis::outcome reenable_launching_services(const basis::astring &secret_word);
    //!< undoes the gagging that the above "shut_down" function does.
    /*!< this allows the launch_manager to continue operating normally. */

  bool services_disabled() const { return _stop_launching; }
    //!< returns true if the capability to launch new processes is revoked.

  void push_timed_activities(processes::process_entry_array &processes);
    //!< keeps any periodic activities going.
    /*!< this includes such tasks as zapping processes that have gone beyond
    their time limit for graceful shutdown. */

  void stop_everything();
    //!< closes down the operation of this object.

private:
  processes::configured_applications &_configs;  //!< manages the entries for companies.
  bool _started_initial_apps;  //!< true if we launched the boot apps.
  launch_manager_thread *_checker;  //!< keeps periodic activities going.
  basis::mutex *_config_lock;  //!< the synchronizer for our configuration entries.
  graceful_array *_going_down;  //!< record of graceful shutdowns in progress.
  basis::mutex *_zombie_lock;  //!< the synchronizer for the dying processes.
  graceful_array *_our_kids;  //!< the processes we've started.
  basis::mutex *_scamp_lock;  //!< the synchronizer for the list of children.
  bool _stop_launching;  //!< true if no launches should be allowed any more.
  timely::time_stamp *_startup_time;
    //!< the time we feel it's safe to launch the startup apps.
    /*!< we delay this some so that the launch_manager doesn't immediately
    soak up too much CPU. */
  processes::process_control *_procs;  //!< gives us access to the process list.
  structures::string_set *_gag_exclusions;  //!< apps that aren't subject to gag law.
  structures::string_set *_tracking_exclusions;  //!< apps that aren't tracked when running.

  bool get_processes(processes::process_entry_array &processes);
    //!< grabs the list of "processes" or returns false.

  bool find_process(const basis::astring &app_name, structures::int_set &pids);
    //!< locates any instances of "app_name" and returns process ids in "pids".

  basis::outcome start_graceful_close(const basis::astring &product, const basis::astring &app_name);
    //!< attempts to close the "app_name".
    /*!< if the application doesn't shut down within the time limit, it is
    eventually zapped harshly. */

  void launch_startup_apps();
    //!< iterates over the list of startup apps and creates a process for each.

  void stop_all_kids();
    //!< closes all dependent processes when its time to stop the service.
};

} //namespace.

#endif

