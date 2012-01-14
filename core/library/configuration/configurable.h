#ifndef CONFIGURABLE_CLASS
#define CONFIGURABLE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : configurable                                                      *
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

//! base class for objects that support configuration_list updates.
/*!
  The configuration_list implements a set of configuration items and it can
  be used to represent a "delta" between the current configuration of an
  object and a new configuration that is desired.  Objects based on the
  configurable class support taking that delta chunk of configuration info
  and adapting their current internal configuration to meet the request, if
  possible.  These objects also support querying their current configuration,
  which reports all configuration item names and current settings.
*/

#include <basis/contracts.h>


// forward.
class configuration_list;

class configurable : public virtual root_object
{
public:
  virtual ~configurable() {}

  DEFINE_CLASS_NAME("configurable");

  virtual void get_config(configuration_list &to_fill, bool append) const = 0;
    //!< interprets the contents of this object as a configuration list.
    /*!< the list of configlets can be stored in any configurator object.
    if the "append" flag is true, then the list is added to.  otherwise it
    is cleared first.  this method can also be used to retrieve the configlets
    that this class defines as its configuration interface.  that list can
    then be filled in using a configurator and passed to set_config(). */

  virtual bool set_config(const configuration_list &to_use) = 0;
    //!< retrieves the config items from "to_use" and stores them here.
    /*!< false is returned if any of the key items are missing or if the
    new key cannot be decoded. */
};

#endif

