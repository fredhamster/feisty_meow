/*****************************************************************************\
*                                                                             *
*  Name   : vsts_version_fixer                                                *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2008-$now By Author.  This program is free software; you can  *
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
#include <basis/astring.h>
#include <basis/environment.h>
#include <basis/functions.h>
#include <filesystem/byte_filer.h>
#include <filesystem/directory.h>
#include <filesystem/filename.h>
#include <processes/launch_process.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_array.h>
#include <timely/time_stamp.h>
#include <versions/version_ini.h>

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)
#undef BASE_LOG
#define BASE_LOG(s) program_wide_logger::get().log(s, ALWAYS_PRINT)

using namespace application;
//using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace processes;
using namespace structures;
using namespace timely;
using namespace versions;

//#define DEBUG_VSTS_VERSION_FIXER
  // uncomment for noisy version.

////////////////////////////////////////////////////////////////////////////

class vsts_version_fixer : public application::application_shell
{
public:
  vsts_version_fixer() : application_shell() {}
  virtual ~vsts_version_fixer() {}

  virtual int execute();

  DEFINE_CLASS_NAME("vsts_version_fixer");

  void remove_confusing_files();
    //!< tosses out the generated files that confuse ms devstudio.

//move these
  typedef bool spider_method(const directory &current);
    //!< prototype for functions that are called during directory spidering.
    /*!< this function should do whatever work is needed on the items in
    that "current" directory.  true should be returned by this method when
    the traversal of the directory is still desired.  if there is a reason
    to stop traversing the directory hierarchy, then it should return false. */

//hmmm: support postfix and in order also.
//hmmm: support reporting where the spidering stopped.
  bool spider_directory(directory start, spider_method to_invoke);
    //!< traverses hierarchy "start" in prefix order while calling "to_invoke".
    /*!< true is returned if all invoked spider methods returned true.
    otherwise, false is returned. */
//move those

  bool perform_version_stamping(const filename &start_name);
    //!< finds all version ini files and applies stamps using them.

  void whack_in_subdirs(const directory &start,
      const string_array &file_whacks, const string_array &dir_whacks);
    //!< recursively cleans all items found in "file_whacks" and "dir_whacks".
    /*!< "file_whacks" is a list of file suffixes to whack.  for example, to
    remove all files matching a pattern *.exe, pass in just ".exe" in the
    "file_whacks".  the "dir_whacks" list is a list of directories to
    completely obliterate where found. */
};

HOOPLE_MAIN(vsts_version_fixer, )

////////////////////////////////////////////////////////////////////////////

//hmmm: move to a useful place; maybe even in directory class?
bool vsts_version_fixer::spider_directory(directory start,
    spider_method to_invoke)
{
  FUNCDEF("spider_directory");

  using namespace basis;

//LOG(astring("spider_directory: ") + start.path());
  // call our method on this directory first.  this ensures that we have
  // dealt with it before we spider off elsewhere.
  bool ret = to_invoke(start);
  if (!ret) return false;  // bail.

  // now let's look at the subdirectories.  we'll recurse on all of them in
  // the order listed.
  const string_array &dirs = start.directories();
//LOG(astring("dirs found to spider: ") + dirs.text_form());
  for (int dir_indy = 0; dir_indy < dirs.length(); dir_indy++) {
    const astring &current_dir = dirs[dir_indy];
//LOG(astring("currdir into ") + current_dir);
    if (current_dir.equal_to(".svn")) continue;  // skip this.
    if (current_dir.equal_to("CVS")) continue;  // skip this also.
    directory new_dir(start.path() + "/" + current_dir, start.pattern().observe());
    bool ret = spider_directory(new_dir, to_invoke);
    if (!ret) return false;  // bail from subdir issue.
  }
  // if we made it to here, everything was groovy.
  return true;
}

////////////////////////////////////////////////////////////////////////////

