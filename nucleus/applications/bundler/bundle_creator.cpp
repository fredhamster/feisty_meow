
//hmmm: anything related to _stub_size should be kept, but that is where
//      we need a redundant search mechanism that can't be fooled so easily
//      by modifying exe; make a pattern that will be found and is the first
//      place to start looking for manifest.

/*****************************************************************************\
*                                                                             *
*  Name   : bundle_creator                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2006-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "common_bundle.h"

#include <application/hoople_main.h>
#include <application/command_line.h>
#include <basis/array.h>
#include <basis/byte_array.h>
#include <basis/environment.h>
#include <configuration/application_configuration.h>
#include <configuration/ini_configurator.h>
#include <configuration/variable_tokenizer.h>
#include <filesystem/byte_filer.h>
#include <filesystem/directory.h>
#include <filesystem/filename.h>
#include <filesystem/file_time.h>
#include <loggers/console_logger.h>
#include <loggers/file_logger.h>
#include <processes/launch_process.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_table.h>
#include <textual/byte_formatter.h>
#include <textual/list_parsing.h>
#include <textual/parser_bits.h>
#include <timely/time_stamp.h>

#include <stdio.h>
#include <sys/stat.h>
#include <zlib.h>
//#ifdef __WIN32__
  //#include <io.h>
//#endif

using namespace application;
using namespace basis;
using namespace configuration;
using namespace filesystem;
using namespace loggers;
using namespace filesystem;
using namespace processes;
using namespace structures;
using namespace textual;
using namespace timely;

const int CHUNKING_SIZE = 256 * KILOBYTE;
  // we'll read this big a chunk from a source file at a time.

const astring SUBVERSION_FOLDER = ".svn";
  // we don't want to include this in a bundle.

#define BASE_LOG(to_print) program_wide_logger::get().log(to_print, ALWAYS_PRINT)
#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger::get(), to_print)

//#define DEBUG_BUNDLER
  // uncomment for noisy debugging version.

// returns the "retval" and mentions that this is a failure at "where".
#define FAIL_RETURN(retval, where) { \
  LOG(astring("failure in ") + where + a_sprintf(", exit=%d", retval)); \
  return retval; \
}

////////////////////////////////////////////////////////////////////////////

bool true_value(const astring &value)
{ return (!value.equal_to("0")) && (!value.equal_to("false")); }

////////////////////////////////////////////////////////////////////////////

// this structure overrides the manifest_chunk by providing a source string.

struct bundled_chunk : manifest_chunk
{
  astring _source;  //!< where the file comes from on the source system.
  virtual ~bundled_chunk() {}
};

////////////////////////////////////////////////////////////////////////////

// main bundler class.

class bundle_creator : public application_shell
{
public:
  bundle_creator()
      : application_shell(),
        _app_name(filename(_global_argv[0]).basename()),
        _bundle(NULL_POINTER), _stub_size(0), _keyword() {}

  virtual ~bundle_creator() {
    WHACK(_bundle);
  }

  DEFINE_CLASS_NAME("bundle_creator");
  virtual int execute();
  int print_instructions();

  astring determine_stub_file_and_validate();
    //!< returns the stub file location if it could be successfully located.

  int open_output_file();
    //!< prepares the output file to be written into.
    /*!< non-zero return indicates an error. */

  int read_manifest();
    //!< reads manifest definition specifying files in the bundle.
    /*!< creates the list of bundle pieces. */

  int write_stub_and_toc();
    //!< stuffs the unpacker stub into output file and table of contents.

  int bundle_sources();
    //!< reads all of the input files and dumps them into the bundle.

  int finalize_file();
    //!< puts finishing touches on the output file and closes it.

  int write_offset();
    //!< writes the offset position into the output file.
    /*!< this happens at the specially marked location (muftiloc). */

  int patch_recursive_target(const astring &source, const astring &target,
          int manifest_index);
    //!< processes the recursive target specified in "curr".
    /*!< the manifest_index tells the function where the external caller
    is currently working on the manifest.  new items will appear just after
    that index. */

  int recurse_into_dir(const astring &source, const astring &target,
          int manifest_index);
    //!< adds all files from "source" to our list, recurses on dirs.

  int patch_wildcard_target(const astring &source, const astring &target,
          int manifest_index);
    //!< processes the wildcard bearing target specified in "curr".
    /*!< any new source items will get dropped on the end of the manifest. */

  int add_files_here(directory &dirndl, const astring &source,
          const astring &target, int manifest_index);
    //!< takes all the files found in "source" and adds them to manifest.

  bool get_file_size(const astring &file, un_int &size, byte_array &timestamp);
    //!< returns the file "size" and "timestamp" found for "file".

