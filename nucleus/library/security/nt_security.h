#ifndef NT_SECURITY_CLASS
#define NT_SECURITY_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : nt_security                                                       *
*  Author : Sue Richeson                                                      *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Provides a Win32 oracle for security questions.                          *
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



#include <basis/utf_conversion.h>


#include <Ntsecapi.h>

// Forward class declarations
#include <basis/astring.h>

class nt_security  
{
public:
  nt_security();
  virtual ~nt_security();

  static bool iequalsUsername(astring name1, astring name2);
    // Compares the two names for equality.  Treats backslash (\) and
    // forward slash (/} as equal.  Ignores case.  Treats the pipe (|) and
    // colon (:) as equal.

  static const astring &normalizeUsername(astring &username);
    // Makes the username all lowercase, converts any forward slash (/)
    // characters to back slashes (\), and converts any pipe characters (|)
    // to colon (:) characters.

  bool GetUserAndDomainName(astring &UserName, astring &DomainName);
    // This function is NT-specific.  It returns the user account name and
    // domain name of the currently logged in user on the machine on which
    // this class is executing.

  astring DomainBinding(const astring &domain);
    // This method will constsruct a distinguished name for the domain received.
  astring DomainUserBinding(const astring &domain, const astring &user_name);
    // This method will construct a distinguished name based on the domain and 
    // user name received.

  DWORD SetPrivilegeOnUser(const astring &domain, const astring &user,
          const astring &privilege, bool bEnable);
    // Sets or disables the privilege for the user in the given domain.
    // Can also be used to set the privilege on a group in the given domain.
    // Returns 0 if successful.  Returns Win32 error code if it fails.
    // Domain - can be blank, in which case the local machine is assumed; can be a machine
    //   name or a network domain name (although, having privilege to change a 
    //   priv in a network domain is highly unlikely and will probably result in 
    //   failure, false, return of this method). Ex:  "Legolas",  "Buildotron"
    // User - the account name for which to change the privilege.  It can include the 
    //   domain also.  Example user names:  "Fred", "Legolas/Bubba", "Buildotron/swbuld"
    //   Can also be a group name.  Examples:  "Administrators", "Legolas/Users"
    // privilege - name of the privilege to be enable/disabled.
    //   For a list of privilges, consult winnt.h, and search for SE_ASSIGNPRIMARYTOKEN_NAME.
    //   For a list of logon rights consult ntsecapi.h, and search for SE_BATCH_LOGON_NAME.
    // bEnable - true to enable the privilege; false to disable the privilege

  DWORD AddUserToGroup(const astring &user_name, const astring &group_name);
    // adds the "user_name" to the local group "group_name".  this only makes
    // the change on the local machine where this is run.

protected:

  PSID GetUserSID(const astring &user_name);
    // Retrieves the security descriptor (SID) for "user_name".
    // PSID is NULL if the method fails.

  DWORD OpenPolicy(const astring &serverName, DWORD DesiredAccess,
          PLSA_HANDLE pPolicyHandle);
    // Open the LSA policy on the given machine.
    // Returns 0 if successful.  Returns Win32 error code if it fails.

  void ClosePolicy(PLSA_HANDLE policyHandle);
    // Close the given LSA policy handle.

  DWORD SetPrivilegeOnAccount(LSA_HANDLE PolicyHandle,    // open policy handle
      PSID AccountSid,              // SID to grant privilege to
      const astring &PrivilegeName, // privilege to grant
      bool bEnable);                // enable or disable
    // Enable or disable the stated privilege on the given account.
    // Returns 0 if successful.  Returns Win32 error code if it fails.
    // PolicyHandle - must already have been opened prior to calling this method.
    // AccountSid - must already have been obtained prior to calling this method.
    // PrivilegeName - must be a valid security privilege name (case sensitive)
    //      For a list of privilges, consult winnt.h, and search for SE_ASSIGNPRIMARYTOKEN_NAME.
    //      For a list of logon rights consult ntsecapi.h, and search for SE_BATCH_LOGON_NAME.
    // bEnable - true to enable the privilege; false to disable the privilege

private:
  astring *m_sDirServiceProvider;  //!< the directory service provider name.
};

#endif

#endif

