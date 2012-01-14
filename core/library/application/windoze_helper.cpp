/*****************************************************************************\
*                                                                             *
*  Name   : windoze_helper                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1994-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "windoze_helper.h"
///#include "array.h"
///#include "build_configuration.h"
///#include "convert_utf.h"
///#include "function.h"
///#include "mutex.h"
///#include "portable.h"
///#include "set.h"
///#include "version_record.h"
#include <configuration/application_configuration.h>

///#include <errno.h>
///#include <stdlib.h>

/*
#ifdef __UNIX__
  #include <limits.h>
  #include <stdio.h>
  #include <string.h>
  #include <sys/poll.h>
  #include <sys/times.h>
  #include <sys/utsname.h>
  #include <sys/wait.h>
  #include <time.h>
  #include <unistd.h>
#endif
#ifdef __XWINDOWS__
//hmmm: need code for the wait cursor stuff.
#endif
#ifdef __WIN32__
  #include <mmsystem.h>
  #include <process.h>
  #include <shellapi.h>
  #include <shlobj.h>
#endif
*/

using namespace basis;
using namespace structures;
using namespace configuration;

#undef static_class_name
#define static_class_name() "windoze_helper"

/*
//#define DEBUG_PORTABLE
  // uncomment for noisy debugging.

//#define DEBUG_UPTIME
  // uncomment to get noisier reporting about system and rolling uptime.

//#define JUMP_TIME_49_DAYS
  // uncomment to make our uptimes start just before the 32 bit uptime rollover.
//#define JUMP_TIME_497_DAYS
  // uncomment to make our uptimes start just before the jiffies rollover.

//#define ENABLE_ROLLOVER_BUG
  // uncomment to get old behavior back where the uptime was not rolling
  // over properly.  this turns off our remainder calculation and leaves the
  // conversion as a simple cast, which will fail and get stuck at 2^32-1.

#define SUPPORT_SHELL_EXECUTE
  // if this is not commented out, then the ShellExecute version of launch_
  // -process() is available.  when commented out, ShellExecute is turned off.
  // disabling this support is the most common because the ShellExecute method
  // in win32 was only supported for wk203 and wxp, that is only after
  // windows2000 was already available.  since nt and w2k don't support this,
  // we just usually don't mess with it.  it didn't answer a single one of our
  // issues on windows vista (wfista) anyway, so it's not helping.

// ensure we always have debugging turned on if the jump is enabled.
#ifdef JUMP_TIME_49_DAYS
  #undef DEBUG_UPTIME
  #define DEBUG_UPTIME
#endif
#ifdef JUMP_TIME_497_DAYS
  #undef DEBUG_UPTIME
  #define DEBUG_UPTIME
#endif
// the JUMP..DAYS macros are mutually exclusive.  use none or one, not both.
#ifdef JUMP_TIME_497_DAYS
  #ifdef JUMP_TIME_49_DAYS
    #error One cannot use both 497 day and 49 day bug inducers
  #endif
#endif
#ifdef JUMP_TIME_49_DAYS
  #ifdef JUMP_TIME_497_DAYS
    #error One cannot use both 49 day and 497 day bug inducers
  #endif
#endif
*/


