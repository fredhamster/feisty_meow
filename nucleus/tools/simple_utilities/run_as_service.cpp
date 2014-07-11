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
  #include <basis/byte_array.cpp>
  #include <basis/callstack_tracker.cpp>
  #include <basis/utf_conversion.cpp>
  #include <basis/definitions.cpp>
  #include <basis/earth_time.cpp>
  #include <basis/guards.cpp>
  #include <basis/astring.cpp>
  #include <basis/log_base.cpp>
  #include <basis/memory_checker.cpp>
  #include <basis/mutex.cpp>
  #include <basis/contracts.h>
  #include <basis/outcome.cpp>
  #include <basis/packable.cpp>
  #include <basis/portable.cpp>
  #include <basis/trap_new.addin>
  #include <basis/untrap_new.addin>
  #include <basis/utility.cpp>
  #include <basis/version_record.cpp>
  #include <structures/bit_vector.cpp>
  #include <structures/byte_hasher.cpp>
  #include <structures/configurator.cpp>
  #include <structures/hash_table.h>
  #include <structures/pointer_hash.h>
  #include <structures/stack.h>
  #include <structures/static_memory_gremlin.cpp>
  #include <structures/string_hash.h>
  #include <structures/string_hasher.cpp>
  #include <structures/string_table.cpp>
  #include <structures/symbol_table.h>
  #include <structures/table_configurator.cpp>
  #include <loggers/console_logger.cpp>
  #include <loggers/file_logger.cpp>
  #include <loggers/locked_logger.cpp>
  #include <loggers/null_logger.cpp>
  #include <loggers/program_wide_logger.cpp>
  #include <filesystem/byte_filer.cpp>
  #include <application/command_line.cpp>
  #include <opsystem/critical_events.cpp>
  #include <filesystem/directory.cpp>
  #include <filesystem/filename.cpp>
  #include <configuration/ini_configurator.cpp>
  #include <opsystem/ini_parser.cpp>
  #include <configuration/application_configuration.cpp>
  #include <application/rendezvous.cpp>
  #include <security/nt_security.cpp>
  #include <security/win32_security.cpp>
  #include <textual/byte_formatter.cpp>
  #include <textual/parser_bits.cpp>
  #include <textual/string_manipulation.cpp>
  #include <configuration/variable_tokenizer.cpp>
#endif // __BUILD_STATIC_APPLICATION__

