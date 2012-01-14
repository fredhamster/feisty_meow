#ifndef INTERNET_ADDRESS_CLASS
#define INTERNET_ADDRESS_CLASS

//////////////
// Name   : internet_address
// Author : Chris Koeritz
//////////////
// Copyright (c) 1995-$now By Author.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation:
//     http://www.gnu.org/licenses/gpl.html
// or under the terms of the GNU Library license:
//     http://www.gnu.org/licenses/lgpl.html
// at your preference.  Those licenses describe your legal rights to this
// software, and no other rights or warranties apply.
// Please send updates for this code to: fred@gruntose.com -- Thanks, fred.
//////////////

#include "base_address.h"

#include <configuration/configurator.h>

namespace sockets {

// forward.
class machine_uid;

//! this type of address describes a destination out on the internet.

class internet_address : public base_address
{
public:
  enum internet_address_constraints {
    ADDRESS_SIZE = 4,
    MAXIMUM_HOSTNAME_LENGTH = 128
  };

  typedef basis::abyte address_array[ADDRESS_SIZE];
  address_array ip_address;
  int port;

  char hostname[MAXIMUM_HOSTNAME_LENGTH];
    // can be resolved to an ip_address if a domain name server
    // is available.

  internet_address();
  internet_address(const basis::byte_array &ip_address, const basis::astring &host,
          int port);

  DEFINE_CLASS_NAME("internet_address");

  machine_uid convert() const;
    // returns the address in the uniquifying format.

  void fill(const basis::byte_array &ip_address, const basis::astring &host, int port);

  bool same_host(const base_address &to_compare) const;
  bool same_port(const base_address &to_compare) const;
  bool shareable(const base_address &to_compare) const;

  bool operator == (const internet_address &to_compare) const {
    return same_host(to_compare) && same_port(to_compare);
  }

  basis::astring text_form() const;

  basis::astring tokenize() const;
  bool detokenize(const basis::astring &info);

  basis::astring normalize_host() const;
    // returns a normal form for the hostname or address.  this will come from
    // the hostname member first, if it's set.  next it will come from the
    // string form of the IP address.

  static const basis::byte_array &nil_address();
    // returns the address that is all zeros, otherwise known as INADDR_ANY.

  bool is_nil_address() const;
    // returns true if this object's address_array is all zeros.

  static bool is_nil_address(const address_array &ip_address);
    // returns true if the array "ip_address" is all zero.

  static bool appropriate_for_ip(const basis::astring &to_check);
    // tests whether the string is possibly an ip address; it must have no
    // characters in it besides dots and numbers.

  static bool valid_address(const basis::astring &to_check);
    // returns true if the address "to_check" seems well-formed for IP.  note
    // that this will accept 0.0.0.0 as valid.

  static bool is_valid_internet_address(const basis::astring &to_check,
          basis::byte_array &ip_form, bool &all_zeros);
    // this function checks the string "to_check" to see if it is a valid
    // internet address (e.g., 143.203.39.222).  if it is a valid address,
    // then the address "ip_form" is filled in with four bytes that are each
    // in the range (0..255).  if the "ip_form" is 0.0.0.0, then all_zeros
    // is set to true.

  static bool ip_appropriate_number(const basis::astring &to_check, int indy,
          basis::astring &accum);
    //!< returns true if "to_check" has a number at "indy" that works in ipv4.
    /*!< this reports if the string at the position specified could be part of
    a valid internet address.  the characters starting at the "indy" must be
    numeric.  up to three numbers will be checked, and when we get three or
    less of the numbers together, we will check that they make an integer less
    than 255.  the "accum" string will be filled with the number we found.
    note that this doesn't care what the characters are after our 1-3 numbers;
    it merely checks whether the portion of the string at "indy" *could* work
    in an IP address. */

  static bool has_ip_address(const basis::astring &to_check, basis::astring &ip_found);
    //!< returns true if "to_check" has an IP address in it somewhere.
    /*!< this looks across the whole string and returns the first IP address
    it finds in "ip_found", if possible. */

  static basis::astring ip_address_text_form(const basis::byte_array &ip_address);
    // returns a string containing the textual form of the "ip_address".  the
    // "ip_address" is expected to have the proper length (four bytes).  if it
    // does not, then an empty string is returned.

  static const basis::byte_array &localhost();
    // provides the array that indicates localhost, rather than a NIC.
    // this is commonly known to be 127.0.0.1.

  bool is_localhost() const;
    // returns true if the address in question is the same as the localhost,
    // either through the name matching "local" or "localhost", or through
    // the address matching 127.0.0.1.  note that the word "local" here is
    // an "enhancement" and would not normally be considered the same as
    // localhost.  beware a networked computer that's actually named "local".
    // also note that an address where the name disagrees with the address is
    // basically broken, but we are not checking that here; either condition
    // of the hostname or the address matching causes this to return true.

  base_address *create_copy() const;

  void pack(basis::byte_array &packed_form) const;
  bool unpack(basis::byte_array &packed_form);

  virtual int packed_size() const;
};

} //namespace.

#endif