#define static_class_name() "vsts_version_fixer"

// global variables used to communicate with whacking_spider.
string_array global_file_whacks;
string_array global_dir_whacks;

bool whacking_spider(const directory &current)
{
  FUNCDEF("whacking_spider");
  using namespace basis;
//LOG(astring("whacking_spider: ") + current.path());
  // iterate across the files in the directory and check for evil ones.
  const string_array &files = current.files();
  for (int file_indy = 0; file_indy < files.length(); file_indy++) {
    const astring &current_file = files[file_indy];
//LOG(astring("currfile ") + current_file);
    // now iterate across our pattern list to see if this thing is
    // one of the offending files.
    for (int pat_indy = 0; pat_indy < global_file_whacks.length(); pat_indy++) {
//LOG(astring("currpat ") + global_file_whacks[pat_indy]);
      if (current_file.iends(global_file_whacks[pat_indy])) {
        filename goner(current.path() + "/" + current_file);
        BASE_LOG(astring("whack file: ") + goner.raw());
        goner.unlink();
        break;  // stop looking at the pattern list for matches.
      }
    }
  }

  // okay, now that we've cleaned out those files, let's look at the
  // subdirectories.
  const string_array &dirs = current.directories();
  for (int dir_indy = 0; dir_indy < dirs.length(); dir_indy++) {
    const astring &current_dir = dirs[dir_indy];
//LOG(astring("currdir ") + current_dir);
    for (int pat_indy = 0; pat_indy < global_dir_whacks.length(); pat_indy++) {
      if (current_dir.iequals(global_dir_whacks[pat_indy])) {
        filename goner(current.path() + "/" + current_dir);
        BASE_LOG(astring("whack dir: ") + goner.raw());
//hmmm: plug in recursive delete here instead.
basis::un_int kid;
launch_process::run("rm", astring("-rf ") + goner.raw(), launch_process::AWAIT_APP_EXIT, kid);
        break;  // skip remainder of patterns for this dir.
      }
    }
  }
  return true;
}

#undef static_class_name

////////////////////////////////////////////////////////////////////////////

void vsts_version_fixer::whack_in_subdirs(const directory &start,
    const string_array &file_whacks, const string_array &dir_whacks)
{
  FUNCDEF("whack_in_subdirs");
  using namespace basis;

  // save the lists so the spider method can see them.
  // note that this approach with a global variable would be bad if there
  // were concurrent invocations of the spidering, but we're not doing
  // that here.
  global_file_whacks = file_whacks;
  global_dir_whacks = dir_whacks;

  bool worked = spider_directory(start, whacking_spider);
  if (!worked) {
    LOG(astring("spidering of ") + start.path() + " failed for some reason.");
  }
}

////////////////////////////////////////////////////////////////////////////

#define static_class_name() "vsts_version_fixer"

basis::astring global_build_ini;

bool stamping_spider(const directory &current)
{
  FUNCDEF("stamping_spider");
  using namespace basis;
//LOG(astring("stamping_spider: ") + current.path());

  const string_array &files = current.files();
  for (int file_indy = 0; file_indy < files.length(); file_indy++) {
    const astring &current_file = files[file_indy];
//LOG(astring("currfile ") + current_file);
    // we won't process the "core_version.ini" file, which is a special
    // case that is somewhat well known as not being a file used (by us)
    // for dlls.
    if (current_file.ends("version.ini")
        && !current_file.iequals("core_version.ini") ) {
//LOG(astring("found ver file: ") + current.path() + "/" + current_file);
//
      astring versions_directory = environment::get("FEISTY_MEOW_GENERATED_STORE");
     // we keep our version files one level below the top of the generated store.
      versions_directory += "/versions";

      version_ini::one_stop_version_stamp(current.path() + "/" + current_file,
          versions_directory, global_build_ini, true);
    }
  }
  return true;
}

#undef static_class_name

////////////////////////////////////////////////////////////////////////////