namespace application {

/*
mutex BASIS_EXTERN &__uptime_synchronizer();
  // used by our low-level uptime methods to protect singleton variables.
mutex BASIS_EXTERN &__process_synchronizer();
  // used for synchronizing our records of child processes.
#ifdef __UNIX__
int_set BASIS_EXTERN &__our_kids();
  // the static list of processes we've started.
#endif

const int MAXIMUM_COMMAND_LINE = 32 * KILOBYTE;
  // maximum command line that we'll deal with here.

#ifdef __UNIX__
  const char *UPTIME_REPORT_FILE = "/tmp/uptime_report.log";
#endif
#ifdef __WIN32__
  const char *UPTIME_REPORT_FILE = "c:/uptime_report.log";
#endif

#undef LOG
#define LOG(s) STAMPED_EMERGENCY_LOG(program_wide_logger(), s);

#define COMPLAIN(to_print) { \
  guards::write_to_console((isprintf("basis/portable::%s: ", func) + to_print).s()); \
}

static const double __rollover_point = 2.0 * double(MAXINT);
  // this number is our rollover point for 32 bit integers.

double rolling_uptime()
{
  auto_synchronizer l(__uptime_synchronizer());
    // protect our rollover records.

  static u_int __last_ticks = 0;
  static int __rollovers = 0;

  u_int ticks_up = system_uptime();
    // acquire the current uptime as a 32 bit unsigned int.

  if (ticks_up < __last_ticks) {
    // rollover happened.  increment our tracker.
    __rollovers++;
  }
  __last_ticks = ticks_up;

  double to_return = double(__rollovers) * __rollover_point + double(ticks_up);

#ifdef DEBUG_UPTIME
  #ifdef __WIN32__
  static FILE *__outfile = fopen(UPTIME_REPORT_FILE, "a+b");
  static double old_value = 0;
  if (absolute_value(old_value - to_return) > 9.9999) {
    // only report when the time changes by more than 10 ms.
    fprintf(__outfile, "-> uptime=%.0f\n", to_return);
    fflush(__outfile);
    old_value = to_return;
    if (__rollover_point - to_return <= 40.00001) {
      fprintf(__outfile, "---> MAXIMUM UPTIME SOON!\n");
      fflush(__outfile);
    }
  }
  #endif
#endif

  return to_return;
}

u_int system_uptime()
{
#ifdef __WIN32__
  return timeGetTime();
#else
  auto_synchronizer l(__uptime_synchronizer());

  static clock_t __ctps = sysconf(_SC_CLK_TCK);  // clock ticks per second.
  static const double __multiplier = 1000.0 / double(__ctps);
    // the multiplier gives us our full range for the tick counter.

#ifdef DEBUG_UPTIME
  static FILE *__outfile = fopen(UPTIME_REPORT_FILE, "wb");
  if (__multiplier - u_int(__multiplier) > 0.000001) {
    fprintf(__outfile, "uptime multiplier is "
        "non-integral (%f)!\n", __multiplier);
    fflush(__outfile);
  }
#endif

  // read uptime info from the OS.
  tms uptime;
  u_int real_ticks = times(&uptime);

#ifdef JUMP_TIME_497_DAYS
  static u_int old_value_497 = 0;
  bool report_497 = (absolute_value(real_ticks - old_value_497) > 99);
  if (report_497) {
    old_value_497 = real_ticks;  // update before changing it.
    fprintf(__outfile, "pre kludge497 tix=%u\n", real_ticks);
    fflush(__outfile);
  }
  real_ticks += u_int(49.0 * double(DAY_ms) + 17.0 * double(HOUR_ms));
  if (report_497) {
    fprintf(__outfile, "post kludge497 tix=%u\n", real_ticks);
    fflush(__outfile);
  }
#endif

  // now turn this into the number of milliseconds.
  double ticks_up = (double)real_ticks;
  ticks_up = ticks_up * __multiplier;  // convert to time here.

#ifdef JUMP_TIME_497_DAYS
  #ifndef ENABLE_ROLLOVER_BUG
    // add in additional time so we don't have to wait forever.  we make the
    // big number above rollover, but that's still got 27.5 or so minutes before
    // we really rollover.  so we add that part of the fraction (lost in the
    // haze of the multiplier) in here.  we don't add this in unless they are
    // not exercising the rollover bug, because we already know that the 497
    // day bug will show without the addition.  but when we're already fixing
    // the uptime, we jump time a bit forward so we only have to wait a couple
    // minutes instead of 27.
    ticks_up += 25.0 * MINUTE_ms;
  #endif
#endif

#ifdef JUMP_TIME_49_DAYS
  static u_int old_value_49 = 0;
  bool report_49 = (absolute_value(u_int(ticks_up) - old_value_49) > 999);
  if (report_49) {
    old_value_49 = u_int(ticks_up);  // update before changing it.
    fprintf(__outfile, "pre kludge49 up=%f\n", ticks_up);
    fflush(__outfile);
  }
  ticks_up += 49.0 * double(DAY_ms) + 17.0 * double(HOUR_ms);
  if (report_49) {
    fprintf(__outfile, "post kludge49 up=%f\n", ticks_up);
    fflush(__outfile);
  }
#endif

#ifndef ENABLE_ROLLOVER_BUG
  // fix the return value if is has actually gone over the 2^32 limit for uint.
  // casting a double larger than 2*MAXINT to a u_int on some platforms does
  // not calculate a rolled-over value, but instead leaves the int at 2*MAXINT.
  // thus we make sure it will be correct, as long as there are no more than
  // 2^32-1 rollovers, which would be about 584,542 millenia.  it's unlikely
  // earth will last that long, so this calculation seems safe.
  u_int divided = u_int(ticks_up / __rollover_point);
  double to_return = ticks_up - (double(divided) * __rollover_point);
#else
  // we use the previous version of this calculation, which expected a u_int
  // to double conversion to provide a modulo operation rather than just leaving
  // the u_int at its maximum value (2^32-1).  however, that expectation is not
  // guaranteed on some platforms (e.g., ARM processor with floating point
  // emulation) and thus it becomes a bug around 49 days and 17 hours into
  // OS uptime because the value gets stuck at 2^32-1 and never rolls over.
  double to_return = ticks_up;
#endif

#ifdef DEBUG_UPTIME
  static u_int old_value = 0;
  int to_print = int(u_int(to_return));
  if (absolute_value(int(old_value) - to_print) > 9) {
    // only report when the time changes by more than 10 ms.
    fprintf(__outfile, "-> uptime=%u\n", to_print);
    fflush(__outfile);
    old_value = u_int(to_print);
    if (__rollover_point - to_return <= 40.00001) {
      fprintf(__outfile, "---> MAXIMUM UPTIME SOON!\n");
      fflush(__outfile);
    }
  }
#endif

  return u_int(to_return);
#endif
}

void sleep_ms(u_int msec)
{
#ifdef __UNIX__
  usleep(msec * 1000);
#endif
#ifdef __WIN32__
  Sleep(msec);
#endif
}

istring null_device()
{
#ifdef __WIN32__
  return "null:";
#else
  return "/dev/null";
#endif
}

#ifdef __WIN32__
bool event_poll(MSG &message)
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

// makes a complaint about a failure.
#ifndef EMBEDDED_BUILD
  #define COMPLAIN_QUERY(to_print) { \
    unlink(tmpfile.s()); \
    COMPLAIN(to_print); \
  }
#else
  #define COMPLAIN_QUERY(to_print) { \
    COMPLAIN(to_print); \
  }
#endif

#ifdef __UNIX__
istring get_cmdline_from_proc()
{
  FUNCDEF("get_cmdline_from_proc");
  isprintf cmds_filename("/proc/%d/cmdline", portable::process_id());
  FILE *cmds_file = fopen(cmds_filename.s(), "r");
  if (!cmds_file) {
    COMPLAIN("failed to open our process's command line file.\n");
    return "unknown";
  }
  size_t size = 2000;
  char *buff = new char[size + 1];
  ssize_t chars_read = getline(&buff, &size, cmds_file);
    // read the first line, giving ample space for how long it might be.
  fclose(cmds_file);  // drop the file again.
  if (!chars_read || negative(chars_read)) {
    COMPLAIN("failed to get any characters from our process's cmdline file.\n");
    return "unknown";
  }
  istring buffer = buff;
  delete [] buff;
  // clean out quote characters from the name.
  for (int i = buffer.length() - 1; i >= 0; i--) {
    if (buffer[i] == '"') buffer.zap(i, i);
  }
  return buffer;
}

// deprecated; better to use the /proc/pid/cmdline file.
istring query_for_process_info()
{
  FUNCDEF("query_for_process_info");
  istring to_return = "unknown";
  // we ask the operating system about our process identifier and store
  // the results in a temporary file.
  chaos rando;
  isprintf tmpfile("/tmp/proc_name_check_%d_%d.txt", portable::process_id(),
      rando.inclusive(0, 128000));
  isprintf cmd("ps h --format \"%%a\" %d >%s", portable::process_id(),
      tmpfile.s());
  // run the command to locate our process info.
  int sysret = system(cmd.s());
  if (negative(sysret)) {
    COMPLAIN_QUERY("failed to run ps command to get process info");
    return to_return;
  }
  // open the output file for reading.
  FILE *output = fopen(tmpfile.s(), "r");
  if (!output) {
    COMPLAIN_QUERY("failed to open the ps output file");
    return to_return;
  }
  // read the file's contents into a string buffer.
  char buff[MAXIMUM_COMMAND_LINE];
  size_t size_read = fread(buff, 1, MAXIMUM_COMMAND_LINE, output);
  if (size_read > 0) {
    // success at finding some text in the file at least.
    while (size_read > 0) {
      const char to_check = buff[size_read - 1];
      if ( !to_check || (to_check == '\r') || (to_check == '\n')
          || (to_check == '\t') )
        size_read--;
      else break;
    }
    to_return.reset(istring::UNTERMINATED, buff, size_read);
  } else {
    // couldn't read anything.
    COMPLAIN_QUERY("could not read output of process list");
  }
  unlink(tmpfile.s());
  return to_return;
}
#endif

// used as a return value when the name cannot be determined.
#ifndef EMBEDDED_BUILD
  #define SET_BOGUS_NAME(error) { \
    COMPLAIN(error); \
    if (output) { \
      fclose(output); \
      unlink(tmpfile.s()); \
    } \
    istring home_dir = env_string("HOME"); \
    to_return = home_dir + "/failed_to_determine.exe"; \
  }
#else
  #define SET_BOGUS_NAME(error) { \
    COMPLAIN(error); \
    to_return = "unknown"; \
  }
#endif

istring application_name()
{
  FUNCDEF("application_name");
  istring to_return;
#ifdef __UNIX__
  to_return = get_cmdline_from_proc();
#elif defined(__WIN32__)
  flexichar low_buff[MAX_ABS_PATH + 1];
  GetModuleFileName(NIL, low_buff, MAX_ABS_PATH - 1);
  istring buff = from_unicode_temp(low_buff);
  buff.to_lower();  // we lower-case the name since windows seems to UC it.
  to_return = buff;
#elif defined(EMBEDDED_BUILD)
  SET_BOGUS_NAME("embedded_exe");
#else
  #pragma error("hmmm: no means of finding app name is implemented.")
  SET_BOGUS_NAME("not_implemented_for_this_OS");
#endif
  return to_return;
}

istring module_name(const void *module_handle)
{
#ifdef __UNIX__
//hmmm: implement module name for linux if that makes sense.
  if (module_handle) {}
  return application_name();
#elif defined(__WIN32__)
  flexichar low_buff[MAX_ABS_PATH + 1];
  GetModuleFileName((HMODULE)module_handle, low_buff, MAX_ABS_PATH - 1);
  istring buff = from_unicode_temp(low_buff);
  buff.to_lower();
  return buff;
#else
  #pragma message("module_name unknown for this operating system.")
  return application_name();
#endif
}

u_int system_error()
{
#if defined(__UNIX__)
  return errno;
#elif defined(__WIN32__)
  return GetLastError();
#else
  #pragma error("hmmm: no code for error number for this operating system")
  return 0;
#endif
}

istring system_error_text(u_int to_name)
{
#if defined(__UNIX__)
  return strerror(to_name);
#elif defined(__WIN32__)
  char error_text[1000];
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NIL, to_name,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)error_text,
      sizeof(error_text) - 1, NIL);
  istring to_return = error_text;
  // trim off the ridiculous carriage return they add.
  while ( (to_return[to_return.end()] == '\r')
      || (to_return[to_return.end()] == '\n') )
    to_return.zap(to_return.end(), to_return.end());
  return to_return;
#else
  #pragma error("hmmm: no code for error text for this operating system")
  return "";
#endif
}

#ifdef __WIN32__

bool is_address_valid(const void *address, int size_expected, bool writable)
{
  return address && !IsBadReadPtr(address, size_expected)
      && !(writable && IsBadWritePtr((void *)address, size_expected));
}

#endif // win32

version get_OS_version()
{
  version to_return;
#ifdef __UNIX__
  utsname kernel_parms;
  uname(&kernel_parms);
  to_return = version(kernel_parms.release);
#elif defined(__WIN32__)
  OSVERSIONINFO info;
  info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  ::GetVersionEx(&info);
  to_return = version(isprintf("%u.%u.%u.%u", u_short(info.dwMajorVersion),
      u_short(info.dwMinorVersion), u_short(info.dwPlatformId),
      u_short(info.dwBuildNumber)));
#elif defined(EMBEDDED_BUILD)
  // no version support.
#else
  #pragma error("hmmm: need version info for this OS!")
#endif
  return to_return;
}

// for non-win32 and non-unix OSes, this might not work.
#if defined(__UNIX__) || defined(__WIN32__)
  u_int process_id() { return getpid(); }
#else
  #pragma error("hmmm: need process id implementation for this OS!")
  u_int process_id() { return 0; }
#endif

istring current_directory()
{
  istring to_return;
#ifdef __UNIX__
  char buff[MAX_ABS_PATH];
  getcwd(buff, MAX_ABS_PATH - 1);
  to_return = buff;
#elif defined(__WIN32__)
  flexichar low_buff[MAX_ABS_PATH + 1];
  GetCurrentDirectory(MAX_ABS_PATH, low_buff);
  to_return = from_unicode_temp(low_buff);
#else
  #pragma error("hmmm: need support for current directory on this OS.")
  to_return = ".";
#endif
  return to_return;
}

istring env_string(const istring &variable_name)
{
#ifdef __WIN32__
  char *value = getenv(variable_name.upper().observe());
    // dos & os/2 require upper case for the name, so we just do it that way.
#else
  char *value = getenv(variable_name.observe());
    // reasonable OSes support mixed-case environment variables.
#endif
  istring to_return;
  if (value)
    to_return = istring(value);
  return to_return;
}

bool set_environ(const istring &variable_name, const istring &value)
{
  int ret = 0;
#ifdef __WIN32__
  ret = putenv((variable_name + "=" + value).s());
#else
  ret = setenv(variable_name.s(), value.s(), true);
#endif
  return !ret;
}

timeval fill_timeval_ms(int duration)
{
  timeval time_out;  // timeval has tv_sec=seconds, tv_usec=microseconds.
  if (!duration) {
    // duration is immediate for the check; just a quick poll.
    time_out.tv_sec = 0;
    time_out.tv_usec = 0;
#ifdef DEBUG_PORTABLE
//    LOG("no duration specified");
#endif
  } else {
    // a non-zero duration means we need to compute secs and usecs.
    time_out.tv_sec = duration / 1000;
    // set the number of seconds from the input in milliseconds.
    duration -= time_out.tv_sec * 1000;
    // now take out the chunk we've already recorded as seconds.
    time_out.tv_usec = duration * 1000;
    // set the number of microseconds from the remaining milliseconds.
#ifdef DEBUG_PORTABLE
//    LOG(isprintf("duration of %d ms went to %d sec and %d usec.", duration,
//        time_out.tv_sec, time_out.tv_usec));
#endif
  }
  return time_out;
}

//hmmm: this doesn't seem to account for quoting properly at all?
basis::char_star_array break_line(istring &app, const istring &parameters)
{
  basis::char_star_array to_return;
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
#ifdef DEBUG_PORTABLE
  for (int q = 0; to_return[q]; q++) {
    printf("%d: %s\n", q, to_return[q]);
  }
#endif
  // now a special detour; fix the app name to remove quotes, which are
  // not friendly to pass to exec.
  if (app[0] == '"') app.zap(0, 0);
  if (app[app.end()] == '"') app.zap(app.end(), app.end());
  return to_return;
}

#ifdef __UNIX__

void exiting_child_signal_handler(int sig_num)
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
#ifdef DEBUG_PORTABLE
      printf("unknown result %d waiting for process %d", exited,
          __our_kids()[i]);
#endif
    }
  }
}
#endif

u_int launch_process(const istring &app_name_in, const istring &command_line,
    int flag, u_int &child_id)
{
  child_id = 0;
  istring app_name = app_name_in;
  if (app_name[0] != '"')
    app_name.insert(0, "\"");
  if (app_name[app_name.end()] != '"')
    app_name += "\"";
#ifdef __UNIX__
  // unix / linux implementation.
  if (flag & RETURN_IMMEDIATELY) {
    // they want to get back right away.
    pid_t kid_pid = fork();
#ifdef DEBUG_PORTABLE
    printf("launch fork returned %d\n", kid_pid);
#endif
    if (!kid_pid) {
      // this is the child; we now need to launch into what we were asked for.
#ifdef DEBUG_PORTABLE
      printf((isprintf("process %d execing ", process_id()) + app_name
          + " parms " + command_line + "\n").s());
#endif
      basis::char_star_array parms = break_line(app_name, command_line);
      execv(app_name.s(), parms.observe());
      // oops.  failed to exec if we got to here.
#ifdef DEBUG_PORTABLE
      printf((isprintf("child of fork (pid %d) failed to exec, "
          "error is ", process_id()) + system_error_text(system_error())
          + "\n").s());
#endif
      exit(0);  // leave since this is a failed child process.
    } else {
      // this is the parent.  let's see if the launch worked.
      if (kid_pid == -1) {
        // failure.
        u_int to_return = system_error();
#ifdef DEBUG_PORTABLE
        printf((isprintf("parent %d is returning after failing to create, "
            "error is ", process_id()) + system_error_text(to_return)
            + "\n").s());
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

#ifdef DEBUG_PORTABLE
        printf((isprintf("parent %d is returning after successfully "
            "creating %d ", process_id(), kid_pid) + app_name
            + " parms " + command_line + "\n").s());
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
//  MessageBox(0, (istring("IS admin with ") + app_name_in + " " + command_line).s(), "launch process", MB_OK);
//} else {
//  MessageBox(0, (istring("NOT admin for ") + app_name_in + " " + command_line).s(), "launch process", MB_OK);
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
      return system_error();
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
    istring parms = app_name + " " + command_line;
    bool success = CreateProcess(NIL, to_unicode_temp(parms), NIL, NIL, false,
        create_flag, NIL, NIL, &startup_info, &process_info);
    if (!success)
      return system_error();
    // success then, merge back into stream.

#ifdef SUPPORT_SHELL_EXECUTE
  }
#endif //shell exec

  // common handling for CreateProcess and ShellExecuteEx.
  child_id = process_info.dwProcessId;
  u_long retval = 0;
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
      sleep_ms(14);
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
  return (u_int)retval;
#else
  #pragma error("hmmm: launch_process: no implementation for this OS.")
#endif
  return 0;
}

////////////////////////////////////////////////////////////////////////////

#ifdef __UNIX__

char *itoa(int to_convert, char *buffer, int radix)
{
  // slow implementation; may need enhancement for serious speed.
  // ALSO: only supports base 10 and base 16 currently.
  const char *formatter = "%d";
  if (radix == 16) formatter = "%x";
  isprintf printed(formatter, to_convert);
  printed.stuff(buffer, printed.length() + 1);
  return buffer;
}

#endif
*/

#ifdef __WIN32__

/*
void show_wait_cursor() { SetCursor(LoadCursor(NULL, IDC_WAIT)); }

void show_normal_cursor() { SetCursor(LoadCursor(NULL, IDC_ARROW)); }

istring rc_string(UINT id, application_instance instance)
{
  flexichar temp[MAX_ABS_PATH + 1];
  int ret = LoadString(instance, id, temp, MAX_ABS_PATH);
  if (!ret) return istring();
  return istring(from_unicode_temp(temp));
}

istring rc_string(u_int id) { return rc_string(id, GET_INSTANCE_HANDLE()); }
*/

const char *opsystem_name(known_operating_systems which)
{
  switch (which) {
    case WIN_95: return "WIN_95";
    case WIN_NT: return "WIN_NT";
    case WIN_2K: return "WIN_2K";
    case WIN_XP: return "WIN_XP";
    case WIN_SRV2K3: return "WIN_SRV2K3";
    case WIN_VISTA: return "WIN_VISTA";
    case WIN_SRV2K8: return "WIN_SRV2K8";
    default: return "UNKNOWN_OS";
  }
}

known_operating_systems determine_OS()
{
  version osver = application_configuration::get_OS_version();
  if ( (osver.v_major() == 4) && (osver.v_minor() == 0) ) {
    if (osver.v_revision() == VER_PLATFORM_WIN32_WINDOWS) return WIN_95;
    if (osver.v_revision() == VER_PLATFORM_WIN32_NT) return WIN_NT;
  } else if ( (osver.v_major() == 5) && (osver.v_minor() == 0) ) {
    return WIN_2K;
  } else if ( (osver.v_major() == 5) && (osver.v_minor() == 1) ) {
    return WIN_XP;
  } else if ( (osver.v_major() == 5) && (osver.v_minor() == 2) ) {
    return WIN_SRV2K3;
  } else if ( (osver.v_major() == 6) && (osver.v_minor() == 0) ) {
    return WIN_VISTA;
  } else if ( (osver.v_major() == 6) && (osver.v_minor() == 1) ) {
    return WIN_SRV2K8;
  }
  return UNKNOWN_OS;
}

#endif // win32

} // namespace.

#undef static_class_name

