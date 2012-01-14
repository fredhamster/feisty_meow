/*****************************************************************************\
*                                                                             *
*  Name   : ini_edit                                                          *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Provides command line ini editing capabilities.  These include both      *
*  reading of ini files and writing new entries to them.                      *
*                                                                             *
*******************************************************************************
* Copyright (c) 2006-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/hoople_main.h>
#include <configuration/ini_configurator.h>
#include <filesystem/filename.h>
#include <loggers/console_logger.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_table.h>
#include <textual/list_parsing.h>

#include <stdlib.h>

using namespace application;
using namespace basis;
using namespace configuration;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;

#undef LOG
#define LOG(to_print) program_wide_logger::get().log(to_print, ALWAYS_PRINT)

class ini_editor : public application_shell
{
public:
  ini_editor()
      : application_shell(),
        _app_name(filename(application::_global_argv[0]).basename()) {}
  DEFINE_CLASS_NAME("ini_editor");
  virtual int execute();
  int print_instructions();

private:
  astring _app_name;
};

int ini_editor::print_instructions()
{
  LOG(a_sprintf("\
%s: This program needs five parameters to process an ini file.\n\
There are two major operations, read and write.  The type of operation\n\
should be the first parameter.  The other parameters are similar for both\n\
operations, except for the last parameter.  These are as follows:\n\
Reading:\n\
\tread inifile section entry defaultvalue\n\
  This reads the \"inifile\" specified and looks for the \"section\" and\n\
\"entry\" name in the file.  It will either return (via standard output)\n\
the value found there or it will return the \"defaultvalue\".  No error\n\
will be raised if the entry is missing, but the default signals that no\n\
value was defined.\n\
  Additionally, if the entry name is the special value \"whole_section\",\n\
then the entire section will be read and returned as a CSV list.  If the\n\
section is empty, then the default string is returned instead.\n\
Writing:\n\
\twrite inifile section entry newvalue\n\
  This writes a new item with contents \"newvalue\" into the \"inifile\"\n\
in the \"section\" at the \"entry\" specified.  This should always succeed\n\
unless the ini file is not writable (in which case an error should be\n\
returned).  Nothing is send to standard output for a write operation.\n\
", _app_name.s()));
  return 23;
}

int ini_editor::execute()
{
  SETUP_CONSOLE_LOGGER;

  if (application::_global_argc < 6) return print_instructions();

  astring operation = application::_global_argv[1];
  bool read_op = true;
  if ( (operation[0] == 'w') || (operation[0] == 'W') ) read_op = false;
  astring ini_file = application::_global_argv[2];
  astring section = application::_global_argv[3];
  astring entry = application::_global_argv[4];
  astring value = application::_global_argv[5];
  ini_configurator ini(ini_file, ini_configurator::RETURN_ONLY);
  if (read_op) {
    // read the entry from the ini file.
    astring found;
    if (entry.equal_to("whole_section")) {
      // they want the whole section back at them.
      string_table found;
      bool worked = ini.get_section(section, found);
      if (!worked) program_wide_logger::get().log(value, ALWAYS_PRINT);  // default.
      else {
        // generate answer as csv.
        astring temp;
        list_parsing::create_csv_line(found, temp);
        program_wide_logger::get().log(temp, ALWAYS_PRINT);  // real value read.
      }
    } else {
      bool worked = ini.get(section, entry, found);
///      program_wide_logger::get().eol(log_base::NO_ENDING);
      if (!worked) program_wide_logger::get().log(value, ALWAYS_PRINT);  // default.
      else program_wide_logger::get().log(found, ALWAYS_PRINT);  // real value read.
    }
  } else {
    // write the entry to the ini file.
    bool worked = ini.put(section, entry, value);
    if (!worked) exit(28);  // failure to write the entry.
  }

  return 0;
}

HOOPLE_MAIN(ini_editor, )

