#ifndef ENVIRONMENT_CLASS
#define ENVIRONMENT_CLASS

//////////////
// Name   : environment
// Author : Chris Koeritz
//////////////
// Copyright (c) 1994-$now By Author.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation:
//     http://www.gnu.org/licenses/gpl.html
// or under the terms of the GNU Library license:
//     http://www.gnu.org/licenses/lgpl.html
// at your preference.  Those licenses describe your legal rights to this
// software, and no other rights or warranties apply.
// Please send updates for this code to: fred@gruntose.com -- Thanks, fred.
//////////////

#include "astring.h"
#include "definitions.h"

namespace basis {

//! Provides access to the system's environment variables.

class environment : public virtual root_object
{
public:
  static astring TMP();
    //!< provides a single place to get the temporary directory.
    /*!< this will locate the value of the TMP variable and return it.  if the TMP
    variable is not currently defined, this will try to do something reasonable for
    a default value. */

  static astring get(const astring &variable_name);
    //!< looks up the "variable_name" in the current environment variables.
    /*!< this returns the value for "variable_name" as it was found in the
    operating system's environment variables that are defined at this point
    in time for the user and process.  the returned string will be empty if no
    variable under that name could be found. */

  static bool set(const astring &variable_name, const astring &value);
    //!< adds or creates "variable_name" in the environment.
    /*!< changes the current set of environment variables by adding or
    modifying the "variable_name".  its new value will be "value". */

  static basis::un_int system_uptime();
    //!< gives the operating system's uptime in a small form that rolls over.
};

} //namespace.

#endif

