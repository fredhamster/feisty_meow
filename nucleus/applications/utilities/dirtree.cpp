/*
*  Name   : dirtree
*  Author : Chris Koeritz
*  Purpose:
*    A utility that shows the directory tree specified on the command line.

* Copyright (c) 2004-$now By Author.  This program is free software; you can
* redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either version 2 of
* the License or (at your option) any later version.  This is online at:
*     http://www.fsf.org/copyleft/gpl.html
* Please send any updates to: fred@gruntose.com
*/

#include <application/hoople_main.h>
#include <basis/guards.h>
#include <filesystem/directory_tree.h>
#include <filesystem/filename.h>
#include <loggers/console_logger.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_array.h>
#include <textual/string_manipulation.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger::get(), to_print)

class dirtree : public application_shell
{
public:
  dirtree() : application_shell() {}
  DEFINE_CLASS_NAME("dirtree");
  int execute();
  int print_instructions_and_exit() {
    LOG(a_sprintf("\
%s: This utility shows the sub-directory structure for a chosen directory.\n\
It expects a directory name to be provided on the command line.  If no\n\
directory is provided, then the current directory is assumed.  The sub-\n\
directories under the chosen directory will be displayed on the console in a\n\
stylized textual tree.  If a second parameter is provided, it is taken as a\n\
file pattern that causes matching files to be displayed.  Without a pattern,\n\
just the directory tree is shown.\n\
For example:\n\
  dirtree\n\
    => shows the directory structure of the current directory.\n\
  dirtree udon\n\
    => shows the structure of directory 'udon'\n\
  dirtree soba \"*.txt\"\n\
    => displays all text files and sub-directories of 'soba'\n\
", filename(application::_global_argv[0]).basename().raw().s()));
    return 23;
  }
};

astring hier_prefix(int depth, int kids)
{
  astring indent = string_manipulation::indentation( (depth - 1) * 2);
  if (!depth) return "";
  else if (!kids) return indent + "|--";
  else return indent + "+--";
}

int dirtree::execute()
{
  astring path;
 
//hmmm: we really need an abstraction to do some checking if they want --help;
//      this comparison way of doing it is lame.
astring helpword = astring("--help");

  if (application::_global_argc <= 1) {
    // plug in our default path if they gave us no parameters.
  	path = ".";
	} else {
		// they gave us some parameters.  but are they asking for help?
		if (helpword == astring(application::_global_argv[1])) {
			return print_instructions_and_exit();
		} else {
			// this seems like a serious path request.
			path = application::_global_argv[1];
		}
	}

  // check if we should show any of the files.
  bool show_files = false;
  astring pattern;
  if (application::_global_argc >= 3) {
    pattern = application::_global_argv[2];
  }
  if (pattern.t()) {
    show_files = true;
  }

//  log(astring("Scanning directory tree at \"") + path + "\"");
//  log(astring("Using pattern-match \"") + pattern + "\"");

  directory_tree dir(path, pattern.s(), !show_files);
  if (!dir.good()) {
    continuable_error(class_name(), "tree construction",
        "the directory could not be read");
    return print_instructions_and_exit();
  }

  dir_tree_iterator *ted = dir.start(directory_tree::prefix);
    // create our iterator to traverse the tree in prefix order.

  filename curr;  // the current path the iterator is at.
  string_array files;  // the filenames held at the iterator.
  int depth;  // current depth in tree.
  int kids;  // number of children below this node.

  while (directory_tree::current(*ted, curr, files)) {
    // we have a good directory to show.
    directory_tree::depth(*ted, depth);
    directory_tree::children(*ted, kids); 
    astring name_to_log = curr.basename().raw();
    if (!depth) {
      name_to_log = curr.raw();
    }
    LOG(hier_prefix(depth, kids) + name_to_log);
    if (show_files) {
      astring names;
      for (int i = 0; i < files.length(); i++) names += files[i] + " ";
      if (names.length()) {
        astring split;
        string_manipulation::split_lines(names, split, depth * 2 + 2);
        // strip eol chars off the string we got back, since we already add that in log.
        while (parser_bits::is_eol(split[split.end()])) {
        	split.zap(split.end(), split.end());
        }
        LOG(split);
      }
    }

    // go to the next place.
    directory_tree::next(*ted);
  }

  directory_tree::throw_out(ted);
  return 0;
}

HOOPLE_MAIN(dirtree, )

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
  #include <application/application_shell.cpp>
  #include <application/command_line.cpp>
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
  #include <filesystem/directory_tree.cpp>
  #include <filesystem/file_info.cpp>
  #include <filesystem/file_time.cpp>
  #include <filesystem/filename.cpp>
  #include <filesystem/filename_list.cpp>
  #include <filesystem/filename_tree.cpp>
  #include <filesystem/huge_file.cpp>
  #include <loggers/combo_logger.cpp>
  #include <loggers/console_logger.cpp>
  #include <loggers/critical_events.cpp>
  #include <loggers/file_logger.cpp>
  #include <loggers/program_wide_logger.cpp>
  #include <nodes/node.cpp>
  #include <nodes/packable_tree.cpp>
  #include <nodes/path.cpp>
  #include <nodes/tree.cpp>
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

