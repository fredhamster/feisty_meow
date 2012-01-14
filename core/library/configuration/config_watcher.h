#ifndef CONFIG_WATCHER_CLASS
#define CONFIG_WATCHER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : config_watcher                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2008-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "configurator.h"
#include "table_configurator.h"

#include <basis/contracts.h>
#include <structures/set.h>

namespace configuration {

//! an object that watches the contents of a configurator for changes.
/*!
  when given a configurator object to check, this will initially build an
  exact copy of the contents seen.  when asked to look for changes to the
  configurator, the previous version is compared with the current state and
  any changed sections are reported.
*/

class config_watcher
{
public:
  config_watcher(configurator &to_watch);
    //!< watches the configurator for changes and tracks them.
    /*!< "to_watch" must exist for lifetime of this class. */

  virtual ~config_watcher();

  DEFINE_CLASS_NAME("config_watcher");

  bool rescan();
    //!< updates the configurator snapshot and enables the comparison methods.
    /*!< call this before testing the changed() method. */

  // these lists describe how sections have changed, if at all.
  structures::string_set new_sections() const;
  structures::string_set deleted_sections() const;
  structures::string_set changed_sections() const;

  // methods for comparing changes within sections in the config.
  structures::string_set new_items(const basis::astring &section_name);
  structures::string_set deleted_items(const basis::astring &section_name);
  structures::string_set changed_items(const basis::astring &section_name);

private:
  configurator &_watching;  //!< the config object we examine.
  table_configurator *_current_config;  //!< most current records.
  table_configurator *_previous_config;  //!< records we saw earlier.
};

} //namespace.

#endif

