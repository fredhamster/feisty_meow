/*****************************************************************************\
*                                                                             *
*  Name   : mutex                                                             *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1996-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

// NOTE: we are explicitly avoiding use of new and delete here because this
//       class is needed by our memory allocation object, which would be
//       providing the new and delete methods.

#include "mutex.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef __UNIX__
  #include <pthread.h>
#endif
#ifdef __WIN32__
  #define _WINSOCKAPI_  // make windows.h happy about winsock.
  // winsock support...
//  #undef FD_SETSIZE
//  #define FD_SETSIZE 1000
    // if you don't set this, you can only select on a default of 64 sockets.
  #include <winsock2.h>
  #include <windows.h>
#endif

namespace basis {

mutex::mutex() { construct(); }

mutex::~mutex() { destruct(); }

void mutex::establish_lock() { lock(); }

void mutex::repeal_lock() { unlock(); }

void mutex::construct()
{
#ifdef __WIN32__
  c_os_mutex = (CRITICAL_SECTION *)malloc(sizeof(CRITICAL_SECTION));
  InitializeCriticalSection((LPCRITICAL_SECTION)c_os_mutex);
#elif defined(__UNIX__)
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  int ret = -1;
#ifdef __APPLE__
  ret = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#else
  ret = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
#endif
  if (ret != 0) {
    printf("failed to initialize mutex attributes!\n"); fflush(NIL);
  }
  c_os_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init((pthread_mutex_t *)c_os_mutex, &attr);
  pthread_mutexattr_destroy(&attr);
#else
  #pragma error("no implementation of mutexes for this OS yet!")
#endif
}

void mutex::destruct()
{
  defang();
}

void mutex::defang()
{
  if (!c_os_mutex) return;  // already defunct.
#ifdef __WIN32__
  DeleteCriticalSection((LPCRITICAL_SECTION)c_os_mutex);
  free(c_os_mutex);
#elif defined(__UNIX__)
  pthread_mutex_destroy((pthread_mutex_t *)c_os_mutex);
  free(c_os_mutex);
#else
  #pragma error("no implementation of mutexes for this OS yet!")
#endif
  c_os_mutex = 0;
}

void mutex::lock()
{
  if (!c_os_mutex) return;
#ifdef __WIN32__
  EnterCriticalSection((LPCRITICAL_SECTION)c_os_mutex);
#elif defined(__UNIX__)
  pthread_mutex_lock((pthread_mutex_t *)c_os_mutex);
#else
  #pragma error("no implementation of mutexes for this OS yet!")
#endif
}

void mutex::unlock()
{
  if (!c_os_mutex) return;
#ifdef __WIN32__
  LeaveCriticalSection((LPCRITICAL_SECTION)c_os_mutex);
#elif defined(__UNIX__)
  pthread_mutex_unlock((pthread_mutex_t *)c_os_mutex);
#else
  #pragma error("no implementation of mutexes for this OS yet!")
#endif
}

} //namespace.

