#ifndef OUTCOME_CLASS
#define OUTCOME_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : outcome                                                           *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1990-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "definitions.h"

namespace basis {

//! Outcomes describe the state of completion for an operation.
/*!
  These are an extremely simple representation of a non-exceptional exit
  value as an integer.  The range of expression is from 0 to MAXINT.
  Outcomes are meant to represent the category of 'how' the operation
  completed; they do not carry along any results of 'what' was produced.
*/

class outcome
{
public:
  outcome(int value = 0) : c_outcome_value(value) {}
    //!< Represents the completion of an operation as a particular "value".
    /*!< The outcomes partition the input space of the operation and represent
    the different conclusions possible during processing.  Values for outcomes
    must be maintained on a system-wide basis as unique identifiers for them
    to be meaningful.  Note that zero is reserved as a default outcome value.
    This usually translates to an outcome of OKAY. */

  ~outcome() { c_outcome_value = 0; }  //!< destructor resets outcome value.

  bool equal_to(const outcome &to_compare) const
      { return c_outcome_value == to_compare.c_outcome_value; }
    //!< Returns true if this outcome is equal to "to_compare".
    /*!< comparisons between outcomes will operate properly provided that
    all system-wide outcome values are unique. */

  bool operator == (int to_compare) const
      { return c_outcome_value == to_compare; }
    //!< Returns true if this outcome is equal to the integer "to_compare".

  int value() const { return c_outcome_value; }
    //<! returns the numerical value for the outcome.

  int packed_size() const { return 4; }
    //!< a convenience function for those packing outcomes.
    /*!< this is inconveniently hard-coded, since packing is only available in the structures
    library and not at the base. */

private:
  int c_outcome_value;  //<! the numerical id for the outcome.
};

//////////////

//! checks if the outcome indicates non-okay completion and returns it if so.
#define PROMOTE_OUTCOME(outc) \
  if (outc != common::OKAY) return outc

//////////////

//! Provides a way to define auto-generated outcome values.
/*! This macro provides a way to mark our outcome values so that parsers
can find all of them when needed.
NOTE: do not use the macro in CPP files; it's only for headers.  if you
define outcomes in CPP files, they will not be added to the build
manifest, nor will they be guaranteed to be unique or auto-generated. */

#define DEFINE_OUTCOME(NAME, CURRENT_VALUE, INFO_STRING) \
  NAME = CURRENT_VALUE

//! Similar to standard outcomes (above), but these cannot be auto-incremented.
/*! This macro specifies an outcome definition where the value is constant,
because it must participate in a network API.  Standard outcomes are just
return values from method calls signifying completion, but the API style
outcomes can be used to describe remote procedural invocations.  Thus, once
they are established with unique values, those values cannot change. */

#define DEFINE_API_OUTCOME(NAME, CURRENT_VALUE, INFO_STRING) \
  NAME = CURRENT_VALUE

//////////////

} //namespace.

#endif

