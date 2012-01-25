#ifndef BASE_APPLICATION_CLASS
#define BASE_APPLICATION_CLASS

//////////////
// Name   : base_application
// Author : Chris Koeritz
//////////////
// Copyright (c) 2000-$now By Author.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation:
//     http://www.gnu.org/licenses/gpl.html
// or under the terms of the GNU Library license:
//     http://www.gnu.org/licenses/lgpl.html
// at your preference.  Those licenses describe your legal rights to this
// software, and no other rights or warranties apply.
// Please send updates for this code to: fred@gruntose.com -- Thanks, fred.
//////////////

#include <basis/contracts.h>
#include <loggers/logging_macros.h>

namespace application {

//! Provides a base object for the root application portion of a program.
/*!
  This mainly defines an entry point into the application's real functionality.
  Derived versions of the base_application can layer in more functionality as
  appropriate for different types of applications.
*/

class base_application : public virtual basis::nameable
{
public:
  virtual const char *class_name() const = 0;  // must be provided by implementor.

  virtual int execute() = 0;
    //!< performs the main activity of this particular application object.
    /*!< the method must be overridden by the derived object.  a return value
    for the program as a whole should be returned. */
};

//////////////

#if 0

//! This is an example usage of the base_application class...
class example_application : public base_application
{
public:
  example_application() : base_application() {}
  DEFINE_CLASS_NAME("example_application");
  int execute() { /* do stuff and return final exit value. */ }
};

//! This is a sample main application for a console mode program...
int __example__main(int argc, char *argv[])
{
  example_application root_program;
  return root_program.execute();
}

#endif // example guard.

} //namespace.

#endif // outer guard.

