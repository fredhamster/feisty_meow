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

#include "key_repository.h"

#include <crypto/blowfish_crypto.h>
#include <structures/symbol_table.h>

using namespace basis;
using namespace crypto;
using namespace structures;

namespace octopi {

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

//#define DEBUG_KEY_REPOSITORY
  // uncomment for noisier execution.  beware however, if the uls is in
  // use, this can cause infinite recursion.

key_repository::~key_repository() {}

octenc_key_record *key_repository::lock(const octopus_entity &ent)
{
#ifdef DEBUG_KEY_REPOSITORY
  FUNCDEF("lock");
  LOG(astring("entity sought=") + ent.text_form());
#endif
  octenc_key_record *to_return = NULL_POINTER;
  _locker.lock();
  to_return = _keys.find(ent.mangled_form());
  if (!to_return) {
#ifdef DEBUG_KEY_REPOSITORY
    LOG(astring("did not find entity=") + ent.text_form());
#endif
    _locker.unlock();
  } else {
#ifdef DEBUG_KEY_REPOSITORY
    LOG(astring("found entity=") + ent.text_form());
#endif
  }
  return to_return;
}

void key_repository::unlock(octenc_key_record *to_unlock)
{
  if (!to_unlock) return;  // dolts!  they cannot unlock a non-record.
  _locker.unlock();
}

outcome key_repository::add(const octopus_entity &ent,
    const blowfish_crypto &key)
{
#ifdef DEBUG_KEY_REPOSITORY
  FUNCDEF("add");
  LOG(astring("adding key for entity=") + ent.text_form());
#endif
  auto_synchronizer loc(_locker);
  octenc_key_record rec(ent, key);
  return _keys.add(ent.mangled_form(), rec);
}

outcome key_repository::whack(const octopus_entity &ent)
{
#ifdef DEBUG_KEY_REPOSITORY
  FUNCDEF("whack");
  LOG(astring("removing key for entity=") + ent.text_form());
#endif
  auto_synchronizer loc(_locker);
  return _keys.whack(ent.mangled_form());
}

} //namespace.

