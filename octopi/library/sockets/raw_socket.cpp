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

#include "raw_socket.h"
#include "tcpip_stack.h"

#include <basis/functions.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <timely/time_stamp.h>

#include <stdlib.h>
#ifdef __APPLE__
  #include <fcntl.h>
#endif
#ifdef __UNIX__
  #include <arpa/inet.h>
  #include <errno.h>
  #include <netinet/tcp.h>
  #include <sys/ioctl.h>
  #include <sys/socket.h>
  #include <unistd.h>
  #define OPTYPE (void *)
#endif
#ifdef __WIN32__
  #define OPTYPE (char *)
#endif

using namespace basis;
using namespace loggers;
using namespace timely;

namespace sockets {

//#define DEBUG_RAW_SOCKET
  // uncomment for noisy diagnostics.

#undef LOG
#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger::get(), to_print)

const int MULTIPLE_DISCONNECT_CHECKS = 28;
  // we will make certain that select really says there's data and ioctl
  // really says there's no data waiting before we believe there's been
  // a disconnect.  we'll check that state the number of times specified.

class fd_set_wrapper : public fd_set {};

//////////////

const basis::un_int NON_BLOCKING = FIONBIO;
const basis::un_int IOCTL_READ = FIONREAD;

#ifdef __WIN32__
/*
// defined by winsock header but not present in the winsock dll.
int PASCAL FAR __WSAFDIsSet(SOCKET fd, fd_set FAR *the_set)
{
  int i = the_set->fd_count;
  while (i--)
    if (the_set->fd_array[i] == fd)
      return true;
  return false;
}
*/
#endif

//////////////

raw_socket::raw_socket()
: _stack(new tcpip_stack())
{}

raw_socket::~raw_socket()
{
  WHACK(_stack);
}

int raw_socket::close(basis::un_int &socket)
{
  int to_return = 0;
#ifdef __WIN32__
  to_return = closesocket(socket);
#endif
#ifdef __UNIX__
  to_return = ::close(socket);
#endif
  socket = 0;
  return to_return;
}

//move this into parser bits as a OR combiner or something.
void combine(astring &existing, const astring &addition)
{
  if (addition.t()) {
    if (existing.t()) existing += " | ";
    existing += addition;
  }
}

astring raw_socket::interest_name(int interest)
{
  astring to_return;
  if (interest & SI_CONNECTED) combine(to_return, "CONNECTED");
  if (interest & SI_DISCONNECTED) combine(to_return, "DISCONNECTED");
  if (interest & SI_WRITABLE) combine(to_return, "WRITABLE");
  if (interest & SI_READABLE) combine(to_return, "READABLE");
  if (interest & SI_ERRONEOUS) combine(to_return, "ERRONEOUS");
  if (!interest) combine(to_return, "NORMAL");
  return to_return;
}

int raw_socket::ioctl(basis::un_int socket, int request, void *argp) const
{
#ifdef __UNIX__
  return ::ioctl(socket, request, argp);
#endif
#ifdef __WIN32__
  return ioctlsocket(socket, request, (un_long *)argp);
#endif
}

bool raw_socket::set_non_blocking(basis::un_int socket, bool non_blocking)
{
  FUNCDEF("set_non_blocking");
#ifdef __APPLE__
  int curr_flags = fcntl(socket, F_GETFL, 0);
  if (fcntl(socket, F_SETFL, curr_flags | O_NONBLOCK) < 0) return false;
#else
  int arg = int(non_blocking);
  if (negative(ioctl(socket, NON_BLOCKING, &arg))) {
    LOG(a_sprintf("Could not set non-blocking (FIONBIO) option on raw_socket %u.", socket));
    return false;
  }
#endif
  return true;
}

bool raw_socket::set_nagle_algorithm(basis::un_int socket, bool use_nagle)
{
  FUNCDEF("set_nagle_algorithm");
  int arg = int(!use_nagle);  // opposite of the flag, since we set no-delay.
  if (negative(setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, OPTYPE &arg,
      sizeof(arg)))) {
    LOG(a_sprintf("Could not change nagle coalescing mode on %u.", socket));
    return false;
  }
  return true;
}