private:
  astring _app_name;  //!< application name for this program.
  astring _output_file;  //!< what bundle file to create.
  astring _manifest_file;  //!< the manifest of what's included in bundle.
  array<bundled_chunk> _manifest_list;  //!< the parsed list of contents.
  byte_filer *_bundle;  //!< points at the bundled output file.
  int _stub_size;  //!< where the TOC will be located.
  astring _keyword;  // set if we were given a keyword on cmd line.
};

////////////////////////////////////////////////////////////////////////////

int bundle_creator::print_instructions()
{
  BASE_LOG(a_sprintf("\
%s: This program needs two parameters on the command line.\n\
The -o flag must point at the bundled output file to create.  The -m flag\n\
must point at a valid manifest file that defines what will be packed into\n\
the output file.  See the example manifest in the bundler example\n\
(in setup_src/bundle_example) for more information on the required file\n\
format.\n\
", _app_name.s()));
  return 4;
}

int bundle_creator::execute()
{
  FUNCDEF("execute");

  BASE_LOG(astring("starting file bundling at ") + time_stamp::notarize(false));

  command_line cmds(_global_argc, _global_argv);
  astring temp;
  if (cmds.get_value('?', temp)) return print_instructions();
  if (cmds.get_value("?", temp)) return print_instructions();
  if (!cmds.get_value('o', _output_file)) return print_instructions();
  if (!cmds.get_value('m', _manifest_file)) return print_instructions();

  if (filename(_output_file).exists()) {
    BASE_LOG(a_sprintf("\
%s: The output file already exists.  Please move it out of\n\
the way; this program will not overwrite existing files.\n",
_app_name.s()));
    return 3;
  }

  if (!filename(_manifest_file).exists()) {
    BASE_LOG(a_sprintf("\
%s: The manifest file does not exist.  This program cannot do anything\n\
without a valid packing manifest.\n", _app_name.s()));
    return 2;
  }

  // test this early on so we don't waste time uselessly.
  astring stub_file_okay = determine_stub_file_and_validate();
  if (!stub_file_okay) {
    BASE_LOG(a_sprintf("\
%s: The unpacking stub file does not exist (check binaries folder).\n\
Abandoning bundling process.\n", _app_name.s()));
    return 4;
  }

  // make sure we snag any keyword that was passed on the command line.
  cmds.get_value("keyword", _keyword);

  // first step is to provide some built-in variables that can be used to
  // make the manifests less platform specific.  this doesn't really help
  // if you bundle it on linux and try to run it on windows.  but either
  // platform's resources can easily be made into a bundle with the same
  // packing manifest.
#ifndef __WIN32__
  environment::set("EXE_END", "");  // executable file ending.
  environment::set("DLL_START", "lib");  // dll file prefix.
  environment::set("DLL_END", ".so");  // dll file ending.
#else
  environment::set("EXE_END", ".exe");
  environment::set("DLL_START", "");
  environment::set("DLL_END", ".dll");
#endif
  // specify a target variable on the source side so that we can operate in there,
  // even if the bundle doesn't specify one.  otherwise we can't run source side commands
  // properly if the paths are based on TARGET (like TMP often is).
  environment::set("TARGET", environment::TMP());

  int ret = 0;
  if ( (ret = read_manifest()) ) FAIL_RETURN(ret, "reading manifest");
    // read manifest to build list of what's what.
  if ( (ret = open_output_file()) ) FAIL_RETURN(ret, "opening output file");
    // open up our output file for the bundled chunks.
  if ( (ret = write_stub_and_toc()) ) FAIL_RETURN(ret, "writing stub and TOC");
    // writes the stub unpacker application and the table of contents to the 
    // output file.
  if ( (ret = bundle_sources()) ) FAIL_RETURN(ret, "bundling source files");
    // stuff all the source files into the output bundle.
  if ( (ret = finalize_file()) ) FAIL_RETURN(ret, "finalizing file");
    // finishes with the file and closes it up.
  if ( (ret = write_offset()) ) FAIL_RETURN(ret, "writing offset");
    // stores the offset of the TOC into the output file in a special location
    // that is delineated by a known keyword (muftiloc) and which should only
    // exist in the file in one location.

  return 0;
}

