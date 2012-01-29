#ifndef CONFIGURED_APPLICATIONS_CLASS
#define CONFIGURED_APPLICATIONS_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : configured_applications
*  Author : Chris Koeritz
*                                                                             *
*******************************************************************************
* Copyright (c) 2000 By Author.  This program is free software; you can       *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <basis/contracts.h>
#include <basis/mutex.h>
#include <configuration/ini_configurator.h>
#include <configuration/section_manager.h>
#include <structures/string_table.h>

namespace processes {

//! Manages the initialization file for a set of registered applications.
/*!
  This records the list of programs that are allowed to be executed as well
  as the list of applications launched at object startup time.
*/

class configured_applications
{
public:
  configured_applications(const basis::astring &config_file, const basis::astring &basename);
    //!< manages application settings for in the "config_file".
    /*!< the "basename" is used for the section name of a list of products
    that can be managed by this class.  each product has a set of applications
    that are part of the product's full package. */

  virtual ~configured_applications();

  // this section has mainly informational functions.

  DEFINE_CLASS_NAME("configured_applications");

  static const char *STARTUP_SECTION();
    //!< the section where startup info is stored.

  static const char *STARTUP_APP_NAME();
    //!< a special placeholder name that will appear in the startup list.
    /*!< it is not to be executed like the other programs for startup. */

  static bool find_entry(const structures::string_table &table, const basis::astring &name,
          basis::astring &location);
    //!< returns true if the key "name" for a program is found in the "table".
    /*!< the "location" is set to the value found in the table if successful.
    */

  static basis::astring make_startup_entry(const basis::astring &product,
          const basis::astring &parms, bool one_shot);
    //!< returns the appropriate string for a startup record.

  static bool parse_startup_entry(const basis::astring &info, basis::astring &product,
          basis::astring &parms, bool &one_shot);
    //!< processes the items in "info" as an application startup list.
    /*!< using a string "info" that was listed as the startup entry for an
    application, the "product", "parms" and "one_shot" bits are parsed out
    and returned. */

  bool product_exists(const basis::astring &product);
    //!< returns true if the section for "product" exists in the TOC.

  basis::astring find_program(const basis::astring &product, const basis::astring &app_name,
          int &level);
    //!< seeks out the entry for the "product" and "app_name" in our info.
    /*!< the returned string will either be empty (on failure) or will contain
    the full path to the application (on success).  the "level" will specify
    the ordering of shutdown, where larger levels are shut down first. */

  // the following functions actually modify the configuration file.

  bool find_section(const basis::astring &section_name, structures::string_table &info_found);
    //!< locates the entries for "section_name" and stores them in "info_found".

  bool add_section(const basis::astring &section_name, const structures::string_table &info);
    //!< puts a chunk of "info" into the section for "section_name".
    /*!< this fails if the section already exists. */

  bool replace_section(const basis::astring &section_name, const structures::string_table &info);
    //!< replaces the section for "section_name" with "info".
    /*!< this fails if the section does not already exist. */

  bool add_program(const basis::astring &product, const basis::astring &app_name,
          const basis::astring &full_path, int level);
    //!< registers a program "app_name" into the "product" section.
    /*!< the "full_path" specifies where to find the program and the "level"
    gives the application an ordering for shutdown.  higher levels are shut
    down before lower ones. */

  bool remove_program(const basis::astring &product, const basis::astring &app_name);
    //!< takes a previously registered "app_name" out of the list for "product".

  bool add_startup_entry(const basis::astring &product, const basis::astring &app_name,
          const basis::astring &parameters, int one_shot);
    //!< establishes the "app_name" as a program launched at object startup.
    /*!< adds an entry to the startup section for a program that will be
    launched when the application manager restarts. */

  bool remove_startup_entry(const basis::astring &product, const basis::astring &app_name);
    //!< takes an existing entry for the "app_name" out of the startup section.

private:
  basis::mutex *_lock;  //!< synchronization protection for our objects.
  configuration::ini_configurator *_config;  //!< manages our configuration settings.
  configuration::section_manager *_sector;  //!< keeps track of our product sections.
};

} //namespace.

#endif