bool vsts_version_fixer::perform_version_stamping(const filename &start_name)
{
  FUNCDEF("perform_version_stamping");
  directory start(start_name);
  return spider_directory(start, stamping_spider);
}

////////////////////////////////////////////////////////////////////////////

void vsts_version_fixer::remove_confusing_files()
{
  using namespace basis;
  // clean out a few directories that show up in the source tree from c#
  // projects compilation.  c# projects always rebuild every time anyways,
  // so this doesn't lose us any compilation time.  the only thing c#
  // projects don't ever seem to rebuild is their version resource, unless
  // they're forced to totally recompile like we cause below.
  string_array source_file_whacks;  // none right now.
  string_array source_dir_whacks;
  source_dir_whacks += "obj";
  source_dir_whacks += "Debug";
  source_dir_whacks += "Release";
  source_dir_whacks += "bin";
  source_dir_whacks += "temp_build";
  directory repo_source(environment::get("FEISTY_MEOW_APEX") + "/source");
  whack_in_subdirs(repo_source, source_file_whacks, source_dir_whacks);
  directory libra_src(environment::get("FEISTY_MEOW_APEX") + "/libraries");
  whack_in_subdirs(libra_src, source_file_whacks, source_dir_whacks);
  directory produ_src(environment::get("FEISTY_MEOW_APEX") + "/products");
  whack_in_subdirs(produ_src, source_file_whacks, source_dir_whacks);

/* this never helped.
  // clean out a variety of bad files in the objects hierarchy.
  // currently this is just the generated RES files which we have seen cause
  // vsts to think apps and dlls are up to date when they are actually not.
  directory repo_objects(environment::get("FEISTY_MEOW_APEX"));
  string_array objects_file_whacks;
  objects_file_whacks += ".res";
  string_array objects_dir_whacks;  // none right now.
  whack_in_subdirs(repo_objects, objects_file_whacks, objects_dir_whacks);
*/
}

int vsts_version_fixer::execute()
{
  FUNCDEF("execute");
  using namespace basis;
  log(time_stamp::notarize(true) + "vsts_version_fixer started.", ALWAYS_PRINT);

  remove_confusing_files();

  astring repo_dir = environment::get("FEISTY_MEOW_APEX");

  // figure out which build parameter file to use.
  global_build_ini = "";
  astring parmfile = environment::get("BUILD_PARAMETER_FILE");
  if (parmfile.t()) {
    global_build_ini = parmfile;
LOG(astring("found parm variable ") + parmfile);
  } else {
    // they didn't specify the file.  argh.
    global_build_ini = repo_dir + "/production/feisty_meow_config.ini";
    if (!filename(global_build_ini).exists()) {
LOG(astring("guess not found: ") + global_build_ini);
      LOG("cannot locate the build configuration file.");
      return 3; 
    }
  }

  // now stamp versions on everything we can find.
  filename repo_source = repo_dir + "/../../libraries";
  if (!repo_source.exists()) {
    repo_source = repo_dir + "/source";
    if (!repo_source.exists()) {
      LOG("cannot locate the main library source location.");
      return 3; 
    }
  }
LOG(astring("chose source dir as ") + repo_source);
  perform_version_stamping(repo_source);

  filename repo_apps = repo_dir + "/../../products";
  if (repo_apps.exists()) {
    perform_version_stamping(repo_apps);
  }
  log(time_stamp::notarize(true) + "vsts_version_fixer finished.", ALWAYS_PRINT);
  return 0;
}

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
  #include <filesystem/filename.cpp>
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
  #include <textual/parser_bits.cpp>
  #include <textual/string_manipulation.cpp>
  #include <timely/earth_time.cpp>
  #include <timely/time_control.cpp>
  #include <timely/time_stamp.cpp>
  #include <versions/version_ini.cpp>
#endif // __BUILD_STATIC_APPLICATION__

