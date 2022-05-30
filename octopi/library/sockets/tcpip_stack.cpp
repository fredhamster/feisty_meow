/**
*  Name   : tcpip_stack
*  Author : Chris Koeritz
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "internet_address.h"
#include "machine_uid.h"
#include "raw_socket.h"
#include "tcpip_stack.h"

#include <basis/functions.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <structures/string_array.h>

//#ifdef __UNIX__
  #include <arpa/inet.h>
  #include <errno.h>
  #include <memory.h>
  #include <netdb.h>
  #include <sys/ioctl.h>
  #include <sys/socket.h>
  #include <sys/time.h>
  #include <sys/types.h>
  #include <termios.h>
  #include <unistd.h>
//#endif

using namespace basis;
using namespace loggers;
using namespace structures;
using namespace textual;

namespace sockets {

//#define DEBUG_TCPIP_STACK
  // turn on for clamor.

#undef LOG
#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger::get(), to_print)

//////////////

	/*
#ifdef __WIN32__
  const WORD WINSOCK_VERSION_REQUIRED = 0x0101;
    // 1.1 version is used by this version of tcp/ip transport.
#endif
*/

//////////////

const char *communication_commons::outcome_name(const outcome &to_name)
{
  switch (to_name.value()) {
    case NO_CONNECTION: return "NO_CONNECTION";
    case NO_SERVER: return "NO_SERVER";
    case NO_ANSWER: return "NO_ANSWER";
    case SHUTDOWN: return "SHUTDOWN";
    case ALREADY_SETUP: return "ALREADY_SETUP";
    case MEDIUM_ERROR: return "MEDIUM_ERROR";
    case BAD_MODE: return "BAD_MODE";
    case ALREADY_CONNECTED: return "ALREADY_CONNECTED";
    case WRONG_ENTITY: return "WRONG_ENTITY";
    case IPC_ERROR: return "IPC_ERROR";
    case TOO_NOISY: return "TOO_NOISY";
    case COMM_ERROR: return "COMM_ERROR";
    default: return common::outcome_name(to_name);
  }
}

//////////////

tcpip_stack::tcpip_stack()
: _healthy(initialize_tcpip())
{}

tcpip_stack::~tcpip_stack()
{
  deinitialize_tcpip();
  _healthy = false;
}

bool tcpip_stack::initialize_tcpip()
{
	/*
#ifdef __WIN32__
  FUNCDEF("initialize_tcpip");
  // make sure we have the right version of WinSock available.
  WORD desired_winsock = WINSOCK_VERSION_REQUIRED;
  WSADATA startup_data;
  int error = WSAStartup(desired_winsock, &startup_data);
  if (error) {
    LOG(astring("startup error: ") + tcpip_error_name(critical_events::system_error()));
    return false;
  }
#endif
*/
  return true;
}

void tcpip_stack::deinitialize_tcpip()
{
/*#ifdef __WIN32__
  WSACleanup();
#endif*/
}

astring tcpip_stack::hostname() const
{
  FUNCDEF("hostname");
  char hostname[256];
  hostname[0] = '\0';
  // gethostname() finds the name for our tcp/ip host.
  if (negative(gethostname(hostname, 255))) {
    LOG(astring(astring::SPRINTF, "gethostname error %s.",
        tcpip_error_name(critical_events::system_error()).s()));
    return hostname;
  }
  return hostname;
}

