#ifndef CHAOS_CLASS
#define CHAOS_CLASS

//////////////
// Name   : chaos
// Author : Chris Koeritz
//////////////
// Copyright (c) 1991-$now By Author.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation:
//     http://www.gnu.org/licenses/gpl.html
// Please send updates for this code to: fred@gruntose.com -- Thanks, fred.
//////////////

#include <basis/definitions.h>
#include <basis/enhance_cpp.h>
#include <basis/environment.h>
#include <timely/time_stamp.h>

//#define DEBUG_CHAOS
  // uncomment for noisy logging.

#ifdef DEBUG_CHAOS
  #include <stdio.h>
#endif
#include <stdlib.h>
#include <time.h>
#ifdef __UNIX__
  #include <sys/times.h>
#endif

namespace mathematics {

// gets a 32 bit integer from the 15 bit random generator.  it's 15 bit because
// rand() only generates numbers up to the MAX_RAND, which in the visual c++
// case is limited to 0x7FFF.  so, we let the first random number generated
// serve as the upper two bytes and we shift another one over a bit to cover
// the whole range of the third and fourth byte, except for that last bit, 
// which is added in as a binary random value.
#define GET_32_BIT_RAND_YO \
  basis::un_int ranval = (basis::un_int(rand()) << 16) + (basis::un_int(rand()) << 1) \
       + (basis::un_int(rand()) % 2)

//! a platform-independent way to acquire random numbers in a specific range.
/*!
  This object also re-seeds the underlying system's random seed when retrain()
  is invoked.
*/

class chaos : public virtual basis::nameable
{
public:
  chaos() { retrain(); }
  virtual ~chaos() {}

  DEFINE_CLASS_NAME("chaos");

  void retrain()
    //!< Scrambles the OS's random seed and then provides pseudo-random values.
  {
    static unsigned int __flaxen = 0;
    if (!__flaxen) {
      time_t time_num;
      time(&time_num);
      // "t" is an ugly pointer into system data that contains the time nicely
      // broken up into segments.  the pointer cannot be freed!
      tm *t = localtime(&time_num);
      int add_in_milliseconds = basis::environment::system_uptime();
      // create a good random number seed from the time.
#ifdef DEBUG_CHAOS
      printf("day %d hour %d min %d sec %d", (int)t->tm_mday, (int)t->tm_hour,
          (int)t->tm_min, (int)t->tm_sec);
#endif
      // we really don't care very much about the small chance of this being
      // initialized to zero again.  the chance of that happening more than once
      // should be pretty astronomical.
      __flaxen = (t->tm_sec + 60 * t->tm_mday + 60 * 31 * t->tm_hour
        + 24 * 60 * 31 * t->tm_min) ^ add_in_milliseconds;
      // initialize random generator.
      srand(int(__flaxen));
    }
  }

  // Keep in mind for the inclusive and exclusive functions that a range which
  // is too large might not be dealt with well.  This is because the random
  // functions provided by the O.S. may not support the full range of integers.

  int inclusive(int low, int high) const
    //!< Returns a pseudo-random number r, such that "low" <= r <= "high".
  {
    if (high < low) return low;
    unsigned int range = high - low + 1;
    GET_32_BIT_RAND_YO;
    int adjusted = ranval % range + low;
    return adjusted;
  }

  int exclusive(int low, int high) const
    //!< Returns a pseudo-random number r, such that "low" < r < "high".
  {
    if (high < low) return low + 1;
    unsigned int range = high - low - 1;
    GET_32_BIT_RAND_YO;
    int adjusted = ranval % range + low + 1;
    return adjusted;
  }

};

} //namespace.

#endif

