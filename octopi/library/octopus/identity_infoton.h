#ifndef IDENTITY_INFOTON_CLASS
#define IDENTITY_INFOTON_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : identity_infoton                                                  *
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

#include "entity_defs.h"
#include "infoton.h"

namespace octopi {

//! Encapsulates just the action of identifying an octopus user.
/*!
  This must be done for an entity before it can begin to request octopus
  services when there is strong security in place.  By default, the octopus
  does not require this.
*/

class identity_infoton : public infoton
{
public:
  octopus_entity _new_name;

  identity_infoton();
  identity_infoton(const octopus_entity &name);
  identity_infoton(const identity_infoton &to_copy);

  virtual ~identity_infoton();

  DEFINE_CLASS_NAME("identity_infoton");

  identity_infoton &operator = (const identity_infoton &to_copy);

  static const structures::string_array &identity_classifier();
    //!< returns the classifier for this type of infoton.

  virtual void text_form(basis::base_string &fill) const;

  virtual int packed_size() const;
  virtual void pack(basis::byte_array &packed_form) const;
  virtual bool unpack(basis::byte_array &packed_form);

  virtual clonable *clone() const;
};

} //namespace.

#endif

