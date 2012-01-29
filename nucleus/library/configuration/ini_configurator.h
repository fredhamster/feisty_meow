#ifndef INI_CONFIGURATOR_CLASS
#define INI_CONFIGURATOR_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : ini_configurator                                                  *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "configurator.h"
#ifndef __WIN32__
  #include "ini_parser.h"
  #include <basis/utf_conversion.h>
#endif

#include <basis/contracts.h>
#include <filesystem/byte_filer.h>
#include <filesystem/filename.h>

namespace configuration {

//! Supports a configurator-based interface on text initialization files.

class ini_configurator : public configurator
{
public:
  //! chooses where the ini file is located if no path to it is provided.
  /*! the ini file being manipulated will be stored in either the same
  directory as the program being executed (APPLICATION_DIRECTORY) or in the
  directory where the operating system resides (OS_DIRECTORY).  however, the
  OS_DIRECTORY choice only really makes sense on windows.  if the flag is
  instead ALL_USERS_DIRECTORY, then the directory pointed at by the
  $ALLUSERSPROFILE variable will be used on windows; otherwise, the default
  is the same as for APPLICATION_DIRECTORY. */
  enum file_location_default {
    APPLICATION_DIRECTORY,  //!< config files live with application.
    OS_DIRECTORY,  //!< config files live in operating system directory.
    ALL_USERS_DIRECTORY  //!< config files live in the "all users" account.
  };

  ini_configurator(const basis::astring &ini_filename,
          treatment_of_defaults behavior = RETURN_ONLY,
          file_location_default where = ALL_USERS_DIRECTORY);
    //!< creates an ini_configurator that stores entries into "ini_filename".
    /*!< the ini config will have the "behavior" specified for how to handle
    missing items.  "where" dictates the file's location if no path is
    specified as part of the "ini_filename". */

  virtual ~ini_configurator();

  DEFINE_CLASS_NAME("ini_configurator");

  void refresh();
    //!< useful mainly on unix/linux, where the file is parsed and held in memory.

//hmmm: where are:
//  save_to_file()
//  is_modified()
//?

  basis::astring name() const;
    //!< observes the name of the file used for ini entries.
  void name(const basis::astring &name);
    //!< modifies the name of the file used for ini entries.

  virtual bool get(const basis::astring &section, const basis::astring &entry,
          basis::astring &found);
    //!< implements the configurator retrieval function.
    /*!< this returns true if the entry was present and stores it in
    "found". */

  virtual void sections(structures::string_array &list);
    //!< retrieves the section names into "list".

  virtual bool section_exists(const basis::astring &section);
    //!< returns true if the "section" was found in the file.
    /*!< NOTE: for an INI file, this is quite a heavy-weight call. */

  virtual bool put(const basis::astring &section, const basis::astring &entry,
          const basis::astring &to_store);
    //!< implements the configurator storage function.

  virtual bool delete_section(const basis::astring &section);
    //!< removes the entire "section" specified.

  virtual bool delete_entry(const basis::astring &section, const basis::astring &entry);
    //!< removes the entry specified by the "section" and "entry" name.

  virtual bool get_section(const basis::astring &section, structures::string_table &info);
    //!< reads the entire "section" into a table called "info".
    /*!< on win95, this will fail if the section's data exceeds 32K. */

  virtual bool put_section(const basis::astring &section, const structures::string_table &info);
    //!< writes a table called "info" into the "section" in the INI file.
    /*!< any existing data for that section is wiped out.  on win95, this will
    fail if the section's data exceeds 32K. */

  // dictates whether the stored entries will have spaces between '='
  // and the key name and value.  only applicable on linux.
  bool add_spaces() const { return _add_spaces; }
  void add_spaces(bool add_them) { _add_spaces = add_them; }

private:
  filesystem::filename *_ini_name;  //!< the file we're manipulating.
#ifdef __UNIX__
  ini_parser *_parser;  //!< used for real storage and parsing.
#endif
  file_location_default _where;  //!< where to find and store the file.
  bool _add_spaces;  //!< tracks whether we're adding spaces around equals.

#ifdef __WIN32__
  bool put_profile_string(const basis::astring &section, const basis::astring &entry,
          const basis::astring &to_store);
    //!< encapsulates windows' ini storage method.
  void get_profile_string(const basis::astring &section, const basis::astring &entry,
          const basis::astring &default_value, basis::flexichar *return_buffer,
          int buffer_size);
    //!< encapsulates windows' ini retrieval method.
#endif
#ifdef __UNIX__
  void read_ini_file();
    //!< reads the INI file's contents into memory.
  void write_ini_file();
    //!< store the current contents into the INI file.
#endif

  // not to be called.
  ini_configurator(const ini_configurator &);
  ini_configurator &operator =(const ini_configurator &);

  static const basis::astring &ini_str_fake_default();
};

} //namespace.

#endif

