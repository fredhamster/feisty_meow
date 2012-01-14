#ifndef APPLICATION_SHELL_CLASS
#define APPLICATION_SHELL_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : application_shell                                                 *
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

#include "base_application.h"

#include <basis/contracts.h>
#include <mathematics/chaos.h>

namespace application {

//! The application_shell is a base object for console programs.
/*!
  It is generally used in that context (console mode), but can be employed as the root of windowed
  programs also.  The application_shell provides a few features, such as logging functionality,
  that make it a bit easier than starting up a program from scratch every time.
*/

class application_shell : public base_application
{
public:  
  application_shell();
    //!< constructs an application_shell to serve as the root of the program.

  virtual ~application_shell();

  DEFINE_CLASS_NAME("application_shell")

  static application_shell *single_instance();
    //!< in a program with a single application_shell extant, this gives out the instance.
    /*!< if there are more than one application_shells floating around a program, then this
    will only give out the most recently registered.  note that this pointer is not the
    slightest bit thread safe if it's changing after the application shell has been constructed,
    and that it cannot be relied upon until that's happened either.  be careful.  do not use
    it in any function that might be invoked during program shutdown. */

  virtual int execute_application();
    //!< runs the base class's execute() method and catches any exceptions due to it.
    /*!< you can override this method, but generally should never need to.  the derived class's
    method should catch exceptions and deal with them meaningfully also. */

  int exit_value() const { return c_exit_value; }
    //!< once the application has finished executing, this will contain the exit value.

  const mathematics::chaos &randomizer() const { return c_rando; }
    //!< provides access to the random number generator owned by this app.

//  static basis::astring application_name();
    //!< returns the full name of the current application.

//  static basis::u_int process_id();
    //!< returns the process id for this task, if that's relevant on the OS.

//  static basis::astring current_directory();
    //!< returns the current directory as reported by the operating system.

  virtual basis::outcome log(const basis::base_string &to_print, int filter = basis::ALWAYS_PRINT);
    //!< as above, logs a line "to_print" but only if the "filter" is enabled.
    /*!< the second version uses the filter value to assess whether to print
    the string or not.  the string will not print if that filter is not
    enabled for the program wide logger. */

#ifdef __UNIX__
//  static basis::astring get_cmdline_from_proc();
    //!< retrieves the command line from the /proc hierarchy on linux.
//  static basis::astring query_for_process_info();
    //!< seeks out process info for a particular process.
#endif

protected:
  virtual int execute() = 0;
    //!< forwards base_application responsibility upwards to derived objects.
    /*!< this should not be invoked by anyone in general; it is automatically called from
    the constructor of this class. */

private:
  mathematics::chaos c_rando;  //!< random number generator.
  int c_exit_value;  //!< how did things end up for the app?

  // not applicable.
  application_shell(const application_shell &);
  application_shell &operator =(const application_shell &);
};

} //namespace.

#endif // outer guard.

