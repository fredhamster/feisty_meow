//////////////
// Name   : environment
// Author : Chris Koeritz
//////////////
// Copyright (c) 1994-$now By Author.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation:
//     http://www.gnu.org/licenses/gpl.html
// or under the terms of the GNU Library license:
//     http://www.gnu.org/licenses/lgpl.html
// at your preference.  Those licenses describe your legal rights to this
// software, and no other rights or warranties apply.
// Please send updates for this code to: fred@gruntose.com -- Thanks, fred.
//////////////

#include "environment.h"

#include <stdlib.h>
#include <sys/types.h>
#ifdef __UNIX__
  #include <unistd.h>
  #include <sys/times.h>
#endif
#ifdef __WIN32__
  #define _WINSOCKAPI_  // make windows.h happy about winsock.
  #include <windows.h>
  #include <mmsystem.h>
#endif

namespace basis {

astring environment::TMP()
{
  const static astring TMP_VARIABLE_NAME("TMP");
  astring to_return = get(TMP_VARIABLE_NAME);
  if (!to_return) {
    // they did not see fit to set this in the environment.  let's make something up.
#ifdef __WIN32__
    // windows default does not necessarily exist.
    to_return = "c:/tmp";
#else
    // most reasonable OSes have a /tmp directory.
    to_return = "/tmp";
#endif
    if (!!to_return) set("TMP", to_return);
  }
  return to_return;
}

astring environment::get(const astring &variable_name)
{
#ifdef __WIN32__
  char *value = getenv(variable_name.upper().observe());
    // dos & os/2 require upper case for the name, so we just do it that way.
#else
  char *value = getenv(variable_name.observe());
    // reasonable OSes support mixed-case environment variables.
#endif
  astring to_return;
  if (value)
    to_return = astring(value);
  return to_return;
}

bool environment::set(const astring &variable_name, const astring &value)
{
  int ret = 0;
#ifdef __WIN32__
  astring assignment = variable_name + "=" + value;
  ret = _putenv(assignment.s());
#else
  ret = setenv(variable_name.s(), value.s(), true);
#endif
  return !ret;
}

basis::un_int environment::system_uptime()
{
#ifdef __WIN32__
  return timeGetTime();
#else
  static clock_t __ctps = sysconf(_SC_CLK_TCK);  // clock ticks per second.
  static const double __multiplier = 1000.0 / double(__ctps);
    // the multiplier gives us our full range for the tick counter.

  // read uptime info from the OS.
  tms uptime;
  basis::un_int real_ticks = times(&uptime);

  // now turn this into the number of milliseconds.
  double ticks_up = (double)real_ticks;
  ticks_up = ticks_up * __multiplier;  // convert to time here.

  // we use the previous version of this calculation, which expected a basis::u_int
  // to double conversion to provide a modulo operation rather than just leaving
  // the basis::un_int at its maximum value (2^32-1).  however, that expectation is not
  // guaranteed on some platforms (e.g., ARM processor with floating point
  // emulation) and thus it becomes a bug around 49 days and 17 hours into
  // OS uptime because the value gets stuck at 2^32-1 and never rolls over.
  return basis::un_int(ticks_up);
#endif
}

} //namespace.

