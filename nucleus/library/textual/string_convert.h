#ifndef STRING_CONVERSION_GROUP
#define STRING_CONVERSION_GROUP

/*****************************************************************************\
*                                                                             *
*  Name   : string_convert                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2007-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <basis/utf_conversion.h>

/*
#ifdef __WIN32__
  #ifndef _MANAGED
      #define _WINSOCKAPI_  // the dance of the windows headers.
  // winsock support...
//  #undef FD_SETSIZE
//  #define FD_SETSIZE 1000
    // if you don't set this, you can only select on a default of 64 sockets.
  #include <winsock2.h>
  #include <windows.h>
  #ifndef __GNU_WINDOWS__
    #include <comdef.h>
  #endif
  #endif
#endif
*/

// forward.
class _bstr_t;  // ATL (Active Template Library) string type.

//! A collection of conversions between popular string types.

namespace string_convert
{

#ifdef _AFXDLL
  //! conversion from MFC CString to astring.
  inline astring to_astring(const CString &original)
  { return astring(from_unicode_temp(original)); }

  //! conversion from astring to MFC CString.
  inline CString to_CString(const astring &original)
  { return CString(to_unicode_temp(original)); }
#endif

#ifdef WIN32
 #ifndef _MANAGED
  #ifndef __GNU_WINDOWS__
    //! conversion from ATL's _bstr_t object to astring.
    inline basis::astring to_astring(const _bstr_t &original) {
       return basis::astring(basis::astring::UNTERMINATED, (const char *)original,
          original.length());
    }

    //! conversion from astring to the ATL _bstr_t object.
    inline _bstr_t to_bstr_t(const basis::astring &original)
    { return _bstr_t(original.s()); }
  #endif
 #endif
#endif

//other conversions.

} //namespace

#endif

