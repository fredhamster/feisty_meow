/*****************************************************************************\
*                                                                             *
*  Name   : unpacker stub program                                             *
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

#include <application/command_line.h>
#include <application/hoople_main.h>
#include <application/window_classist.h>
#include <basis/array.h>
#include <basis/byte_array.h>
#include <basis/environment.h>
#include <basis/guards.h>
#include <basis/utf_conversion.h>
#include <configuration/application_configuration.h>
#include <configuration/variable_tokenizer.h>
#include <filesystem/byte_filer.h>
#include <filesystem/directory.h>
#include <filesystem/filename.h>
#include <filesystem/file_time.h>
#include <filesystem/heavy_file_ops.h>
#include <loggers/console_logger.h>
#include <loggers/critical_events.h>
#include <loggers/file_logger.h>
#include <processes/launch_process.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_table.h>
#include <textual/parser_bits.h>

#include <stdio.h>
#include <sys/stat.h>
#include <zlib.h>
#ifdef __UNIX__
  #include <utime.h>
#endif
#ifdef __WIN32__
  #include <direct.h>
  #include <io.h>
  #include <shlobj.h>
  #include <sys/utime.h>
#endif

using namespace application;
using namespace basis;
using namespace configuration;
using namespace filesystem;
using namespace loggers;
using namespace processes;
using namespace structures;
using namespace textual;

const int CHUNKING_SIZE = 64 * KILOBYTE;
  // we'll read this big a chunk from a source file at a time.

const astring TARGET_WORD = "TARGET";
const astring LOGDIR_WORD = "LOGDIR";

#define BASE_LOG(to_print) program_wide_logger::get().log(to_print, ALWAYS_PRINT)
#define LOG(to_print) STAMPED_EMERGENCY_LOG(program_wide_logger::get(), to_print)

//#define DEBUG_STUB
  // uncomment for noisier version.

const char *ERROR_TITLE = "An Error Caused Incomplete Installation";
  // used in error messages as the title.

////////////////////////////////////////////////////////////////////////////

class unpacker_stub : public application_shell
{
public:
  unpacker_stub() : application_shell(), _app_name(filename(_global_argv[0]).basename()) {}
  DEFINE_CLASS_NAME("unpacker_stub");

  int print_instructions();

  virtual int execute();

private:
  astring _app_name;
  array<manifest_chunk> _manifest;  //!< the list of chunks to unpack.
  string_table _variables;  //!< our list of variable overrides.
};

////////////////////////////////////////////////////////////////////////////

void show_message(const astring &msg, const astring &title)
{
#ifndef __WIN32__
  BASE_LOG(title);
  BASE_LOG(astring('-', title.length()));
  BASE_LOG(msg);
#else
  MessageBox(0, to_unicode_temp(msg), to_unicode_temp(title),
      MB_OK | MB_ICONINFORMATION);
#endif
}

int unpacker_stub::print_instructions()
{
  a_sprintf msg("\
    %s: This program unpacks its contents into the locations\n\
 specified at packing time.  The --target flag can be used to specify a\n\
different %s directory for the installation (for components that use\n\
the default %s variable to specify their install folder).\n\
    One can also pass a --keyword flag to specify a keyword; the files in the\n\
bundle that are marked with that keyword will be installed, but files that\n\
are missing the keyword will not be.\n\
    Further, variables can be overridden on the command line in the\n\
form: X=Y.\n\n\
The line below uses all these parameters as an example:\n\n\
  %s --target c:\\Program Files\\gubernator --keyword dlls_only SILENT=true\n\
\n\
Additional Notes:\n\
\n\
    One helpful variable is \"%s\".  This is where the unpacking log file\n\
will be written to.  The default is \"~/logs\" (or \"$TMP/logs\" on win32)\n\
until overridden.\n\
\n", _app_name.s(), TARGET_WORD.s(), TARGET_WORD.s(), _app_name.s(), LOGDIR_WORD.s());
  show_message(msg, "Unpacking Instructions");
  return 12;
}

// creates a unique backup file name, if it can.
// we assume that this file already exists, but we want to check for
// our backup file naming scheme in case we already backed this up
// some previous time.
astring find_unique_backup_name(astring original_file)
{
const int MAXIMUM_BACKUPS = 200;

  for (int i = 0; i < MAXIMUM_BACKUPS; i++) {
    filename target_file = original_file + a_sprintf(".%04d", i);
    if (target_file.exists()) {
//BASE_LOG(astring("bkup already here: ") + target_file);
      continue;
    }
    // this file is okay to use.
    return target_file;
  }
  return "";  // nothing found.
}

 
// the string embedded into the array is not mentioned anywhere else in this
// program, which should allow the packer to find it and fix the manifest
// size.  the keyword is: "muftiloc", and the first bytes that can be
// overwritten are its beginning offset plus its length of 8 chars.  there
// is room for 8 bytes after the tag, but currently the first 4 are used as
// a 32 bit offset.
abyte MANIFEST_OFFSET_ARRAY[]
    = { 'm', 'u', 'f', 't', 'i', 'l', 'o', 'c', 0, 0, 0, 0, 0, 0, 0, 0 };

int unpacker_stub::execute()
{
#ifdef ADMIN_CHECK
  #ifdef __WIN32__
    if (IsUserAnAdmin()) {
      ::MessageBox(0, to_unicode_temp("IS admin in bundler"), to_unicode_temp("bundler"), MB_OK);
    } else {
      ::MessageBox(0, to_unicode_temp("NOT admin in bundler"), to_unicode_temp("bundler"), MB_OK);
    }
  #endif
#endif

  command_line cmds(_global_argc, _global_argv);

  int indy = 0;
  if (cmds.find('?', indy)) return print_instructions();
  if (cmds.find("?", indy)) return print_instructions();

  // make sure we provide the same services as the bundle creator for the
  // default set of variables.
#ifndef __WIN32__
  environment::set("EXE_END", "");  // executable file ending.
  environment::set("DLL_START", "lib");  // dll file prefix.
  environment::set("DLL_END", ".so");  // dll file ending.
#else
  environment::set("EXE_END", ".exe");
  environment::set("DLL_START", "");
  environment::set("DLL_END", ".dll");
#endif

  // set TARGET directory if passed on the command line,
  bool provided_target = false;  // true if the command line specified target.
  astring target;
  cmds.get_value("target", target);
  if (!target) {
    provided_target = false;
  } else {
//LOG(astring("target is now ") + target);
    environment::set(TARGET_WORD, target);
    provided_target = true;
  }

  {
    astring logdir = environment::get(LOGDIR_WORD);
#ifdef __WIN32__
    if (!logdir) {
      logdir = environment::TMP() + "/logs";
      environment::set(LOGDIR_WORD, logdir);
    }
#else
    if (!logdir) {
      astring homedir = environment::get("HOME");
      logdir = homedir + "/logs";
      environment::set(LOGDIR_WORD, logdir);
    }
#endif
  }

  astring keyword;  // set if we were given a keyword on cmd line.
  cmds.get_value("keyword", keyword);

  astring vars_set;  // we will document the variables we saw and show later.

  for (int x = 0; x < cmds.entries(); x++) {
    command_parameter curr = cmds.get(x);
    if (curr.type() != command_parameter::VALUE) continue;  // skip it.
    if (curr.text().find('=', 0) < 0) continue;  // no equals character.
    variable_tokenizer t;
    t.parse(curr.text());
    if (!t.symbols()) continue;  // didn't parse right.
    astring var = t.table().name(0);
    astring value = t.table()[0];
    vars_set += astring("variable set: ") + var + "=" + value
        + parser_bits::platform_eol_to_chars();
    if (var == TARGET_WORD) {
      provided_target = true;
    }
//hmmm: handle LOGDIR passed as variable this way also!
    _variables.add(var, value);
    environment::set(var, value);
  }

  // get the most up to date version of the variable now.
  astring logdir = environment::get(LOGDIR_WORD);

  astring appname = filename(application_configuration::application_name()).rootname();

  astring logname = logdir + "/" + appname + ".log";
//  log_base *old_log = set_PW_logger_for_combo(logname);
  standard_log_base *old_log = program_wide_logger::set(new combo_logger(logname));
  WHACK(old_log);

  BASE_LOG(astring('#', 76));
  BASE_LOG(appname + " command-line parameters:");
  BASE_LOG(cmds.text_form());
  BASE_LOG(astring('#', 76));

  BASE_LOG(vars_set);

#ifdef __WIN32__
  // create a window so that installshield won't barf.  this is only needed
  // on windows when using this as a prerequisite for installshield.
  window_handle f_window = create_simplistic_window("temp_stubby_class",
      "stubby window title");
#endif

  // read position for manifest offset out of our array.
  byte_array temp_packed(2 * sizeof(int), MANIFEST_OFFSET_ARRAY + 8);
  un_int manifest_offset;
  if (!structures::obscure_detach(temp_packed, manifest_offset)) {
    show_message(astring("could not read manifest offset in: ") + _global_argv[0],
        ERROR_TITLE);
    return 24;
  }
  
  filename this_exe(_global_argv[0]);
  if (!this_exe.exists()) {
    show_message(astring("could not access this exe image: ") + this_exe.raw(),
        ERROR_TITLE);
    return 23;
  }

  // start reading the manifest...
  byte_filer our_exe(this_exe, "rb");
  our_exe.seek(manifest_offset);  // go to where the manifest starts.

  // get number of chunks in manifest as the first bytes.
  if (our_exe.read(temp_packed, 2 * sizeof(int)) <= 0) {
    show_message(astring("could not read the manifest length in: ")
        + this_exe.raw(), ERROR_TITLE);
    return 26;
  }
  un_int item_count;
  structures::obscure_detach(temp_packed, item_count);
//check result of detach!
  _manifest.insert(0, item_count);  // add enough spaces for our item list.

  // read each item definition out of the manifest now.
  for (int i = 0; i < (int)item_count; i++) {
    manifest_chunk &curr = _manifest[i];
    bool worked = manifest_chunk::read_manifest(our_exe, curr);

#ifdef DEBUG_STUB
    astring tmpork;
    curr.text_form(tmpork);
    LOG(a_sprintf("item %d: ", i) + tmpork);
#endif

    if (!worked) {
      show_message(a_sprintf("could not read chunk for item #%d [%s]: ", i,
          curr._payload.s())
          + this_exe.raw(), ERROR_TITLE);
      return 86;
    }
  }

#ifdef DEBUG_STUB
  LOG("read the following info from manifest:");
  astring temp;
  for (int i = 0; i < _manifest.length(); i++) {
    manifest_chunk &curr = _manifest[i];
    temp += a_sprintf("(%d) size %d, %s\n", i, curr._size,
          curr._payload.s());
  }
  critical_events::alert_message(temp, "manifest contents");
#endif

  // now we should be just after the last byte of the manifest, at the
  // first piece of data.  we should read each chunk of data out and store
  // it where it's supposed to go.
  for (int festdex = 0; festdex < _manifest.length(); festdex++) {
    manifest_chunk &curr = _manifest[festdex];
    int size_left = curr._size;

    // patch in our environment variables.
    curr._payload = parser_bits::substitute_env_vars(curr._payload, false);
    curr._parms = parser_bits::substitute_env_vars(curr._parms, false);
#ifdef DEBUG_STUB
    BASE_LOG(astring("processing ") + curr._payload
        + a_sprintf(", size=%d, flags=%d", curr._size, curr._flags));
    if (!!curr._parms)
      BASE_LOG(astring("   parms: ") + curr._parms);
#endif

    // see if they specified a keyword on the command line.
    bool keyword_good = true;
    if (!!keyword && !curr._keywords.member(keyword)) {
      // their keyword choice didn't match what we were pickled with.
      keyword_good = false;
//BASE_LOG(astring("skipping ") + curr._payload + " for wrong keyword " + keyword);
    }

    if (curr._flags & SET_VARIABLE) {
      if (keyword_good) {
        // this is utterly different from a real target.  we just set the
        // variable and move on.
        if (provided_target && (curr._payload == TARGET_WORD) ) {
          BASE_LOG(astring("skipping ") + curr._payload + "=" + curr._parms
              + ": was provided explicitly as " + target);
        } else if (_variables.find(curr._payload)) {
          BASE_LOG(astring("skipping ") + curr._payload + "=" + curr._parms
              + ": was provided on command line.");
        } else {
          BASE_LOG(astring("setting ") + curr._payload + "=" + curr._parms);
          environment::set(curr._payload, curr._parms);

          // special code for changing logging directory midstream.
          if (curr._payload == LOGDIR_WORD) {
            astring logdir = curr._parms;
            astring appname = filename(application_configuration::application_name()).rootname();
            astring logname = logdir + "/" + appname + ".log";
            standard_log_base *old_log = program_wide_logger::set(new combo_logger(logname));
///            log_base *old_log = set_PW_logger_for_combo(logname);
            WHACK(old_log);
          }
          if (curr._payload == TARGET_WORD)  {
            // well we've now seen this defined.
            provided_target = true;
          }
        }
      }
      continue;
    } else if (curr._flags & TEST_VARIABLE_DEFINED) {
      if (keyword_good) {
        astring var_value = environment::get(curr._payload);
        if (var_value.empty()) {
          BASE_LOG(astring("assertion failed: ") + curr._payload + " is not defined!");
          show_message(a_sprintf("failed test for variable %s: it is "
              "*not* defined, for item #%d.", curr._payload.s(), festdex),
              ERROR_TITLE);
          return 98;
        }
        BASE_LOG(astring("assertion succeeded: ") + curr._payload + " defined as " + var_value);
      }
      continue;
    }

    if (! (curr._flags & OMIT_PACKING) ) {
      // this one has a payload, so install it now if appropriate.
      if (!provided_target) {
//error!
        BASE_LOG(astring("No TARGET has been specified; please provide one on the command line.") + parser_bits::platform_eol_to_chars());
	return print_instructions();
      }

      // make sure that the directories needed are present for the outputs.
      filename target_dir = filename(curr._payload).dirname();

      if (keyword_good && !target_dir.exists()
          && !directory::recursive_create(target_dir)) {
        LOG(a_sprintf("failed to create directory %s for item #%d: ",
            target_dir.raw().s(), festdex) + curr._payload);
      }

      // test whether they wanted to allow overwriting.
      if (curr._flags & NO_OVERWRITE) {
        filename target_file(curr._payload);
        if (target_file.exists()) {
          BASE_LOG(astring("not overwriting existing ") + curr._payload);
          keyword_good = false;
        }
      }

      // see if this is supposed to be backed up before installation.
      if (curr._flags & MAKE_BACKUP_FILE) {
        filename target_file(curr._payload);
        if (target_file.exists()) {
          astring new_file_name = find_unique_backup_name(curr._payload);
          if (!new_file_name) {
            BASE_LOG(astring("failed to calculate new filename for ") + curr._payload);
            keyword_good = false;  // cancel the overwrite, couldn't backup.
          } else {
            // make a backup of the file by moving the old file name to the
            // new file name, which should be unique, and then the current
            // name is all clear to be written as a new file.
//            BASE_LOG(astring("backing up ") + curr._payload + " --> " + new_file_name);
            int retval = rename(curr._payload.s(), new_file_name.s());
            if (retval) {
              BASE_LOG(astring("failed to rename ") + curr._payload + " as " + new_file_name);
              keyword_good = false;  // cancel the overwrite, couldn't backup.
            }
          }
        }
      }

      byte_filer *targo = NIL;
      if (keyword_good) targo = new byte_filer(curr._payload, "wb");
      byte_array uncompressed(256 * KILOBYTE);  // fluff it out to begin with.
      byte_array temp(256 * KILOBYTE);

      bool first_read = true;
        // true if there haven't been any reads of the file before now.
      bool too_tiny_complaint_already = false;
        // becomes true if we complain about the file's size being larger than
        // expected.  this allows us to only complain once about each file.

      // read a chunk at a time out of our exe image and store it into the
      // target file.
      while (first_read || !our_exe.eof()) {
        first_read = false;
        un_int real_size = 0, packed_size = 0;
        // read in the real size from the file.
        bool worked = manifest_chunk::read_an_obscured_int(our_exe, real_size);
        if (!worked) {
          show_message(a_sprintf("failed while reading real size "
              "for item #%d: ", festdex) + curr._payload, ERROR_TITLE);
          return 99;
        }
        // read in the packed size now.
        worked = manifest_chunk::read_an_obscured_int(our_exe, packed_size);
        if (!worked) {
          show_message(a_sprintf("failed while reading packed size "
              "for item #%d: ", festdex) + curr._payload, ERROR_TITLE);
          return 99;
        }

        // make sure we don't eat the whole package--did the file end?
        if ( (real_size == -1) && (packed_size == -1) ) {
          // we've hit our sentinel; we've already unpacked all of this file.
          break;
        }

#ifdef DEBUG_STUB
        BASE_LOG(a_sprintf("chunk packed_size=%d, real_size=%d", packed_size,
            real_size));
#endif

        // now we know how big our next chunk is, so we can try reading it.
        if (packed_size) {
          int ret = our_exe.read(temp, packed_size);
          if (ret <= 0) {
            show_message(a_sprintf("failed while reading item #%d: ", festdex)
                + curr._payload, ERROR_TITLE);
            return 99;
          } else if (ret != packed_size) {
            show_message(a_sprintf("bad trouble ahead, item #%d had different "
                " size on read (expected %d, got %d): ", festdex, packed_size,
                ret) + curr._payload, ERROR_TITLE);
          }

          uncompressed.reset(real_size + KILOBYTE);  // add some for paranoia.
          uLongf destlen = uncompressed.length();
          int uncomp_ret = uncompress(uncompressed.access(), &destlen,
              temp.observe(), packed_size);
          if (uncomp_ret != Z_OK) {
            show_message(a_sprintf("failed while uncompressing item #%d: ",
                festdex) + curr._payload, ERROR_TITLE);
            return 99;
          }
  
          if (int(destlen) != real_size) {
            LOG(a_sprintf("got a different unpacked size for item #%d: ",
                festdex) + curr._payload);
          }

          // update the remaining size for this data chunk.
          size_left -= real_size;
          if (size_left < 0) {
            if (!too_tiny_complaint_already) {
              LOG(a_sprintf("item #%d was larger than expected (non-fatal): ",
                  festdex) + curr._payload);
              too_tiny_complaint_already = true;
            }
          }
          // toss the extra bytes out.
          uncompressed.zap(real_size, uncompressed.length() - 1);

          if (targo) {
            // stuff the data we read into the target file.
            ret = targo->write(uncompressed);
            if (ret != uncompressed.length()) {
              show_message(a_sprintf("failed while writing item #%d: ", festdex)
                  + curr._payload, ERROR_TITLE);
              return 93;
            }
          }
        }
      }
      if (targo) targo->close();
      WHACK(targo);
      // the file's written, but now we slap it's old time on it too.
      file_time t;
      if (!t.unpack(curr.c_filetime)) {
        show_message(astring("failed to interpret timestamp for ")
            + curr._payload, ERROR_TITLE);
        return 97;
      }
      // put the timestamp on the file.
      t.set_time(curr._payload);
//      utimbuf held_time;
//      held_time.actime = t.raw();
//      held_time.modtime = t.raw();
//      // put the timestamp on the file.
//      utime(curr._payload.s(), &held_time);
    }

    // now that we're pretty sure the file exists, we can run it if needed.
    if ( (curr._flags & TARGET_EXECUTE) && keyword_good) {    
      // change the mode on the target file so we can execute it.
      chmod(curr._payload.s(), 0766);
      astring prev_dir = application_configuration::current_directory();

      BASE_LOG(astring("launching ") + curr._payload);
      if (!!curr._parms)
        BASE_LOG(astring("  with parameters: ") + curr._parms);
      BASE_LOG(astring('-', 76));

      basis::un_int kid;
      basis::un_int retval = launch_process::run(curr._payload, curr._parms,
          launch_process::AWAIT_APP_EXIT | launch_process::HIDE_APP_WINDOW, kid);
      if (retval != 0) {
        if (! (curr._flags & IGNORE_ERRORS) ) {
          if (curr._flags & QUIET_FAILURE) {
            // no message box for this, but still log it.
            LOG(astring("failed to launch process, targ=")
                + curr._payload + " with parms " + curr._parms
                + a_sprintf(" error=%d", retval));
          } else {
            show_message(astring("failed to launch process, targ=")
                + curr._payload + " with parms " + curr._parms
                + a_sprintf(" error=%d", retval), ERROR_TITLE);
          }
          return retval;  // pass along same exit value we were told.
        } else {
          LOG(astring("ignoring failure to launch process, targ=")
              + curr._payload + " with parms " + curr._parms
              + a_sprintf(" error=%d", retval));
        }
      }

      chdir(prev_dir.s());  // reset directory pointer, just in case.

      BASE_LOG(astring('-', 76));
    }

  }

#ifdef __WIN32__
  whack_simplistic_window(f_window);
#endif

  return 0;
}

////////////////////////////////////////////////////////////////////////////

HOOPLE_MAIN(unpacker_stub, )

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
  #include <filesystem/file_info.cpp>
  #include <filesystem/filename.cpp>
  #include <filesystem/filename_list.cpp>
  #include <filesystem/file_time.cpp>
  #include <filesystem/heavy_file_ops.cpp>
  #include <filesystem/huge_file.cpp>
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
#endif // __BUILD_STATIC_APPLICATION__

