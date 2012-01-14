#ifndef KEY_REPOSITORY_CLASS
#define KEY_REPOSITORY_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : key_repository                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2004-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/mutex.h>
#include <crypto/blowfish_crypto.h>
#include <structures/symbol_table.h>
#include <octopus/entity_defs.h>

namespace octopi {

//! Tracks the keys that have been assigned for a secure channel.
/*!
  This class is thread-safe, as long as one uses the lock() method below in
  the proper manner.

  NOTE: this is a heavy-weight header; do not include in other headers.
*/

class octenc_key_record
{
public:
  octopus_entity _entity;  //!< who the key belongs to.
  crypto::blowfish_crypto _key;  //!< used for communicating with an entity.

  octenc_key_record() : _key(200) {}  //!< bogus blank constructor.

  octenc_key_record(const octopus_entity &entity, const crypto::blowfish_crypto &key) 
  : _entity(entity), _key(key) {}
};

//////////////

class key_repository
{
public:
  key_repository() : _locker(), _keys() {}
  virtual ~key_repository();

  DEFINE_CLASS_NAME("key_repository");

  octenc_key_record *lock(const octopus_entity &ent);
    //!< locates the key for "ent", if it's stored.
    /*!< the returned object, unless it's NIL, must be unlocked. */

  void unlock(octenc_key_record *to_unlock);
    //!< drops the lock on the key record in "to_unlock".

  basis::outcome add(const octopus_entity &ent, const crypto::blowfish_crypto &key);
    //!< adds a "key" for the "ent".  this will fail if one is already listed.

  basis::outcome whack(const octopus_entity &ent);
    //!< removes the key for "ent".

private:
  basis::mutex _locker;  //!< protects our list of keys.
  structures::symbol_table<octenc_key_record> _keys;  //!< the list of keys.
};

} //namespace.

#endif // outer guard.

