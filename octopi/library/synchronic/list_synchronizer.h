#ifndef LIST_SYNCHRONIZER_CLASS
#define LIST_SYNCHRONIZER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : list_synchronizer                                                 *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Manages a collection of lists of synchronizable state information.       *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <octopus/octopus.h>

namespace synchronic {

// forward.
class list_manager;

class list_synchronizer : public octopi::octopus
{
public:
  list_synchronizer();
  ~list_synchronizer();

  basis::outcome add_list(list_manager *to_add);
    // adds a new list synchronization manager "to_add" to the crew of lists.

  basis::outcome zap_list(const structures::string_array &list_name);
    // takes a list registered under "list_name" back out of the synchronizer.
    // the list_manager for the "list_name" is destroyed on success.

  void clean(int older_than);
    // cleans out any items that are older than the "older_than" number of
    // milliseconds.

  bool update(const structures::string_array &object_id);
    // marks the item specified by the "object_id" as updated.
};

}

#endif