astring tcpip_stack::tcpip_error_name(int error_value)
{
  switch (error_value) {
    // winsock errors:
    case SOCK_EINTR: return "EINTR";
    case SOCK_EBADF: return "EBADF";
    case SOCK_EACCES: return "EACCES";
    case SOCK_EFAULT: return "EFAULT";
    case SOCK_EINVAL: return "EINVAL";
    case SOCK_EMFILE: return "EMFILE";
    case SOCK_EWOULDBLOCK: return "EWOULDBLOCK";
    case SOCK_EINPROGRESS: return "EINPROGRESS";
    case SOCK_EALREADY: return "EALREADY";
    case SOCK_ENOTSOCK: return "ENOTSOCK";
    case SOCK_EDESTADDRREQ: return "EDESTADDRREQ";
    case SOCK_EMSGSIZE: return "EMSGSIZE";
    case SOCK_EPROTOTYPE: return "EPROTOTYPE";
    case SOCK_ENOPROTOOPT: return "ENOPROTOOPT";
    case SOCK_EPROTONOSUPPORT: return "EPROTONOSUPPORT";
    case SOCK_ESOCKTNOSUPPORT: return "ESOCKTNOSUPPORT";
    case SOCK_EOPNOTSUPP: return "EOPNOTSUPP";
    case SOCK_EPFNOSUPPORT: return "EPFNOSUPPORT";
    case SOCK_EAFNOSUPPORT: return "EAFNOSUPPORT";
    case SOCK_EADDRINUSE: return "EADDRINUSE";
    case SOCK_EADDRNOTAVAIL: return "EADDRNOTAVAIL";
    case SOCK_ENETDOWN: return "ENETDOWN";
    case SOCK_ENETUNREACH: return "ENETUNREACH";
    case SOCK_ENETRESET: return "ENETRESET";
    case SOCK_ECONNABORTED: return "ECONNABORTED";
    case SOCK_ECONNRESET: return "ECONNRESET";
    case SOCK_ENOBUFS: return "ENOBUFS";
    case SOCK_EISCONN: return "EISCONN";
    case SOCK_ENOTCONN: return "ENOTCONN";
    case SOCK_ESHUTDOWN: return "ESHUTDOWN";
    case SOCK_ETOOMANYREFS: return "ETOOMANYREFS";
    case SOCK_ETIMEDOUT: return "ETIMEDOUT";
    case SOCK_ECONNREFUSED: return "ECONNREFUSED";
    case SOCK_ELOOP: return "ELOOP";
    case SOCK_ENAMETOOLONG: return "ENAMETOOLONG";
    case SOCK_EHOSTDOWN: return "EHOSTDOWN";
    case SOCK_EHOSTUNREACH: return "EHOSTUNREACH";
    case SOCK_ENOTEMPTY: return "ENOTEMPTY";
    case SOCK_EUSERS: return "EUSERS";
    case SOCK_EDQUOT: return "EDQUOT";
    case SOCK_ESTALE: return "ESTALE";
    case SOCK_EREMOTE: return "EREMOTE";
/* #ifdef __WIN32__
    case SOCK_EPROCLIM: return "EPROCLIM";
    case SOCK_SYSNOTREADY: return "SYSNOTREADY";
    case SOCK_VERNOTSUPPORTED: return "VERNOTSUPPORTED";
    case SOCK_HOST_NOT_FOUND: return "HOST_NOT_FOUND";
    case SOCK_TRY_AGAIN: return "TRY_AGAIN";
    case SOCK_NO_RECOVERY: return "NO_RECOVERY";
    case SOCK_NO_DATA: return "NO_DATA";  // or NO_ADDRESS.
    case SOCK_NOTINITIALISED: return "NOTINITIALISED";
#endif
*/
  }

  // return a standard OS error...
  return critical_events::system_error_text(error_value);
}

//////////////

bool tcpip_stack::enumerate_adapters(string_array &ip_addresses,
    bool add_local) const
{
#ifdef DEBUG_TCPIP_STACK
  FUNCDEF("enumerate_adapters");
#endif
  ip_addresses.reset();
  // see if they want to put the local adapter in there.
  if (add_local)
    ip_addresses += "127.0.0.1";
  astring this_host = hostname();
#ifdef DEBUG_TCPIP_STACK
  LOG(astring("hostname is \"") + this_host + astring("\"."));
#endif
  if (!this_host) {
#ifdef DEBUG_TCPIP_STACK
    LOG("failed to get the hostname for this machine!");
#endif
    return false;
  }

  hostent *host_entry = gethostbyname(this_host.s());
  if (!host_entry) {
#ifdef DEBUG_TCPIP_STACK
    LOG(astring("failed to get host entry for \"") + this_host + "\".");
#endif
    return false;
  } 
  for (int adapter_num = 0; /* check is inside loop */; adapter_num++) {
    in_addr *current_entry = (in_addr *)host_entry->h_addr_list[adapter_num];
    if (!current_entry) break;
    char *ip_address = inet_ntoa(*current_entry);
#ifdef DEBUG_TCPIP_STACK
    LOG(astring("current is: ") + astring(ip_address));
#endif
    ip_addresses += ip_address;
  }

#ifdef DEBUG_TCPIP_STACK
  LOG(astring("read addresses:") + parser_bits::platform_eol_to_chars()
      + ip_addresses.text_form());
#endif

  return !!ip_addresses.length();
}

