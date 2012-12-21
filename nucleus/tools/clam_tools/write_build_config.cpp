/*****************************************************************************\
*                                                                             *
*  Name   : write_build_config                                                *
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

#include "write_build_config.h"

#include <application/hoople_main.h>
#include <application/windoze_helper.h>
#include <basis/functions.h>
#include <configuration/variable_tokenizer.h>
#include <filesystem/byte_filer.h>
#include <filesystem/filename.h>
#include <loggers/console_logger.h>
#include <loggers/critical_events.h>
#include <structures/set.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_table.h>
#include <versions/version_ini.h>

#include <stdio.h>

using namespace application;
using namespace basis;
using namespace configuration;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;
using namespace versions;

const int MAX_LINE_SIZE = 2048;
  //!< we should never see an ini line longer than this.

const int MAX_HEADER_FILE = 128 * KILOBYTE;
  //!< an excessively long allowance for the maximum generated header size.

const char *DEFINITIONS_STATEMENT = "DEFINITIONS";
  //!< the tag we see in the config file for directly compatible macros.

const char *EXPORT_STATEMENT = "export ";
  //!< a tag we see on variables to be inherited by subshells

// make conditionals that we will eat.
const char *IFEQ_STATEMENT = "ifeq";
const char *IFNEQ_STATEMENT = "ifneq";
const char *ENDIF_STATEMENT = "endif";

#undef LOG
#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger::get(), to_print)

write_build_config::write_build_config()
: application_shell(),
  _end_matter(new astring),
  _ver(new version),
  _nesting(0)
{}

write_build_config::~write_build_config()
{
  WHACK(_end_matter);
  WHACK(_ver);
}

const string_set &write_build_config::exclusions() 
{
  static string_set _hidden;
  static bool _initted = false;
  if (!_initted) {
    _hidden += "DEBUG";
    _hidden += "OPTIMIZE";
    _hidden += "STRICT_WARNINGS";
  }
  return _hidden;
}

// adds some more material to dump at the end of the file.
#define ADD_COMMENT_RETURN(sym, val) { \
  *_end_matter += astring("  ") + sym + " = " + val + "\n"; \
  return common::OKAY; \
}

outcome write_build_config::output_macro(const astring &symbol,
    const astring &value, astring &accumulator)
{
  // drop any excluded items to avoid interfering with devstu solution.
  if (exclusions().member(symbol))
    ADD_COMMENT_RETURN(symbol, value);
  // drop any malformed symbols or values.
  if (symbol.contains("\"") || value.contains("\""))
    ADD_COMMENT_RETURN(symbol, value);
  accumulator += "  #ifndef ";
  accumulator += symbol;
  accumulator += "\n";
  accumulator += "    #define ";
  accumulator += symbol;
  accumulator += " \"";
  accumulator += value;
  accumulator += "\"\n";
  accumulator += "  #endif\n";
  return common::OKAY;
}

bool write_build_config::process_version_parts(const astring &symbol,
    const astring &value)
{
  if (symbol.equal_to("major"))
    { _ver->set_component(version::MAJOR, value); return true; }
  if (symbol.equal_to("minor"))
    { _ver->set_component(version::MINOR, value); return true; }
  if (symbol.equal_to("revision"))
    { _ver->set_component(version::REVISION, value); return true; }
  if (symbol.equal_to("build"))
    { _ver->set_component(version::BUILD, value); return true; }
  return false;
}

bool write_build_config::check_nesting(const astring &to_check)
{
  if (to_check.compare(IFEQ_STATEMENT, 0, 0, int(strlen(IFEQ_STATEMENT)), true)
      || to_check.compare(IFNEQ_STATEMENT, 0, 0, int(strlen(IFNEQ_STATEMENT)), true)) {
    _nesting++;
    return true;
  }
  if (to_check.compare(ENDIF_STATEMENT, 0, 0, int(strlen(ENDIF_STATEMENT)), true)) {
    _nesting--;
    return true;
  }
  return false;
}

outcome write_build_config::output_decorated_macro(const astring &symbol_in,
    const astring &value, astring &cfg_accumulator, astring &ver_accumulator)
{
  // make sure we catch any conditionals.
  if (check_nesting(symbol_in))
    ADD_COMMENT_RETURN(symbol_in, value);
  // toss out any exclusions.
  if (exclusions().member(symbol_in))
    ADD_COMMENT_RETURN(symbol_in, value);
  if (symbol_in.contains("\"") || value.contains("\""))
    ADD_COMMENT_RETURN(symbol_in, value);
  if (symbol_in[0] == '[')
    ADD_COMMENT_RETURN(symbol_in, value);
  if (_nesting)
    ADD_COMMENT_RETURN(symbol_in, value);
  // switch the output stream based on whether its a version component or not.
  astring *the_accumulator = &cfg_accumulator;
  if (process_version_parts(symbol_in, value)) {
    the_accumulator = &ver_accumulator;
  }
  // add a special tag so that we won't be colliding with other names.
  astring symbol = astring("__build_") + symbol_in;
  return output_macro(symbol, value, *the_accumulator);
}

outcome write_build_config::output_definition_macro
    (const astring &embedded_value, astring &accumulator)
{
  FUNCDEF("output_definition_macro");
//LOG(astring("into output def with: ") + embedded_value);
  variable_tokenizer t;
  t.parse(embedded_value);
  if (!t.symbols())
    ADD_COMMENT_RETURN("bad definition", embedded_value);
  if (exclusions().member(t.table().name(0)))
    ADD_COMMENT_RETURN(t.table().name(0), t.table()[0]);
  if (_nesting)
    ADD_COMMENT_RETURN(t.table().name(0), t.table()[0]);
  return output_macro(t.table().name(0), t.table()[0], accumulator);
}

bool write_build_config::write_output_file(const astring &filename,
    const astring &new_contents)
{
  FUNCDEF("write_output_file");
  // now read the soon-to-be output file so we can check its current state.
  bool write_header = true;
  byte_filer check_header(filename, "rb");
  if (check_header.good()) {
    byte_array file_contents;
    int read = check_header.read(file_contents, MAX_HEADER_FILE);
if (read < 1) LOG("why is existing header contentless?");
    if (read > 0) {
      astring found(astring::UNTERMINATED, (char *)file_contents.observe(),
          read);
//LOG(astring("got existing content:\n-----\n") + found + "\n-----\n");
//LOG(astring("new_content has:\n-----\n") + new_contents + "\n-----\n");
      if (found == new_contents) {
        write_header = false;
      }
    }
  }
  // writing only occurs when we know that the build configurations have
  // changed.  if the file is the same, we definitely don't want to write
  // it because pretty much all the other files depend on it.
  if (write_header) {
    // we actually want to blast out a new file.
    byte_filer build_header(filename, "wb");
    if (!build_header.good())
      non_continuable_error(static_class_name(), func, astring("failed to create "
          "build header file in ") + build_header.name());
    build_header.write(new_contents);
    LOG(astring(static_class_name()) + ": wrote config to "
        + build_header.name());
  } else {
    // nothing has changed.
//    LOG(astring(static_class_name()) + ": config already up to date in "
//        + filename);
  }
  return true;
}

int write_build_config::execute()
{
  FUNCDEF("execute");
  SETUP_CONSOLE_LOGGER;  // override the file_logger from app_shell.

  // find our build ini file.
  astring repodir = environment::get("FEISTY_MEOW_DIR");

  // the below code should never be needed for a properly configured build.
#ifdef __WIN32__
  if (!repodir) repodir = "l:";
#else  // unix and other locations.
  if (!repodir)
    repodir = environment::get("HOME") + "/hoople";
#endif

  astring fname;
    // where we seek out our build settings.
  astring parmfile = environment::get("BUILD_PARAMETER_FILE");
  if (parmfile.t()) fname = parmfile;
  else fname = repodir + "/build.ini";

  // find our storage area for the build headers.  we know a couple build
  // configurations by now, but this should really be coming out of a config
  // file instead.
  astring library_directory = repodir + "/nucleus/library";
  if (!filename(library_directory).good()) {
    non_continuable_error(static_class_name(), func,
        astring("failed to locate the library folder storing the generated files."));
  }

  // these are very specific paths, but they really are where we expect to
  // see the headers.

  astring cfg_header_filename = library_directory + "/"
      "__build_configuration.h";
  astring ver_header_filename = library_directory + "/"
      "__build_version.h";

  // open the ini file for reading.
  byte_filer ini(fname, "r");
  if (!ini.good())
    non_continuable_error(static_class_name(), func, astring("failed to open "
        "build configuration file for reading at ") + ini.name());
//hmmm: parameterize the build ini thing above!

  // now we build strings that represents the output files we want to create.
  astring cfg_accumulator;
  astring ver_accumulator;

  // odd indentation below comes from the strings being c++ code that will be
  // written to a file.
//hmmm: another location to fix!!!
  cfg_accumulator += "\
#ifndef BUILD_SYSTEM_CONFIGURATION\n\
#define BUILD_SYSTEM_CONFIGURATION\n\n\
  // This file provides all of the code flags which were used when this build\n\
  // was generated.  Some of the items in the build configuration have been\n\
  // stripped out because they are not used.\n\n";
  ver_accumulator += "\
#ifndef BUILD_VERSION_CONFIGURATION\n\
#define BUILD_VERSION_CONFIGURATION\n\n\
  // This file provides the version macros for this particular build.\n\n";

  // iterate through the entries we read in earlier and generate our header.
  astring symbol, value;
  astring buffer;
  while (!ini.eof()) {
    int chars = ini.getline(buffer, MAX_LINE_SIZE);
    if (!chars) continue;  // hmmm.
    
    variable_tokenizer t;
    t.parse(buffer);
    if (!t.symbols()) continue;  // not so good.

    // pull out the first pair we found and try to parse it.
    symbol = t.table().name(0);
    value = t.table()[0];
    symbol.strip_spaces(astring::FROM_BOTH_SIDES);

    // clean out + characters that can come from += declarations.
    while ( (symbol[symbol.end()] == '+') || (symbol[symbol.end()] == ':') ) {
      symbol.zap(symbol.end(), symbol.end());
      symbol.strip_spaces(astring::FROM_END);
    }

    if (symbol[0] == '#') continue;  // toss out any comments.

    if (!symbol) continue;  // seems like that one didn't work out so well.

    if (symbol.compare(EXPORT_STATEMENT, 0, 0, int(strlen(EXPORT_STATEMENT)), true)) {
      // clean out export statements in front of our variables.
      symbol.zap(0, int(strlen(EXPORT_STATEMENT) - 1));
    }

    // check for a make-style macro definition.
    if (symbol.compare(DEFINITIONS_STATEMENT, 0, 0, int(strlen(DEFINITIONS_STATEMENT)), true)) {
      // found a macro definition.  write that up after pulling the real
      // contents out.
      output_definition_macro(value, cfg_accumulator);
    } else {
      // this one is hopefully a very tasty specialized macro.  we will
      // show it with added text to make it unique.
      output_decorated_macro(symbol, value, cfg_accumulator, ver_accumulator);
    }
  }

  // write some calculated macros now.
  ver_accumulator += "\n";
  ver_accumulator += "  // calculated macros are dropped in here.\n\n";

  // we write our version in a couple forms.  hopefully we accumulated it.

  // this one is the same as the file version currently (below), but may go to
  // a different version at some point.
  ver_accumulator += "  #define __build_SYSTEM_VERSION \"";
  ver_accumulator += _ver->flex_text_form();
  ver_accumulator += "\"\n\n";

  // we drop in the version as numbers also, since the version RC wants that.
  ver_accumulator += "  #define __build_FILE_VERSION_COMMAS ";
  ver_accumulator += _ver->flex_text_form(version::COMMAS);
  ver_accumulator += "\n";
  // another form of the file version for dotted notation.
  ver_accumulator += "  #define __build_FILE_VERSION \"";
  ver_accumulator += _ver->flex_text_form();
  ver_accumulator += "\"\n";

  // product version is just the first two parts.
  _ver->set_component(version::REVISION, "0");
  _ver->set_component(version::BUILD, "0");
  // product version as a list of numbers.
  ver_accumulator += "  #define __build_PRODUCT_VERSION_COMMAS ";
  ver_accumulator += _ver->flex_text_form(version::COMMAS);
  ver_accumulator += "\n";
  // another form of the product version for use as a string.
  ver_accumulator += "  #define __build_PRODUCT_VERSION \"";
  ver_accumulator += _ver->flex_text_form(version::DOTS, version::MINOR);
  ver_accumulator += "\"\n";

  // write a blob of comments at the end with what we didn't use.
  cfg_accumulator += "\n";
  cfg_accumulator += "/*\n";
  cfg_accumulator += "These settings were not used:\n";
  cfg_accumulator += *_end_matter;
  cfg_accumulator += "*/\n";
  cfg_accumulator += "\n";
  cfg_accumulator += "#endif /* outer guard */\n\n";

  // finish up the version file also.
  ver_accumulator += "\n";
  ver_accumulator += "#endif /* outer guard */\n\n";

  if (!write_output_file(cfg_header_filename, cfg_accumulator)) {
    LOG(astring("failed writing output file ") + cfg_header_filename);
  }
  if (!write_output_file(ver_header_filename, ver_accumulator)) {
    LOG(astring("failed writing output file ") + ver_header_filename);
  }

  return 0;
}

HOOPLE_MAIN(write_build_config, )

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

