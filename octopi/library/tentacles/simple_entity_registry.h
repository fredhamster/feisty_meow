#ifndef SIMPLE_CLIENT_REGISTRY_CLASS
#define SIMPLE_CLIENT_REGISTRY_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : simple_entity_registry                                            *
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

namespace octopi {

// forward.
class octopus_entity;
class recognized_entity;
class recognized_entity_list;

//! Provides a basic implementation of an entity_registry for octopus.
/*!
  Requests to add an entity are always permitted; the registration is a
  formality that allows us to establish a record for the entity.
  This base class just implements authorized() by invoking locate_entity()
  to check if the entity exists.  Thus, using it as a security model really
  just requires calling add_entity() whenever a new entity is seen.
  A different security model can be implemented in a derived class by over-
  riding the authorized() method and making it perform an application-
  specific security check.
*/

class simple_entity_registry : public entity_registry
{
public:
  simple_entity_registry();
  virtual ~simple_entity_registry();

  virtual bool authorized(const octopus_entity &entity);
    //!< returns true if the "entity" is a registered and authorized entity.

  virtual bool locate_entity(const octopus_entity &entity,
          timely::time_stamp &last_active, basis::byte_array &verification);
    //!< retrieves the "security_record" for the "entity" if it exists.
    /*!< true is returned on success. */

  virtual bool add_entity(const octopus_entity &entity,
          const basis::byte_array &verification);
    //!< adds the "entity" to the list of authorized users.
    /*!< note that this will still succeed if the entity is already present,
    allowing it to serve as an entity refresh method.  if the "verification"
    has any length, it will be stored in the record for the "entity". */

  virtual bool refresh_entity(const octopus_entity &entity);
    //!< this should be used to refresh the entity's health record.
    /*!< it indicates that this entity is still functioning and should not be
    removed due to inactivity. */

  virtual bool zap_entity(const octopus_entity &entity);
    //!< removes an "entity" if the entity can be found.
    /*!< true is returned if that "entity" existed. */

  virtual basis::astring text_form();
    //!< shows the contents of the registry.

private:
  basis::mutex *_secure_lock;  //!< the synchronizer for our tentacle list.
  recognized_entity_list *_entities;  //!< list of recognized entities.
};

} //namespace.

#endif