bool raw_socket::set_broadcast(basis::un_int socket, bool broadcasting)
{
  FUNCDEF("set_broadcast");
  int arg = int(broadcasting);
  if (negative(setsockopt(socket, SOL_SOCKET, SO_BROADCAST, OPTYPE &arg,
      sizeof(arg)))) {
    LOG(a_sprintf("Could not change broadcast mode on %u.", socket));
    return false;
  }
  return true;
}

bool raw_socket::set_reuse_address(basis::un_int socket, bool reuse)
{
  FUNCDEF("set_reuse_address");
  int arg = int(reuse);
  if (negative(setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, OPTYPE &arg,
      sizeof(arg)))) {
    LOG(a_sprintf("Could not set reuse address mode on %u.", socket));
    return false;
  }
  return true;
}

bool raw_socket::set_keep_alive(basis::un_int socket, bool keep_alive)
{
  FUNCDEF("set_keep_alive");
  int arg = int(keep_alive);
  if (negative(setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, OPTYPE &arg,
      sizeof(arg)))) {
    LOG(a_sprintf("Could not set keep alive mode on %u.", socket));
    return false;
  }
  return true;
}

int raw_socket::select(basis::un_int socket, int mode, int timeout) const
{
  FUNCDEF("select [single]");
  if (!socket) return SI_ERRONEOUS;
  fd_set_wrapper read_list, write_list, exceps;
  int ret = inner_select(socket, mode, timeout, read_list, write_list, exceps);
  if (!ret) return 0;  // nothing is happening.
  if (ret == SI_ERRONEOUS) return SI_ERRONEOUS;  // something bad happened.
  // otherwise we should be at base-line status.
  return analyze_select_result(socket, mode, read_list, write_list, exceps);
}

int raw_socket::inner_select(basis::un_int socket, int mode, int timeout,
    fd_set_wrapper &read_list, fd_set_wrapper &write_list,
    fd_set_wrapper &exceptions) const
{
#ifdef DEBUG_RAW_SOCKET
  FUNCDEF("inner_select");
#endif
  // setup the file descriptor sets for the select.  we check readability,
  // writability and exception status.
  FD_ZERO(&read_list); FD_SET(socket, &read_list);
  FD_ZERO(&write_list); FD_SET(socket, &write_list);
  FD_ZERO(&exceptions); FD_SET(socket, &exceptions);

  timeval time_out = time_stamp::fill_timeval_ms(timeout);
    // timeval has tv_sec=seconds, tv_usec=microseconds.

  // select will tell us about the socket.
  int ret = ::select(socket + 1,
      (mode & SELECTING_JUST_WRITE)? NIL : &read_list,
      (mode & SELECTING_JUST_READ)? NIL : &write_list,
      &exceptions, &time_out);
  int error = critical_events::system_error();

  if (!ret) return 0;  // nothing to report.

  if (ret == SOCKET_ERROR) {
    switch (error) {
      // all real errors fall out to the error handling stuff.
      case SOCK_EFAULT:  // intentional fall-through.
      case SOCK_ENETDOWN:  // intentional fall-through.
      case SOCK_EINVAL:  // intentional fall-through.
      case SOCK_EINTR:  // intentional fall-through.
#ifdef __WIN32__
      case SOCK_NOTINITIALISED:  // intentional fall-through.
#endif
      case SOCK_ENOTSOCK:
        break;

      // hopefully all these others are bogus errors...
      case SOCK_EINPROGRESS:  // intentional fall-through.
      case 0:  // intentional fall-through.
      default:
#ifdef DEBUG_RAW_SOCKET
        LOG("got to weird case, in progress or zero.");
#endif
        return 0;  // not really an error.
    }
#ifdef DEBUG_RAW_SOCKET
    LOG(a_sprintf("socket %u had error %d in select: %s.",
        socket, error, _stack->tcpip_error_name(error).s()));
#endif
    return SI_ERRONEOUS;
  }

  // if we got to here, then there are some things to report...
  return SI_BASELINE;
}

