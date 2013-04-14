#ifndef DEFINITIONS_GROUP
#define DEFINITIONS_GROUP

/*****************************************************************************\
*                                                                             *
*  Name   : definitions                                                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

//! @file "definitions.h" Constants and objects used throughout HOOPLE.
/*! @file
  Defines a set of useful universal constants (for our chosen universe) and
  a set of aliases for convenient abstract and concrete data types.
  This is the lowest-level header in hoople and should not include any others.
*/

namespace basis {

//////////////

// Constants...

//! The value representing a pointer to nothing, or nothing itself.
#define NIL 0

//! A fundamental constant measuring the number of bits in a byte.
#define BITS_PER_BYTE 8

//! An approximation of the fundamental circular constant.
#define PI_APPROX 3.14159265358

//////////////

// Data Structures & Functions

//! This macro just eats what it's passed; it marks unused formal parameters.
#define formal(parameter)

//! A fairly important unit which is seldom defined...
typedef unsigned char abyte;
/* ridiculous!  all to shut microsoft up about ambiguous byte definitions,
which seems like a bug. 
struct byte {
  byte(unsigned char b = 0) : c_data(b) {}
//  byte(char b) : c_data(b) {}
  operator unsigned char() const { return c_data; }
  operator char() const { return c_data; }
  operator int() const { return c_data; }
  operator unsigned int() const { return c_data; }
  operator bool() const { return (bool)c_data; }
  byte operator &(byte and_with) { return c_data & and_with; }
  byte operator |(byte or_with) { return c_data & or_with; }
  unsigned char c_data;
};
*/

#if defined(UNICODE) && defined(__WIN32__)
  //! the flexichar type is always appropriate to hold data for win32 calls.
  typedef wchar_t flexichar;
#else
  // this version simply defangs any conversions.
  typedef char flexichar;
#endif

//! Abbreviated name for unsigned integers.
typedef unsigned int un_int;
//! Abbreviated name for unsigned short integers.
typedef unsigned short un_short;
//! Abbreviated name for unsigned long integers.
typedef unsigned long un_long;

// some maximum and minimum values that are helpful.
#ifndef MAXINT32
  //! Maximum 32-bit integer value.
  #define MAXINT32 0x7fffffff
#endif
#ifndef MININT32
  //! Minimum 32-bit integer value.
  #define MININT32 0x80000000
#endif
#ifndef MAXINT16
  //! Maximum 32-bit integer value.
  #define MAXINT16 0x7fff
#endif
#ifndef MININT16
  //! Minimum 32-bit integer value.
  #define MININT16 0x8000
#endif
#ifndef MAXCHAR
  //! Maximum byte-based character value.
  #define MAXCHAR 0x7f
#endif
#ifndef MINCHAR
  //! Minimum byte-based character value.
  #define MINCHAR 0x80
#endif
#ifndef MAXBYTE
  //! Maximum unsigned byte value.
  #define MAXBYTE 0xff
#endif
#ifndef MINBYTE
  //! Minimum unsigned byte value.
  #define MINBYTE 0x00
#endif

// Provide definitions for integers with platform independent specific sizes.
// Note that these may have to be adjusted for 64 bit platforms.
typedef char int8;
typedef unsigned char uint8;
typedef signed short int16;
typedef unsigned short uint16;
typedef signed int int32;
typedef unsigned int uint32;

//////////////

// useful time constants.

// the _ms suffix indicates that these are measured in milliseconds.
const int SECOND_ms = 1000;  //!< Number of milliseconds in a second.
const int MINUTE_ms = 60 * SECOND_ms;  //!< Number of milliseconds in a minute.
const int HOUR_ms = 60 * MINUTE_ms;  //!< Number of milliseconds in an hour.
const int DAY_ms = 24 * HOUR_ms;  //!< Number of milliseconds in a day.

// the _s suffix indicates that these are measured in seconds.
const int MINUTE_s = 60;  //!< Number of seconds in a minute.
const int HOUR_s = 60 * MINUTE_s;  //!< Number of seconds in an hour.
const int DAY_s = 24 * HOUR_s;  //!< Number of seconds in a day.

//////////////

// useful general constants.

const int KILOBYTE = 1024;  //!< Number of bytes in a kilobyte.
const int MEGABYTE = KILOBYTE * KILOBYTE;  //!< Number of bytes in a megabyte.
const int GIGABYTE = MEGABYTE * KILOBYTE;  //!< Number of bytes in a gigabyte.
const double TERABYTE = double(GIGABYTE) * double(KILOBYTE);
//double TERABYTE() { return double(GIGABYTE) * double(KILOBYTE); }
  //!< Number of bytes in a terabyte.
//  /*!< Implemented as a function to avoid annoying link errors for double
//  floating point constants in some compilers. */

//////////////

// Super basic objects...

//! lowest level object for all hoople objects.  supports run-time type id.

class root_object
{
public:
  virtual ~root_object() {}
};

//////////////

// compiler specific dumping ground for global settings...

#ifdef _MSC_VER
  // turns off annoying complaints from visual c++.
  #pragma warning(disable : 4251 4275 4003 4800 4355 4786 4290 4996 4407)
  #pragma warning(error : 4172)
    // 4251 and 4275 turn off warnings regarding statically linked code
    //    not being marked with dll import/export flags.
    // 4003 turns off warnings about insufficient number of parameters passed
    //    to a macro.
    // 4800 turns off the warning about conversion from int to bool not being
    //    efficient.
    // 4355 turns off the warning re 'this' used in base member init list.
    // 4786 turns off the warning about 'identifier' truncated to 'number'
    //    characters in the debug information which frequenly happens when
    //    STL pair and set templates are expanded.
    // 4172 is made an error because this warning is emitted for a dangerous
    //    condition; the address of a local variable is being returned, making
    //    the returned object junk in almost all cases.
    // 4996 turns off warnings about deprecated functions, which are mostly
    //    nonsense, since these are mainly the core posix functions.
#endif  // ms visual c++.

//////////////

} //namespace.

#endif // outer guard.