bool tcpip_stack::enumerate_adapters(machine_uid_array &ip_addresses,
    bool add_local) const
{
  ip_addresses.reset();
  string_array text_list;
  if (!enumerate_adapters(text_list, add_local))
    return false;
  for (int i = 0; i < text_list.length(); i++) {
    bool worked;
    internet_address addr_form = fill_and_resolve(text_list[i], 0, worked);
    if (worked) {
      hostname().stuff(addr_form.hostname, addr_form.MAXIMUM_HOSTNAME_LENGTH);
      ip_addresses += addr_form.convert();
    }
  }
  return !!ip_addresses.elements();
}

sockaddr tcpip_stack::convert(const internet_address &make_from)
{
  FUNCDEF("convert [to sockaddr]");
  sockaddr_in new_socket;  // our socket.
  memset(&new_socket, 0, sizeof(new_socket));  // clear it out.
  new_socket.sin_family = AF_INET;
  byte_array ip(internet_address::ADDRESS_SIZE, make_from.ip_address);
  ip.stuff(internet_address::ADDRESS_SIZE, (abyte *)&new_socket.sin_addr);
  new_socket.sin_port = htons(basis::un_short(make_from.port));
    // possibly unportable conversion to short above.
  // now we need to return the more generic form of socket address.
  sockaddr to_return;
  memset(&to_return, 0, sizeof(to_return));
  memcpy(&to_return, (sockaddr *)&new_socket, sizeof(new_socket));
    // sockaddr_in guaranteed to be smaller or equal to sockaddr.
  return to_return;
}

internet_address tcpip_stack::convert(const sockaddr &make_from_o)
{
  const sockaddr_in *make_from = (const sockaddr_in *)&make_from_o;
  byte_array ip(internet_address::ADDRESS_SIZE, (abyte *)&make_from->sin_addr);
  internet_address to_return;
  to_return.fill(ip, "", ntohs(make_from->sin_port));
  return to_return;
}

byte_array tcpip_stack::full_resolve(const astring &hostname,
    astring &full_hostname) const
{
  FUNCDEF("full_resolve");
  if (!hostname) return byte_array();  // blank hostnames go nowhere.
  full_hostname.reset();
  // check first for local host equivalents.
  if ( hostname.iequals("local") || hostname.iequals("localhost") ) {
    byte_array to_return = internet_address::localhost();
    full_hostname = "localhost";
    return to_return;
  } else if (hostname.iequals("inaddr_any")
        || hostname.iequals("any-address")) {
    byte_array to_return = internet_address::nil_address();
    full_hostname = "inaddr_any";
    return to_return;
  }
  // gethostbyname() fills in details about the host, such as its IP address
  // and full hostname.
  hostent *machine = gethostbyname(hostname.observe());
  if (!machine) {
    LOG(astring(astring::SPRINTF, "gethostbyname error %s.",
        tcpip_error_name(critical_events::system_error()).s()));
    return byte_array();
  }
  full_hostname = astring(machine->h_name);
  return byte_array(machine->h_length, (abyte *)machine->h_addr);
}

bool tcpip_stack::resolve_any(const astring &hostname,
    internet_address &to_return) const
{
  FUNCDEF("resolve_any");
  to_return = internet_address();
  if (!hostname) return false;  // blank hostnames go nowhere.
  astring full_host;
  byte_array ip = full_resolve(hostname, full_host);
  if (!ip.length()) return false;
  // success then.  fill out the address object.
  full_host.stuff(to_return.hostname,
      internet_address::MAXIMUM_HOSTNAME_LENGTH);
  // copy the ip address into our structure.
  ip.stuff(internet_address::ADDRESS_SIZE, (abyte *)&to_return.ip_address);
  return true;
}

