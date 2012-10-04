/*
*  Name   : application_configuration
*  Author : Chris Koeritz

* Copyright (c) 1994-$now By Author.  This program is free software; you can 
* redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either version 2 of
* the License or (at your option) any later version.  This is online at:
*     http://www.fsf.org/copyleft/gpl.html
* Please send any updates to: fred@gruntose.com
*/

#include "application_configuration.h"
#include "ini_configurator.h"

#include <application/windoze_helper.h>
#include <basis/environment.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/mutex.h>
#include <basis/utf_conversion.h>
#include <filesystem/directory.h>
#include <filesystem/filename.h>
#include <mathematics/chaos.h>
#include <structures/static_memory_gremlin.h>
#include <textual/parser_bits.h>

#ifdef __APPLE__
  #include <mach-o/dyld.h>
  #include <limits.h>
#endif
#ifdef __WIN32__
  #include <direct.h>
  #include <process.h>
#else
  #include <dirent.h>
#endif
#ifdef __UNIX__
  #include <sys/utsname.h>
  #include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

using namespace basis;
using namespace filesystem;
using namespace mathematics;
using namespace structures;
using namespace textual;

#undef LOG
#define LOG(to_print) printf("%s\n", astring(to_print).s())

namespace configuration {

const int MAXIMUM_COMMAND_LINE = 32 * KILOBYTE;
  // maximum command line that we'll deal with here.

#ifdef __UNIX__
astring application_configuration::get_cmdline_from_proc()
{
  FUNCDEF("get_cmdline_from_proc");
  static astring __check_once_app_path;
//hmmm: we want to use a single per app static synch here!
  if (__check_once_app_path.length()) return __check_once_app_path;

#ifdef __APPLE__
  __check_once_app_path = query_for_process_info();
  return __check_once_app_path;
#endif

  // we have not looked this app's name up in the path yet.
  a_sprintf cmds_filename("/proc/%d/cmdline", process_id());
  FILE *cmds_file = fopen(cmds_filename.s(), "r");
  if (!cmds_file) {
    LOG("failed to open our process's command line file.\n");
    return "unknown";
  }
//hmmm: this would be a lot nicer using a byte filer.
  size_t size = 2000;
  char *filebuff = new char[size + 1];
  ssize_t chars_read = getline((char **)&filebuff, &size, cmds_file);
    // read the first line, giving ample space for how long it might be.
  fclose(cmds_file);  // drop the file again.
  if (!chars_read || negative(chars_read)) {
    LOG("failed to get any characters from our process's cmdline file.\n");
    return "unknown";
  }
  // copy the buffer into a string, which works great since the entries in the
  // command line are all separated by zero characters.
  __check_once_app_path = filebuff;
  delete [] filebuff;
//printf("got an app name before chewing: %s\n", __check_once_app_path.s());
  // clean out quote characters from the name.
  for (int i = __check_once_app_path.length() - 1; i >= 0; i--) {
    if (__check_once_app_path[i] == '"') __check_once_app_path.zap(i, i);
  }
  // check if the thing has a path attached to it.  if it doesn't, we need to accentuate
  // our knowledge about the file.
  filename testing(__check_once_app_path);
  if (testing.had_directory()) return __check_once_app_path;  // all set.

//printf("no dir part found, app name after chewing: %s\n", __check_once_app_path.s());

//hmmm: the below might be better off as a find app in path method, which relies on which.
  // there was no directory component, so we'll try to guess one.
  astring temp_filename(environment::TMP()
      + a_sprintf("/zz_cmdfind.%d", chaos().inclusive(0, 999999999)));
  system((astring("which ") + __check_once_app_path + " >" + temp_filename).s());
  FILE *which_file = fopen(temp_filename.s(), "r");
  if (!which_file) {
    LOG("failed to open the temporary output from which.\n");
    return "unknown";
  }
  // reallocate the file buffer.
  size = 2000;
  filebuff = new char[size + 1];
  chars_read = getline((char **)&filebuff, &size, which_file);
  fclose(which_file);
  unlink(temp_filename.s());
  if (!chars_read || negative(chars_read)) {
    LOG("failed to get any characters from the which cmd output.\n");
    return "unknown";
  } else {
    // we had some luck using 'which' to locate the file, so we'll use this version.
    __check_once_app_path = filebuff;
    while (parser_bits::is_eol(__check_once_app_path[__check_once_app_path.end()])) {
      __check_once_app_path.zap(__check_once_app_path.end(), __check_once_app_path.end());
    }
  }
  delete [] filebuff;
  return __check_once_app_path;  // return whatever which told us.
}

// deprecated; better to use the /proc/pid/cmdline file.
astring application_configuration::query_for_process_info()
{
  FUNCDEF("query_for_process_info");
  astring to_return = "unknown";
  // we ask the operating system about our process identifier and store
  // the results in a temporary file.
  chaos rando;
  a_sprintf tmpfile("/tmp/proc_name_check_%d_%d.txt", process_id(),
      rando.inclusive(0, 128000));
#ifdef __APPLE__
  a_sprintf cmd("ps -o args=\"\" %d >%s", process_id(),
      tmpfile.s());
#else
  a_sprintf cmd("ps h -O \"args\" %d >%s", process_id(),
      tmpfile.s());
#endif
  // run the command to locate our process info.
  int sysret = system(cmd.s());
  if (negative(sysret)) {
    LOG("failed to run ps command to get process info");
    return to_return;
  }
  // open the output file for reading.
  FILE *output = fopen(tmpfile.s(), "r");
  if (!output) {
    LOG("failed to open the ps output file");
    return to_return;
  }
  // read the file's contents into a string buffer.
  char buff[MAXIMUM_COMMAND_LINE];
  size_t size_read = fread(buff, 1, MAXIMUM_COMMAND_LINE, output);
  if (size_read > 0) {
    // success at finding some text in the file at least.
    while (size_read > 0) {
      const char to_check = buff[size_read - 1];
      if ( !to_check || (to_check == '\r') || (to_check == '\n')
          || (to_check == '\t') )
        size_read--;
      else break;
    }
    to_return.reset(astring::UNTERMINATED, buff, size_read);
  } else {
    // couldn't read anything.
    LOG("could not read output of process list");
  }
  unlink(tmpfile.s());
  return to_return;
}
#endif

// used as a return value when the name cannot be determined.
#define SET_BOGUS_NAME(error) { \
  LOG(error); \
  if (output) { \
    fclose(output); \
    unlink(tmpfile.s()); \
  } \
  astring home_dir = environment::get("HOME"); \
  to_return = home_dir + "/failed_to_determine.exe"; \
}

