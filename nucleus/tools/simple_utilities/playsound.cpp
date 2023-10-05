/*****************************************************************************\
*                                                                             *
*  Name   : playsound                                                         *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    A program intended to be as simple as possible and to play only WAV      *
*  files on the win32 platform.  It was decided that this was needed because  *
*  windows media player suddenly became political and started complaining     *
*  when other programs were registered to play different sound types.         *
*    All this does is play WAV files.  That's it.                             *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/utf_conversion.h>
#include <basis/astring.h>
#include <application/windoze_helper.h>
#include <loggers/console_logger.h>
#include <structures/static_memory_gremlin.h>

#ifdef __WIN32__
  #include <mmsystem.h>
#endif

using namespace basis;
using namespace loggers;
using namespace structures;

///HOOPLE_STARTUP_CODE;
//hmmm: this needs to arise from obscurity too?

int main(int argc, char *argv[])
{
  console_logger out;
  if (argc < 2) {
    out.log(astring(argv[0]) + " usage:", ALWAYS_PRINT);
    out.log(astring("This program takes one or more parameters which are interpreted"), ALWAYS_PRINT);
    out.log(astring("as the names of sound files."), ALWAYS_PRINT);
    return 12;
  }
  for (int i = 1; i < argc; i++) {
    astring sound_file = argv[i];
out.log(a_sprintf("soundfile %d: %s", i, sound_file.s()), ALWAYS_PRINT);
    if (sound_file.ends(".mp3")) {
      out.log(astring("skipping MP3 file ") + sound_file, ALWAYS_PRINT);
      continue;
    }
#ifdef __WIN32__
    if (!PlaySound(to_unicode_temp(sound_file), NULL_POINTER, SND_FILENAME))
      out.log(astring("failed to play ") + sound_file, ALWAYS_PRINT);
#else
    out.log(astring("this program is a NO-OP, ignoring ") + sound_file, ALWAYS_PRINT);
#endif
  }
  return 0;
}

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
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
  #include <loggers/console_logger.cpp>
  #include <loggers/program_wide_logger.cpp>
  #include <structures/checksums.cpp>
  #include <structures/object_packers.cpp>
  #include <structures/static_memory_gremlin.cpp>
  #include <structures/string_hasher.cpp>
  #include <structures/string_table.cpp>
  #include <structures/version_record.cpp>
  #include <textual/parser_bits.cpp>
  #include <timely/earth_time.cpp>
  #include <timely/time_stamp.cpp>
#endif // __BUILD_STATIC_APPLICATION__