int bundle_creator::open_output_file()
{
  FUNCDEF("open_output_file");
  _bundle = new byte_filer(_output_file, "wb");
  if (!_bundle->good()) {
    LOG(astring("failed to open the output file: ") + _output_file);
    return 65;
  }
  return 0;
}

bool bundle_creator::get_file_size(const astring &infile, un_int &size,
    byte_array &time_stamp)
{
  FUNCDEF("get_file_size");
  time_stamp.reset();
  // access the source file to get its size.
  byte_filer source_file(infile, "rb");
  if (!source_file.good()) {
    LOG(astring("could not access the file for size check: ") + infile);
    return false;
  }
  size = int(source_file.length());
  file_time tim(infile);
  tim.pack(time_stamp);
  return true;
}

int bundle_creator::add_files_here(directory &dirndl, const astring &source,
    const astring &target, int manifest_index)
{
  FUNCDEF("add_files_here");
  for (int i = 0; i < dirndl.files().length(); i++) {
    astring curry = dirndl.files()[i];
    // skip .svn folders and contents.
    if (curry.contains(SUBVERSION_FOLDER)) continue;
//hmmm: this could be a much nicer generalized file exclusion list.

//LOG(astring("file is: ") + curry);
    bundled_chunk new_guy;
    new_guy._source = source + "/" + curry;  // the original full path to it.
    new_guy._payload = target + "/" + curry;
    new_guy._keywords = _manifest_list[manifest_index]._keywords;
    // copy the flags from the parent, so we don't forget options.
    new_guy._flags = _manifest_list[manifest_index]._flags;
    // remove some flags that make no sense for the new guy.
    new_guy._flags &= ~RECURSIVE_SRC;

//LOG(a_sprintf("adding: source=%s targ=%s", new_guy._source.s(), new_guy._payload.s()));
    bool okaysize = get_file_size(new_guy._source, new_guy._size, new_guy.c_filetime);
    if (!okaysize || (new_guy._size < 0) ) {
      LOG(astring("failed to get file size for ") + new_guy._source);
      return 75;
    }

    _manifest_list.insert(manifest_index + 1, 1);
    _manifest_list[manifest_index + 1] = new_guy;
  }
  return 0;
}

int bundle_creator::recurse_into_dir(const astring &source,
    const astring &target, int manifest_index)
{
  FUNCDEF("recurse_into_dir");
//LOG(astring("src=") + source + " dest=" + target);

  // we won't include the subversion folder.
  if (source.contains(SUBVERSION_FOLDER)) return 0;

  string_array dirs;  // culled from the directory listing.
  {
    // don't pay for the directory object on the recursive invocation stack;
    // just have what we need on the stack (the directory list).
    directory dirndl(source);
//check dir for goodness!
    int ret = add_files_here(dirndl, source, target, manifest_index);
      // add in just the files that were found.
    if (ret != 0) {
      // this is a failure, but the function complains about it already.
      return 75;
    }
    dirs = dirndl.directories();
  }

//LOG("now scanning directories...");

  // now scan across the directories we found.
  for (int i = 0; i < dirs.length(); i++) {
    astring s = dirs[i];
//LOG(astring("curr dir is ") + s);
    int ret = recurse_into_dir(source + "/" + s, target + "/"
        + s, manifest_index);
    if (ret != 0) return ret;  // bail out.
  }

  return 0;
}

int bundle_creator::patch_recursive_target(const astring &source,
    const astring &target, int manifest_index)
{
  FUNCDEF("patch_recursive_target");
//LOG(astring("patch recurs src=") + source + " targ=" + target);
  return recurse_into_dir(source, target, manifest_index);
}

int bundle_creator::patch_wildcard_target(const astring &source,
    const astring &target, int manifest_index)
{
  FUNCDEF("patch_wildcard_target");
  // find the last slash.  the rest is our wildcard component.
  int src_end = source.end();
  int slash_indy = source.find('/', src_end, true);
  astring real_source = source.substring(0, slash_indy - 1);
  astring wild_pat = source.substring(slash_indy + 1, src_end);
//BASE_LOG(astring("got src=") + real_source + " wildpat=" + wild_pat);

  directory dirndl(real_source, wild_pat.s());
//check dir for goodness!
  int ret = add_files_here(dirndl, real_source, target, manifest_index);
  if (ret != 0) {
    // this is a failure, but the function complains about it already.
    return 75;
  }

  return 0;
}

