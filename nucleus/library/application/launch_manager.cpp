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

#include "hoople_service.h"
#include "launch_manager.h"

#include <application/windoze_helper.h>
#include <basis/astring.h>
#include <basis/mutex.h>
#include <configuration/configurator.h>
#include <configuration/section_manager.h>
#include <filesystem/filename.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <processes/configured_applications.h>
#include <processes/ethread.h>
#include <processes/launch_process.h>
#include <structures/set.h>
#include <structures/string_table.h>
#include <structures/unique_id.h>
#include <textual/parser_bits.h>
#include <timely/time_control.h>
#include <timely/time_stamp.h>

using namespace basis;
using namespace configuration;
using namespace filesystem;
using namespace loggers;
using namespace processes;
using namespace structures;
using namespace textual;
using namespace timely;

namespace application {

#define DEBUG_PROCESS_MANAGER
  // uncomment for verbose diagnostics.

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

const int CHECK_INTERVAL = 4 * SECOND_ms;
  // this is how frequently the checking thread executes to ensure that
  // processes are gone when they should be.

const int GRACEFUL_SLACK = 90 * SECOND_ms;
  // the length of time before a graceful shutdown devolves into a forced
  // shutdown.

const int MAXIMUM_INITIAL_APP_WAIT = 4 * SECOND_ms;
  // this is the longest we bother to wait for a process we just started.
  // if it hasn't begun by then, we decide it will never do so.

const int STARTUP_APPS_DELAY_PERIOD = 2 * SECOND_ms;
  // we delay for this long before the initial apps are started.

const int MAXIMUM_REQUEST_PAUSE = 42 * SECOND_ms;
  // the longest we will ever wait for a response to be generated based on
  // our last request.

// these are concurrency control macros for the lists managed here.
#define LOCK_CONFIG auto_synchronizer l(*_config_lock)
#define LOCK_ZOMBIES auto_synchronizer l(*_zombie_lock)
#define LOCK_KIDS auto_synchronizer l(*_scamp_lock)

// error messages.
#ifdef DEBUG_PROCESS_MANAGER
  #define COMPLAIN_APPLICATION \
    LOG(astring("the application called ") + app_name + " could not be found.")
  #define COMPLAIN_PRODUCT \
    LOG(astring("the section for ") + product + " could not be found.")
#else
  #define COMPLAIN_APPLICATION {}
  #define COMPLAIN_PRODUCT {}
#endif

//////////////

class launch_manager_thread : public ethread
{
public:
  launch_manager_thread(launch_manager &parent)
  : ethread(CHECK_INTERVAL, ethread::SLACK_INTERVAL),
    _parent(parent) {}
  virtual ~launch_manager_thread() {}

  virtual void perform_activity(void *)
      { _parent.push_timed_activities(_processes); }

private:
  launch_manager &_parent;  // the owner of the object.
  process_entry_array _processes;  // will be filled as needed.
};

//////////////

class graceful_record
{
public:
  astring _product;  // the product name the app is listed under.
  astring _app_name;  // the application's name.
  time_stamp _started;  // when the graceful shutdown started.
  int _pid;  // the particular process id for this app.
  int _level;  // the shutdown ordering specifier.