int raw_socket::test_readability(basis::un_int socket) const
{
  FUNCDEF("test_readability");
  basis::un_int len;
  if (negative(ioctl(socket, IOCTL_READ, &len))) {
    LOG(astring(astring::SPRINTF, "socket %u had ioctl error: %s.",
        socket, _stack->tcpip_error_name(critical_events::system_error()).s()));
    return SI_ERRONEOUS;
  } else {
    if (positive(len)) return SI_READABLE;
    else return SI_DISCONNECTED;
  }
}

int raw_socket::analyze_select_result(basis::un_int socket, int mode,
    fd_set_wrapper &read_list, fd_set_wrapper &write_list,
    fd_set_wrapper &exceptions) const
{
#ifdef DEBUG_RAW_SOCKET
  FUNCDEF("analyze_select_result");
#endif
  int to_return = 0;

  // in case of an exception, we return an error.
  if (FD_ISSET(socket, &exceptions)) {
#ifdef DEBUG_RAW_SOCKET
    LOG(astring(astring::SPRINTF, "exception seen for socket %u!", socket));
#endif
  }

  // check to see if there are bytes to read.
  if ( ! (mode & SELECTING_JUST_WRITE) && FD_ISSET(socket, &read_list)) {
    // make sure we have data.  if no data is available, it means a
    // disconnect occurred.

    int readable = test_readability(socket);
    if (readable == SI_ERRONEOUS)
      to_return |= SI_ERRONEOUS;
    else if (readable == SI_READABLE)
      to_return |= SI_READABLE;
    else if (readable == SI_DISCONNECTED) {
      // we need to check multiple times to be sure the OS really means this.
      // either windoze seems to report an erroneous disconnect every few
      // days or there's a bad synchronization issue as yet uncovered.
      bool really_disconnected = true;
      for (int i = 0; i < MULTIPLE_DISCONNECT_CHECKS; i++) {
        fd_set_wrapper read_list, write_list, exceps;
        int temp_ret = inner_select(socket, SELECTING_JUST_READ, 0, read_list,
            write_list, exceps);
        // check the return value first...
        if (!temp_ret) {
          // nothing happening (a zero return) means the socket's no longer
          // claiming to have a readable state; our disconnect condition is
          // thus violated and we can leave.
          really_disconnected = false;
          break;
        }
        if (temp_ret == SI_ERRONEOUS) {
          // this, on the other hand, sounds really bad.  the socket doesn't
          // seem to exist any more or something else horrid happened.
          really_disconnected = true;
          break;
        }
        // if the select worked, we can check the fd_set now for readability.
        if (!FD_ISSET(socket, &read_list)) {
          // we are not in a disconnected state without being told we're
          // readable.  supposedly.
          really_disconnected = false;
          break;
        }
        // now we really test the socket for readability by making sure there
        // really is data pending on the socket.  if it's readable but there's
        // no data, then either a disconnection has occurred or is in progress.
        readable = test_readability(socket);
        if (readable != SI_DISCONNECTED) {
          // we are not disconnected if there's really data waiting.
          really_disconnected = false;
          break;
        }
      }
      if (really_disconnected) {
#ifdef DEBUG_RAW_SOCKET
        LOG(a_sprintf("connection closed on socket %u.", socket));
#endif
        to_return |= SI_DISCONNECTED;
      }
    }
  }

  // check writability state.
  if (! (mode & SELECTING_JUST_READ) && FD_ISSET(socket, &write_list)) {
    to_return |= SI_WRITABLE;
  }

  return to_return;
}

