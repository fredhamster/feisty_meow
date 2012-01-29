/*****************************************************************************\
*                                                                             *
*  Name   : create_guid                                                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    This program generates a globally unique identifier using the operating  *
*  system's support.  The resulting id can be used to tag items that must be  *
*  uniquely named.                                                            *
*                                                                             *
*******************************************************************************
* Copyright (c) 2006-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/application_shell.h>
#include <application/hoople_main.h>
#include <application/windoze_helper.h>
#include <basis/astring.h>
#include <loggers/console_logger.h>
#include <mathematics/chaos.h>
#include <structures/static_memory_gremlin.h>
#include <textual/string_manipulation.h>

#ifdef __WIN32__
  #include <comdef.h>
#endif

using namespace application;
using namespace basis;
using namespace loggers;
using namespace mathematics;
using namespace structures;
using namespace textual;

#define BASE_LOG(to_print) program_wide_logger::get().log(to_print, ALWAYS_PRINT)

// this is an example GUID in the DCE format:
//
//    {12345678-1234-1234-1234-123456789012}
//
// each position can be a hexadecimal digit, ranging from 0 to F.
// the full size is measured as 32 nibbles or 16 bytes or 128 bits.

class create_guid : public application_shell
{
public:
  create_guid() : application_shell() {}
  DEFINE_CLASS_NAME("create_guid");
  int execute();
};

int create_guid::execute()
{
//  FUNCDEF("execute");
  SETUP_CONSOLE_LOGGER;
#ifdef __UNIX__

// this is completely bogus for the time being.  it just produces a random
// number rather than a guid.
  #define add_random \
    faux_guid += astring(string_manipulation::hex_to_char \
        (randomizer().inclusive(0, 0xf)), 1)

  astring faux_guid("{");
  for (int i = 0; i < 8; i++) add_random;
  faux_guid += "-";
  for (int j = 0; j < 3; j++) {
    for (int i = 0; i < 4; i++) add_random;
    faux_guid += "-";
  }
  for (int i = 0; i < 8; i++) add_random;
  faux_guid += "}";
  BASE_LOG(faux_guid.lower());
#elif defined (__WIN32__)
  GUID guid;
  CoCreateGuid(&guid);
  const int BUFFER_SIZE = 1024;
  LPOLESTR wide_buffer = new WCHAR[BUFFER_SIZE + 4];
  StringFromGUID2(guid, wide_buffer, BUFFER_SIZE);
  const int BYTE_BUFFER_SIZE = BUFFER_SIZE * 2 + 4;
  char buffer[BYTE_BUFFER_SIZE];
  WideCharToMultiByte(CP_UTF8, 0, wide_buffer, -1, buffer, BYTE_BUFFER_SIZE,
      NULL, NULL);
  astring guid_text = buffer;
  delete [] wide_buffer;
  BASE_LOG(guid_text);
#else
  #error unknown operating system; no support for guids.
#endif

  return 0;
}

HOOPLE_MAIN(create_guid, )

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
#endif // __BUILD_STATIC_APPLICATION__

