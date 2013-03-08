#ifndef RAW_SOCKET_CLASS
#define RAW_SOCKET_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : raw_socket                                                        *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

//! Provides access to the operating system's socket methods.

/*!
  NOTE: This class does not provide any sort of synchronization of the
  sockets involved.  If you have multiple threads arbitrarily calling select
  and read, then some selects will claim the socket was disconnected (since
  the other thread might have grabbed the data that seemed to be ready but
  then wasn't--a sign of a disconnect).  Ensure that your accesses of the
  socket are serialized.
*/

#include <basis/array.h>
#include <basis/astring.h>
#include <basis/contracts.h>

namespace sockets {

// forward declarations.
class fd_set_wrapper;
class tcpip_stack;

//////////////

// sockets can be intrigued by the occurrence of the following conditions:
enum socket_interests {
  SI_READABLE      = 0x1,   // the socket is readable; there's data there.
  SI_WRITABLE      = 0x2,   // the socket will accept data if it's sent.
  SI_CONNECTED     = 0x4,   // the socket is connected.
  SI_DISCONNECTED  = 0x8,   // the socket is disconnected.
  SI_ERRONEOUS     = 0x10,  // the socket is in an erroneous state.
  SI_BASELINE      = 0x20,  // the socket seems okay but not interesting.

  SI_ALL_SOCK_INT  = 0xFF   // all socket interests are active.
};

//////////////

class raw_socket : public virtual basis::root_object
{
public:
  raw_socket();

  ~raw_socket();

  DEFINE_CLASS_NAME("raw_socket");

  int close(basis::un_int &socket);
    // disconnects, destroys and resets the "socket" to zero.

  int ioctl(basis::un_int socket, int request, void *argp) const;
    // manipulates the device parameters for the "socket".

  bool set_non_blocking(basis::un_int socket, bool non_blocking = true);
    // makes the "socket" into a non-blocking socket if "non_blocking" is true.
    // if "non_blocking" is false, then the socket is reset to blocking mode.

  bool set_nagle_algorithm(basis::un_int socket, bool use_nagle = true);
    // sets the nagle algorithm on "socket" if "use_nagle" is true.  note that
    // true is the default when a socket is created; to change that for the
    // socket, "use_nagle" should be false.

  bool set_broadcast(basis::un_int socket, bool broadcasting = true);
    // sets broadcast mode on the socket so that it can receive packets that
    // are broadcast to the network.

  bool set_reuse_address(basis::un_int socket, bool reuse = true);
    // sets the socket to allow an address to be re-used when it's already
    // in use, rather than getting a bind error.

  bool set_keep_alive(basis::un_int socket, bool keep_alive = true);
    // marks a connected-mode socket so that keep alive packets will be sent
    // occasionally to ensure that a disconnection is noticed.

  static basis::astring interest_name(int to_name);
    // returns the textual form for the interests set in "to_name".

  // outlines any special constraints on the select invocation.
  enum select_types {
    SELECTING_JUST_WRITE = 0x1,
    SELECTING_JUST_READ = 0x2
  };

  int select(basis::un_int socket, int selection_mode, int timeout = 0) const;
    // this is similar to the low-level select on a bsd socket.  usually this
    // will return events of any type--read, write and exception.  exceptions
    // will always be cause for a return, but if you just want to look at a
    // read condition, include the JUST_READ flag in the "selection_mode".  to
    // just check for write, add the JUST_WRITE flag.  this function returns a
    // bitwise ORed value from the different types of 'socket_interests' (see
    // tcpip definitions, which is where the enum is currently stored).  if
    // there are no special conditions noted on the socket, then zero is
    // returned.  if the "timeout" is zero, the function will return right
    // away.  if "timeout" (measured in milliseconds) is non-zero, then the
    // function will wait until the condition becomes true or the "timeout"
    // elapses.  note: no infinite timeouts are provided here.

  int select(basis::int_array &read_sox, basis::int_array &write_sox, int timeout = 0) const;
    // similar to select above, but operates on a list of sockets in the
    // "read_sox" and "write_sox".  the "read_sox" are checked to see whether
    // those sockets have data pending that could be read.  the "write_sox" are
    // checked to see if the sockets are currently writable without blocking.
    // if any sockets have events applicable to the "selection_mode", then they
    // will still be present in the lists when the call returns.  zero is
    // returned if absolutely nothing happened during the "timeout" period;
    // otherwise, a non-zero number is returned but the individual sockets
    // must still be inspected to determine what happened.

  int analyze_select_result(basis::un_int socket, int mode, fd_set_wrapper &read_list,
          fd_set_wrapper &write_list, fd_set_wrapper &exceptions) const;
    // examines the "socket" in the fd_sets passed in and returns a result
    // based on those sets and the "mode".

private:
  tcpip_stack *_stack;

  int test_readability(basis::un_int socket) const;
    // checks on the readability state for the "socket", assuming that select()
    // reported the socket as readable, and returns either SI_ERRONEOUS,
    // SI_READABLE or SI_DISCONNECTED based on what ioctl() says about it.

  int inner_select(basis::un_int socket, int selection_mode, int timeout,
          fd_set_wrapper &read_list, fd_set_wrapper &write_list,
          fd_set_wrapper &exceptions) const;
    // intermediate function that doesn't attempt to fully analyze the select
    // result.  the returned value will be non-zero if something interesting
    // is happening on the socket.  if that value is SI_ERRONEOUS, then
    // something bad happened to the socket.  the other non-zero value is
    // SI_BASELINE, which means the socket has something to report in the
    // fd_set parameters.
};

//////////////

#ifdef __UNIX__
  // provide some unifying definitions.
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR -1
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
#endif //unix.

//////////////

#ifdef __WIN32__
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
#endif //win32.

//////////////

} //namespace.

#endif // outer guard.

