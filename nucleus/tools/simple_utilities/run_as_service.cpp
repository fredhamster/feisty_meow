/*****************************************************************************\
*                                                                             *
*  Name   : run_as_service                                                    *
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

#include <loggers/console_logger.h>
#include <filesystem/filename.h>
#include <structures/static_memory_gremlin.h>
#include <security/nt_security.h>

HOOPLE_STARTUP_CODE;

//////////////

bool run_as_service(char *user, log_base &out)
{
#ifdef __WIN32__
  // ensure that the user has the "logon as a service" right.
  nt_security secu;
  long err = secu.SetPrivilegeOnUser("", user, "SeServiceLogonRight", true);
  if (err) {
    // that didn't work; probably the user name is bad?
    out.log(astring(astring::SPRINTF, "There was a problem giving "
        "\"%s\" the \"Logon as a Service\" right:\r\n%s", user,
        critical_events::system_error_text(err).s()));
    return false;
  }
#else
  astring junk = user;
  out.eol();
  junk += "";
#endif
  return true;
}

int main(int argc, char *argv[])
{
  console_logger out;
  if (argc < 2) {
    out.log(filename(argv[0]).rootname() + " usage:\n\
The first parameter must be a user name that will be given the\n\
\"login as a service\" access rights.\n");
    return 1;
  }
  bool did_it = run_as_service(argv[1], out);
  if (did_it)
    out.log(astring("Success giving \"") + argv[1] + "\" the 'login as service' rights.");
  else
    out.log(astring("Failed in giving \"") + argv[1] + "\" the 'login as service' rights!");
  return !did_it;
}

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
  #include <application/callstack_tracker.cpp>
  #include <basis/astring.cpp>
  #include <basis/common_outcomes.cpp>
  #include <basis/environment.cpp>
  #include <basis/guards.cpp>
  #include <basis/mutex.cpp>
  #include <basis/utf_conversion.cpp>
  #include <filesystem/filename.cpp>
  #include <loggers/console_logger.cpp>
  #include <security/nt_security.cpp>
  #include <security/win32_security.cpp>
  #include <structures/object_packers.cpp>
  #include <structures/static_memory_gremlin.cpp>
  #include <textual/parser_bits.cpp>
#endif // __BUILD_STATIC_APPLICATION__

