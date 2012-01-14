/*****************************************************************************\
*                                                                             *
*  Name   : identity_tentacle                                                 *
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

#include "identity_tentacle.h"
#include "identity_infoton.h"
#include "octopus.h"

#include <structures/string_hash.h>
#include <timely/time_stamp.h>

using namespace basis;
using namespace structures;
using namespace timely;

namespace octopi {

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

//#define DEBUG_IDENTITY_TENTACLE
  // uncomment for debugging version.

//////////////

identity_tentacle::identity_tentacle(octopus &parent)
: tentacle_helper<identity_infoton>(identity_infoton::identity_classifier(),
      false),
  _parent(parent)
{}

identity_tentacle::~identity_tentacle() {}

outcome identity_tentacle::reconstitute(const string_array &classifier,
    byte_array &packed_form, infoton * &reformed)
{
  if (classifier != identity_infoton::identity_classifier()) 
    return NO_HANDLER;

  return reconstituter(classifier, packed_form, reformed,
      (identity_infoton *)NIL);
}

outcome identity_tentacle::consume(infoton &to_chow,
    const octopus_request_id &item_id, byte_array &transformed)
{
#ifdef DEBUG_IDENTITY_TENTACLE
  FUNCDEF("consume");
#endif
  transformed.reset();
  identity_infoton *inf = dynamic_cast<identity_infoton *>(&to_chow);
  if (!inf) {
    // if the infoton doesn't cast, then it is not for us.  we need to vet
    // that the identity looks pretty much okay.

//hmmm: check host?
//      that would imply that all users of octopi have correctly identified
//      themselves.  this is not currently the case.  we need a way to
//      automate that step for a user of an octopus?
bool uhhh = true;

    if (uhhh) {
      // this infoton's entity was allowed, so we call it partially processed.
      return PARTIAL;
    }
#ifdef DEBUG_IDENTITY_TENTACLE
    LOG(astring("denying infoton ") + item_id.mangled_form());
#endif
    // the infoton's identity is invalid; it needs to be dropped.
    return DISALLOWED;
  }

#ifdef DEBUG_IDENTITY_TENTACLE
  LOG(astring("old name, storing under: ") + item_id.mangled_form());
#endif

  // this is definitely for an identity request now.
  inf->_new_name = _parent.issue_identity();

#ifdef DEBUG_IDENTITY_TENTACLE
  LOG(astring("new name: ") + inf->_new_name.mangled_form());
#endif

  if (!store_product(dynamic_cast<infoton *>(inf->clone()), item_id))
    return NO_SPACE;
  return OKAY;
}

} //namespace.

