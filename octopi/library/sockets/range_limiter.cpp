/*****************************************************************************\
*                                                                             *
*  Name   : range_limiter                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "range_limiter.h"
#include "machine_uid.h"

#include <basis/astring.h>
#include <structures/amorph.h>

using namespace basis;
using namespace structures;

namespace sockets {

class range_record
{
public:
  machine_uid _first;
  machine_uid _second;
  astring _host;
  bool _is_host;  // true if the host has any useful information.
  bool _single;  // true if only the first address matters.
};

//////////////

class limiter_range_list : public amorph<range_record>
{
public:
};

//////////////

range_limiter::range_limiter()
: _ranges(new limiter_range_list)
{
}

range_limiter::range_limiter(const astring &source_file,
    const astring &section)
: _ranges(new limiter_range_list)
{
  load(source_file, section);
}

range_limiter::~range_limiter() { WHACK(_ranges); }

bool range_limiter::is_allowed(const machine_uid &host)
{
if (host.valid()) {}
return false;
}

bool range_limiter::is_allowed(const astring &hostname)
{
if (!hostname) {}
return false;
}

range_limiter::capabilities range_limiter::get_default()
{
return DENY;
}

void range_limiter::set_default(capabilities rights)
{
if (!rights) {}
}

bool range_limiter::add(const machine_uid &address, capabilities rights)
{
if (address.valid() || rights) {}
return false;
}

bool range_limiter::add(const astring &hostname, capabilities rights)
{
if (!hostname || rights) {}
return false;
}

bool range_limiter::add(const machine_uid &first, const machine_uid &second,
    capabilities rights)
{
if (first.valid() || second.valid() || rights) {}
return false;
}

bool range_limiter::remove(const machine_uid &address)
{
if (address.valid()) {}
return false;
}

bool range_limiter::remove(const astring &hostname)
{
if (!hostname) {}
return false;
}

bool range_limiter::remove(const machine_uid &first, const machine_uid &second)
{
if (first.valid() || second.valid()) {}
return false;
}

bool range_limiter::load(const astring &file_name, const astring &section)
{
if (!file_name || !section) {}
return false;
}

bool range_limiter::save(const astring &file_name, const astring &section)
{
if (!file_name || !section) {}
return false;
}

} //namespace.


