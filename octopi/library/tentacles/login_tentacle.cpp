/*****************************************************************************\
*                                                                             *
*  Name   : login_tentacle                                                    *
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

#include "entity_registry.h"
#include "login_tentacle.h"
#include "security_infoton.h"

#include <octopus/entity_defs.h>
#include <structures/string_hash.h>
#include <timely/time_stamp.h>

using namespace basis;
using namespace octopi;
using namespace structures;
using namespace timely;

namespace octopi {

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

//////////////

login_tentacle::login_tentacle(entity_registry &security, int dormancy_period)
: tentacle_helper<security_infoton>(security_infoton::security_classifier(),
      false),
  _security(security),
  _dormancy_period(dormancy_period)
{}

login_tentacle::~login_tentacle() {}

outcome login_tentacle::reconstitute(const string_array &classifier,
    byte_array &packed_form, infoton * &reformed)
{
  if (classifier != security_infoton::security_classifier()) 
    return NO_HANDLER;

  return reconstituter(classifier, packed_form, reformed,
      (security_infoton *)NULL_POINTER);
}

void login_tentacle::expunge(const octopus_entity &to_remove)
{
  _security.zap_entity(to_remove);  // trash it and we're done.
}

outcome login_tentacle::consume(infoton &to_chow,
    const octopus_request_id &item_id, byte_array &transformed)
{
  FUNCDEF("consume");
  transformed.reset();
  security_infoton *inf = dynamic_cast<security_infoton *>(&to_chow);
  if (!inf) {
    // if the infoton doesn't cast, then it is not for us.  we need to vet
    // that the entity it came from is known and approved.
    if (_security.authorized(item_id._entity)) {
      // this infoton's entity was allowed, so we call it partially processed.
      return PARTIAL;
    }
    // the infoton's entity is not authorized; it needs to be dropped.
    return DISALLOWED;
  }

  switch (inf->_mode) {
    case security_infoton::LI_REFRESH:  // intentional fall through.
    case security_infoton::LI_LOGIN: {
      bool success = _security.add_entity(item_id._entity,
          inf->verification());
      inf->_success = success? OKAY : DISALLOWED;
      break;
    }
    case security_infoton::LI_LOGOUT: {
      bool success = _security.zap_entity(item_id._entity);
      inf->_success = success? OKAY : DISALLOWED;
      break;
    }
    default: {
      inf->_success = BAD_INPUT;
      break;
    }
  }
  inf->verification().reset();  // we don't need to send that back.
  if (!store_product(dynamic_cast<infoton *>(inf->clone()), item_id))
    return NO_SPACE;
  return OKAY;
}

} //namespace.

