#ifndef CONTRACTS_GROUP
#define CONTRACTS_GROUP

/*****************************************************************************\
*                                                                             *
*  Name   : contracts                                                         *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1989-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

/*! @file contracts.h
  This is a collection of fairly vital interface classes.
*/

#include "outcome.h"

namespace basis {

// forward declarations.
class base_string;

//////////////

//! Defines an attribute base class that supports get and set operations.

class attribute : public virtual root_object
{
public:
  virtual const root_object &get() const = 0;
  virtual void set(const root_object &new_value) = 0;
};

//////////////

//! Base class for object that can tell itself apart from other instances.
class equalizable : public virtual root_object
{
public:
  virtual bool equal_to(const equalizable &s2) const = 0;
    //! the virtual method for object equality.
  virtual bool operator == (const equalizable &s2) const { return equal_to(s2); }
    //! synactic sugar for comparison operators.
};

//////////////

//! A base for objects that can be alphabetically (lexicographically) ordered.

class orderable : public virtual equalizable
{
public:
  virtual bool less_than(const orderable &s2) const  = 0;
    //! the virtual method for object ordering.
  virtual bool operator < (const orderable &s2) const { return less_than(s2); }
    //! synactic sugar for comparison operators.
};

//////////////

//! Provides an abstract base for logging mechanisms.

class base_logger : public virtual root_object
{
public:
  virtual outcome log(const base_string &info, int filter) = 0;
    //!< writes the information in "info" to the logger using the "filter".
    /*!< the derived class can interpret the filter appropriately and only
    show the "info" if the filter is enabled. */
};

//////////////

//! Macro for defining a logging filter value.
#define DEFINE_FILTER(NAME, CURRENT_VALUE, INFO_STRING) NAME = CURRENT_VALUE

//! These filter values are the most basic, and need to be known everywhere.
enum root_logging_filters {
  DEFINE_FILTER(NEVER_PRINT, -1, "This diagnostic entry should be dropped and never seen"),
  DEFINE_FILTER(ALWAYS_PRINT, 0, "This diagnostic entry will always be shown or recorded")
};

//////////////

//! Interface for a simple form of synchronization.
/*!
  Derived classes must provide a locking operation and a corresponding
  unlocking operation.
*/

class base_synchronizer : public virtual root_object
{
public:
  virtual void establish_lock() = 0;
  virtual void repeal_lock() = 0;
};

//////////////

//! A clonable object knows how to make copy of itself.

class clonable : public virtual root_object
{
public:
  virtual clonable *clone() const = 0;
};

//////////////

//! Root object for any class that knows its own name.
/*!
  This is a really vital thing for debugging to be very helpful, and unfortunately it's not
  provided by C++.
*/

class nameable : public virtual root_object
{
public:
  virtual const char *class_name() const = 0;
    //!< Returns the bare name of this class as a constant character pointer.
    /*!< The name returned here is supposed to be just a class name and not
    provide any more information than that.  It is especially important not to
    add any syntactic elements like '::' to the name, since a bare alphanumeric
    name is expected. */
};

//////////////

//! A base class for objects that can provide a synopsis of their current state.
/*!
  This helps a lot during debugging and possibly even during normal runtime, since it causes
  the object to divulge its internal state for viewing in hopefully readable text.
*/

class text_formable : public virtual nameable
{
public:
  virtual const char *class_name() const = 0;  // forwarded requirement from nameable.

  virtual void text_form(base_string &state_fill) const = 0;
    //!< Provides a text view of all the important info owned by this object.
    /*!< It is understood that there could be a large amount of information and that this
    function might take a relatively long time to complete. */
};

//////////////

//! the base class of the most easily used and tested objects in the library.
/*!
  Each hoople_standard object must know its name, how to print out its data members, and whether
  it's the same as another object or not.
*/

class hoople_standard : public virtual text_formable, public virtual equalizable
{
public:
  // this is a union class and has no extra behavior beyond its bases.
};

//////////////

//! a base for classes that can stream their contents out to a textual form.

class text_streamable : public virtual nameable
{
public:
  virtual bool produce(base_string &target) const = 0;
    //!< sends the derived class's member data into the "target" in a reversible manner.
    /*!< this should use a tagging system of some sort so that not only can the derived class
    verify that its type is really right there in the string, but also that it gets all of its
    class data and no other data.  the "target" will be destructively consumed, and after a
    successful call will no longer contain the object's streamed form at its head. */
  virtual bool consume(const base_string &source) = 0;
    //!< chows down on a string that supposedly contains a streamed form.
    /*!< the derived class must know how to eat just the portion of the string that holds
    its data type and no more. */
};

} //namespace.

#endif

