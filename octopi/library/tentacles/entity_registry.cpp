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

#include "entity_registry.h"

#include <mathematics/chaos.h>
#include <processes/safe_roller.h>
#include <octopus/entity_defs.h>

using namespace basis;
using namespace mathematics;
using namespace processes;
using namespace timely;

namespace octopi {

entity_registry::entity_registry()
: _sequencer(new safe_roller(1, MAXINT32 / 2)),
  _rando(new chaos)
{
}

entity_registry::~entity_registry()
{
  WHACK(_sequencer);
  WHACK(_rando);
}

//////////////

astring blank_entity_registry::text_form()
{ return "blank_entity_registry--all are allowed, none are remembered."; }

bool blank_entity_registry::locate_entity(const octopus_entity &formal(entity),
    time_stamp &last_active, byte_array &verification)
{
  last_active = time_stamp();
  verification = byte_array();
  return true;
}

} //namespace.

