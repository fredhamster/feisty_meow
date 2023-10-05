#ifndef TCPIP_DEFINITIONS_GROUP
#define TCPIP_DEFINITIONS_GROUP

/*****************************************************************************\
*                                                                             *
*  Name   : tcpip_definitions                                                 *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Provides some variables that make porting between Unix and W32 easier.   *
*                                                                             *
*******************************************************************************
* Copyright (c) 2001-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/windoze_helper.h>

#ifndef SOCKET_ERROR 
  #define SOCKET_ERROR -1
#endif

#ifndef INVALID_SOCKET 
  #define INVALID_SOCKET -1
#endif

//#ifdef __UNIX__
  // provide some unifying definitions.
  typedef void sock_hop;

  // provide synonyms for errors so we don't conflict with the windows
  // brain-deadness.  they define error values like EACCESS but they're not
  // the real values you need to use with tcp/ip.  french fried gates time.
  #define SOCK_EACCES EACCES
  #define SOCK_EADDRINUSE EADDRINUSE
  #define SOCK_EADDRNOTAVAIL EADDRNOTAVAIL
  #define SOCK_EAFNOSUPPORT EAFNOSUPPORT
  #define SOCK_EALREADY EALREADY
  #define SOCK_EBADF EBADF
  #define SOCK_ECONNABORTED ECONNABORTED
  #define SOCK_ECONNREFUSED ECONNREFUSED
  #define SOCK_ECONNRESET ECONNRESET
  #define SOCK_EDESTADDRREQ EDESTADDRREQ
  #define SOCK_EDQUOT EDQUOT
  #define SOCK_EFAULT EFAULT
  #define SOCK_EHOSTDOWN EHOSTDOWN
  #define SOCK_EHOSTUNREACH EHOSTUNREACH
  #define SOCK_EINPROGRESS EINPROGRESS
  #define SOCK_EINTR EINTR
  #define SOCK_EINVAL EINVAL
  #define SOCK_EISCONN EISCONN
  #define SOCK_ELOOP ELOOP
  #define SOCK_EMFILE EMFILE
  #define SOCK_EMSGSIZE EMSGSIZE
  #define SOCK_ENAMETOOLONG ENAMETOOLONG
  #define SOCK_ENETDOWN ENETDOWN
  #define SOCK_ENETUNREACH ENETUNREACH
  #define SOCK_ENETRESET ENETRESET
  #define SOCK_ENOBUFS ENOBUFS
  #define SOCK_ENOPROTOOPT ENOPROTOOPT
  #define SOCK_ENOTCONN ENOTCONN
  #define SOCK_ENOTEMPTY ENOTEMPTY
  #define SOCK_ENOTSOCK ENOTSOCK
  #define SOCK_EOPNOTSUPP EOPNOTSUPP
  #define SOCK_EPFNOSUPPORT EPFNOSUPPORT
  #define SOCK_EPROCLIM EPROCLIM
  #define SOCK_EPROTOTYPE EPROTOTYPE
  #define SOCK_EPROTONOSUPPORT EPROTONOSUPPORT
  #define SOCK_EREMOTE EREMOTE
  #define SOCK_ESHUTDOWN ESHUTDOWN
  #define SOCK_ESOCKTNOSUPPORT ESOCKTNOSUPPORT
  #define SOCK_ESTALE ESTALE
  #define SOCK_ETIMEDOUT ETIMEDOUT
  #define SOCK_ETOOMANYREFS ETOOMANYREFS
  #define SOCK_EWOULDBLOCK EWOULDBLOCK
  #define SOCK_EUSERS EUSERS
//#endif

  /*
#ifdef __WIN32__
  #include <application/windoze_helper.h>

  // provide some aliases for w32.
#if COMPILER_VERSION==6
  #define hostent HOSTENT
#endif
  typedef char sock_hop;
  typedef int socklen_t;

  // provide close to the real BSD error names using windows values.
  #define SOCK_EACCES WSAEACCES
  #define SOCK_EADDRINUSE WSAEADDRINUSE
  #define SOCK_EADDRNOTAVAIL WSAEADDRNOTAVAIL
  #define SOCK_EAFNOSUPPORT WSAEAFNOSUPPORT
  #define SOCK_EALREADY WSAEALREADY
  #define SOCK_EBADF WSAEBADF
  #define SOCK_ECONNABORTED WSAECONNABORTED
  #define SOCK_ECONNREFUSED WSAECONNREFUSED
  #define SOCK_ECONNRESET WSAECONNRESET
  #define SOCK_EDESTADDRREQ WSAEDESTADDRREQ
  #define SOCK_EDQUOT WSAEDQUOT
  #define SOCK_EFAULT WSAEFAULT
  #define SOCK_EHOSTDOWN WSAEHOSTDOWN
  #define SOCK_EHOSTUNREACH WSAEHOSTUNREACH
  #define SOCK_EINPROGRESS WSAEINPROGRESS
  #define SOCK_EINTR WSAEINTR
  #define SOCK_EINVAL WSAEINVAL
  #define SOCK_EISCONN WSAEISCONN
  #define SOCK_ELOOP WSAELOOP
  #define SOCK_EMFILE WSAEMFILE
  #define SOCK_EMSGSIZE WSAEMSGSIZE
  #define SOCK_ENAMETOOLONG WSAENAMETOOLONG
  #define SOCK_ENETDOWN WSAENETDOWN
  #define SOCK_ENETUNREACH WSAENETUNREACH
  #define SOCK_ENETRESET WSAENETRESET
  #define SOCK_ENOBUFS WSAENOBUFS
  #define SOCK_ENOPROTOOPT WSAENOPROTOOPT
  #define SOCK_ENOTCONN WSAENOTCONN
  #define SOCK_ENOTEMPTY WSAENOTEMPTY
  #define SOCK_ENOTSOCK WSAENOTSOCK
  #define SOCK_EOPNOTSUPP WSAEOPNOTSUPP
  #define SOCK_EPFNOSUPPORT WSAEPFNOSUPPORT
  #define SOCK_EPROCLIM WSAEPROCLIM
  #define SOCK_EPROTOTYPE WSAEPROTOTYPE
  #define SOCK_EPROTONOSUPPORT WSAEPROTONOSUPPORT
  #define SOCK_EREMOTE WSAEREMOTE
  #define SOCK_ESHUTDOWN WSAESHUTDOWN
  #define SOCK_ESOCKTNOSUPPORT WSAESOCKTNOSUPPORT
  #define SOCK_ESTALE WSAESTALE
  #define SOCK_ETIMEDOUT WSAETIMEDOUT
  #define SOCK_ETOOMANYREFS WSAETOOMANYREFS
  #define SOCK_EUSERS WSAEUSERS

  // windows specific names.
  #define SOCK_EWOULDBLOCK WSAEWOULDBLOCK
  #define SOCK_HOST_NOT_FOUND WSAHOST_NOT_FOUND
  #define SOCK_NO_DATA WSANO_DATA
  #define SOCK_NO_RECOVERY WSANO_RECOVERY
  #define SOCK_NOTINITIALISED WSANOTINITIALISED
  #define SOCK_SYSNOTREADY WSASYSNOTREADY
  #define SOCK_TRY_AGAIN WSATRY_AGAIN
  #define SOCK_VERNOTSUPPORTED WSAVERNOTSUPPORTED
#endif
*/

#endif