int raw_socket::select(int_array &read_sox, int_array &write_sox,
    int timeout) const
{
#ifdef DEBUG_RAW_SOCKET
  FUNCDEF("select [multiple]");
#endif
  if (!read_sox.length() && !write_sox.length())
   return 0;  // nothing happened to nothing.

  int to_return = 0;  // will get bits slammed into it to report results.

  // setup the file descriptor sets for the select.  we check readability,
  // writability and exception status.
  fd_set_wrapper read_list; FD_ZERO(&read_list);
  fd_set_wrapper write_list; FD_ZERO(&write_list);
  fd_set_wrapper exceptions; FD_ZERO(&exceptions);
  // set up the lists with the sets we were handed.
  basis::un_int highest = 0;
  int i = 0;
  for (i = 0; i < read_sox.length(); i++) {
    basis::un_int sock = (basis::un_int)read_sox[i];
    if (sock > highest) highest = sock;
    FD_SET(sock, &read_list);
  }
  for (i = 0; i < write_sox.length(); i++) {
    basis::un_int sock = (basis::un_int)write_sox[i];
    if (sock > highest) highest = sock;
    FD_SET(sock, &write_list);
  }

  timeval time_out = time_stamp::fill_timeval_ms(timeout);
    // timeval has tv_sec=seconds, tv_usec=microseconds.

  // select will tell us about the socket.
  int ret = ::select(highest + 1,
      (read_sox.length())? &read_list : NIL,
      (write_sox.length())? &write_list : NIL,
      &exceptions, &time_out);
  int error = critical_events::system_error();

  if (ret == SOCKET_ERROR) {
    switch (error) {
      // all real errors fall out to the error handling stuff.
      case SOCK_EFAULT:  // intentional fall-through.
      case SOCK_ENETDOWN:  // intentional fall-through.
      case SOCK_EINVAL:  // intentional fall-through.
      case SOCK_EINTR:  // intentional fall-through.
#ifdef __WIN32__
      case SOCK_NOTINITIALISED:  // intentional fall-through.
#endif
      case SOCK_ENOTSOCK:
        break;

      // hopefully all these others are bogus errors...
      case SOCK_EINPROGRESS:  // intentional fall-through.
      case 0:  // intentional fall-through.
      default:
#ifdef DEBUG_RAW_SOCKET
        LOG("got to weird case, in progress or zero.");
#endif

//hmmm: fix retd sox?  what's this outcome mean for the list?

        return 0;  // not really an error.
    }
#ifdef DEBUG_RAW_SOCKET
    LOG(a_sprintf("sockets had error %d in select: %s.",
        error, _stack->tcpip_error_name(error).s()));
#endif

//hmmm: fix retd sox?  what's this outcome mean for the list?

    return SI_ERRONEOUS;
  } else if (!ret) {
    // we know of nothing exciting for any of these.
    read_sox.reset();
    write_sox.reset();
    return 0;  // nothing to report.
  }

  // if we got to here, then there are some things to report...
  // iterate through the lists and check results.
  for (int k = 0; k < read_sox.length(); k++) {
    basis::un_int socket = read_sox[k];
    int interim = analyze_select_result(socket, SELECTING_JUST_READ, read_list,
        write_list, exceptions);
    if (!interim) {
      // nothing happened on that guy.
      read_sox.zap(k, k);
      k--;  // skip back to before the whack.
      continue;
    }
    to_return |= interim;  // join the results with overall result.
  }
  for (int p = 0; p < write_sox.length(); p++) {
    basis::un_int socket = write_sox[p];
    int interim = analyze_select_result(socket, SELECTING_JUST_WRITE, read_list,
        write_list, exceptions);
    if (!interim) {
      // nothing happened on that guy.
      write_sox.zap(p, p);
      p--;  // skip back to before the whack.
      continue;
    }
    to_return |= interim;  // join the results with overall result.
  }

  return to_return;
}

} //namespace.

