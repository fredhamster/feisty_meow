#ifndef CONFIGURATOR_CLASS
#define CONFIGURATOR_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : configurator                                                      *
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

#include <basis/astring.h>
#include <basis/contracts.h>
#include <structures/set.h>
#include <structures/string_array.h>
#include <structures/string_table.h>

namespace configuration {

//! Provides a base class for configuration repositories.
/*!
  All items that can be stored are modelled as having an entry name and a
  value.  Groups of entries are stored in sections, in which the data
  usually have some relation to each other or share a common purpose.
*/

class configurator : public virtual basis::root_object
{
public:
  enum treatment_of_defaults { AUTO_STORE, RETURN_ONLY };
    //!< Governs how missing items are treated.
    /*!< When the default value is used in the get() method below, it can
    either be written to the ini file automatically (AUTO_STORE) or it can
    just be returned (RETURN_ONLY). */

  configurator(treatment_of_defaults behavior = RETURN_ONLY) : _behavior(behavior) {}
  virtual ~configurator();

  //! observes the behavior chosen for the load() function.
  treatment_of_defaults behavior() const { return _behavior; }
  //! modifies the behavior of the load() function.
  void behavior(treatment_of_defaults new_behavior) {
    _behavior = (new_behavior == RETURN_ONLY)? RETURN_ONLY : AUTO_STORE;
  }

  virtual bool get(const basis::astring &section, const basis::astring &entry,
          basis::astring &found) = 0;
    //!< Retrieves an item from the configuration store.
    /*!< This retrieves a string into "found" that is listed in the "section"
    specified under the "entry" name.  if the string is not found, false is
    returned. */

  virtual bool put(const basis::astring &section, const basis::astring &entry,
          const basis::astring &to_store) = 0;
    //!< Places an item into the configuration store.
    /*!< places an entry into the "section" under the "entry" name using the
    string "to_store".  if the storage was successful, true is returned.
    reasons for failure depend on the derived class implementations. */

  bool store(const basis::astring &section, const basis::astring &entry,
          const basis::astring &to_store);
    //!< a synonym for put.

  //! a synonym for get that implements the auto-store behavior.
  /*! if the behavior is set to auto-store, then the default value will be
  written when no value existed prior to the load() invocation. */
  basis::astring load(const basis::astring &section, const basis::astring &entry,
          const basis::astring &default_value);

  //! stores an integer value from the configuration store.
  bool store(const basis::astring &section, const basis::astring &entry, int value);
  //! loads an integer value from the configuration store.
  int load(const basis::astring &section, const basis::astring &entry, int def_value);

  // the various methods below that operate on sections and entries might not
  // be provided by all configurators.  that is why there are empty default
  // (or simplistic and slow) implementations provided below.

  virtual void sections(structures::string_array &list);
    //!< retrieves the section names into "list".

  void section_set(structures::string_set &list);
    //!< similar to above, but stores section names into a set.
    /*!< this never needs to be overridden; it's simply a set instead
    of an array.  the real sections method is above for string_array. */

  virtual bool delete_entry(const basis::astring & formal(section),
          const basis::astring & formal(entry)) { return false; }
    //!< eliminates the entry specified by the "section" and "entry" name.

  virtual bool delete_section(const basis::astring & formal(section) )
          { return false; }
    //!< whacks the entire "section" specified.

  virtual bool section_exists(const basis::astring &section);
    //!< returns true if the "section" is found in the configurator.
    /*!< the default implementation is quite slow; if there is a swifter means
    for a particular type of configurator, then this should be overridden. */

  virtual bool get_section(const basis::astring & formal(section),
          structures::string_table & formal(found) ) { return false; }
    //!< retrieves an entire "section", if supported by the derived object.
    /*!< the symbol table "found" gets the entries from the "section".
    see symbol_table.h for more details about string_tables.  true is
    returned if the section existed and its contents were put in "found". */

  virtual bool put_section(const basis::astring & formal(section),
          const structures::string_table & formal(to_store) ) { return false; }
    //!< stores an entire "section" from the table in "to_store", if supported.
    /*!< if any entries already exist in the "section", then they are
    eliminated before the new entries are stored.  true is returned if the
    write was successful. */

private:
  treatment_of_defaults _behavior;  //!< records the treatment for defaults.
};

} //namespace.

#endif