astring application_configuration::application_name()
{
  FUNCDEF("application_name");
  astring to_return;
#ifdef __APPLE__
  char buffer[MAX_ABS_PATH] = { '\0' };
  uint32_t buffsize = MAX_ABS_PATH - 1;
  _NSGetExecutablePath(buffer, &buffsize);
  to_return = (char *)buffer;
#elif __UNIX__
  to_return = get_cmdline_from_proc();
#elif defined(__WIN32__)
  flexichar low_buff[MAX_ABS_PATH + 1];
  GetModuleFileName(NIL, low_buff, MAX_ABS_PATH - 1);
  astring buff = from_unicode_temp(low_buff);
  buff.to_lower();  // we lower-case the name since windows seems to UC it.
  to_return = buff;
#else
  #pragma error("hmmm: no means of finding app name is implemented.")
  SET_BOGUS_NAME("not_implemented_for_this_OS");
#endif
  return to_return;
}

#if defined(__UNIX__) || defined(__WIN32__)
  basis::un_int application_configuration::process_id() { return getpid(); }
#else
  #pragma error("hmmm: need process id implementation for this OS!")
  basis::un_int application_configuration::process_id() { return 0; }
#endif

astring application_configuration::current_directory()
{
  astring to_return;
#ifdef __UNIX__
  char buff[MAX_ABS_PATH];
  getcwd(buff, MAX_ABS_PATH - 1);
  to_return = buff;
#elif defined(__WIN32__)
  flexichar low_buff[MAX_ABS_PATH + 1];
  GetCurrentDirectory(MAX_ABS_PATH, low_buff);
  to_return = from_unicode_temp(low_buff);
#else
  #pragma error("hmmm: need support for current directory on this OS.")
  to_return = ".";
#endif
  return to_return;
}

