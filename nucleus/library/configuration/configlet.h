#ifndef CONFIGLET_CLASS
#define CONFIGLET_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : configlet                                                         *
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

#include <basis/astring.h>
#include <basis/contracts.h>

namespace configuration {

//! Represents an atom of configuration info.
/*!
  The configlet has a location in a configuration repository that is defined
  by its section and key name.  Derived types can also have a value that is
  stored in that location.
*/

class configlet : public virtual basis::root_object
{
public:
  configlet(const basis::astring &section, const basis::astring &entry);
    //!< creates a configlet that lives in the "section" at the "entry".
  configlet(const configlet &to_copy);

  virtual ~configlet();

  DEFINE_CLASS_NAME("configlet");

  configlet &operator =(const configlet &to_copy);

  const basis::astring &section() const;
    //!< observes the section of this configlet.
  const basis::astring &entry() const;
    //!< observes the entry name of this configlet.

  void section(const basis::astring &new_section) const;
    //!< modifies the configlet section location.
  void entry(const basis::astring &new_entry) const;
    //!< modifies the configlet entry name.

  virtual bool load(configurator &config) = 0;
    //!< retrieves the configlet's information from the "config".
    /*!< true is returned when this is successful.  note that false is returned
    if the entry was not originally present; if the configurator has the
    AUTO_STORE behavior, then we will write out the default value on failure.
    the next load() would be a success in that case, but would return the
    default. */

  virtual bool store(configurator &config) const = 0;
    //!< writes the configlet's information out to the "config".

  virtual configlet *duplicate() const = 0;
    //!< a virtual copy constructor for configlets.
    /*!< the returned object will be a new copy of this configlet. */

private:
  basis::astring *_section;  //!< the section name, with whatever form is appropriate.
  basis::astring *_entry;  //!< the entry name in the native representation.
};

//////////////

//! a string_configlet holds onto a character string value.
/*!
  it has a current value, which could change due to updates to the
  configuration, and a default value that probably won't change as often.
  if the "load" operation fails, the default value will be used.
*/

class string_configlet : public configlet
{
public:
  string_configlet(const basis::astring &section, const basis::astring &entry,
          const basis::astring &current_value = basis::astring::empty_string(),
          const basis::astring &default_value = basis::astring::empty_string());
  string_configlet(const string_configlet &to_copy);
  virtual ~string_configlet();

  string_configlet &operator =(const string_configlet &to_copy);

  const basis::astring &current_value() const;
  const basis::astring &default_value() const;

  void current_value(const basis::astring &new_current);
  void default_value(const basis::astring &new_default);
  
  virtual bool load(configurator &config);
  virtual bool store(configurator &config) const;

  configlet *duplicate() const;

private:
  basis::astring *_current;
  basis::astring *_default;
};

//////////////

//! Stores a simple integer in a configuration repository.

class int_configlet : public configlet
{
public:
  int_configlet(const basis::astring &section, const basis::astring &entry,
          int current_value = 0, int default_value = 0);
  virtual ~int_configlet();

  int current_value() const { return _current; }

  virtual void current_value(int new_current);
    //!< the modifier function is virtual so derived classes can extend.

  int default_value() const { return _default; }
  void default_value(int new_default) { _default = new_default; }

  virtual bool load(configurator &config);
  virtual bool store(configurator &config) const;

  configlet *duplicate() const;

private:
  int _current;
  int _default;
};

//////////////

//! Stores an integer in a configuration repository with range checking.
/*!
  a bounded_int_configlet has current and default values but also specifies a
  valid range for the current value.  if the current value falls outside
  of that range (even via a "set" operation), then the default value is
  used for the current.
*/

class bounded_int_configlet : public int_configlet
{
public:
  bounded_int_configlet(const basis::astring &section, const basis::astring &entry,
          int current_value, int default_value, int minimum, int maximum);
  virtual ~bounded_int_configlet();

  virtual void current_value(int new_current);

  int minimum() const { return _minimum; }
  int maximum() const { return _maximum; }
  
  void minimum(int new_min) { _minimum = new_min; }
  void maximum(int new_max) { _maximum = new_max; }

  configlet *duplicate() const;

private:
  int _minimum;
  int _maximum;
};

} //namespace.

#endif