int bundle_creator::read_manifest()
{
  FUNCDEF("read_manifest");
  ini_configurator ini(_manifest_file, configurator::RETURN_ONLY);
  string_table toc;
  bool worked = ini.get_section("toc", toc);
  if (!worked) {
    LOG(astring("failed to read TOC section in manifest:\n") + _manifest_file
        + "\ndoes that file exist?");
    return 65;
  }

//hmmm: make a class member.
  file_logger noisy_logfile(application_configuration::make_logfile_name
      ("bundle_creator_activity.log"));
  noisy_logfile.log(astring('-', 76));
  noisy_logfile.log(astring("Bundling starts at ") + time_stamp::notarize(false));

  // add enough items in the list for our number of sections.
  _manifest_list.insert(0, toc.symbols());
  astring value;  // temporary string used below.
  int final_return = 0;  // if non-zero, an error occurred.

#define BAIL(retval) \
  final_return = retval; \
  toc.zap_index(i); \
  _manifest_list.zap(i, i); \
  i--; \
  continue

  for (int i = 0; i < toc.symbols(); i++) {
    // read all the info in this section and store it into our list.
    astring section_name = toc.name(i);
    section_name.strip_spaces(astring::FROM_FRONT);
    if (section_name[0] == '#') {
//hmmm: this looks a bit familiar from bail macro above.  abstract out?
      toc.zap_index(i);
      _manifest_list.zap(i, i);
      i--;
      continue;  // skip comments.
    }

    // check for any keywords on the section.  these are still needed for
    // variables, which otherwise would skip the rest of the field checks.
    if (ini.get(section_name, "keyword", value)) {
///LOG(astring("into keyword processing--value held is ") + value);
      string_array keys;
      bool worked = list_parsing::parse_csv_line(value, keys);
      if (!worked) {
        LOG(astring("failed to parse keywords for section ")
            + section_name + " in " + _manifest_file);
        BAIL(82);
      }
///LOG(astring("parsed list is ") + keys.text_form());
      _manifest_list[i]._keywords = keys;
      astring dumped;
      list_parsing::create_csv_line(_manifest_list[i]._keywords, dumped);
      noisy_logfile.log(section_name + " keywords: " + dumped);
    }

    if (ini.get(section_name, "variable", value)) {
      // this is a variable assignment.  it is the only thing we care about
      // for this section, so the rest is ignored.
      variable_tokenizer zohre;
      zohre.parse(value);
      if (zohre.symbols() < 1) {
        LOG(astring("failed to parse a variable statement from ") + value);
        BAIL(37);
      }
      _manifest_list[i]._flags = SET_VARIABLE;  // not orred, just this.
      // set the two parts of our variable.
      _manifest_list[i]._payload = zohre.table().name(0);
      _manifest_list[i]._parms = zohre.table()[0];
      BASE_LOG(astring("will set ") + _manifest_list[i]._payload + " = "
          + _manifest_list[i]._parms);
      astring new_value = parser_bits::substitute_env_vars(_manifest_list[i]._parms);
      environment::set(_manifest_list[i]._payload, new_value);
          
#ifdef DEBUG_BUNDLER
      BASE_LOG(astring("** variable ") + _manifest_list[i]._payload + " should have value=" + new_value);
      BASE_LOG(astring("** variable ") + _manifest_list[i]._payload + " now does have value=" + environment::get(_manifest_list[i]._payload));
#endif

      continue;
    } else if (ini.get(section_name, "assert_defined", value)) {
      // they are just asking for a variable test, to see if a variable
      // that the installer needs is actually defined at unpacking time.
      _manifest_list[i]._payload = value;
      _manifest_list[i]._flags = TEST_VARIABLE_DEFINED;
      BASE_LOG(astring("will test ") + _manifest_list[i]._payload + " is "
          + "defined at unpacking time.");
      continue;
    }

    if (!ini.get(section_name, "source", _manifest_list[i]._source)) {
      // check whether they told us not to pack and it's executable.
      bool okay_to_omit_source = false;
      astring value2;
      if (ini.get(section_name, "no_pack", value)
          && ini.get(section_name, "exec_target", value2) ) {
        if (true_value(value) && true_value(value2)) {
          // this type of section doesn't need source declared.
          okay_to_omit_source = true;
        }
      }
      if (!okay_to_omit_source) {
        LOG(astring("failed to read the source entry for section ")
            + section_name + " in " + _manifest_file);
        BAIL(67);
      }
    }
    // fix meshugener backslashes so we can count on the slash direction.
    _manifest_list[i]._source.replace_all('\\', '/');

    if (!ini.get(section_name, "target", _manifest_list[i]._payload)) {
      // check whether they told us not to pack and it's executable.
      bool okay_to_omit_target = false;
      astring value2;
      if (ini.get(section_name, "no_pack", value)
          && ini.get(section_name, "exec_source", value2) ) {
        if (true_value(value) && true_value(value2)) {
          // this type of section doesn't need target declared.
          okay_to_omit_target = true;
        }
      }
      if (!okay_to_omit_target) {
        LOG(astring("failed to read the target entry for section ")
            + section_name + " in " + _manifest_file);
        BAIL(68);
      }
    }
    // fix backslashes in target also.
    _manifest_list[i]._payload.replace_all('\\', '/');

    // capture any parameters they have specified for exec or other options.
    if (ini.get(section_name, "parms", value)) {
      _manifest_list[i]._parms = value;
#ifdef DEBUG_BUNDLER
      BASE_LOG(astring("got parms for ") + section_name + " as: " + value);
#endif
      if (value[0] != '"') {
        // repair the string if we're running on windows.
        _manifest_list[i]._parms = astring("\"") + value + "\"";
      }
      noisy_logfile.log(section_name + " parms: " + _manifest_list[i]._parms);
    }

    // check for the ignore errors flag.
    if (ini.get(section_name, "error_okay", value)) {
      if (true_value(value))
        _manifest_list[i]._flags |= IGNORE_ERRORS;
    }

    // see if they are saying not to overwrite the target file.
    if (ini.get(section_name, "no_replace", value)) {
      if (true_value(value))
        _manifest_list[i]._flags |= NO_OVERWRITE;
    }

    // test whether they are saying not to complain about a failure with
    // our normal pop-up dialog (on winders).
    if (ini.get(section_name, "quiet", value)) {
      if (true_value(value))
        _manifest_list[i]._flags |= QUIET_FAILURE;
    }

    // did they want a backup of the original to be made, instead of
    // just overwriting the file?
    if (ini.get(section_name, "make_backup", value)) {
      if (true_value(value))
        _manifest_list[i]._flags |= MAKE_BACKUP_FILE;
    }

    // look for our recursion flag.
    if (ini.get(section_name, "recurse", value)) {
      if (true_value(value))
        _manifest_list[i]._flags |= RECURSIVE_SRC;
    } else {
      // the options here are only appropriate when the target is NOT set to
      // be recursive.

      if (ini.get(section_name, "no_pack", value)) {
        // allow either side to not be required if this is an executable.
        if (true_value(value))
          _manifest_list[i]._flags |= OMIT_PACKING;
      }

      // check if they have specified a source side executable.
      if (ini.get(section_name, "exec_source", value)) {
        if (true_value(value)) {
          _manifest_list[i]._flags |= SOURCE_EXECUTE;
        }
      } else {
        // check if they have specified a target side executable.  this is
        // mutually exclusive with a source side exec.
        if (ini.get(section_name, "exec_target", value)) {
          if (true_value(value))
            _manifest_list[i]._flags |= TARGET_EXECUTE;
        }
      }
    }

    // replace environment variables in the source now...
    _manifest_list[i]._source = parser_bits::substitute_env_vars
        (_manifest_list[i]._source, false);

    // look for wildcards in the source.
    int indy = _manifest_list[i]._source.find("*");

    // see if they specified a keyword on the command line and if this matches.
    // if not we need to abandon this item.
    if (!!_keyword && !_manifest_list[i]._keywords.member(_keyword)) {
      // their keyword choice didn't match what we were told to use.
      noisy_logfile.log(astring("skipping ") + _manifest_list[i]._payload
          + " file check; doesn't match keyword \"" + _keyword + "\"");
      continue;
    }

    // we only access the source file here if it's finalized.  we can't do
    // this if the target is supposed to be recursive or if it's got a wildcard
    // pattern in it.
    if (!(_manifest_list[i]._flags & RECURSIVE_SRC) && negative(indy)
        && !(_manifest_list[i]._flags & OMIT_PACKING) ) {
      // access the source file to get its size.
      byte_filer source_file(_manifest_list[i]._source, "rb");
      if (!source_file.good()) {
        LOG(astring("could not access the source file for bundling: ")
            + _manifest_list[i]._source);
        BAIL(69);
      }
      bool okaysize = get_file_size(_manifest_list[i]._source,
          _manifest_list[i]._size, _manifest_list[i].c_filetime);
      if (!okaysize || (_manifest_list[i]._size < 0) ) {
        // this is a failure, but the function complains about it already.
        BAIL(75);
      }
    }
  }

  // patch the manifest list for wildcards and recursive sources.
  for (int i = 0; i < _manifest_list.length(); i++) {
    bundled_chunk curr = _manifest_list[i];

    if (!!_keyword && !curr._keywords.member(_keyword)) {
      // this item's keyword doesn't match the one we were given, so skip it.
      noisy_logfile.log(astring("zapping entry for ") + curr._payload
          + "; doesn't match keyword \"" + _keyword + "\"");
      _manifest_list.zap(i, i);
      i--;  // skip back since we eliminated an index.
      continue;
    }

    if (curr._flags & SET_VARIABLE) {
      // we're done working on this.
      continue;
    } else if (curr._flags & TEST_VARIABLE_DEFINED) {
      // this also requires no further effort.
      continue;
    } else if (curr._flags & RECURSIVE_SRC) {
      // handle a recursive style target.
      int star_indy = curr._source.find("*");
      if (non_negative(star_indy)) {
        // this is currently illegal.  we don't allow recursion + wildcards.
        LOG(astring("illegal combination of recursion and wildcard: ")
            + curr._source);
        BAIL(70);
      }
      // handle the recursive guy.
      int ret = patch_recursive_target(curr._source, curr._payload, i);
      if (ret != 0) {
        LOG(astring("failed during packing of recursive source: ")
            + curr._source);
        BAIL(72);
      }
      // take this item out of the picture, since all contents got included.
      _manifest_list.zap(i, i);
      i--;  // skip back since we eliminated an index.
      continue;
    } else if (curr._flags & SOURCE_EXECUTE) {
      // we have massaged the current manifest chunk as much as we can, so now
      // we will execute the source item if that was specified.
      BASE_LOG(astring("launching ") + curr._source);
      if (!!curr._parms) {
        curr._parms = parser_bits::substitute_env_vars(curr._parms, false);
        BASE_LOG(astring("\tparameters ") + curr._parms);
      }
      BASE_LOG(astring('-', 76));
      basis::un_int kid;
      basis::un_int retval = launch_process::run(curr._source, curr._parms,
          launch_process::AWAIT_APP_EXIT, kid);
      if (retval != 0) {
        LOG(astring("failed to launch process, source=") + curr._source
            + ", with parms " + curr._parms);
        if (! (curr._flags & IGNORE_ERRORS) ) {
          BAIL(92);
        }
      }
      BASE_LOG(astring('-', 76));
      if (curr._flags & OMIT_PACKING) {
        // this one shouldn't be included in the package.
        _manifest_list.zap(i, i);
        i--;  // skip back since we eliminated an index.
      }
      continue;
    } else {
      // check for a wildcard.
      int star_indy = curr._source.find("*");
      if (negative(star_indy)) continue;  // simple targets are boring.
      // this does have a wildcard in it.  let's make sure it's in the right
      // place for a wildcard in our scheme.
      int slash_indy = curr._source.find('/', curr._source.end(), true);
      if (star_indy < slash_indy) {
        BASE_LOG(astring("illegal wildcard placement in ") + curr._source);
        BASE_LOG(astring("  (the wildcard must be in the last component of the path)"));
        BAIL(71);
      }
      // handle the wildcarded source.
      int ret = patch_wildcard_target(curr._source, curr._payload, i);
      if (ret != 0) {
        LOG(astring("failed during packing of wildcarded source: ")
            + curr._source);
        BAIL(73);
      }
      _manifest_list.zap(i, i);
      i--;  // skip back since we eliminated an index.
      continue;
    }
  }

#ifdef DEBUG_BUNDLER
  if (!final_return) {
    // we had a successful run so we can print this stuff out.
    LOG("read the following info from manifest:");
    for (int i = 0; i < _manifest_list.length(); i++) {
      bundled_chunk &curr = _manifest_list[i];
      BASE_LOG(a_sprintf("(%d) size %d, %s => %s", i, curr._size,
          curr._source.s(), curr._payload.s()));
    }
  }
#endif

  return final_return;
}

