#ifndef TCPIP_STACK_GROUP
#define TCPIP_STACK_GROUP

/*
Name   : tcpip_stack
Author : Chris Koeritz
*******************************************************************************
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "tcpip_definitions.h"

#include <basis/byte_array.h>
#include <basis/contracts.h>
#include <structures/string_array.h>

// forward declarations.
struct sockaddr;

namespace sockets {

// forward declarations.
class internet_address;
class machine_uid;
class machine_uid_array;

//! Helpful functions for interacting with TCP/IP stacks.
/*!
  This class hides details of the platform specifics of the stack.
*/

class tcpip_stack : public virtual basis::root_object
{
public:
  tcpip_stack();
  virtual ~tcpip_stack();

  bool healthy() const { return _healthy; }
    // returns true if the stack seems to be functioning properly.

  DEFINE_CLASS_NAME("tcpip_stack");

  static basis::astring tcpip_error_name(int error_value);
    // returns the name for the "error_value" specified, according to the
    // WinSock 1.1 specification.

  basis::astring hostname() const;
    // gets the string form of the host's name for tcp/ip.

  machine_uid this_host(int location_type) const;
    // returns the unique identifier of "this" host given the "location_type"
    // of interest.  the type should be a member of the machine_uid::
    // known_location_types enum.

  static sockaddr convert(const internet_address &to_convert);
    // returns a low-level address created from our style of address.
  static internet_address convert(const sockaddr &to_convert);
    // returns our style address from the low-level address.

  basis::byte_array full_resolve(const basis::astring &hostname, basis::astring &full_host) const;
    // finds the ip address for a "hostname".  the array will have zero
    // length on failure.  on success, the "full_host" will have the
    // possibly more authoratitative name for the host.

  bool resolve_any(const basis::astring &name, internet_address &resolved) const;
    // translates "name" into a resolved form, where "name" can be either a
    // hostname or an ip address.  true is returned on success.

  basis::astring dns_resolve(const basis::astring &hostname) const;
    // returns a string form of the IP address for "hostname" or an empty
    // string if hostname cannot be found.

  bool enumerate_adapters(structures::string_array &ip_addresses, bool add_local = false) const;
    // returns a list of the ip addresses that TCP/IP reports for this machine.
    // if there's more than one address, then this machine is multi-homed,
    // which could be due to an active dialup networking session or due to
    // there being more than one network interface.  if the function returns
    // false, then tcp/ip failed to report any addresses at all.  if the
    // "add_local" parameter is true, then the localhost IP address is added
    // to the list also.

  internet_address fill_and_resolve(const basis::astring &machine, int port,
          bool &worked) const;
    // creates an address for TCP/IP given the "machine" and the "port".
    // the "machine" can either be in dotted number notation or can be a
    // hostname.  a special value of "local" or the empty string in "machine"
    // causes _this_ host to be used in the address.  otherwise, if the
    // "machine" is a textual hostname, then it is plugged into the returned
    // address and resolved if possible.  if the resolution of the "machine"
    // is successful, then "worked" is set to true.

  bool enumerate_adapters(machine_uid_array &ip_addresses,
          bool add_local = false) const;
    // similar to other function of same name but provides a list of
    // machine_uid objects.

private:
  bool _healthy;  // records if stack started properly.

  static bool initialize_tcpip();
    // starts up the socket mechanisms.  true is returned on success.  if
    // true is returned, each call to initialize must be paired with a call
    // to deinitialize.

  static void deinitialize_tcpip();
    // shuts down the socket mechanisms.
};

//////////////

//! Defines our communication related outcome values.

class communication_commons
{
public:
  enum outcomes {
    DEFINE_API_OUTCOME(NO_CONNECTION, -27, "The connection was dropped or "
        "could not be made"),
    DEFINE_API_OUTCOME(NO_SERVER, -28, "The server is not responding to "
        "requests"),
    DEFINE_API_OUTCOME(NO_ANSWER, -29, "The server does not seem to be "
        "listening for connections"),
    DEFINE_API_OUTCOME(SHUTDOWN, -30, "The object has been shut down or was "
        "never started up"),
    DEFINE_API_OUTCOME(ALREADY_SETUP, -31, "The object has already been setup"),
    DEFINE_API_OUTCOME(MEDIUM_ERROR, -32, "The communications medium is in an "
        "unusable state currently"),
    DEFINE_API_OUTCOME(BAD_MODE, -33, "The transport cannot operate in the "
        "mode specified"),
    DEFINE_API_OUTCOME(ALREADY_CONNECTED, -34, "This object is already "
        "connected"),
    DEFINE_API_OUTCOME(WRONG_ENTITY, -35, "This is the wrong entity type for "
         "the request"),
    DEFINE_API_OUTCOME(IPC_ERROR, -36, "An error has occurred in interprocess "
         "communication"),
    DEFINE_API_OUTCOME(TOO_NOISY, -37, "The communications medium is currently "
         "too noisy to be used"),
    DEFINE_API_OUTCOME(COMM_ERROR, -38, "There was an unspecified "
         "communication error")
  };

  static const char *outcome_name(const basis::outcome &to_name);
    // returns a string representation of the outcome "to_name" if it's a
    // member of the communication_commons::outcomes enum.
};

} //namespace.

#endif

