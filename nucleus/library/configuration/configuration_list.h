#ifndef CONFIGURATION_LIST_CLASS
#define CONFIGURATION_LIST_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : configuration_list                                                *
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

#include <basis/contracts.h>

namespace configuration {

// forward.
class cl_figlet_list;
class configlet;
#include <configuration/configurator.h>

//! Manages a collection of configlet objects.
/*!
  This class provides the ability to operate on the collection of configlets
  as a whole.  They can be retrieved from or stored to a configurator object.
*/

class configuration_list : public virtual basis::root_object
{
public:
  configuration_list();
  virtual ~configuration_list();

  DEFINE_CLASS_NAME("configuration_list");

  void reset();  //!< removes all items from the list.

  void add(const configlet &new_item);
    //!< adds another configuration atom into the list.

  const configlet *find(const configlet &to_find) const;
    //!< locates the actual configlet with the section and entry of "to_find".
    /*!< note that this might fail if no matching section and entry are found,
    thus returning NIL.  the returned object is still kept in the list, so
    do not try to destroy it.  also note that the object passed in must be
    the same type as the object to be found; otherwise, NIL will be
    returned. */

  bool zap(const configlet &dead_item);
    //!< removes a previously added configuration item.
    /*!< the "dead_item" need only provide the section and entry names. */

  //! reads the values of all the configlets stored in "config" into this.
  bool load(configurator &config);
  //! writes the current values of all the configlets in "this" into "config".
  bool store(configurator &config) const;

private:
  cl_figlet_list *_figs;  //!< our list of configlets.
};

} //namespace.

#endif

