#ifndef INT_HASH_CLASS
#define INT_HASH_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : int_hash                                                          *
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

#include "byte_hasher.h"
#include "hash_table.h"

#include <basis/outcome.h>
#include <structures/set.h>

namespace structures {

//! A hash table for storing integers.
/*!
  Implements a hash table indexed on integers that maintains a separate set of
  identifiers for listing the items that are presently in the hash table.  This
  slows down additions somewhat, but finds are not affected.  The advantage of
  the separate index is that the apply() method is much faster.
*/

template <class contents>
class int_hash : public hash_table<int, contents>
{
public:
  int_hash(int max_bits);
  ~int_hash();

  const int_set &ids() const;
  void ids(int_set &ids) const;
    //!< provides the current list of valid identifiers.

  basis::outcome add(int key, contents *to_store);
    //!< overrides base add() and ensures that the id list stays up to date.
  contents *acquire(int key);
    //!< overrides base acquire() by ensuring that the ids stay up to date.
  bool zap(int key);
    //!< overrides base zap() method plus keeps id list updated.
  void reset();
    //!< overrides base reset() and ensures that the id list stays up to date.

  typedef bool apply_function(const int &key, contents &current,
        void *data_link);

  void apply(apply_function *to_apply, void *data_link);
    //!< operates on every item in the int_hash table.

private:
  int_set *_ids;
    //!< a separate list of the identifiers stored here.
    /*! this provides a fairly quick way to iterate rather than having to span
    the whole hash table.  it does slow down zap() a bit though. */
};

//////////////

// implementations below...

template <class contents>
int_hash<contents>::int_hash(int max_bits)
: hash_table<int, contents>(rotating_byte_hasher(), max_bits),
  _ids(new int_set)
{}

template <class contents>
int_hash<contents>::~int_hash()
{ WHACK(_ids); }

template <class contents>
const int_set &int_hash<contents>::ids() const { return *_ids; }

template <class contents>
void int_hash<contents>::ids(int_set &ids) const { ids = *_ids; }

template <class contents>
basis::outcome int_hash<contents>::add(int key, contents *to_store)
{
  _ids->add(key);
  return hash_table<int, contents>::add(key, to_store);
}

template <class contents>
contents *int_hash<contents>::acquire(int key)
{
  _ids->remove(key);
  return hash_table<int, contents>::acquire(key);
}

template <class contents>
bool int_hash<contents>::zap(int key)
{
  _ids->remove(key);
  return hash_table<int, contents>::zap(key);
}

template <class contents>
void int_hash<contents>::reset()
{
  _ids->clear();
  hash_table<int, contents>::reset();
}

template <class contents>
void int_hash<contents>::apply(apply_function *to_apply, void *data_link)
{
  for (int i = 0; i < _ids->elements(); i++) {
    int current = (*_ids)[i];
    contents *found = hash_table<int, contents>::find(current);
    if (!found) {
      _ids->remove(current);
      continue;
    }
    to_apply(current, *found, data_link);
  }
}

} //namespace.

#endif // outer guard.

