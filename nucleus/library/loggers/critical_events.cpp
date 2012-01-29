/*****************************************************************************\
*                                                                             *
*  Name   : critical_events                                                   *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1989-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "critical_events.h"
#include "program_wide_logger.h"

#include <application/application_shell.h>
#include <application/windoze_helper.h>
#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/mutex.h>
#include <configuration/application_configuration.h>
#include <structures/static_memory_gremlin.h>
#include <textual/parser_bits.h>
#include <timely/time_stamp.h>

#include <stdio.h>
#ifdef __UNIX__
  #include <errno.h>
#endif

using namespace basis;
using namespace structures;
using namespace configuration;
using namespace textual;
using namespace timely;

const int MESSAGE_SPACE_PROVIDED = 4096;
  // the strings should not be larger than this for diagnostic / error messages.

namespace loggers {

SAFE_STATIC(astring, critical_events::hidden_critical_events_dir, )
  // define the function that holds the directory string.

astring default_critical_location()
{
  // ensure that the critical events function logs to the appropriate place.
  return application_configuration::get_logging_directory();
}

SAFE_STATIC(mutex, __critical_event_dir_lock, )

basis::un_int critical_events::system_error()
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

astring critical_events::system_error_text(basis::un_int to_name)
{
#if defined(__UNIX__)
  return strerror(to_name);
#elif defined(__WIN32__)
  char error_text[1000];
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NIL, to_name,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)error_text,
      sizeof(error_text) - 1, NIL);
  astring to_return = error_text;
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

astring critical_events::critical_events_directory()
{
  static bool initted = false;
  if (!initted) {
    auto_synchronizer l(__critical_event_dir_lock());
    if (!initted) {
      set_critical_events_directory(default_critical_location());
      initted = true;
    }
  }
  return hidden_critical_events_dir();
}

void critical_events::set_critical_events_directory(const astring &directory)
{ hidden_critical_events_dir() = directory; }

void critical_events::write_to_critical_events(const char *to_write)
{
  astring filename = critical_events_directory();
  filename += "/runtime_issues.log";
  FILE *errfile = fopen(filename.s(), "ab");
  if (errfile) {
    astring app_name = application_configuration::application_name();
    int indy = app_name.find('/', app_name.end(), true);
    if (non_negative(indy)) app_name.zap(0, indy);
    indy = app_name.find('\\', app_name.end(), true);
    if (non_negative(indy)) app_name.zap(0, indy);
    fprintf(errfile, "%s [%s]:%s", time_stamp::notarize(true).s(),
        app_name.s(), parser_bits::platform_eol_to_chars());
    fprintf(errfile, "%s%s", to_write, parser_bits::platform_eol_to_chars());
    fclose(errfile);
  }
}

void critical_events::write_to_console(const char *guards_message_space)
{ fprintf(stderr, "%s", (char *)guards_message_space); fflush(stderr); }

void critical_events::alert_message(const char *info, const char *title)
{
  astring to_print;
  if (strlen(title)) {
    const char border = '=';
    to_print += astring(border, int(strlen(title)) + 4);
    to_print += parser_bits::platform_eol_to_chars();
    to_print += border;
    to_print += ' ';
    to_print += title;
    to_print += ' ';
    to_print += border;
    to_print += parser_bits::platform_eol_to_chars();
    to_print += astring(border, int(strlen(title)) + 4);
    to_print += parser_bits::platform_eol_to_chars();
  }

  to_print += info;
  program_wide_logger::get().log(to_print, ALWAYS_PRINT);
  fflush(NIL);  // flush all output streams.
}

void critical_events::alert_message(const astring &info) { alert_message(info.s()); }

void critical_events::alert_message(const astring &info, const astring &title)
{ alert_message(info.s(), title.s()); }

void critical_events::make_error_message(const char *file, int line,
    const char *error_class, const char *error_function,
    const char *info, char *guards_message_space)
{
  strcpy(guards_message_space, "\nProblem reported for \"");
//hmmm: only copy N chars of each into the space.
//      say 40 for class/function each, then the space - consumed for the
//      info.  get strlen for real on the class and function name to know
//      actual size for that computation.
  strcat(guards_message_space, error_class);
  strcat(guards_message_space, "::");
  strcat(guards_message_space, error_function);
  strcat(guards_message_space, "\"\n(invoked at line ");
  char line_num[20];
  sprintf(line_num, "%d", line);
  strcat(guards_message_space, line_num);
  strcat(guards_message_space, " in ");
  strcat(guards_message_space, file);
  strcat(guards_message_space, " at ");
  strcat(guards_message_space, time_stamp::notarize(false).s());
  strcat(guards_message_space, ")\n");
  strcat(guards_message_space, info);
  strcat(guards_message_space, "\n\n\n");
}

void critical_events::FL_deadly_error(const char *file, int line, const char *error_class,
    const char *error_function, const char *info)
{
  FL_continuable_error(file, line, error_class, error_function, info,
      "Deadly Error Information");
  CAUSE_BREAKPOINT;
  throw "deadly_error";
    // abort() is not as good an approach as throwing an exception.  aborts are
    // harder to track with some compilers, but all should be able to trap an
    // exception.
}

void critical_events::FL_deadly_error(const astring &file, int line,
    const astring &error_class, const astring &error_function,
    const astring &info)
{
  FL_deadly_error(file.s(), line, error_class.s(), error_function.s(),
      info.s()); 
}

void critical_events::FL_continuable_error_real(const char *file, int line,
    const char *error_class, const char *error_function, const char *info,
    const char *title)
{
  char guards_message_space[MESSAGE_SPACE_PROVIDED];
    // this routine could still fail, if the call stack is already jammed
    // against its barrier.  but if that's the case, even the simple function
    // call might fail.  in any case, we cannot deal with a stack overflow
    // type error using this function.  but we would rather deal with that
    // than make the space static since that would cause corruption when
    // two threads wrote errors at the same time.
  make_error_message(file, line, error_class, error_function, info,
      guards_message_space);
  alert_message(guards_message_space, title);
}

void critical_events::FL_continuable_error(const char *file, int line,
    const char *error_class, const char *error_function, const char *info,
    const char *title)
{
  FL_continuable_error_real(file, line, error_class, error_function, info, title);
}

void critical_events::FL_continuable_error(const astring &file, int line,
    const astring &error_class, const astring &error_function,
    const astring &info, const astring &title)
{
  FL_continuable_error_real(file.s(), line, error_class.s(),
      error_function.s(), info.s(), title.s());
}

void critical_events::FL_non_continuable_error(const char *file, int line,
    const char *error_class, const char *error_function, const char *info,
    const char *title)
{
  FL_continuable_error_real(file, line, error_class, error_function, info,
      title);
  exit(EXIT_FAILURE);  // let the outside world know that there was a problem.
}

void critical_events::FL_non_continuable_error(const astring &file, int line,
    const astring &error_class, const astring &error_function,
    const astring &info, const astring &title)
{
  FL_continuable_error_real(file.s(), line, error_class.s(),
      error_function.s(), info.s(), title.s());
  exit(EXIT_FAILURE);  // let the outside world know that there was a problem.
}

void critical_events::FL_console_error(const char *file, int line, const char *error_class,
    const char *error_function, const char *info)
{
  char guards_message_space[MESSAGE_SPACE_PROVIDED];
  make_error_message(file, line, error_class, error_function, info,
      guards_message_space);
  write_to_console(guards_message_space);
}

void critical_events::FL_out_of_memory_now(const char *file, int line,
    const char *the_class_name, const char *the_func)
{
  FL_non_continuable_error(file, line, the_class_name, the_func, 
      "Program stopped due to memory allocation failure.",
      "Out of Memory Now");
}

void critical_events::implement_bounds_halt(const char *the_class_name, const char *the_func,
    const char *value, const char *low, const char *high,
    const char *error_addition)
{
  char message[400];
  strcpy(message, "bounds error caught");
  strcat(message, error_addition);
  strcat(message, ":\r\n");
  strcat(message, value);
  strcat(message, " is not between ");
  strcat(message, low);
  strcat(message, " and ");
  strcat(message, high);
#ifdef ERRORS_ARE_FATAL
  deadly_error(the_class_name, the_func, message);
#else
  continuable_error(the_class_name, the_func, message);
#endif
}

} //namespace.

