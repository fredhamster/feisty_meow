#ifndef BUILD_CONFIGURATION_GROUP
#define BUILD_CONFIGURATION_GROUP

//////////////
// Name   : build configuration
// Author : Chris Koeritz
//////////////
// Copyright (c) 1995-$now By Author.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation:
//     http://www.gnu.org/licenses/gpl.html
// or under the terms of the GNU Library license:
//     http://www.gnu.org/licenses/lgpl.html
// at your preference.  Those licenses describe your legal rights to this
// software, and no other rights or warranties apply.
// Please send updates for this code to: fred@gruntose.com -- Thanks, fred.
//////////////

/*! @file build_configuration.h
  @brief Contains definitions that control how libraries and programs are built.

  These definitions can be used for declaring classes and functions that need
  to be called from a DLL or that need to be compiled into a DLL.  The import
  style is used to declare that the item is being pulled out of a DLL.  The
  export style is used to declare that the item is being included in a DLL for
  use by other functions.
*/

#include <basis/definitions.h>

// Here is an example of a dynamic library header to be used by a particular library
// (presumably the gurpta library):
#if 0
 #ifndef GURPTA_DYNAMIC_LIBRARY_HEADER
 #define GURPTA_DYNAMIC_LIBRARY_HEADER
   // define BUILD_GURPTA when you are creating the dynamic library and
   // define DYNAMIC_HOOPLE when you are importing code from the dynamic library.
   #include <basis/build_configuration.h>
   #if defined(BUILD_GURPTA)
     #define GURPTA_LIBRARY HOOPLE_DYNAMIC_EXPORT
   #elif defined(DYNAMIC_HOOPLE)
     #define GURPTA_LIBRARY HOOPLE_DYNAMIC_IMPORT
   #else
     #define GURPTA_LIBRARY
   #endif
 #endif // outer guard.
#endif

#ifdef __WIN32__
  #define HOOPLE_DYNAMIC_EXPORT __declspec(dllexport)
  #define HOOPLE_DYNAMIC_IMPORT __declspec(dllimport)
#else
  // no known requirements for these tags, so set them to nothing.
  #define HOOPLE_DYNAMIC_EXPORT
  #define HOOPLE_DYNAMIC_IMPORT
#endif

#endif // outer guard.

