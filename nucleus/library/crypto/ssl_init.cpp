/*****************************************************************************\
*                                                                             *
*  Name   : SSL initialization helper                                         *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "ssl_init.h"

#include <basis/functions.h>
#include <basis/mutex.h>
#include <loggers/program_wide_logger.h>
#include <structures/static_memory_gremlin.h>

#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/provider.h>
#include <openssl/rand.h>

using namespace basis;
using namespace loggers;
using namespace mathematics;
using namespace structures;

namespace crypto {

#define DEBUG_SSL
  // uncomment to cause more debugging information to be generated, plus
  // more checking to be performed in the SSL support.

#undef ALWAYS_LOG
#define ALWAYS_LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)
#ifdef DEBUG_SSL
  #undef LOG
  #define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)
#else
  #undef LOG
  #define LOG(s)
#endif

const int SEED_SIZE = 100;
  // the size of the random seed that we'll use.

// our global initialization object.
SAFE_STATIC_CONST(ssl_init, static_ssl_initializer, )

ssl_init::ssl_init()
: c_rando(),
  c_default_provider(NULL_POINTER),
  c_legacy_provider(NULL_POINTER)
{
  FUNCDEF("ctor");

  LOG("prior to provider setup");
  // new code needed because blowfish is considered legacy code now.  ugh.
  c_legacy_provider = OSSL_PROVIDER_load(NULL_POINTER, "legacy");
  if (!c_legacy_provider) {
    ALWAYS_LOG("failed to load legacy openssl provider!  mega boofer fail!");
    exit(1);
  }
  // also load the default provider or the standard, still accepted, algorithms will not be available.
  c_default_provider = OSSL_PROVIDER_load(NULL_POINTER, "default");
  if (!c_default_provider) {
    ALWAYS_LOG("failed to load default openssl provider!  mega flopsweat fail!");
    exit(1);
  }
  LOG("after provider setup");

  LOG("prior to rand seed");
  RAND_seed(random_bytes(SEED_SIZE).observe(), SEED_SIZE);
  LOG("after rand seed");
}

ssl_init::~ssl_init()
{
  FUNCDEF("destructor");
  LOG("prior to crypto cleanup");

  // clean up the providers again.  not super necessary since the program will
  // exit shortly, but it's good to be tidy.
  if (c_default_provider) OSSL_PROVIDER_unload(c_default_provider);
  if (c_legacy_provider) OSSL_PROVIDER_unload(c_legacy_provider);

  CRYPTO_cleanup_all_ex_data();
}

const chaos &ssl_init::randomizer() const { return c_rando; }

byte_array ssl_init::random_bytes(int length) const
{
  byte_array seed;
  for (int i = 0; i < length; i++)
    seed += abyte(c_rando.inclusive(0, 255));
  return seed;
}

} //namespace.


