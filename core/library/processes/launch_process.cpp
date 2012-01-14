
// Name   : launch_process
// Author : Chris Koeritz
/******************************************************************************
* Copyright (c) 1994-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "launch_process.h"

#include <application/windoze_helper.h>
#include <basis/utf_conversion.h>
#include <basis/mutex.h>
#include <configuration/application_configuration.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <structures/set.h>
#include <timely/time_control.h>

#include <stdlib.h>
#ifdef __UNIX__
  #include <signal.h>
  #include <sys/types.h>
  #include <sys/wait.h>
  #include <unistd.h>
#endif
#ifdef __WIN32__
  #include <process.h>
  #include <shellapi.h>
  #include <shlobj.h>
#endif

//#define DEBUG_LAUNCH_PROCESS
  // uncomment for noisier debugging info.

using namespace basis;
using namespace configuration;
using namespace loggers;
using namespace structures;
using namespace timely;

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s);

namespace processes {

//hmmm: some of these should probably be safe statics.

mutex &__process_synchronizer() {
  static mutex __hidden_synch;
  return __hidden_synch;
}

int_set __our_kids() {
  static int_set __hidden_kids;
  return __hidden_kids;
}

#ifdef __WIN32__
bool launch_process::event_poll(MSG &message)
{
  message.hwnd = 0;
  message.message = 0;
  message.wParam = 0;
  message.lParam = 0;
  if (!PeekMessage(&message, NIL, 0, 0, PM_REMOVE))
    return false;
  TranslateMessage(&message);
  DispatchMessage(&message);
  return true;
}
#endif

#define SUPPORT_SHELL_EXECUTE
  // if this is not commented out, then the ShellExecute version of launch_
  // -process() is available.  when commented out, ShellExecute is turned off.
  // disabling this support is the most common because the ShellExecute method
  // in win32 was only supported for wk203 and wxp, that is only after
  // windows2000 was already available.  since nt and w2k don't support this,
  // we just usually don't mess with it.  it didn't answer a single one of our
  // issues on windows vista (wfista) anyway, so it's not helping.

//const int MAXIMUM_COMMAND_LINE = 32 * KILOBYTE;
  // maximum command line that we'll deal with here.

#ifdef __UNIX__
void launch_process::exiting_child_signal_handler(int sig_num)
{
  FUNCDEF("exiting_child_signal_handler");
  if (sig_num != SIGCHLD) {
    // uhhh, this seems wrong.
  }
  auto_synchronizer l(__process_synchronizer());
  for (int i = 0; i < __our_kids().length(); i++) {
    int status;
    pid_t exited = waitpid(__our_kids()[i], &status, WNOHANG);
    if ( (exited == -1) || (exited == __our_kids()[i]) ) {
      // negative one denotes an error, which we are going to assume means the
      // process has exited via some other method than our wait.  if the value
      // is the same as the process we waited for, that means it exited.
      __our_kids().zap(i, i);
      i--;
    } else if (exited != 0) {
      // zero would be okay; this result we do not understand.
#ifdef DEBUG_LAUNCH_PROCESS
      LOG(a_sprintf("unknown result %d waiting for process %d", exited, __our_kids()[i]));
#endif
    }
  }
}
#endif

//hmmm: this doesn't seem to account for quoting properly at all?
char_star_array launch_process::break_line(astring &app, const astring &parameters)
{
  FUNCDEF("break_line");
  char_star_array to_return;
  int_array posns;
  int num = 0;
  // find the positions of the spaces and count them.
  for (int j = 0; j < parameters.length(); j++) {
    if (parameters[j] == ' ') {
      num++;
      posns += j;
    }
  }
  // first, add the app name to the list of parms.
  to_return += new char[app.length() + 1];
  app.stuff(to_return[0], app.length());
  int last_posn = 0;
  // now add each space-separated parameter to the list.
  for (int i = 0; i < num; i++) {
    int len = posns[i] - last_posn;
    to_return += new char[len + 1];
    parameters.substring(last_posn, posns[i] - 1).stuff(to_return[i + 1], len);
    last_posn = posns[i] + 1;
  }
  // catch anything left after last separator.
  if (last_posn < parameters.length() - 1) {
    int len = parameters.length() - last_posn;
    to_return += new char[len + 1];
    parameters.substring(last_posn, parameters.length() - 1)
        .stuff(to_return[to_return.last()], len);
  }
  // add the sentinel to the list of strings.
  to_return += NIL;
#ifdef DEBUG_LAUNCH_PROCESS
  for (int q = 0; to_return[q]; q++) {
    LOG(a_sprintf("%d: %s\n", q, to_return[q]));
  }
#endif
  // now a special detour; fix the app name to remove quotes, which are
  // not friendly to pass to exec.
  if (app[0] == '"') app.zap(0, 0);
  if (app[app.end()] == '"') app.zap(app.end(), app.end());
  return to_return;
}

basis::un_int launch_process::run(const astring &app_name_in, const astring &command_line,
    int flag, basis::un_int &child_id)
{
#ifdef DEBUG_LAUNCH_PROCESS
  FUNCDEF("run");
#endif
  child_id = 0;
  astring app_name = app_name_in;
  if (app_name[0] != '"')
    app_name.insert(0, "\"");
  if (app_name[app_name.end()] != '"')
    app_name += "\"";
#ifdef __UNIX__
  // unix / linux implementation.
  if (flag & RETURN_IMMEDIATELY) {
    // they want to get back right away.
    pid_t kid_pid = fork();
#ifdef DEBUG_LAUNCH_PROCESS
    LOG(a_sprintf("launch fork returned %d\n", kid_pid));
#endif
    if (!kid_pid) {
      // this is the child; we now need to launch into what we were asked for.
#ifdef DEBUG_LAUNCH_PROCESS
      LOG(a_sprintf("process %d execing ", application_configuration::process_id()) + app_name
          + " parms " + command_line + "\n");
#endif
      char_star_array parms = break_line(app_name, command_line);
      execv(app_name.s(), parms.observe());
      // oops.  failed to exec if we got to here.
#ifdef DEBUG_LAUNCH_PROCESS
      LOG(a_sprintf("child of fork (pid %d) failed to exec, error is ",
          application_configuration::process_id())
          + critical_events::system_error_text(critical_events::system_error())
          + "\n");
#endif
      exit(0);  // leave since this is a failed child process.
    } else {
      // this is the parent.  let's see if the launch worked.
      if (kid_pid == -1) {
        // failure.
        basis::un_int to_return = critical_events::system_error();
#ifdef DEBUG_LAUNCH_PROCESS
        LOG(a_sprintf("parent %d is returning after failing to create, "
            "error is ", application_configuration::process_id())
            + critical_events::system_error_text(to_return)
            + "\n");
#endif
        return to_return;
      } else {
        // yes, launch worked okay.
        child_id = kid_pid;
        {
          auto_synchronizer l(__process_synchronizer());
          __our_kids() += kid_pid;
        }
        // hook in our child exit signal handler.
        signal(SIGCHLD, exiting_child_signal_handler);

#ifdef DEBUG_LAUNCH_PROCESS
        LOG(a_sprintf("parent %d is returning after successfully "
            "creating %d ", application_configuration::process_id(), kid_pid) + app_name
            + " parms " + command_line + "\n");
#endif
        return 0;
      }
    }
  } else {
    // assume they want to wait.
    return system((app_name + " " + command_line).s());
  }
#elif defined(__WIN32__)

//checking on whether we have admin rights for the launch.
//if (IsUserAnAdmin()) {
//  MessageBox(0, (astring("IS admin with ") + app_name_in + " " + command_line).s(), "launch process", MB_OK);
//} else {
//  MessageBox(0, (astring("NOT admin for ") + app_name_in + " " + command_line).s(), "launch process", MB_OK);
//}

  PROCESS_INFORMATION process_info;
  ZeroMemory(&process_info, sizeof(PROCESS_INFORMATION));

#ifdef SUPPORT_SHELL_EXECUTE
  if (flag & SHELL_EXECUTE) {
    // new code for using shell execute method--required on vista for proper
    // launching with the right security tokens.
    int show_cmd = 0;
    if (flag & HIDE_APP_WINDOW) {
      // magic that hides a console window for mswindows.
      show_cmd = SW_HIDE;
    } else {
      show_cmd = SW_SHOWNORMAL;
    }

    SHELLEXECUTEINFO exec_info;
    ZeroMemory(&exec_info, sizeof(SHELLEXECUTEINFO));
    exec_info.cbSize = sizeof(SHELLEXECUTEINFO);
    exec_info.fMask = SEE_MASK_NOCLOSEPROCESS  // get the process info.
        | SEE_MASK_FLAG_NO_UI;  // turn off any visible error dialogs.
    exec_info.hwnd = GetDesktopWindow();
//hmmm: is get desktop window always appropriate?
    to_unicode_persist(temp_verb, "open");
//does "runas" work on xp also?  or anywhere?
    exec_info.lpVerb = temp_verb;
    to_unicode_persist(temp_file, app_name);
    exec_info.lpFile = temp_file;
    to_unicode_persist(temp_parms, command_line);
    exec_info.lpParameters = temp_parms;
    exec_info.nShow = show_cmd;
//    exec_info.hProcess = &process_info;

    BOOL worked = ShellExecuteEx(&exec_info);
    if (!worked)
      return critical_events::system_error();
	// copy out the returned process handle.
    process_info.hProcess = exec_info.hProcess;
    process_info.dwProcessId = GetProcessId(exec_info.hProcess);
  } else {
#endif //shell exec
    // standard windows implementation using CreateProcess.
    STARTUPINFO startup_info;
    ZeroMemory(&startup_info, sizeof(STARTUPINFO));
    startup_info.cb = sizeof(STARTUPINFO);
    int create_flag = 0;
    if (flag & HIDE_APP_WINDOW) {
      // magic that hides a console window for mswindows.
//      version ver = portable::get_OS_version();
//      version vista_version(6, 0);
//      if (ver < vista_version) {
//        // we suspect that this flag is hosing us in vista.
        create_flag = CREATE_NO_WINDOW;
//      }
    }
    astring parms = app_name + " " + command_line;
    bool success = CreateProcess(NIL, to_unicode_temp(parms), NIL, NIL, false,
        create_flag, NIL, NIL, &startup_info, &process_info);
    if (!success)
      return critical_events::system_error();
    // success then, merge back into stream.

#ifdef SUPPORT_SHELL_EXECUTE
  }
#endif //shell exec

  // common handling for CreateProcess and ShellExecuteEx.
  child_id = process_info.dwProcessId;
  basis::un_long retval = 0;
  if (flag & AWAIT_VIA_POLLING) {
    // this type of waiting is done without blocking on the process.
    while (true) {
      MSG msg;
      event_poll(msg);
      // check if the process is gone yet.
      BOOL ret = GetExitCodeProcess(process_info.hProcess, &retval);
      if (!ret) {
        break;
      } else {
        // if they aren't saying it's still active, then we will leave.
        if (retval != STILL_ACTIVE)
          break;
      }
      time_control::sleep_ms(14);
    }
  } else if (flag & AWAIT_APP_EXIT) {
    // they want to wait for the process to exit.
    WaitForInputIdle(process_info.hProcess, INFINITE); 
    WaitForSingleObject(process_info.hProcess, INFINITE);
    GetExitCodeProcess(process_info.hProcess, &retval);
  }
  // drop the process and thread handles.
  if (process_info.hProcess)
    CloseHandle(process_info.hProcess);
  if (process_info.hThread)
	CloseHandle(process_info.hThread);
  return (basis::un_int)retval;
#else
  #pragma error("hmmm: launch_process: no implementation for this OS.")
#endif
  return 0;
}

} // namespace.