  graceful_record(int pid = 0, const astring &product = "",
          const astring &app_name = "", int level = 0)
  : _product(product), _app_name(app_name), _pid(pid), _level(level) {}
};

class graceful_array : public array<graceful_record> {};

//////////////

launch_manager::launch_manager(configured_applications &config)
: _configs(config),
  _started_initial_apps(false),
  _checker(new launch_manager_thread(*this)),
  _config_lock(new mutex),
  _going_down(new graceful_array),
  _zombie_lock(new mutex),
  _our_kids(new graceful_array),
  _scamp_lock(new mutex),
  _stop_launching(false),
  _startup_time(new time_stamp(STARTUP_APPS_DELAY_PERIOD)),
  _procs(new process_control),
  _gag_exclusions(new string_set),
  _tracking_exclusions(new string_set)
{
  FUNCDEF("constructor");

  // start the application checking thread.
  _checker->start(NIL);

  _checker->reschedule(200);  // make it start pretty quickly.
}

launch_manager::~launch_manager()
{
  FUNCDEF("destructor");
  stop_everything();

  WHACK(_checker);
  WHACK(_going_down);
  WHACK(_our_kids);
  WHACK(_scamp_lock);
  WHACK(_zombie_lock);
  WHACK(_config_lock);
  WHACK(_startup_time);
  WHACK(_procs);
  WHACK(_gag_exclusions);
  WHACK(_tracking_exclusions);
  LOG("launch_manager is now stopped.");
}

void launch_manager::add_gag_exclusion(const astring &exclusion)
{ *_gag_exclusions += exclusion; }

void launch_manager::add_tracking_exclusion(const astring &exclusion)
{ *_tracking_exclusions += exclusion; }

const char *launch_manager::outcome_name(const outcome &to_name)
{
  switch (to_name.value()) {
    case FILE_NOT_FOUND: return "FILE_NOT_FOUND";
    case NO_ANCHOR: return "NO_ANCHOR";
    case NO_PRODUCT: return "NO_PRODUCT";
    case NO_APPLICATION: return "NO_APPLICATION";
    case BAD_PROGRAM: return "BAD_PROGRAM";
    case LAUNCH_FAILED: return "LAUNCH_FAILED";
    case NOT_RUNNING: return "NOT_RUNNING";
    case FROZEN: return "FROZEN";
    default: return common::outcome_name(to_name);
  }
}

void launch_manager::stop_everything()
{
  _stop_launching = true;  // at least deny any connected clients.
  stop_all_kids();  // shut down all programs that we started.
  _checker->stop();  // stop our thread.
}

void launch_manager::stop_all_kids()
{
  FUNCDEF("stop_all_kids");
  _stop_launching = true;  // set this for good measure to keep clients out.
  LOG("zapping any active sub-processes prior to exit.");

  // now we wait for the process closures to take effect.  we are relying on
  // the graceful shutdown devolving to a process zap and the timing is
  // rooted around that assumption.

  for (int lev = 100; lev >= 0; lev--) {
    // loop from our highest level to our lowest for the shutdown.
#ifdef DEBUG_PROCESS_MANAGER
    LOG(a_sprintf("level %d", lev));
#endif
    bool zapped_any = false;
    {
      // this shuts down all the child processes we've started at this level.
      LOCK_KIDS;  // lock within this scope.
      for (int i = _our_kids->length() - 1; i >= 0; i--) {
        // now check each record and see if it's at the appropriate level.
        graceful_record &grace = (*_our_kids)[i];
        if (lev == grace._level) {
          // start a graceful shutdown.
          zap_process(grace._product, grace._app_name, true);
          // remove it from our list.
          _our_kids->zap(i, i);
          zapped_any = true;  // set our flag.
        }
      }
    }
    int num_dying = 1;  // go into the loop once at least.

#ifdef DEBUG_PROCESS_MANAGER
    time_stamp next_print(4 * SECOND_ms);
#endif

    while (num_dying) {
#ifdef DEBUG_PROCESS_MANAGER
      if (time_stamp() >= next_print) {
        LOG("waiting...");
        next_print.reset(4 * SECOND_ms);
      }
#endif

      // while there are any pending process zaps, we will wait here.  this
      // will hose us but good if the processes aren't eventually cleared up,
      // but that shouldn't happen.

      {
        LOCK_ZOMBIES;
        num_dying = _going_down->length();
      }

      if (!num_dying) break;  // jump out of loop.

      _checker->reschedule(0);  // make thread check as soon as possible.

      time_control::sleep_ms(40);
    }
#ifdef DEBUG_PROCESS_MANAGER
    LOG("done waiting...");
#endif
  }
}

void launch_manager::launch_startup_apps()
{
  FUNCDEF("launch_startup_apps");

  // read the startup section.
  string_table startup_info;
  {
    LOCK_CONFIG;
    if (!_configs.find_section(_configs.STARTUP_SECTION(), startup_info)) {
      // if there's no startup section, we do nothing right now.
      LOG("the startup section was not found!");
      return;
    }
  }
#ifdef DEBUG_PROCESS_MANAGER
  LOG(astring("table has: ") + startup_info.text_form());
#endif

  for (int i = 0; i < startup_info.symbols(); i++) {
    astring app = startup_info.name(i);
    if (app.equal_to(configured_applications::STARTUP_APP_NAME())) continue;
      // skip bogus name that keeps the section present.
    astring info = startup_info[i];
    LOG(astring("launching application ") + app + "...");
    // parse the items that are in the entry for this program.
    astring product, parms;
    bool one_shot;
    if (!configured_applications::parse_startup_entry(info, product, parms,
        one_shot)) {
      LOG("the startup entry was not malformed; we could not parse it!");
      continue;
    }

    LOG(app + a_sprintf(" is %ssingle shot.", one_shot? "" : "not "));

    // now try to send the program off on its way.
    launch_now(product, app, parms);

    if (one_shot) {
      // it's only supposed to be started once, so now toss out the entry.
      remove_from_startup(product, app);
    }
  }
}

outcome launch_manager::launch_now(const astring &product,
    const astring &app_name, const astring &parameters)
{
  FUNCDEF("launch_now");
  LOG(astring("product \"") + product + "\", application \"" + app_name
      + (parameters.length() ? astring("\"")
            : astring("\", parms=") + parameters));

  if (_stop_launching) {
    // if the application is one of the exceptions to the gag rule, then
    // we will still launch it.  otherwise, we'll ignore this request.
    if (_gag_exclusions->member(app_name)) {
      // this is one of the apps that can still be launched when gagged.
    } else {
      LOG(astring("application \"") + app_name + "\" cannot be launched;"
         + parser_bits::platform_eol_to_chars() + "launching services have been "
         "shut down.");
      return LAUNCH_FAILED;
    }
  }

  astring entry_found;
  int level;
  {
    LOCK_CONFIG;

    // get the specific entry for the program they want.
    entry_found = _configs.find_program(product, app_name, level);
    if (!entry_found) {
      if (!_configs.product_exists(product)) {
        // return more specific error for missing product.
        COMPLAIN_PRODUCT;
        return NO_PRODUCT;
      }
      COMPLAIN_APPLICATION;
      return NO_APPLICATION;
    }
  }

  filename existence_check(entry_found);
  if (!existence_check.exists()) {
    LOG(astring("file or path wasn't found for ") + entry_found + ".");
    return FILE_NOT_FOUND;
  }

  basis::un_int kid = 0;
  int ret = launch_process::run(entry_found, parameters,
      launch_process::RETURN_IMMEDIATELY | launch_process::HIDE_APP_WINDOW, kid);
  if (!ret) {
    // hey, it worked!  so now make sure we track its lifetime...

    if (_tracking_exclusions->member(app_name)) {
      // this is one of the apps that we don't track.  if it's still
      // running when we're shutting down, it's not our problem.
      return OKAY;
    }

    if (kid) {
      // we were told the specific id for the process we started.
      LOCK_KIDS;
      LOG(a_sprintf("adding given process id %d for app %s at level %d.",
          kid, app_name.s(), level));
      graceful_record to_add(kid, product, app_name, level);
      *_our_kids += to_add;
      return OKAY;
    }
#ifdef DEBUG_PROCESS_MANAGER
    LOG("was not told child process id!!!");
#endif
    // we weren't told the process id; let's see if we can search for it.
    int_set pids;
    time_stamp give_it_up(MAXIMUM_INITIAL_APP_WAIT);
    while (give_it_up > time_stamp()) {
      // find the process id for the program we just started.
      if (find_process(app_name, pids)) break;
      time_control::sleep_ms(10);  // pause to see if we can find it yet.
    }

    if (time_stamp() >= give_it_up) {
      // we could not launch it for some reason, or our querier has failed.
      // however, as far as we know, it launched successfully.  if the id is
      // missing, then that's not really our fault.  we will just not be able
      // to track the program, possibly because it's already gone.
      LOG(astring("no process found for product \"") + product
          + "\", application \"" + app_name + "\"; not adding "
          "tracking record.");
      return OKAY;
    }
  
    LOCK_KIDS;

    // add all the processes we found under that name.
    for (int i = 0; i < pids.elements(); i++) {
      LOG(a_sprintf("adding process id %d for app %s at level %d.",
          pids[i], app_name.s(), level));
      graceful_record to_add(pids[i], product, app_name, level);
      *_our_kids += to_add;
    }
    return OKAY;
  }

  // if we reached here, things are not good.  we must not have been able to
  // start the process.

#ifdef __WIN32__
  if (ret == NO_ERROR) return OKAY;  // how would that happen?
  else if ( (ret == ERROR_FILE_NOT_FOUND) || (ret == ERROR_PATH_NOT_FOUND) ) {
    LOG(astring("file or path wasn't found for ") + app_name + ".");
    return FILE_NOT_FOUND;
  } else if (ret == ERROR_BAD_FORMAT) {
    LOG(astring(app_name) + " was not in EXE format.");
    return BAD_PROGRAM;
  } else {
    LOG(astring("there was an unknown error while trying to run ")
        + app_name + "; the error is:" + parser_bits::platform_eol_to_chars()
        + critical_events::system_error_text(ret));
    return LAUNCH_FAILED;
  }
#else
  LOG(astring("error ") + critical_events::system_error_text(ret)
      + " occurred attempting to run: " + app_name + " " + parameters);
  return FILE_NOT_FOUND;
#endif
}

outcome launch_manager::launch_at_startup(const astring &product,
    const astring &app_name, const astring &parameters, int one_shot)
{
  FUNCDEF("launch_at_startup");
  LOCK_CONFIG;  // this whole function modifies the config file.

#ifdef DEBUG_PROCESS_MANAGER
  LOG(astring("product \"") + product + "\", application \"" + app_name
      + (one_shot? astring("\", OneShot") : astring("\", MultiUse")));
#endif

  // get the specific entry for the program they want.
  int level;
  astring entry_found = _configs.find_program(product, app_name, level);
  if (!entry_found) {
    if (!_configs.product_exists(product)) {
      // return more specific error for missing product.
      COMPLAIN_PRODUCT;
      return NO_PRODUCT;
    }
    COMPLAIN_APPLICATION;
    return NO_APPLICATION;
  }

  if (!_configs.add_startup_entry(product, app_name, parameters, one_shot)) {
    // most likely problem is that it was already there.
    return EXISTING;
  }

  return OKAY;
}

outcome launch_manager::remove_from_startup(const astring &product,
    const astring &app_name)
{
  FUNCDEF("remove_from_startup");
  LOCK_CONFIG;  // this whole function modifies the config file.

#ifdef DEBUG_PROCESS_MANAGER
  LOG(astring("product \"") + product + "\", application \"" + app_name + "\"");
#endif

  // get the specific entry for the program they want.
  int level;
  astring entry_found = _configs.find_program(product, app_name, level);
  if (!entry_found) {
    if (!_configs.product_exists(product)) {
      // return more specific error for missing product.
      COMPLAIN_PRODUCT;
      return NO_PRODUCT;
    }
    COMPLAIN_APPLICATION;
    return NO_APPLICATION;
  }
//hmmm: is product required for this for real?

  if (!_configs.remove_startup_entry(product, app_name)) {
    // the entry couldn't be removed, probably doesn't exist.
    return NO_APPLICATION;
  }

  return OKAY;
}

outcome launch_manager::query_application(const astring &product,
    const astring &app_name)
{
  FUNCDEF("query_application");
#ifdef DEBUG_PROCESS_MANAGER
  LOG(astring("product \"") + product + "\", application \"" + app_name + "\"");
#endif

  {
    LOCK_CONFIG;
    // get the specific entry for the program they want.
    int level;
    astring entry_found = _configs.find_program(product, app_name, level);
    if (!entry_found) {
      if (!_configs.product_exists(product)) {
        // return more specific error for missing product.
        COMPLAIN_PRODUCT;
        return NO_PRODUCT;
      }
      COMPLAIN_APPLICATION;
      return NO_APPLICATION;
    }
  }

  // now seek that program name in the current list of processes.
  astring program_name(app_name);
  program_name.to_lower();

  int_set pids;
  if (!find_process(app_name, pids))
    return NOT_RUNNING;
  return OKAY;
}

outcome launch_manager::start_graceful_close(const astring &product,
    const astring &app_name)
{
  FUNCDEF("start_graceful_close");
//hmmm: record this app as one we need to watch.

  {
    // find that program name to make sure it's not already shutting down.
    LOCK_ZOMBIES;

    for (int i = _going_down->length() - 1; i >= 0; i--) {
      graceful_record &grace = (*_going_down)[i];
      if (grace._app_name.iequals(app_name)) {
        return OKAY;
      }
    }
  }

  if (!hoople_service::close_application(app_name))
    return NO_ANCHOR;

  int_set pids;
  if (!find_process(app_name, pids)) {
    LOG(astring("Failed to find process id for [") + app_name
        + astring("], assuming it has already exited."));
    return OKAY;
  }

  {
    // add all the process ids, just in case there were multiple instances
    // of the application somehow.
    LOCK_ZOMBIES;
    for (int i = 0; i < pids.elements(); i++) {
      graceful_record to_add(pids[i], product, app_name);
      *_going_down += to_add;
    }
  }

  return OKAY;
}

bool launch_manager::get_processes(process_entry_array &processes)
{
  FUNCDEF("get_processes");
  if (!_procs->query_processes(processes)) {
    LOG("failed to query processes!");
    return false;
  }
  return true;
}

bool launch_manager::find_process(const astring &app_name_in, int_set &pids)
{
  FUNCDEF("find_process");
  pids.clear();
  process_entry_array processes;
  if (!get_processes(processes)) return false;
  return process_control::find_process_in_list(processes, app_name_in, pids);
}

outcome launch_manager::zap_process(const astring &product,
    const astring &app_name_key, bool graceful)
{
  FUNCDEF("zap_process");

#ifdef DEBUG_PROCESS_MANAGER
  LOG(astring("product \"") + product + "\", application \"" + app_name_key
      + (graceful? "\", Graceful Close" : "\", Brutal Close"));
#endif

  if (_tracking_exclusions->member(app_name_key)) {
    // the non-tracked applications are never reported as running since they're
    // not allowed to be zapped anyhow.
    return NOT_RUNNING;
  }

  // use the real program name from here onward.
  astring app_name;
  {
    LOCK_CONFIG;

    // get the specific entry for the program they want.
    int level;
    app_name = _configs.find_program(product, app_name_key, level);
    if (!app_name) {
      if (!_configs.product_exists(product)) {
        // return more specific error for missing product.
        COMPLAIN_PRODUCT;
        return NO_PRODUCT;
      }
      COMPLAIN_APPLICATION;
      return NO_APPLICATION;
    }
    // truncate the directory and just use the base.
    app_name = filename(app_name).basename();
  }

  // if they want a graceful close, start by trying that.  if it appears to
  // have succeeded, then we exit and let the thread take care of ensuring
  // the app goes down.
  outcome to_return = NOT_RUNNING;
  if (graceful)
    to_return = start_graceful_close(product, app_name);
  if (to_return == OKAY)
    return OKAY;  // maybe finished the close.

  // they want a harsh close of this application or the graceful close failed.
  astring program_name(app_name);
  int_set pids;
  if (!find_process(program_name, pids)) {
#ifdef DEBUG_PROCESS_MANAGER
    LOG(program_name + " process was not running.")
#endif
    return NOT_RUNNING;
  }

  // search for the application in the process list.
  bool failed = false;
  for (int i = 0; i < pids.elements(); i++) {
    bool ret = _procs->zap_process(pids[i]);
    if (ret) {
      LOG(astring(astring::SPRINTF, "Killed process %d [",
          pids[i]) + program_name + astring("]"));
    } else {
      LOG(astring(astring::SPRINTF, "Failed to zap process %d [",
          pids[i]) + program_name + astring("]"));
    }
    if (!ret) failed = true;
  }
// kind of a bizarre return, but whatever.  it really should be some
// failure result since the zap failed.
  return failed? ACCESS_DENIED : OKAY;
}

outcome launch_manager::shut_down_launching_services(const astring &secret_word)
{
  FUNCDEF("shut_down_launching_services");
  LOG("checking secret word...");
//hmmm: changing the secret word here.
  if (secret_word.equal_to("MarblesAreRoundishYo")) {
    LOG("it's correct; ending launch capabilities.");
  } else {
    LOG("the secret word is wrong.  continuing normal operation.");
    return BAD_PROGRAM;
  }
  _stop_launching = true;
  return OKAY;
}

outcome launch_manager::reenable_launching_services(const astring &secret_word)
{
  FUNCDEF("reenable_launching_services");
  LOG("checking secret word...");
  if (secret_word.equal_to("MarblesAreRoundishYo")) {
    LOG("it's correct; resuming launch capabilities.");
  } else {
    LOG("the secret word is wrong.  continuing with prior mode.");
    return BAD_PROGRAM;
  }
  _stop_launching = false;
  return OKAY;
}

#define GET_PROCESSES \
  if (!retrieved_processes) { \
    retrieved_processes = true; \
    if (!get_processes(processes)) { \
      LOG("failed to retrieve process list from OS!"); \
      return;  /* badness. */ \
    } \
  }

void launch_manager::push_timed_activities(process_entry_array &processes)
{
  FUNCDEF("push_timed_activities");

  // make sure we started the applications that were slated for execution at
  // system startup time.  we wait on this until the first thread activation
  // to give other processes some breathing room right at startup time.
  // also, it then doesn't block the main service thread.
  if (!_started_initial_apps && (*_startup_time <= time_stamp()) ) {
    // launch all the apps that are listed for system startup.
    LOG("starting up the tasks registered for system initiation.");
    launch_startup_apps();
    _started_initial_apps = true;
  }

  if (!_started_initial_apps) {
    _checker->reschedule(200);
      // keep hitting this function until we know we can relax since the
      // startup apps have been sent off.
  }

  bool retrieved_processes = false;

  {
    // loop over the death records we've got and check on the soon to be gone.
    LOCK_ZOMBIES;

    for (int i = _going_down->length() - 1; i >= 0; i--) {
      graceful_record &grace = (*_going_down)[i];

      GET_PROCESSES;  // load them if they hadn't been.

      int_set pids;
      if (!process_control::find_process_in_list(processes, grace._app_name,
          pids)) {
        // the app can't be found as running, so whack the record for it.
#ifdef DEBUG_PROCESS_MANAGER
        LOG(astring("cannot find app ") + grace._app_name
            + " as running still; removing its dying record");
#endif
        _going_down->zap(i, i);
        continue;
      }
      if (!pids.member(grace._pid)) {
        // that particular instance exited on its own, so whack the record.
#ifdef DEBUG_PROCESS_MANAGER
        LOG(astring("app ") + grace._app_name
            + " exited on its own; removing its dying record");
#endif
        _going_down->zap(i, i);
        continue;
      }

      if (grace._started <= time_stamp(-GRACEFUL_SLACK)) {
        // time to force it.
        LOG(astring("Application ") + grace._app_name + " is unresponsive to "
            "the graceful shutdown request.  Now zapping it instead.");
        if (!_procs->zap_process(grace._pid))
          LOG("Devolved graceful shutdown failed as zap_process also.");
        _going_down->zap(i, i);
        continue;
      }

      // it's not time to dump this one yet, so keep looking at others.
    }
  }

  {
    // now loop over the list of our active kids and make sure that they are
    // all still running.  if they aren't, then toss the record out.
    LOCK_KIDS;

    for (int i = _our_kids->length() - 1; i >= 0; i--) {
      graceful_record &grace = (*_our_kids)[i];

      GET_PROCESSES;  // load them if they hadn't been.

      int_set pids;
      if (!process_control::find_process_in_list(processes, grace._app_name,
          pids)) {
        // the app can't be found as running, so whack the record for it.
#ifdef DEBUG_PROCESS_MANAGER
        LOG(astring("cannot find kid ") + grace._app_name
            + " as still running; removing its normal record");
#endif
        _our_kids->zap(i, i);
        continue;
      }
      if (!pids.member(grace._pid)) {
        // that particular instance exited on its own, so whack the record.
#ifdef DEBUG_PROCESS_MANAGER
        LOG(astring("kid ") + grace._app_name
            + " exited on its own; removing its normal record");
#endif
        _our_kids->zap(i, i);
        continue;
      }

      // this kid is still going, so keep its record.
    }
  }
}

} //namespace.

