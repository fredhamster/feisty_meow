#ifndef APPLICATION_CONFIGURATION_GROUP
#define APPLICATION_CONFIGURATION_GROUP

/**
 * Name: path configuration definitions
 * Author: Chris Koeritz
 ****
 * Copyright (c) 2000-$now By Author.  This program is free software; you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either version 2 of
 * the License or (at your option) any later version.  This is online at:
 *     http://www.fsf.org/copyleft/gpl.html
 * Please send any updates to: fred@gruntose.com
 */

#include <basis/astring.h>
#include <basis/definitions.h>
#include <structures/version_record.h>

namespace configuration {

//! Defines installation-specific locations in the file system.

class application_configuration : public virtual basis::root_object
{
public:
  virtual ~application_configuration() {}

  // these methods are mainly about the application itself.

  static basis::astring application_name();
    //!< returns the full name of the current application.

  static basis::astring application_directory();
    //!< returns the directory name where this application is running from.

  static basis::un_int process_id();
    //!< returns the process id for this task, if that's relevant on the OS.

  // these interface with the operating system.

  static structures::version get_OS_version();
    //!< returns the operating system's version information.
    /*!< for linux, this has: major/minor/release/revision as components.
    for ms-windows, this has: major/minor/platform_ID/build_number. */

  static basis::astring current_directory();
    //!< returns the current directory as reported by the operating system.

  // the following are more about the installation than this application...

  static const char *software_product_name();
    //!< This global function is available to the system at large for branding info.

//  static basis::astring installation_root();
    //!< returns the top-level directory of this installation.
    /*!< returns the folder containing the application directory (usually) as
    well as the other folders of configuration information and fonts and
    such needed by this installation. */

  static const char *APPLICATION_CONFIGURATION_FILENAME();
    //!< the configuration file provides a set of paths needed here.
    /*!< this file stores paths that the low-level libraries need for
    proper operation.  this is just the base filename; the fully qualified
    path to the path configuration file is provided below. */

  static basis::astring application_configuration_file();
    //!< the fully specified path to the main path configuration file.
    /*!< the location of this file is determined by the directory where this
    executable is running.  this is the only configuration file that should
    reside there, given the itchy vista compliance needs. */

////  static basis::astring core_bin_directory();
    //!< retrieves the core binary directory location from paths.ini.

  static basis::astring get_logging_directory();
    //!< returns the directory where log files will be stored.

  // the following are key names within the main configuration file.

  static const basis::astring &GLOBAL_SECTION_NAME();
    //!< the root section name for our configuration items in the main ini file.
    /*!< within the configuration file, there is a section named as above.
    this section should only be used to define path configuration entries that
    affect the general operation of the system.  entries that are specific
    to particular programs or subsystems should be contained in their own
    section. */

///  static const basis::astring &LOCAL_FOLDER_NAME();
    //!< entry name in the config file that points at the installation root.
    /*!< this is where all files for this product are stored on "this" machine. */

  static const basis::astring &LOGGING_FOLDER_NAME();
    //!< the location where the log files for the system are stored.
    /*!< this is always considered to be a directory under the local folder.
    the make_logfile_name() function (see below) can be used to create a
    properly formed filename for logging. */

  // helper methods.

  static basis::astring read_item(const basis::astring &key_name);
    //!< returns the entry listed under the "key_name".
    /*!< if the "key_name" is valid for the root configuration file, then this
    should always return an appropriate value.  a failure is denoted by return
    of an empty string. */

  static basis::astring make_logfile_name(const basis::astring &base_name);
    //!< generates an installation appropriate log file name from "base_name".
    /*!< creates and returns a full path name for a log file with the
    "base_name" specified, using the LOGGING_FOLDER_NAME() entry as the
    directory name.  if the logging directory does not exist, it is created if
    that is possible.  the returned name is suitable for logging mechanisms that
    need a filename.  an empty string is returned on failure to create the
    directory. */

#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
  #ifdef __APPLE__
    static basis::astring get_cmdline_for_apple();
  #endif
  static basis::astring get_cmdline_from_proc();
    //!< retrieves the command line from the /proc hierarchy on linux.
  static basis::astring query_for_process_info();
    //!< seeks out process info for a particular process.
#endif
};

} // namespace.

#endif

