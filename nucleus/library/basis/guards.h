#ifndef GUARDS_GROUP
#define GUARDS_GROUP

//////////////
// Name   : guards
// Author : Chris Koeritz
//////////////
// Copyright (c) 1989-$now By Author.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation:
//     http://www.gnu.org/licenses/gpl.html
// or under the terms of the GNU Library license:
//     http://www.gnu.org/licenses/lgpl.html
// at your preference.  Those licenses describe your legal rights to this
// software, and no other rights or warranties apply.
// Please send updates for this code to: fred@gruntose.com -- Thanks, fred.
//////////////

//! The guards collection helps in testing preconditions and reporting errors.
/*!
  It also provides checking of boundary conditions, macros for causing
  immediate program exit, and other sentinels for constructing preconditions
  and postconditions.
*/

namespace basis {

// forward declaration.
class astring;
class base_string;

//////////////

// simpler guards first...

//! Returns true if the value is within the range specified.
template <class contents>
bool in_range(const contents &value, const contents &low, const contents &high)
{ return !( (high < low) || (value < low) || (value > high) ); }

//////////////

//! Verifies that "value" is between "low" and "high", inclusive.
/*! When the number is not in bounds, the function that is currently executing
returns the "to_return" default provided.  "to_return" can be empty for
functions that return void.  Note: it is also considered a failure for high to
be less than low. */
#define bounds_return(value, low, high, to_return) \
    { if (!basis::in_range(value, low, high)) return to_return; }

//////////////

//! Verifies that "value" is between "low" and "high", inclusive.
/*! "Value" must be an object for which greater than and less than are defined.
The static_class_name() method and func definition are used to tag the
complaint that is emitted when problems are detected.  Note that if
CATCH_ERRORS is defined, then the program is _halted_ if the value is out
of bounds.  Otherwise, the "to_return" value is returned. */
#ifdef CATCH_ERRORS
  #define bounds_halt(value, low, high, to_return) { \
    if (((value) < (low)) || ((value) > (high))) { \
      throw_error(basis::astring(static_class_name()), basis::astring(func), \
          basis::astring("value ") + #value \
          + " was not in range " + #low + " to " + #high \
          + " at " + __WHERE__); \
      return to_return; \
    } \
  }
#else
  #define bounds_halt(a, b, c, d) bounds_return(a, b, c, d)
#endif

//////////////

//! writes a string "to_fill" in a nicely formatted manner using the class and function names.
void format_error(const base_string &class_name, const base_string &func_name,
    const base_string &error_message, base_string &to_fill);

//! throws an error that incorporates the class name and function name.
void throw_error(const base_string &class_name, const base_string &func_name,
    const base_string &error_message);

//! synonym method using astrings for easier char * handling.
void throw_error(const astring &class_name, const astring &func_name,
    const astring &error_message);

//////////////

}  // namespace.

#endif

