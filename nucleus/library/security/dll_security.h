#ifndef SECURITY_DLL_DEFINITIONS
#define SECURITY_DLL_DEFINITIONS

/*****************************************************************************\
*                                                                             *
*  Name   : security DLL helper                                               *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Allows the security support to work within a DLL.                        *
*                                                                             *
*******************************************************************************
* Copyright (c) 2008-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "basis/build_configuration.h"

// windows class tags for the data structures library:
// define BUILD_SECURITY when you are creating the dll and
// define USE_HOOPLE_DLLS when you are importing a class from the dll.
#ifdef BUILD_SECURITY
  #define    HOOPLE_DLL_EXPORT_CLASS
  #define HOOPLE_DLL_EXPORT_FUNCTION
#elif defined(USE_HOOPLE_DLLS)
  #define    HOOPLE_DLL_IMPORT_CLASS
  #define HOOPLE_DLL_IMPORT_FUNCTION
#else
  #define SECURITY_CLASS_STYLE
  #define SECURITY_FUNCTION_STYLE
#endif

#endif