// implement the software product function.
const char *application_configuration::software_product_name()
{
#ifdef GLOBAL_PRODUCT_NAME
  return GLOBAL_PRODUCT_NAME;
#else
  return "hoople"; 
#endif
}

astring application_configuration::application_directory()
{ return filename(application_name()).dirname().raw(); }

structures::version application_configuration::get_OS_version()
{
  version to_return;
#ifdef __UNIX__
  utsname kernel_parms;
  uname(&kernel_parms);
  to_return = version(kernel_parms.release);
#elif defined(__WIN32__)
  OSVERSIONINFO info;
  info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  ::GetVersionEx(&info);
  to_return = version(a_sprintf("%u.%u.%u.%u", basis::un_short(info.dwMajorVersion),
      basis::un_short(info.dwMinorVersion), basis::un_short(info.dwPlatformId),
      basis::un_short(info.dwBuildNumber)));
#else
  #pragma error("hmmm: need version info for this OS!")
#endif
  return to_return;
}

//////////////

const char *PATH_CONFIGURATION_FILENAME() { return "paths.ini"; }

astring application_configuration::application_configuration_file()
{
  filename cfg_file(application_directory() + "/" + PATH_CONFIGURATION_FILENAME());
  return cfg_file.raw();
}

const astring &application_configuration::GLOBAL_SECTION_NAME() { STATIC_STRING("Common"); }

const astring &application_configuration::LOGGING_FOLDER_NAME() { STATIC_STRING("LogPath"); }

//////////////

////const int MAX_LOG_PATH = 512;
  // the maximum length of the entry stored for the log path.

astring application_configuration::get_logging_directory()
{
  // new scheme is to just use the temporary directory, which can vary per user
  // and which hopefully is always set to something usable.
  astring def_log = environment::TMP();
  // add logs directory underneath that.
  def_log += "/logs";
    // add the subdirectory for logs.

  // now grab the current value for the name, if any.
  astring log_dir = read_item(LOGGING_FOLDER_NAME());
    // get the entry for the logging path.
  if (!log_dir) {
    // if the entry was absent, we set it.
//printf("did not find log dir in config file\n");
    ini_configurator ini(application_configuration_file(),
        ini_configurator::RETURN_ONLY,
        ini_configurator::APPLICATION_DIRECTORY);
    ini.store(GLOBAL_SECTION_NAME(), LOGGING_FOLDER_NAME(), def_log);
  } else {
    // they gave us something.  let's replace the environment variables
    // in their string so we resolve paths and such.
    log_dir = parser_bits::substitute_env_vars(log_dir);
//printf("%s", (char *)a_sprintf("got log dir with %s value\n", log_dir.s()).s());
  }

  // now we make sure the directory exists.
  filename testing(log_dir);
  if (!testing.exists()) {
    bool okay = directory::recursive_create(log_dir);
    if (!okay) {
      LOG(astring("failed to create logging directory: ") + log_dir);
      // return a directory almost guaranteed to exist; best we can do in this case.
#ifdef __UNIX__
      return "/tmp";
#endif
#ifdef __WIN32__
      return "c:/";
#endif
    }
  }
    
  return log_dir;
}

astring application_configuration::make_logfile_name(const astring &base_name)
{ return get_logging_directory() + "/" + base_name; }

astring application_configuration::read_item(const astring &key_name)
{
  filename ini_name = application_configuration_file();
  ini_configurator ini(ini_name, ini_configurator::RETURN_ONLY,
      ini_configurator::APPLICATION_DIRECTORY);
  astring to_return =  ini.load(GLOBAL_SECTION_NAME(), key_name, "");
  if (!!to_return) {
    // if the string has any length, then we process any environment
    // variables found encoded in the value.
    to_return = parser_bits::substitute_env_vars(to_return);
  }
  return to_return;
}

} // namespace.

