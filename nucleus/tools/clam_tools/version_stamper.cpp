/*****************************************************************************\
*                                                                             *
*  Name   : version_stamper                                                   *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1995-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/application_shell.h>
#include <application/command_line.h>
#include <application/hoople_main.h>
#include <application/windoze_helper.h>
#include <loggers/program_wide_logger.h>
#include <basis/astring.h>
#include <basis/enhance_cpp.h>
#include <filesystem/byte_filer.h>
#include <filesystem/directory.h>
#include <filesystem/filename.h>
#include <loggers/console_logger.h>
#include <structures/static_memory_gremlin.h>
#include <structures/version_record.h>
#include <versions/version_ini.h>

#undef LOG
#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger(), to_print)

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace versions;

//! This class creates resource information for applications and libraries to be version stamped.
/*!
  This creates a resource (.rc) file and a C++ header (.h) file when given the directory where
  a version information file (version.ini) resides.  It creates the files in that directory.
*/

class version_stamper : public application_shell
{
public:
  version_stamper();
  ~version_stamper();

  DEFINE_CLASS_NAME("version_stamper");

  int execute();
    // performs the main action of creating resource and code files.
};

//////////////

version_stamper::version_stamper() : application_shell()
{
}

version_stamper::~version_stamper() {}

int version_stamper::execute()
{
///  FUNCDEF("execute");
  SETUP_CONSOLE_LOGGER;  // override the file_logger from app_shell.
  if (application::_global_argc < 2) {
    log(astring("The directory where the 'version.ini' file is located\n"
        "must be specified as the first parameter of this program.  Another\n"
        "version file may optionally be specified as the second parameter of\n"
        "the program; the version contained in this file will be used to set\n"
        "the version of the file specified in the first parameter.\n"
        "Additionally, if the environment variable 'DEBUG' exists, then the\n"
        "generated RC file will be marked as a debug build.  Otherwise it is\n"
        "marked as a release build.  Note that the CLAM system automatically\n"
        "sets this for you.\n\n"), ALWAYS_PRINT);
    return 1;
  }

  astring path_name = application::_global_argv[1];
  astring source_version_file;  // blank by default.
  if (application::_global_argc > 2)
    source_version_file = application::_global_argv[2];
  bool ret = version_ini::one_stop_version_stamp(path_name, source_version_file, true);
  if (!ret) return 1;  // failure.
  return 0;  // success.
}

HOOPLE_MAIN(version_stamper, )

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
  #include <application/application_shell.cpp>
  #include <application/command_line.cpp>
  #include <basis/astring.cpp>
  #include <basis/common_outcomes.cpp>
  #include <basis/environment.cpp>
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
  #include <versions/version_ini.cpp>
#endif // __BUILD_STATIC_APPLICATION__