astring tcpip_stack::dns_resolve(const astring &hostname) const
{
  FUNCDEF("dns_resolve");
  if (!hostname) return "";  // blank hostnames go nowhere.
  if (hostname.iequals("local") || hostname.iequals("localhost")) {
    return "127.0.0.1";
  } else if (hostname.iequals("inaddr_any")
      || hostname.iequals("any-address")) {
    return "0.0.0.0";
  }
  // gethostbyname() fills out details about the host, such as its IP address
  // and full hostname.
  hostent *machine = gethostbyname(hostname.observe());
  if (!machine) {
    return "";
  }
  byte_array ip(machine->h_length, (abyte *)machine->h_addr);
    // copy the ip address into an array for easier manipulation.
  astring to_return;
  for (int i = 0; i < ip.length(); i++) {
    to_return += astring(astring::SPRINTF, "%d", int(ip[i]));
    if (i != ip.length() - 1) to_return += ".";
  }
  return to_return;
}

/*

//decide if this should be kept or not.

internet_address tcpip_stack::deprecated_lookup
    (const byte_array &ip_address)
{
  FUNCDEF("deprecated_lookup");
  // lookup the client's hostname through DNS.
  hostent *entry = gethostbyaddr((char *)ip_address.observe(),
      ip_address.length(), PF_INET);
  astring hostname = "unknown_host";
  if (entry) hostname = entry->h_name;

  internet_address to_return;
  to_return.fill(ip_address, hostname, 0);
    // the zero is above because we don't know the port here.
  return to_return;
}
*/

machine_uid tcpip_stack::this_host(int location_type) const
{
  switch (location_type) {
    case machine_uid::TCPIP_LOCATION: {
      astring host = hostname();
      astring full_host;
      byte_array ip = full_resolve(host, full_host);
      if (!ip.length()) return machine_uid();  // failure.
      return internet_machine_uid(full_host, ip);
    }
    case machine_uid::IPX_LOCATION: {
///uhhh...
return machine_uid();  // return junk.
      break;
    }
    case machine_uid::NETBIOS_LOCATION: {
///uhhh...
return machine_uid();  // return junk.
      break;
    }
    default:
//complain.
      return machine_uid();  // return junk.
  }
}

internet_address tcpip_stack::fill_and_resolve(const astring &machine_in,
    int port, bool &worked) const
{
  internet_address to_return;
  astring machine = machine_in;
  machine.to_lower();
  if (!machine) machine = "local";  // assume they mean this machine.
  bool resolved = false;  // true if we know the name.
  byte_array ip_addr;  // the ip address we're guessing/decoding/looking up.
  bool is_ip = false;  // set to true if we have the IP address in ip_addr.
  if (machine.iequals("local") || machine.iequals("localhost")) {
    // handle our special flag and the normal localhost machine type.
    machine = "localhost";
    ip_addr = internet_address::localhost();
    is_ip = true;  // we have the address already.
    resolved = true;  // we know the name also.
  } else if (machine.iequals("inaddr_any")
      || machine.iequals("any-address")) {
    // handle the any address case.
    machine = "inaddr_any";
    ip_addr = internet_address::nil_address();
    is_ip = true;  // we have the address already.
    resolved = true;  // we know the name also.
  } else {
    // well, we now are just going to guess about it being an IP address.
    bool all_zeros;
    is_ip = internet_address::is_valid_internet_address(machine, ip_addr,
        all_zeros);
  }
  if (is_ip) {
    // we try to fill in the hostname if given ip only.
//hmmm: use reverse dns to get the machine name for real!
//      for cases where we really had an ip address, using the machine there
//      would be a mistake.
    if (resolved) to_return.fill(ip_addr, machine, port);
    else to_return.fill(ip_addr, astring::empty_string(), port);
    resolved = true;  // claim we are happy with it now.
  } else {
    // we try to fill in the ip address for the host.
    astring full_host;
    ip_addr = full_resolve(machine, full_host);
    if (ip_addr.length()) {
      to_return.fill(ip_addr, machine, port);
      resolved = true;
    }
  }
  worked = resolved;
  return to_return;
}

} //namespace.


