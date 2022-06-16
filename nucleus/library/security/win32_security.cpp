


/*****************************************************************************\
*                                                                             *
*  Name   : win32_security                                                    *
*  Author : Sue Richeson                                                      *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#ifdef __WIN32__

#include "win32_security.h"

#include <basis/utf_conversion.h>
#include <basis/astring.h>

#include <comdef.h>
#include <lm.h>

// This piece of code is borrowed from the following July 1999 MSDN article
//   HOWTO: Look Up Current User Name and Domain Name, ID: Q155698 
//NOTE: It has been modified for inclusion here and it is Win32-specific.

bool win32_security::GetUserAndDomainName(astring &UserName, astring &DomainName)
{
   HANDLE hToken;

   // Initialize the return parameters.
   UserName = "";
   DomainName = "";

   #define MY_BUFSIZE 512  // highly unlikely to exceed 512 bytes
   UCHAR InfoBuffer[ MY_BUFSIZE + 1 ];
   DWORD cbInfoBuffer = MY_BUFSIZE;
   SID_NAME_USE snu;

   BOOL bSuccess;

   if(!OpenThreadToken(
       GetCurrentThread(),
       TOKEN_QUERY,
       TRUE,
       &hToken
       )) {

       if(GetLastError() == ERROR_NO_TOKEN) {

           // 
           // attempt to open the process token, since no thread token
           // exists
           // 

           if(!OpenProcessToken(
               GetCurrentProcess(),
               TOKEN_QUERY,
               &hToken
               )) return FALSE;

       } else {

           // 
           // error trying to get thread token
           // 

           return FALSE;
       }
   }

   bSuccess = GetTokenInformation( hToken,
                                   TokenUser,
                                   InfoBuffer,
                                   cbInfoBuffer,
                                   &cbInfoBuffer);

   if(!bSuccess) {
       if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

           // 
           // alloc buffer and try GetTokenInformation() again
           // 

           CloseHandle(hToken);
           return FALSE;

       } else {

           // 
           // error getting token info
           // 

           CloseHandle(hToken);
           return FALSE;
       }
   }

   CloseHandle(hToken);

   TCHAR User[MY_BUFSIZE + 1];;
   DWORD cchUserName = MY_BUFSIZE;
   TCHAR Domain[MY_BUFSIZE +  1];
   DWORD cchDomainName = MY_BUFSIZE;
                      
    bSuccess = LookupAccountSid(NULL,
                           ((PTOKEN_USER)InfoBuffer)->User.Sid,
                           User,
                           &cchUserName,
                           Domain,
                           &cchDomainName,
                           &snu);

    if (bSuccess)
    {
        UserName = from_unicode_temp(User);
        DomainName = from_unicode_temp(Domain);
    }

   return bSuccess;
}

astring win32_security::full_user()
{
  astring user, temp_domain;
  GetUserAndDomainName(user, temp_domain);
  user += astring("[") + temp_domain + "]";
  return user;
}

#endif // windows.




