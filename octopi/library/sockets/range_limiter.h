
//not implemented yet.

#ifndef ADDRESS_LIMITER_CLASS
#define ADDRESS_LIMITER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : range_limiter                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Provides a way to check whether an IP address is within a range of       *
*  allowed addresses.  Also manages a configuration file that stores the      *
*  sets of ranges.                                                            *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>

namespace sockets {

// forward.
class limiter_range_list;
class machine_uid;

//! provides a mechanism for restricting access to a resource by the client's IP address.
class range_limiter
{
public:
  range_limiter();
    // constructs a blank range_limiter.

  range_limiter(const basis::astring &source_file, const basis::astring &section);
    // constructs an range_limiter by loading from the "source_file" in
    // the ini "section".

  ~range_limiter();

  enum capabilities {
    ALLOW,
    DENY
  };

  bool is_allowed(const machine_uid &host);
    // checks whether a "host" is in one of the allowed ranges.
  bool is_allowed(const basis::astring &hostname);
    // checks whether a "hostname" is in one of the allowed ranges.  this can
    // either be a text string such as "jumbo.gruntose.blurgh" or it can be
    // a dotted number representation (like "128.28.48.119").

  // observes or modifies the default access permission.  the default will
  // be used when no other permissions apply.
  capabilities get_default();
  void set_default(capabilities rights);

  // these add addresses to the list with the "rights" specified.
  bool add(const machine_uid &address, capabilities rights);
  bool add(const basis::astring &hostname, capabilities rights);
  bool add(const machine_uid &first, const machine_uid &second,
          capabilities rights);

  // takes addresses out of the list of filters.
  bool remove(const machine_uid &address);
  bool remove(const basis::astring &hostname);
  bool remove(const machine_uid &first, const machine_uid &second);

  // retrieves or stores the range and capability information.
  bool load(const basis::astring &file_name, const basis::astring &section);
  bool save(const basis::astring &file_name, const basis::astring &section);

private:
  limiter_range_list *_ranges;
};

} //namespace.

#endif