astring bundle_creator::determine_stub_file_and_validate()
{
  FUNCDEF("determine_stub_file_and_validate");
  // define our location to find the unpacking stub program.
//hmmm: make this a command line parameter.
#ifdef __UNIX__
  astring stub_filename("unpacker_stub");
#endif
#ifdef __WIN32__
  astring stub_filename("unpacker_stub.exe");
#endif
  astring repo_dir = "$RUNTIME_PATH";
  astring stub_file = parser_bits::substitute_env_vars
      (repo_dir + "/binaries/" + stub_filename, false);
  if (!filename(stub_file).exists()) {
    // we needed to find that to build the bundle.
    LOG(astring("could not find unpacking stub file at: ") + stub_file);
    return astring::empty_string();
  }
  return stub_file;
}

int bundle_creator::write_stub_and_toc()
{
  FUNCDEF("write_stub_and_toc");

  astring stub_file = determine_stub_file_and_validate();
  if (!stub_file) return 1;
 
  // make sure the stub is accessible.
  byte_filer stubby(stub_file, "rb");
  if (!stubby.good()) {
    FAIL_RETURN(80, astring("could not read the unpacking stub at: ") + stub_file);
  }
  _stub_size = int(stubby.length());  // get the stub size for later reference.
  byte_array whole_stub;
  stubby.read(whole_stub, _stub_size + 100);
  stubby.close();
  _bundle->write(whole_stub);

  byte_array packed_toc_len;
  structures::obscure_attach(packed_toc_len, _manifest_list.length());
  int ret = _bundle->write(packed_toc_len);
  if (ret < 0) {
    LOG(astring("could not write the TOC length to the bundle: ")
        + _output_file);
    return 81;
  }

  // dump out the manifest list in our defined format.
  for (int i = 0; i < _manifest_list.length(); i++) {
    bundled_chunk &curr = _manifest_list[i];
//LOG(a_sprintf("flag %d is %d", i, curr._flags));
    byte_array chunk;
    curr.pack(chunk);
    if (_bundle->write(chunk) <= 0) {
      LOG(a_sprintf("could not write item #%d [%s] to the bundle: ", i,
          curr._source.s())
          + _output_file);
      return 88;
    }
  }

  return 0;
}

