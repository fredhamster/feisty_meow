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
#include <structures/static_memory_gremlin.h>

#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/rand.h>

using namespace basis;
using namespace mathematics;
using namespace structures;

namespace crypto {

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

const int SEED_SIZE = 100;
  // the size of the random seed that we'll use.

// our global initialization object.
SAFE_STATIC_CONST(ssl_init, static_ssl_initializer, )

//#define DEBUG_SSL
  // uncomment to cause more debugging information to be generated, plus
  // more checking to be performed in the SSL support.

ssl_init::ssl_init()
: c_rando()
{
#ifdef DEBUG_SSL
  CRYPTO_malloc_debug_init();
  CRYPTO_dbg_set_options(V_CRYPTO_MDEBUG_ALL);
  CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);
#endif
  RAND_seed(random_bytes(SEED_SIZE).observe(), SEED_SIZE);
}

ssl_init::~ssl_init()
{
  CRYPTO_cleanup_all_ex_data();
  ERR_remove_state(0);
  CRYPTO_mem_leaks_fp(stderr);
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

