#ifndef WX_DEFINITIONS_GROUP
#define WX_DEFINITIONS_GROUP

/*****************************************************************************\
*                                                                             *
*  Name   : definitions for wx                                                *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Some macros that support the use of WX widgets.                          *
*                                                                             *
*******************************************************************************
* Copyright (c) 2006-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/utf_conversion.h>

#include <wx/defs.h>

namespace loggers {

#ifdef wxUSE_UNICODE
  //! converts to wx's UTF-16 string format.
  #define to_unicode_wx(s) ((const wxChar *)(const flexichar *)transcode_to_utf16(s))
  //! converts a wx UTF-16 string to UTF-8.
  #define from_unicode_wx(s) ((const char *)(const UTF8 *)transcode_to_utf8(s))
#else
  // turn off the unicode macro to ensure nothing gets confused.
  #undef UNICODE
  // placeholder macros try not to do much.
  #define to_unicode_wx(s) ((const wxChar *)null_transcoder(s, false))
  #define from_unicode_wx(s) ((const char *)null_transcoder((const wxChar *)s, false))
#endif

} //namespace.

#endif

