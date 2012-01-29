#ifndef VERSION_INI_CLASS
#define VERSION_INI_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : version_ini editing support                                       *
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

#include <basis/contracts.h>
#include <configuration/ini_configurator.h>
#include <filesystem/filename.h>
#include <structures/version_record.h>

namespace versions {

//! This provides support for writing windows version files.
/*!
  The class loads version information out of an initialization file and writes
  it into a standard windows VERSION_INFO RC file entry.  It also creates the
  headers we use with our version control support.
*/

class version_ini : public virtual basis::root_object
{
public:
  version_ini(const basis::astring &path_name);
    //!< the "path_name" is where the version INI file is located.
    /*!< this file will be read for the key fields that will be used to name
    the version RC file and generate information in the resource. */

  ~version_ini();

  DEFINE_CLASS_NAME("version_ini");

  bool writable();
    //!< returns true if the INI file specified in the constructor is writable.

  structures::version get_version();
    //!< observes or modifies the version number structure held here.
    /*!< if the record had not been previously loaded, it is loaded. */

  void set_version(const structures::version &to_write, bool write_to_ini);
    //!< sets the version held here.
    /*!< if "write_ini" is true, then the ini file is written to also.  note
    that if get_record() had not previously been done and "write_ini" is not
    true, then the next get_record() or get_version() will wipe out the
    version that is set in the interim. */

  structures::version_record get_record();
    //!< observes the entire version record.
    /*!< The information is assembled from any cached record, the ini file and
    other sources.  if the record has been loaded before, the last loaded
    version is returned plus any changes that have been made to the held
    record since then.  otherwise, the record is read from the ini file. */

  structures::version_record &access_record();
    //!< provides access to change the version_record held here.
    /*!< this does not read from the INI file, unlike get_record(). */

  void set_record(const structures::version_record &to_write, bool write_to_ini);
    //!< modifies the entire version record.
    /*!< if "write_to_ini" is true, then the new information is actually
    written to our configuration file.  otherwise the information just
    replaces the source record here. */

  bool executable();
    //!< returns true if this version file is for an executable.
  bool library();
    //!< returns true if this version file is for a dynamic library.

  bool ole_auto_registering();
    //!< returns true if this version file specifies ole auto registration.

  bool write_rc(const structures::version_record &to_write);
    //!< writes out the file 'X_version.rc' for the X library or application.
    /*!< the contents will include the information specified in "to_write",
    where X is the library name from that record. */

  bool write_code(const structures::version_record &to_write);
    //!< writes out the header ('X_version.h') with the version information.
    /*!< this file is needed for other libraries or application to know this
    project's version number.  the users can make sure that the header info
    agrees with the actual version seen on the file. */

  bool write_assembly(const structures::version_record &to_write, bool do_logging);
    //!< fixes any assemblies with the info in "to_write".

  static bool executable(const basis::astring &path_name);
    //!< returns true if "path_name" is for an executable.
  static bool library(const basis::astring &path_name);
    //!< returns true if "path_name" is for a dynamic library.

  structures::version read_version_from_ini();
    //!< specialized version ignores cache and gets version directly from file.

  static bool one_stop_version_stamp(const basis::astring &path,
          const basis::astring &source_version, bool do_logging);
    //!< performs version stamping using the ini file in "path".
    /*!< "source_version" supplies the name of the main version file where
    we retrieve the current version numbers.  if that is not specified, then
    only the version header and RC file are created.  if "do_logging" is
    true, then version info will be sent to the program-wide logger. */

  // constants for strings found in the version INI file.
  static const char *VERSION_SECTION;
  static const char *COMPANY_KEY;
  static const char *COPYRIGHT_KEY;
  static const char *LEGAL_INFO_KEY;
  static const char *PRODUCT_KEY;
  static const char *WEB_SITE_KEY;
  static const char *MAJOR;
  static const char *MINOR;
  static const char *REVISION;
  static const char *BUILD;
  static const char *DESCRIPTION;
  static const char *ROOT;
  static const char *NAME;
  static const char *EXTENSION;
  static const char *OLE_AUTO;

private:
  bool _loaded;  //!< did we grab the data from the ini file yet?
  filesystem::filename *_path_name;  //!< where to find the ini file.
  configuration::ini_configurator *_ini;  //!< accesses the ini file.
  structures::version_record *_held_record;  //!< the data we cache for the ini file.

  static void check_name(filesystem::filename &to_examine);
    //!< adds the default file name if "to_examine" turns out to be a directory.
};

} //namespace.

#endif

