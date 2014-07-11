


/*****************************************************************************\
*                                                                             *
*  Name   : nt_security                                                       *
*  Author : Sue Richeson                                                      *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1999-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#ifdef __WIN32__

#include "nt_security.h"
#include "win32_security.h"

#include <basis/astring.h>

#include <lm.h>

// LSA success status value.
#ifndef STATUS_SUCCESS
  #define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)
#endif

nt_security::nt_security()
: m_sDirServiceProvider(NIL)
{
  // Eventually, construction of nt_security should determine if WinNT://
  // or LDAP://, etc is the service provider and set a private member variable  
  // with the appropriate distinguished name qualifier.
  m_sDirServiceProvider = new astring("WinNT://");
}
//---------------------------------------------------------------------------


nt_security::~nt_security()
{
  if (m_sDirServiceProvider)
    delete m_sDirServiceProvider;
}


//---------------------------------------------------------------------------

bool nt_security::iequalsUsername(astring name1, astring name2)
{
    return normalizeUsername(name1) == normalizeUsername(name2);
}

const astring &nt_security::normalizeUsername(astring &username)
{
    username.replace_all('/', '\\');
    username.replace_all('|', ':');
    username.to_lower();
    return username;
}

  //---------------------------------------------------------------------------

astring nt_security::DomainBinding(const astring &domain)
{
  astring tempstring = *m_sDirServiceProvider + domain;
  return (tempstring);
}
//---------------------------------------------------------------------------


astring nt_security::DomainUserBinding(const astring &domain, const astring &user_name)
{
  astring tempstring = *m_sDirServiceProvider + domain + astring("/") + user_name;
  return (tempstring);
}
//---------------------------------------------------------------------------

// This piece of code is borrowed from the following July 1999 MSDN article
//     HOWTO: Look Up Current User Name and Domain Name
//     ID: Q155698 
//NOTE: It has been modified for inclusion in this class and it is NT-specific.

bool nt_security::GetUserAndDomainName(astring &UserName, astring &DomainName)
{
  return win32_security::GetUserAndDomainName(UserName, DomainName);
}


//---------------------------------------------------------------------------
// The following routines were taken from the lsapriv sample application by
// Scott Field on the MSDN January 2001.
//---------------------------------------------------------------------------

/*---------------------------------------------------------------------------
This function attempts to obtain a SID representing the supplied
account on the supplied system.

If the function succeeds, the return value is the SID. A buffer is
allocated which contains the SID representing the supplied account.
This buffer should be freed when it is no longer needed by calling
HeapFree(GetProcessHeap(), 0, buffer)

If the function fails, the return SID is NULL.

Scott Field (sfield)    12-Jul-95
Sue Richeson   27-FEB-2001  Mods for use in LightLink.
Chris Koeritz  02-JAN-2008  changed some more for use within hoople.
---------------------------------------------------------------------------*/

PSID nt_security::GetUserSID(const astring &user_name)
{
    PSID psid = NULL;
    char * ReferencedDomain = NULL;
    DWORD cbSid = 128;    // initial allocation attempt
    DWORD cchReferencedDomain = 16; // initial allocation size
    SID_NAME_USE peUse;
    bool bSuccess = false;

    try 
    {   //
        // initial memory allocations
        //
        psid = (PSID)HeapAlloc(GetProcessHeap(), 0, cbSid);
        if (psid != NULL)
        {
            ReferencedDomain = (char *)HeapAlloc(GetProcessHeap(), 0, cchReferencedDomain * sizeof(TCHAR));
            if (ReferencedDomain != NULL)
            {
                // Obtain the SID of the specified account on the specified system.
                //
                bSuccess = true;
                while (!LookupAccountName(NULL, // machine to lookup account on
                    to_unicode_temp(user_name),  // account to lookup
                    psid,               // SID of interest
                    &cbSid,             // size of SID
                    to_unicode_temp(ReferencedDomain),  // domain where found.
                    &cchReferencedDomain,
                    &peUse)) 
                {
                    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
                    {
                        // reallocate memory
                        //
                        psid = (PSID)HeapReAlloc(GetProcessHeap(), 0, psid, cbSid);
                        if(psid != NULL)
                        {
                            ReferencedDomain = (char *)HeapReAlloc(GetProcessHeap(), 0,
                                                                   ReferencedDomain,
                                                                   cchReferencedDomain * sizeof(TCHAR));
                            if (ReferencedDomain == NULL)
                            {
                                break;
                                bSuccess = false;
                            }
                        }
                        else
                        {
                            break;
                            bSuccess = false;
                        }
                    }
                    else
                    {
                        break;
                        bSuccess = false;
                    }
                } // end while
            } // if ReferencedDomain
        } // if psid
    }
    catch(...) 
    {
        bSuccess = false;
    }

    // Cleanup and indicate failure, if appropriate.
    //
    if (ReferencedDomain != NULL)
        HeapFree(GetProcessHeap(), 0, ReferencedDomain);

    if (!bSuccess) 
    {
        if (psid != NULL) 
        {
            HeapFree(GetProcessHeap(), 0, psid);
            psid = NULL;
        }
    }

    return psid;
}
//---------------------------------------------------------------------------


