/*****************************************************************************\
*                                                                             *
*  Name   : memory_limiter                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2001-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "int_hash.h"
#include "memory_limiter.h"

#include <basis/functions.h>

#include <stdio.h>

using namespace basis;

namespace structures {

#undef LOG
#define LOG(to_print) printf("%s\n", astring(to_print).s())

class ml_memory_record
{
public:
  int _usage;

  ml_memory_record(int initial) : _usage(initial) {}
};

//////////////

class ml_memory_state_meter : public int_hash<ml_memory_record>
{
public:
  ml_memory_state_meter() : int_hash<ml_memory_record>(10) {}
};

//////////////

memory_limiter::memory_limiter(int overall_limit, int individual_limit)
: _overall_limit(overall_limit),
  _individual_limit(individual_limit),
  _overall_size(0),
  _individual_sizes(new ml_memory_state_meter)
{
}

memory_limiter::~memory_limiter()
{
  WHACK(_individual_sizes);
}

void memory_limiter::reset()
{
  _overall_size = 0;
  _individual_sizes->reset();
}

const int_set &memory_limiter::individuals_listed() const
{ return _individual_sizes->ids(); }

ml_memory_record *memory_limiter::find_individual(int individual) const
{
  ml_memory_record *to_return = NIL;
  if (!_individual_sizes->find(individual, to_return)) return NIL;
    // no record for that guy.
  return to_return;
}

int memory_limiter::individual_usage(int individual) const
{
  ml_memory_record *found = find_individual(individual);
  if (!found) return 0;
  return found->_usage;
}

int memory_limiter::individual_space_left(int individual) const
{
  if (!individual_limit()) return 0;
  return individual_limit() - individual_usage(individual);
}

astring memory_limiter::text_form(int indent) const
{
  astring to_return;
  astring indentat(' ', indent);

  astring allowed = overall_limit()?
      astring(astring::SPRINTF, "%dK", overall_limit() / KILOBYTE)
      : "unlimited";
  astring avail = overall_limit()?
      astring(astring::SPRINTF, "%dK", overall_space_left() / KILOBYTE)
      : "unlimited";

  to_return += astring(astring::SPRINTF, "Overall Limit=%s, Allocations=%dK, "
      "Free Space=%s", allowed.s(), overall_usage() / KILOBYTE, avail.s());
  to_return += "\n";

  int_set individuals = _individual_sizes->ids();
  for (int i = 0; i < individuals.elements(); i++) {
    astring allowed = individual_limit()?
        astring(astring::SPRINTF, "%dK", individual_limit() / KILOBYTE)
        : "unlimited";
    astring avail = individual_limit()?
        astring(astring::SPRINTF, "%dK",
        individual_space_left(individuals[i]) / KILOBYTE) : "unlimited";

    to_return += indentat + astring(astring::SPRINTF, "individual %d: "
        "Limit=%s, Used=%dK, Free=%s", individuals[i], allowed.s(),
        individual_usage(individuals[i]) / KILOBYTE, avail.s());
    to_return += "\n";
  }
  if (!individuals.elements()) {
    to_return += indentat + "No allocations owned currently.";
    to_return += "\n";
  }
  return to_return;
}

bool memory_limiter::okay_allocation(int individual, int memory_desired)
{
//  FUNCDEF("okay_allocation");
  // check the overall allocation limits first.
  if (_overall_limit
      && (_overall_size + memory_desired > _overall_limit) ) return false;
  // now check sanity of this request.
  if (_individual_limit && (memory_desired > _individual_limit) ) return false;
  // now check the allocations per user.
  ml_memory_record *found = find_individual(individual);
  if (!found) {
    _individual_sizes->add(individual, new ml_memory_record(0));
    found = find_individual(individual);
    if (!found) {
      LOG("ERROR: adding a new record to the memory state!");
      return false;
    }
  }
  if (_individual_limit
      && (found->_usage + memory_desired > _individual_limit) )
    return false; 
  found->_usage += memory_desired;
  _overall_size += memory_desired;
  return true;
}

bool memory_limiter::record_deletion(int individual, int memory_deleted)
{
  if (memory_deleted < 0) return false;  // bogus.
  // make sure the individual exists.
  ml_memory_record *found = find_individual(individual);
  if (!found) return false;
  // the individual must have actually allocated at least that much previously.
  if (found->_usage < memory_deleted) return false;
  // okay, we think that's reasonable.
  found->_usage -= memory_deleted;
  _overall_size -= memory_deleted;
  // clean out an empty locker.
  if (!found->_usage) _individual_sizes->zap(individual);
  return true;
}

} //namespace.