int bundle_creator::bundle_sources()
{
  FUNCDEF("bundle_sources");
  // go through all the source files and append them to the bundled output.
  file_logger noisy_logfile(application_configuration::make_logfile_name
      ("bundle_creator_activity.log"));
  for (int i = 0; i < _manifest_list.length(); i++) {
    bundled_chunk &curr = _manifest_list[i];

    if (curr._flags & SET_VARIABLE) {
      // all we need to do is keep this in the manifest.
      noisy_logfile.log(astring("bundling: variable setting ") + curr._payload
          + "=" + curr._parms);
      continue;
    } else if (curr._flags & TEST_VARIABLE_DEFINED) {
      // just remember to test this when running the unpack.
      noisy_logfile.log(astring("bundling: test variable ") + curr._payload
          + " is defined.");
      continue;
    } else if (curr._flags & OMIT_PACKING) {
      // this one shouldn't be included in the package.
      continue;
    }

    noisy_logfile.log(astring("bundling: ") + curr._source);
    byte_filer source(curr._source, "rb");
    if (!source.good()) {
      LOG(a_sprintf("could not read item #%d for the bundle: \"", i)
          + curr._source + "\"");
      return 98;
    }

    byte_array compressed(256 * KILOBYTE);  // expand the buffer to start with.
    byte_array temp;  // temporary read buffer.

    // chew on the file a chunk at a time.  this allows us to easily handle
    // arbitrarily large files rather than reading their entirety into memory.
    int total_written = 0;
    do {
      int ret = source.read(temp, CHUNKING_SIZE);
      if (ret < 0) {
        LOG(a_sprintf("failed while reading item #%d: ", i) + curr._source);
        return 99;
      } 
      total_written += ret;  // add in what we expect to write.
      // skip compressing if there's no data.
      uLongf destlen = 0;
      bool null_chunk = false;
      if (ret == 0) {
        compressed.reset();
	null_chunk = true;
      } else {
        compressed.reset(int(0.1 * ret) + ret + KILOBYTE);
          // provide some extra space as per zlib instructions.  we're giving it
          // way more than they request.
        destlen = compressed.length();
        // pack the chunks first so we can know sizes needed.
        int comp_ret = compress(compressed.access(), &destlen, temp.observe(),
            temp.length());
        if (comp_ret != Z_OK) {
          LOG(a_sprintf("failed while compressing item #%d: ", i)
              + curr._source);
          return 99;
        }
        compressed.zap(destlen, compressed.length() - 1);
      }
      byte_array just_sizes;
      structures::obscure_attach(just_sizes, temp.length());
        // add in the real size.
      structures::obscure_attach(just_sizes, int(destlen));
        // add in the packed size.
      ret = _bundle->write(just_sizes);
      if (ret <= 0) {
        LOG(a_sprintf("failed while writing sizes for item #%d: ", i)
            + curr._source);
        return 93;
      }
      if (!null_chunk) {
        ret = _bundle->write(compressed);
        if (ret <= 0) {
          LOG(a_sprintf("failed while writing item #%d: ", i) + curr._source);
          return 93;
        } else if (ret != compressed.length()) {
          LOG(a_sprintf("wrote different size for item #%d (tried %d, "
              "wrote %d): ", i, compressed.length(), ret) + curr._source);
          return 93;
        }
      }
    } while (!source.eof());
//hmmm: very common code to above size writing.
    byte_array just_sizes;
    structures::obscure_attach(just_sizes, -1);
    structures::obscure_attach(just_sizes, -1);
    int ret = _bundle->write(just_sizes);
    if (ret <= 0) {
      LOG(a_sprintf("failed while writing sentinel of item #%d: ", i)
          + curr._source);
      return 96;
    }
    source.close();
    if (total_written != curr._size) {
      LOG(a_sprintf("size (%d) disagrees with initial size (%d) for "
          "item #%d: ", total_written, curr._size, i) + curr._source);
    }
  }
  noisy_logfile.log(astring("Bundling run ends at ") + time_stamp::notarize(false));
  noisy_logfile.log(astring('-', 76));

  return 0;
}

