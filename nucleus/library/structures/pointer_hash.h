#ifndef POINTER_HASH_CLASS
#define POINTER_HASH_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : pointer_hash                                                      *
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
#include "pointer_hash.h"
#include "set.h"

// forward.
class pointer_set;

namespace structures {

//! A hash table for storing pointers.
/*!
  Implements a hash table indexed on pointer values that maintains a separate
  set to list the items that are presently in the hash table.  This slows down
  additions somewhat, but finds are not affected.  The advantage of the
  separate index is that the apply() method is much faster.
*/

template <class contents>
class pointer_hash : public hash_table<void *, contents>
{
public:
  pointer_hash(int estimated_elements);
  ~pointer_hash();

  const pointer_set &ids() const;
  void ids(pointer_set &ids) const;
    //!< provides the current list of valid identifiers.

  basis::outcome add(void *key, contents *to_store);
    //!< overrides base add() and ensures that the id list stays up to date.
  contents *acquire(void *key);
    //!< overrides base acquire() by ensuring that the ids stay up to date.
  bool zap(void *key);
    //!< overrides base zap() method plus keeps id list updated.
  void reset();
    //!< overrides base reset() and ensures that the id list stays up to date.

  typedef bool apply_function(const void * &key, contents &current,
        void *data_link);

  void apply(apply_function *to_apply, void *data_link);
    //!< operates on every item in the pointer_hash table.

private:
  pointer_set *_ids;
    //!< a separate list of the identifiers stored here.
    /*! this provides a fairly quick way to iterate rather than having to span
    the whole hash table.  it does slow down zap() a bit though. */
};

//////////////

// implementations for larger methods below...

template <class contents>
pointer_hash<contents>::pointer_hash(int estimated_elements)
: hash_table<void *, contents>(rotating_byte_hasher(), estimated_elements),
  _ids(new pointer_set)
{}

template <class contents>
pointer_hash<contents>::~pointer_hash()
{ WHACK(_ids); }

template <class contents>
const pointer_set &pointer_hash<contents>::ids() const { return *_ids; }

template <class contents>
void pointer_hash<contents>::ids(pointer_set &ids) const { ids = *_ids; }

template <class contents>
basis::outcome pointer_hash<contents>::add(void *key, contents *to_store)
{
  _ids->add(key);
  return hash_table<void *, contents>::add(key, to_store);
}

template <class contents>
contents *pointer_hash<contents>::acquire(void *key)
{
  _ids->remove(key);
  return hash_table<void *, contents>::acquire(key);
}

template <class contents>
bool pointer_hash<contents>::zap(void *key)
{
  _ids->remove(key);
  return hash_table<void *, contents>::zap(key);
}

template <class contents>
void pointer_hash<contents>::reset()
{
  _ids->clear();
  hash_table<void *, contents>::reset();
}

template <class contents>
void pointer_hash<contents>::apply(apply_function *to_apply, void *data_link)
{
  for (int i = 0; i < _ids->elements(); i++) {
    void *current = (*_ids)[i];
    contents *found = hash_table<void *, contents>::find(current);
    if (!found) {
      _ids->remove(current);
      continue;
    }
    to_apply(current, *found, data_link);
  }
}

} //namespace.

#endif // outer guard.

