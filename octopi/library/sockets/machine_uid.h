#ifndef MACHINE_UID_CLASS
#define MACHINE_UID_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : machine_uid                                                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    This object identifies a machine uniquely within a particular network    *
*  configuration.  It is not world-unique, since some location names are not  *
*  either.  For example, a TCP/IP address like 10.2.13.9 might be used many   *
*  times around the world, since it is reserved for private networks.  But    *
*  within one valid network configuration, there should not be more than one  *
*  of these addresses.  If there are, then be aware that multiple machines    *
*  might answer to a particular address and invalidate some assumptions of    *
*  uniqueness.                                                                *
*    The machine_uid is most useful when a program wishes to ensure that it   *
*  treats a machine only once when it is performing some form of processing   *
*  for that address.  For example, the id could be contained within a 'set'   *
*  to ensure that a message is only sent once to each machine of interest.    *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/contracts.h>

namespace sockets {

// forward.
class internal_machine_uid_array;

class machine_uid : public virtual basis::packable
{
public:
  enum known_location_types {
    INVALID_LOCATION,            // this id has not been initialized.
    TCPIP_LOCATION,              // a location on the internet.
    IPX_LOCATION,                // a host on an IPX/SPX network.
    NETBIOS_LOCATION             // a machine reachable by SMB protocol.
  };
  static const basis::astring &type_name(known_location_types type);
    // returns the text form for the "type" specified.

  machine_uid();  // constructs an invalid id.

  machine_uid(known_location_types type, const basis::byte_array &address);
    // sets up an id for the "type" specified given the serialized "address".
    // the format used here is that bytes follow in the natural listing order
    // for the address "type".

  machine_uid(const machine_uid &to_copy);  // copy constructor.

  virtual ~machine_uid();

  known_location_types type() const;
    // returns the type currently held.

  bool valid() const { return type() != INVALID_LOCATION; }
    // returns true if this id seems possibly valid.  an id for which this
    // returns true could still have protocol specific issues that make it
    // invalid, but as far as this class can tell, it looks okay.

  machine_uid &operator =(const machine_uid &to_copy);

  void reset(known_location_types type, const basis::byte_array &address);
    // reconstructs the machine_uid with new parameters.

  basis::astring text_form() const;
    // returns a string that describes the address held here.

  basis::astring compact_form() const;
    // returns a non-readable string form of the id that is more efficient.
  static machine_uid expand(const basis::astring &compacted);
    // gets the original machine_uid out of the "compacted" form.

  basis::byte_array native() const;
    // returns the identifier for the machine in the native format.  for
    // internet1, this is 4 bytes of IP address.

  // equality means: same protocol type, same key length, and same contents.
  bool operator == (const machine_uid &to_compare) const;

  // meets the requirements of packable.
  virtual int packed_size() const;
  virtual void pack(basis::byte_array &packed_form) const;
  virtual bool unpack(basis::byte_array &packed_form);

  const basis::byte_array &raw() const;
    // returns the raw internal representation for the machine_uid.

private:
  basis::byte_array *_contents;
};

//////////////

class internet_machine_uid : public machine_uid
{
public:
  internet_machine_uid(const basis::astring &hostname, const basis::byte_array &ip_address);
    // constructs a machine uid that almost always uniquely identifies a
    // machine on the internet.  if all ip addresses in a system are unique,
    // then IMU is always unique.  however, if all ip addresses in a system
    // are not unique, then the IMU can discriminate the hostname as long as
    // no two hostnames resolve to the same checksum.
};

//////////////

// this object contains a list of unique machine identifiers.  this is
// intentionally just an array (and not a set) because some usages of the
// list of machine_uids need a notion of ordering (such as for a list of
// machines that have touched a packet).

class machine_uid_array : public virtual basis::root_object
{
public:
  machine_uid_array();
  machine_uid_array(const machine_uid_array &to_copy);
  ~machine_uid_array();
  DEFINE_CLASS_NAME("machine_uid_array");

  static const machine_uid_array &blank_array();

  machine_uid_array &operator =(const machine_uid_array &to_copy);

  bool operator += (const machine_uid &to_add);

  basis::astring text_form() const;

  int elements() const;

  void reset();

  machine_uid &operator [] (int index);
  const machine_uid &operator [] (int index) const;

  bool member(const machine_uid &to_test) const;
    // returns true if the id "to_test" is a member of the list of machine
    // ids held here.  if the machine_uid's are of different sizes, then a
    // prefix compare is used.  this allows an id without the host
    // discriminator portion to match the full version.

private:
  internal_machine_uid_array *_uids;  // the underlying machine_uid list.
};

} //namespace.

#endif

