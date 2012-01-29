#ifndef TABLE_CONFIGURATOR_CLASS
#define TABLE_CONFIGURATOR_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : table_configurator                                                *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2001-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "configurator.h"

#include <basis/contracts.h>

namespace configuration {

// forward.
class table_o_string_tables;

//! Supports the configurator interface using a collection of string tables.

class table_configurator : public virtual configurator
{
public:
  table_configurator(treatment_of_defaults behavior = AUTO_STORE);
    //!< Constructor just needs to know what to do for missing items.
    /*!< Creates a table_configurator that loads and stores entries into
    the internal collection of tables.  It will use the "behavior" regarding
    missing entries when load() is invoked. */

  table_configurator(const table_configurator &to_copy);

  virtual ~table_configurator();

  table_configurator &operator =(const table_configurator &to_copy);

  DEFINE_CLASS_NAME("table_configurator");

  virtual void sections(structures::string_array &list);
    //!< retrieves the section names into "list".

  void reset();  // clears out all contents.

  virtual bool get(const basis::astring &section, const basis::astring &entry,
          basis::astring &found);
    //!< implements the configurator retrieval function.

  virtual bool put(const basis::astring &section, const basis::astring &entry,
          const basis::astring &to_store);
    //!< implements the configurator storage function.

  virtual bool section_exists(const basis::astring &section);
    //!< true if the "section" is presently in the table config.

  virtual bool delete_section(const basis::astring &section);
    //!< removes the entire "section" specified.

  virtual bool delete_entry(const basis::astring &section, const basis::astring &entry);
    //!< removes the entry specified by the "section" and "entry" name.

  virtual bool get_section(const basis::astring &section, structures::string_table &info);
    //!< reads the entire table held under "section" into a table called "info".

  virtual bool put_section(const basis::astring &section, const structures::string_table &info);
    //!< writes a table called "info" into the "section" held here.

private:
  table_o_string_tables *_real_table;
    //!< the data structure we're actually operating on.
};

} //namespace.

#endif

