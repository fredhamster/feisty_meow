#ifndef WIN32_SECURITY_CLASS
#define WIN32_SECURITY_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : win32_security                                                    *
*  Author : Sue Richeson                                                      *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Some helper functions for security calls.  These are lower level than    *
*  nt_security and only require bare OS calls.                                *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000 By Author.  This program is free software; you can       *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#ifdef __WIN32__



// forward.
#include <basis/astring.h>

class win32_security
{
public:
  static bool GetUserAndDomainName(astring &UserName, astring &DomainName);
    //!< returns user account and windows domain of logged in user.

  static astring full_user();
    //!< returns user and domain combined into a parsable form: user[domain]
};

#endif // windows.

#endif // outer guard.

