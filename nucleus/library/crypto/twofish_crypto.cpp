/*
*  Name   : twofish encryption
*  Author : Chris Koeritz
*****
* Copyright (c) 2005-$now By Author.  This program is free software; you can
* redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either version 2 of
* the License or (at your option) any later version.  This is online at:
*     http://www.fsf.org/copyleft/gpl.html
* Please send any updates to: fred@gruntose.com
*/

#include "twofish_crypto.h"
#include "ssl_init.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/mutex.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <mathematics/chaos.h>
#include <structures/static_memory_gremlin.h>

#include <openssl/err.h>
#include <openssl/evp.h>

using namespace basis;
using namespace loggers;
using namespace mathematics;
using namespace structures;

/*
 Q: is twofish truly requiring 128/192/256 key sizes only?
 A: no, schneier says any key size up to 256 bits.
 Q: can twofish really support *any* key length up to 256?
 A: well, not in openssl.  we are getting invalid key length complaints
    with keys below 92 bits.  this was calculated by testing for quite
    a while with only those size keys, and 92 seems reliable as a minimum.
*/

namespace crypto {

//#define DEBUG_TWOFISH
  // uncomment for noisier version.
#undef ALWAYS_LOG
#define ALWAYS_LOG(t) CLASS_EMERGENCY_LOG(program_wide_logger::get(), t)
#ifdef DEBUG_TWOFISH
  #undef LOG
  #define LOG(t) CLASS_EMERGENCY_LOG(program_wide_logger::get(), t)
#else
  #undef LOG
  #define LOG(t) 
#endif

twofish_crypto::twofish_crypto(int key_size)
: cryptical_envelopment(EVP_aes_256_cbc())
{
  set_key(key_size);
}

twofish_crypto::twofish_crypto(const byte_array &key, int key_size)
: cryptical_envelopment(EVP_aes_256_cbc())
{
  set_key(key, key_size);
}

twofish_crypto::twofish_crypto(const twofish_crypto &to_copy)
: root_object(),
  cryptical_envelopment(*this)
{
}

twofish_crypto::~twofish_crypto()
{
}

twofish_crypto &twofish_crypto::operator = (const twofish_crypto &to_copy)
{
  if (this == &to_copy) return *this;
  *((cryptical_envelopment *)this) = *((cryptical_envelopment *)&to_copy);
//hmmm: is that the best way to do this?
  return *this;
}

} //namespace.