DWORD nt_security::OpenPolicy(const astring &serverName, DWORD DesiredAccess, PLSA_HANDLE pPolicyHandle)
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    DWORD winerror = 0;

    // Always initialize the object attributes to all zeroes.
    //
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

    transcode_to_utf16 temp_server(serverName);
    LSA_UNICODE_STRING server;
    server.Buffer = (PWSTR)(UTF16 *)temp_server;
    server.Length = serverName.length() * (int)sizeof(UTF16);
    server.MaximumLength = (serverName.length() +1) * (int)sizeof(UTF16);

    // Attempt to open the policy.
    Status = LsaOpenPolicy(&server, &ObjectAttributes, DesiredAccess, pPolicyHandle);
    if (STATUS_SUCCESS != Status)
    {
        winerror = LsaNtStatusToWinError(Status);
        ClosePolicy(pPolicyHandle);
        return winerror;
    }
    else
        return winerror;
}
//---------------------------------------------------------------------------


void nt_security::ClosePolicy(PLSA_HANDLE policyHandle)
{
    if (policyHandle != NULL)
        LsaClose(policyHandle);
}
//---------------------------------------------------------------------------


/*
void nt_security::InitLsaString(LPWSTR inString, PLSA_UNICODE_STRING LsaString)
{
    DWORD StringLength;

    if (inString == NULL) 
    {
        LsaString->Buffer = NULL;
        LsaString->Length = 0;
        LsaString->MaximumLength = 0;
        return;
    }

    StringLength = wcslen(inString);
    LsaString->Buffer = inString;
    LsaString->Length = (USHORT) StringLength * sizeof(WCHAR);
    LsaString->MaximumLength=(USHORT)(StringLength+1) * sizeof(WCHAR);
}
*/
//---------------------------------------------------------------------------


DWORD nt_security::SetPrivilegeOnAccount(LSA_HANDLE PolicyHandle,       // open policy handle
                                            PSID AccountSid,               // SID to grant privilege to
                                            const astring &PrivilegeName,  // privilege to grant (Unicode)
                                            bool bEnable)                  // enable or disable
{
    NTSTATUS Status;
    DWORD winerror = 0;

    // Create a LSA_UNICODE_STRING for the privilege name.
    //
    transcode_to_utf16 temp_priv(PrivilegeName);
    LSA_UNICODE_STRING privs;
    privs.Buffer = (PWSTR)(UTF16 *)temp_priv;
    privs.Length = PrivilegeName.length() * (int)sizeof(UTF16);
    privs.MaximumLength = (PrivilegeName.length() +1) * (int)sizeof(UTF16);

    // grant or revoke the privilege, accordingly
    //
    if (bEnable) 
    {
        Status = LsaAddAccountRights(PolicyHandle,       // open policy handle
                                     AccountSid,         // target SID
                                     &privs,   // privileges
                                     1);                 // privilege count
    }
    else 
    {
        Status = LsaRemoveAccountRights(PolicyHandle,       // open policy handle
                                        AccountSid,         // target SID
                                        FALSE,              // do not disable all rights
                                        &privs,   // privileges
                                        1);                 // privilege count
    }

    if (Status == STATUS_SUCCESS)
        return winerror;
    else
    {
        winerror = LsaNtStatusToWinError(Status);
        return winerror;
    }
}
//---------------------------------------------------------------------------


DWORD nt_security::SetPrivilegeOnUser(const astring &domain,
                                         const astring &user,
                                         const astring &privilege,
                                         bool bEnable)
{
    LSA_HANDLE policyHandle;
    PSID psid = NULL;
    DWORD winerror = 0;

    // Open the policy on the target machine.
    //
    winerror = OpenPolicy(domain, (POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES), &policyHandle);
    if (winerror != 0)
        return winerror;

    // Obtain the SID of the user/group.
    // Note that we could target a specific machine, but we don't.
    // Specifying NULL for target machine searches for the SID in the
    // following order: well-known, Built-in and local, primary domain,
    // trusted domains.
    //
    psid = GetUserSID(user);
    if (psid == NULL)
    {
        ClosePolicy(&policyHandle);
        return ERROR_NO_SUCH_USER;
    }

    // Grant the SeServiceLogonRight to users represented by psid.
    //
    winerror = SetPrivilegeOnAccount(policyHandle, psid, privilege, bEnable);

    // Close the policy handle.
    //
    ClosePolicy(&policyHandle);

    //
    // Free memory allocated for SID.
    //
    if (psid != NULL) 
        HeapFree(GetProcessHeap(), 0, psid);

    return winerror;
}

DWORD nt_security::AddUserToGroup(const astring &user_name, const astring &group_name)
{
   LOCALGROUP_MEMBERS_INFO_3 lgmi3;
   DWORD dwLevel = 3;
   DWORD totalEntries = 1;
   NET_API_STATUS nStatus;

// fwprintf(stderr, L"Usage: %s ServerName GroupName MemberAccountName-(DomainName\\AccountName)\n", argv[0]);

   // Set up the LOCALGROUP_MEMBERS_INFO_3 structure.
   // Assign the member account name in form of DomainName\AccountName
   transcode_to_utf16 temp_user(user_name);
   lgmi3.lgrmi3_domainandname = (LPWSTR)(UTF16 *)temp_user;

   // Call the NetLocalGroupAddMembers() function, specifying level 3.
   // Level 0 can use SID
   transcode_to_utf16 temp_group(group_name);
   nStatus = NetLocalGroupAddMembers(L"", (LPWSTR)(UTF16 *)temp_group, dwLevel,
       (LPBYTE)&lgmi3, totalEntries);

//printf("got error code %d\n" , nStatus);

   if (nStatus == ERROR_MEMBER_IN_ALIAS) nStatus = 0;  // not an error.

   return nStatus;
}

#endif  //win32