int bundle_creator::finalize_file()
{
  _bundle->close();
  return 0;
}

int bundle_creator::write_offset()
{
  FUNCDEF("write_offset");
  byte_filer bun(_output_file, "r+b");  // open the file for updating.

  astring magic_string("muftiloc");  // our sentinel string.
  astring temp_string;  // data from the file.

  while (!bun.eof()) {
    // find the telltale text in the file.
    bool found_it = false;  // we'll set this to true if we see the string.
    int location = 0;  // where the sentinel's end is.
    for (int i = 0; i < magic_string.length(); i++) {
      int ret = bun.read(temp_string, 1);
      if (ret <= 0) break;
      if (temp_string[0] != magic_string[i]) break;  // no match.
      if (i == magic_string.end()) {
        // we found a match to our string!
        found_it = true;
        location = int(bun.tell());
//LOG(a_sprintf("found the sentinel in the file!  posn=%d", location));
      }
    }
    if (!found_it) continue;  // keep reading.
    bun.seek(location);
    byte_array packed_offset;
    structures::obscure_attach(packed_offset, _stub_size);
//LOG(astring("pattern of len is:\n") + byte_format::text_dump(packed_offset));
    // write the offset into the current position, which should be just after
    // the sentinel's location.
    bun.write(packed_offset);
//LOG(a_sprintf("wrote manifest offset before posn=%d", bun.tell()));
    break;  // done with looking for that pattern.
  }
  bun.close();  // completely finished now.

  chmod(_output_file.s(), 0766);
    // make sure it's an executable file when we're done with it.

  BASE_LOG(astring("done file bundling at ") + time_stamp::notarize(false));

  return 0;
}

////////////////////////////////////////////////////////////////////////////

HOOPLE_MAIN(bundle_creator, )

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
  #include <filesystem/file_time.cpp>
  #include <loggers/combo_logger.cpp>
  #include <loggers/console_logger.cpp>
  #include <loggers/critical_events.cpp>
  #include <loggers/file_logger.cpp>
  #include <loggers/program_wide_logger.cpp>
  #include <processes/launch_process.cpp>
  #include <structures/bit_vector.cpp>
  #include <structures/checksums.cpp>
  #include <structures/object_packers.cpp>
  #include <structures/static_memory_gremlin.cpp>
  #include <structures/string_hasher.cpp>
  #include <structures/string_table.cpp>
  #include <structures/version_record.cpp>
  #include <textual/byte_formatter.cpp>
  #include <textual/list_parsing.cpp>
  #include <textual/parser_bits.cpp>
  #include <textual/string_manipulation.cpp>
  #include <timely/earth_time.cpp>
  #include <timely/time_control.cpp>
  #include <timely/time_stamp.cpp>
#endif // __BUILD_STATIC_APPLICATION__

