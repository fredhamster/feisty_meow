#ifndef SSL_INIT_CLASS
#define SSL_INIT_CLASS

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

#include <basis/byte_array.h>
#include <mathematics/chaos.h>

namespace crypto {

//! provides some initialization for the RSA and blowfish crypto.
/*!
  This class does the SSL initialization needed before any functions can
  be used.  It also sets up the random seed for SSL.  NOTE: you should never
  need to use this class directly; just use the accessor function at the
  very bottom and it will be managed globally for the entire program.
*/

//we define NEWER_OPENSSL for those places where we're using openssl 1.1.1.
#if defined(_MSC_VER)
  #define NEWER_OPENSSL
#else
//  #define OLDER_OPENSSL
#endif

class ssl_init : public virtual basis::nameable
{
public:
  ssl_init();
  ~ssl_init();

  DEFINE_CLASS_NAME("ssl_init");

  basis::byte_array random_bytes(int length) const;
    //!< can be used to generate a random array of "length" bytes.

  const mathematics::chaos &randomizer() const;
    //!< provides a random number generator for any encryption routines.

private:
  mathematics::chaos c_rando;  //!< used for generating random numbers.
};

extern const ssl_init &static_ssl_initializer();
  //!< the main method for accessing the SSL initialization support.

} //namespace.

#endif

