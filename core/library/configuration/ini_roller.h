#ifndef INI_ROLLER_CLASS
#define INI_ROLLER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : ini_roller                                                        *
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

#include "configurator.h"

#include <basis/definitions.h>
#include <basis/mutex.h>
#include <structures/roller.h>

namespace configuration {

//! Implements an id generator that interacts with a configuration file.
/*!
  This provides an int_roller (which provides rolling ids given a range to
  issue them from) that is stored in a configurator.  The instantiated
  object is the real source of the ids, but the configurator file is
  periodically updated to reflect the current id state.  If a program
  is restarted later, then it will start using ids that it had not already
  issued in its last run, as long as the configurator is a persistent object.
  Note that the range of ids had better be quite large; otherwise the program
  could still have live entries under an id that is about to be reissued
  due to wrap-around.
*/

class ini_roller : public virtual basis::root_object
{
public:
  ini_roller(configurator &config, const basis::astring &section,
          const basis::astring &entry, int min, int max);
    //!< creates a roller that updates "config" with the current id value.
    /*!< the updates are not continuous, but are done periodically to avoid
    constantly writing to the "config".  the "section" and "entry" dictate
    where the entry is saved in the "config".  the "min" and "max" provide the
    range of the id, where it will start at "min", increment by one until it
    reaches at most "max", and then it will start back at "min" again.  note
    that "config" must exist for the duration of the ini_roller. */

  virtual ~ini_roller();

  DEFINE_CLASS_NAME("ini_roller");

  int next_id();
    //!< returns the next number to be issued.

  int current_id() const;
    //!< returns the current id; this is the one that was last issued.

private:
  configurator &_ini;  //!< provides access to the ini file.
  structures::int_roller *_ids;  //!< the current id number is managed here.
  basis::astring *_section;  //!< remembers the right section for our id entry.
  basis::astring *_entry;  //!< remembers the entry name.
  basis::mutex *_lock;  //!< keeps us thread safe.
};

} //namespace.

#endif

