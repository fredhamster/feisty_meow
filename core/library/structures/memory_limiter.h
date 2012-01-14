#ifndef MEMORY_LIMITER_CLASS
#define MEMORY_LIMITER_CLASS

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

#include <basis/astring.h>
#include <basis/contracts.h>
#include <structures/set.h>

namespace structures {

// forward.
class ml_memory_record;
class ml_memory_state_meter;

//! Tracks memory currently in use by memory manager objects.
/*!
  The manager is given the ability to control overall memory usage as well
  as to track memory usage per each user of the memory (assuming that the
  memory is granted to unique users indexable via an integer).
*/

class memory_limiter
{
public:
  memory_limiter(int overall_limit, int individual_limit);
    //!< creates a limiter that allows "overall_limit" bytes to be in use.
    /*!< any attempts to add new memory after that limit is reached will be
    rejected.  if "overall_limit" is zero, then no limit is enforced on the
    amount of memory that can be used in total.  the "individual_limit"
    specifies per-user limits, where each user of memory is identified by a
    unique integer and where all users are granted equal rights to allocate
    memory.  "individual_limit" can also be given as zero, meaning no limit
    is enforced per individual.  note that "overall_limit" should usually be
    many multiples of the "individual_limit", as appropriate to how many users
    are expected. */

  virtual ~memory_limiter();

  DEFINE_CLASS_NAME("memory_limiter");

  int overall_limit() const { return _overall_limit; }
    //!< returns the current overall limit.
  int individual_limit() const { return _individual_limit; }
    //!< returns the current individual limit.

  int overall_usage() const { return _overall_size; }
    //!< returns the size used by all managed memory.

  int overall_space_left() const { return overall_limit() - overall_usage(); }
    //!< returns the overall space left for allocation.

  int individual_usage(int individual) const;
    //!< returns the amount of memory used by "individual".

  int individual_space_left(int individual) const;
    //!< returns the space left for the individual specified.

  basis::astring text_form(int indent = 0) const;
    //!< returns a string that lists out the overall plus individual limits.
    /*!< "indent" is used for spacing the printed rows of information. */

  bool okay_allocation(int individual, int memory_desired);
    //!< returns true if "individual" may allocate "memory_desired" bytes.
    /*!< false indicates that this memory must not be allocated if the limits
    are to be adhered to, either because there is already too much used in
    the system at large or because this user is already using their limit. */

  bool record_deletion(int individual, int memory_deleted);
    //!< acknowledges that the "individual" freed "memory_deleted" bytes.
    /*!< returns true if the "individual" is known and if "memory_deleted"
    could be subtracted from that object's usage count.  failure of this method
    indicates that this class is not being used properly; if memory was
    okayed to be granted in okay_allocation, then the paired record_deletion
    will always succeed (in any arbitrary order where the okay_allocation
    succeeds and proceeds the matching record_deletion).  if there are no
    remaining allocations for this individual, then its record is removed. */

  void reset();
    //!< returns the object to a pristine state.

  const structures::int_set &individuals_listed() const;
    //!< reports the current set of individuals using memory.
    /*!< to know whether one is using this class appropriately, check the
    returned set.  if one does not think there should be any memory in use,
    then the set should be empty and overall_usage() should return zero.
    if the overall_usage() is zero, but there are members in the set, then
    there is an implementation error in memory_limiter.  otherwise, if the
    set is non-empty, then deleted memory has not been recorded. */

private:
  int _overall_limit;  //!< how many total bytes allowed?
  int _individual_limit;  //!< how many bytes may each user consume?
  int _overall_size;  //!< the current measured overall memory usage in bytes.
  ml_memory_state_meter *_individual_sizes;  //!< tracks memory per individual.

  ml_memory_record *find_individual(int individual) const;
    //!< locates the record held for the "individual" specified or returns NIL.
};

} //namespace.

#endif

