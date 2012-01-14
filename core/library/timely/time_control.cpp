// Name   : time_control
// Author : Chris Koeritz
/******************************************************************************
* Copyright (c) 1994-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "time_control.h"

#include <application/windoze_helper.h>
#include <basis/utf_conversion.h>

#include <time.h>
#if defined(__WIN32__) || defined(__UNIX__)
  #include <sys/timeb.h>
#endif
#ifdef __UNIX__
  #include <unistd.h>
#endif

using namespace basis;
using namespace structures;

namespace timely {

void time_control::sleep_ms(basis::un_int msec)
{
#ifdef __UNIX__
  usleep(msec * 1000);
#endif
#ifdef __WIN32__
  Sleep(msec);
#endif
}

bool time_control::set_time(const time_locus &new_time)
{
#ifdef __WIN32__
  SYSTEMTIME os_time;
  os_time.wYear = WORD(new_time.year);
  os_time.wMonth = new_time.month;
  os_time.wDayOfWeek = new_time.day_of_week;
  os_time.wDay = new_time.day_of_year;
  os_time.wHour = new_time.hour;
  os_time.wMinute = new_time.minute;
  os_time.wSecond = new_time.second;
  os_time.wMilliseconds = 0;  // currently unused.

  // get our process token for manipulation.
  HANDLE petoken;
  OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES
      | TOKEN_QUERY, &petoken);
  // get our 
//something or other
// identifier so we can adjust our privileges.
  LUID our_id;
  LookupPrivilegeValue(NULL, to_unicode_temp("SeSystemTimePrivilege"), &our_id);
  // make up a privilege structure for the adjustment.
  TOKEN_PRIVILEGES privs;
  privs.PrivilegeCount = 1;
  privs.Privileges[0].Luid = our_id;
  privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  // enable system-time privileges.
  AdjustTokenPrivileges(petoken, false, &privs, sizeof(TOKEN_PRIVILEGES),
      NULL, NULL);

  SetLocalTime(&os_time);  // actually set the time.

  // disable the time adjustment privileges again.
  AdjustTokenPrivileges(petoken, true, &privs, sizeof(TOKEN_PRIVILEGES),
      NULL, NULL);

  // let all the main windows know that the time got adjusted.
//do we need to do this ourselves?
  ::PostMessage(HWND_BROADCAST, WM_TIMECHANGE, 0, 0);

//hmmm: make sure this seems right.
  CloseHandle(petoken);

  return true;
#elif defined(__UNIX__)
//no implem yet.

//temp to shut up warnings
time_locus ted = new_time;
return ted.year ? 0:1;

#else
  return false;
#endif
}

} // namespace.

