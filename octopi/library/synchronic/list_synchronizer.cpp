/*****************************************************************************\
*                                                                             *
*  Name   : list_synchronizer                                                 *
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

#include "list_manager.h"
#include "list_synchronizer.h"

#include <structures/string_array.h>
#include <textual/string_manipulation.h>

using namespace basis;
using namespace structures;
using namespace textual;

namespace synchronic {

const int MAX_PER_ENT = 10 * MEGABYTE;
  // our arbitrary limit for how much we allow the entity data bin to store.

list_synchronizer::list_synchronizer()
: octopus(string_manipulation::make_random_name(), MAX_PER_ENT)
{
}

list_synchronizer::~list_synchronizer()
{
}

outcome list_synchronizer::add_list(list_manager *to_add)
{ return add_tentacle(to_add); }

outcome list_synchronizer::zap_list(const string_array &list_name)
{ return zap_tentacle(list_name); }

bool list_synchronizer::update(const string_array &object_id)
{
  lock_tentacles();
  bool to_return = false;
  for (int i = 0; i < locked_tentacle_count(); i++) {
    list_manager *t = dynamic_cast<list_manager *>(locked_get_tentacle(i));
    if (!t) continue;
    if (t->list_name().prefix_compare(object_id)) {
      // this is the right one to ask about the object.
      to_return = t->update(object_id);
      break;
    }
  }
  unlock_tentacles();
  return to_return;
}

void list_synchronizer::clean(int older_than)
{
  lock_tentacles();
  for (int i = 0; i < locked_tentacle_count(); i++) {
    list_manager *t = dynamic_cast<list_manager *>(locked_get_tentacle(i));
    if (t) t->clean(older_than);
  }
  unlock_tentacles();
}

} //namespace.

