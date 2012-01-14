#ifndef CLIENT_REGISTRY_CLASS
#define CLIENT_REGISTRY_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : entity_registry                                                   *
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

#include <basis/astring.h>
#include <basis/byte_array.h>
#include <mathematics/chaos.h>
#include <timely/time_stamp.h>
#include <processes/safe_roller.h>
#include <sockets/tcpip_stack.h>

namespace octopi {

class octopus_entity;

//! Provides a security model for the octopus.
/*!
  Derived versions of this class can be hooked to an octopus to enforce
  a particular security model.
*/

class entity_registry
{
public:
  entity_registry();
  virtual ~entity_registry();

  virtual bool authorized(const octopus_entity &entity) = 0;
    //!< returns true if the "entity" is a registered entity.
    /*!< this indicates that the entity is authorized to use this octopus'
    services. */

  virtual bool locate_entity(const octopus_entity &entity,
          timely::time_stamp &last_active, basis::byte_array &verification) = 0;
    //!< retrieves the security records for the "entity", if any exist.
    /*!< true is returned on success and "last_active" is set to the entity's
    last time of activity and "verification" is set to the cilent's
    verification token from its login. */

  virtual bool add_entity(const octopus_entity &entity,
          const basis::byte_array &verification) = 0;
    //!< adds the "entity" to the list of authorized users if allowed.
    /*!< note that this will still succeed if the entity is already present.
    the "verification" is used by the entity to prove its case for admittance,
    but if it has zero length, it's ignored.  true is returned if the entity
    was allowed to login or refresh its record.  false is returned if the
    entity was denied, possibly because of a bad or missing verification
    token. */

  virtual bool refresh_entity(const octopus_entity &entity) = 0;
    //!< this should be used to refresh the entity's health record.
    /*!< it indicates that this entity is still functioning and should not
    be removed due to inactivity. */

  virtual bool zap_entity(const octopus_entity &entity) = 0;
    //!< removes a "entity" if the entity can be found.
    /*!< true is returned if that "entity" existed. */

  virtual basis::astring text_form() = 0;
    //!< prints out the contents of the entity registry.

private:
  processes::safe_roller *_sequencer;  //!< issues unique ids for entities.
  mathematics::chaos *_rando;  //!< issues random portions of ids.
  sockets::tcpip_stack *_stack;  //!< provides hostname and network info.
};

//////////////

//! the blank_entity_registry can be used when security is not an issue.
/*!
  it always assumes every entity is valid.  when the locate() method is
  invoked, a feel-good record is produced.
*/

class blank_entity_registry : public entity_registry
{
public:
  bool authorized(const octopus_entity &formal(entity)) { return true; }
  bool locate_entity(const octopus_entity &entity,
          timely::time_stamp &last_active, basis::byte_array &verification);
  bool add_entity(const octopus_entity &formal(entity),
          const basis::byte_array &formal(verification)) { return true; }
  bool refresh_entity(const octopus_entity &formal(entity)) { return true; }
  bool zap_entity(const octopus_entity &formal(entity)) { return true; }
  basis::astring text_form();
};

} //namespace.

#endif

