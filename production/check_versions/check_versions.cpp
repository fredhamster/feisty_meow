/*****************************************************************************\
*                                                                             *
*  Name   : version checks                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Ensures that all the libraries have a version stamp and that they match  *
*  the version we expect.                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/portable.h>
#include <basis/string_array.h>
#include <basis/version_checker.h>
#include <basis/version_record.h>
#include <loggers/console_logger.h>
#include <opsystem/directory.h>
#include <loggers/file_logger.h>
#include <data_struct/static_memory_gremlin.h>

#include <__build_version.h>

HOOPLE_STARTUP_CODE;

#undef LOG
#define LOG(s) program_wide_logger().log(s)

static bool failure = false;
static string_array badness_list;

#define complain(where) { \
  LOG(istring("the file ") +  where + " failed the version check."); \
  badness_list += where; \
  failure = true; \
}

int main(int formal(argc), char *formal(argv)[])
{
  SET_DEFAULT_COMBO_LOGGER;
  // get our main repository directory for the source.
  istring repodir = portable::env_string("REPOSITORY_DIR");

  // find all the dlls.
#ifdef __WIN32__
  directory dlldir(repodir + "/dll", "*.dll");
#else
  directory dlldir(repodir + "/dll", "*.so");
#endif
  string_array dll_files = dlldir.files();

  // find all the exes.
#ifdef __WIN32__
  directory exedir(repodir + "/exe", "*.exe");
#else
  directory exedir(repodir + "/exe", "*");
#endif
  string_array exe_files = exedir.files();

  // set our path to include the dll and exe directories.
  istring path = dlldir.path() + ";" + exedir.path() + ";"
      + portable::env_string("PATH");
  portable::set_environ("PATH", path);
//LOG(istring("path is now: ") + portable::env_string("PATH"));

  // calculate the proper version.
  version good_version(__build_FILE_VERSION);
  LOG(istring("this is build ") + good_version.flex_text_form());

  for (int i = 0; i < dll_files.length(); i++) {
    const istring &current = dll_files[i];
    version found = version_checker::get_version(current);
    if (good_version != found)
      complain(current);
  }
  for (int i = 0; i < exe_files.length(); i++) {
    const istring &current = exe_files[i];
    // skip any obvious non-executable products.
    if ( (current == "manifest.txt") || (current == "paths.ini")
        || (current == "shutdown_list.dat") ) continue;
    version found = version_checker::get_version(current);
    if (good_version != found) {
      complain(current);
    }
  }

  istring lib_type = "release";
#ifdef _DEBUG
  lib_type = "debug";
#endif
  LOG(istring("finished checking ") + lib_type + " versions.");
  if (failure) {
    LOG("one or more version checks failed!  this is the full set:");
    LOG(badness_list.text_form());
  } else {
    LOG("all attempted version checks succeeded.");
  }

  return !!failure;
}

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
  #include <algorithms/sorts.cpp>
  #include <application/application_shell.cpp>
  #include <application/callstack_tracker.cpp>
  #include <application/windoze_helper.cpp>
  #include <basis/astring.cpp>
  #include <basis/common_outcomes.cpp>
  #include <basis/environment.cpp>
  #include <basis/guards.cpp>
  #include <basis/mutex.cpp>
  #include <basis/utf_conversion.cpp>
  #include <configuration/application_configuration.cpp>
  #include <configuration/configurator.cpp>
  #include <configuration/ini_configurator.cpp>
  #include <configuration/ini_parser.cpp>
  #include <configuration/table_configurator.cpp>
  #include <configuration/variable_tokenizer.cpp>
  #include <filesystem/byte_filer.cpp>
  #include <filesystem/directory.cpp>
  #include <filesystem/filename.cpp>
  #include <loggers/combo_logger.cpp>
  #include <loggers/console_logger.cpp>
  #include <loggers/critical_events.cpp>
  #include <loggers/file_logger.cpp>
  #include <loggers/program_wide_logger.cpp>
  #include <structures/bit_vector.cpp>
  #include <structures/checksums.cpp>
  #include <structures/object_packers.cpp>
  #include <structures/static_memory_gremlin.cpp>
  #include <structures/string_hasher.cpp>
  #include <structures/string_table.cpp>
  #include <structures/version_record.cpp>
  #include <textual/byte_formatter.cpp>
  #include <textual/parser_bits.cpp>
  #include <textual/string_manipulation.cpp>
  #include <timely/earth_time.cpp>
  #include <timely/time_stamp.cpp>
#endif // __BUILD_STATIC_APPLICATION__

