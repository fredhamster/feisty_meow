#ifndef LOGIN_TENTACLE_CLASS
#define LOGIN_TENTACLE_CLASS

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

#include "security_infoton.h"

#include <octopus/tentacle_helper.h>

namespace octopi {

// forward.
class entity_registry;
class octopus_request_id;

//! Provides rudimentary login services.
/*!
  This is a way for entities to become logged into an octopus system,
  should that be required by the application.
*/

class login_tentacle
: public tentacle_helper<security_infoton>
{
public:
  login_tentacle(entity_registry &security,
          int dormancy_period = 7 * basis::MINUTE_ms);
    //!< constructs a login manager based on "security".
    /*!< this will allow an entity to persist for "dormancy_period"
    milliseconds without a refresh.  after that time, the entities we haven't
    heard from are whacked.  the "security" object will provide our login
    checking. */

  virtual ~login_tentacle();

  DEFINE_CLASS_NAME("login_tentacle");

  virtual basis::outcome reconstitute(const structures::string_array &classifier,
          basis::byte_array &packed_form, infoton * &reformed);
    //!< recreates a "reformed" infoton from the packed data.
    /*!< this uses the "classifier" and packed infoton data in "packed_form".
    this will only succeed if the classifier's first name is understood here.
    */

  virtual basis::outcome consume(infoton &to_chow, const octopus_request_id &item_id,
          basis::byte_array &transformed);
    //!< the base login_tentacle allows anyone to log in.
    /*!< this permits any entity that tries to log in to become a verified
    entity.  derived login_tentacles can force the entity to prove that it's
    worthy in an application specific manner. */

  virtual void expunge(const octopus_entity &to_remove);
    //!< trashes the records we were keeping for the entity.

private:
  entity_registry &_security;  //!< allows or disallows entity access.
  int _dormancy_period;  //!< time allowed before an entity is dropped.
};

} //namespace.

#endif

