#ifndef SECTION_MANAGER_CLASS
#define SECTION_MANAGER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : section_manager                                                   *
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

#include <structures/string_table.h>

namespace configuration {

//! Tracks a collection of related configurations in a configurator.
/*!
  If there is a set of items that need to be stored in a configurator, where
  each item has its own configuration section, then this object can help out.
  It manages a collection of uniquely named sections in a configurator object
  and provides a table of contents (TOC) feature for the names of the sections.
  Each item lives in its own distinct section but the whole set can be
  operated on as one entity.
*/

class section_manager : public virtual basis::nameable
{
public:
  section_manager(configurator &config, const basis::astring &toc_title,
          const basis::astring &header_prefix);
    //!< creates a section_manager that uses the "config" for storage.
    /*!< the "toc_title" is the name of the section to be used for storing the
    table of contents for the managed configuration items.  the "header_prefix"
    will be prepended to the names of each section to facilitate locating them.
    for example, if the "toc_title" is "client channels" and the "header_prefix"
    is "CliChan__", then the resulting configuration might look similar to the
    following: @code
      [client channels]
      joe
      ted
      [CliChan__joe]
      port=58
      [CliChan__ted]
      address=13.8.92.4
      auth=primary
    @endcode */

  ~section_manager();

  DEFINE_CLASS_NAME("section_manager");

  bool section_exists(const basis::astring &section_name);
    //!< returns true if the section called "section_name" exists in the config.

  bool get_section_names(structures::string_array &sections);
    //!< loads the "sections" array with all section names.
    /*!< this comes from our table of contents.  true is returned if there
    were any names to load. */

  bool add_section(const basis::astring &section_name, const structures::string_table &to_add);
    //!< stores a new section for "section_name" using the table "to_add".
    /*!< this will fail if the section already exists. */

  bool replace_section(const basis::astring &section, const structures::string_table &replacement);
    //!< replaces the contents of "section" with the "replacement" table.
    /*!< this will fail if the section does not already exist. */

  bool zap_section(const basis::astring &section_name);
    //!< removes the data for "section_name" from both the config and TOC.
    /*!< this will fail if the section is not present. */

  bool find_section(const basis::astring &section_name, structures::string_table &found);
    //!< loads the data from "section_name" into the table "found".
    /*!< this fails if the section doesn't exist or if the section's contents
    couldn't be detokenized into a table of name/value pairs. */

  configurator &config() { return _config; }
    //!< allows access to the configurator we operate on.
    /*!< getting single items from the config will be signficantly faster
    using it directly; the make_section_heading() method can be used to locate
    the proper section. */

  bool get_toc(structures::string_table &toc);
    //!< reads the table of contents into "toc".

  basis::astring make_section_heading(const basis::astring &section);
    //!< provides the appropriate heading string for the "section" name.
    /*!< this can be used to find entries using the config(). */

private:
  configurator &_config;  //!< the configuration object we interact with.
  basis::astring *_toc_title;  //!< the table of contents' section name.
  basis::astring *_section_prefix;  //!< prefix attached to the name of the section.

  section_manager(const section_manager &);  //!< currently forbidden.
  section_manager &operator =(const section_manager &);
    //!< currently forbidden.
};

} //namespace.

#endif

