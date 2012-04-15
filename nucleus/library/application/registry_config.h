#ifndef REGISTRY_CONFIGURATOR_CLASS
#define REGISTRY_CONFIGURATOR_CLASS

/*
*  Name   : registry_configurator                                             *
*  Author : Chris Koeritz                                                     *
**
* Copyright (c) 2004-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <basis/contracts.h>
#include <configuration/configurator.h>
#include <filesystem/byte_filer.h>
#include <filesystem/filename.h>

namespace configuration {

//! Supports the configurator class interface on the windows registry.

class registry_configurator : public configurator
{
public:
  //! the hives are major partitions of the registry.
  enum registry_hives {
    hkey_classes_root,
    hkey_current_user,
    hkey_local_machine,
    hkey_users,
    hkey_current_config,
    // abbreviations for the above sections...
    HKCR = hkey_classes_root,
    HKCU = hkey_current_user,
    HKLM = hkey_local_machine,
    HKU = hkey_users,
    HKCC = hkey_current_config
  };

  registry_configurator(registry_hives hive, treatment_of_defaults behavior);
    //!< creates a registry_configurator that stores entries into the "hive".
    /*!< applies the "behavior" to items that are not found. */

  virtual ~registry_configurator();

  DEFINE_CLASS_NAME("registry_configurator");

  virtual bool get(const basis::astring &section, const basis::astring &entry,
          basis::astring &found);
    //!< implements the configurator retrieval function.
    /*!< note that for registry based retrieval, an empty "entry" is allowed,
    and that specifies the default item in the "section". */

  virtual bool section_exists(const basis::astring &section);
    //!< returns true if the "section" was found in the file.

  virtual bool put(const basis::astring &section, const basis::astring &entry,
          const basis::astring &to_store);
    //!< implements the configurator storage function.
    /*!< put interprets an empty string for "entry" as pointing at the
    default item in the "section". */

  virtual bool delete_section(const basis::astring &section);
    //!< removes the entire "section" specified.

  virtual bool delete_entry(const basis::astring &section, const basis::astring &entry);
    //!< removes the entry specified by the "section" and "entry" name.

  virtual bool get_section(const basis::astring &section, structures::string_table &info);
    //!< reads the entire "section" into a table called "info".
    /*!< on win-9x, this will fail if the section's data exceeds 32K. */

  virtual bool put_section(const basis::astring &section, const structures::string_table &info);
    //!< writes a table called "info" into the "section" in the INI file.
    /*!< any existing data for that section is wiped out.  on win-9x, this will
    fail if the section's data exceeds 32K. */

  void *translate_hive(registry_hives hive);
    //!< translates from our enum to the windows specific type for hives.

  basis::astring fix_section(const basis::astring &section);
    //!< repairs malformed section names.
    /*!< translates a section name that might use forward slashes into the
    form required for windows that uses backslashes. */

private:
  registry_hives _hive;  //!< which hive our entries are stored in.

  // not to be called.
  registry_configurator(const registry_configurator &);
  registry_configurator &operator =(const registry_configurator &);

  static const basis::astring &reg_str_fake_default();
};

}

#endif

