#ifndef CRITICAL_EVENTS_GROUP
#define CRITICAL_EVENTS_GROUP

/*****************************************************************************\
*                                                                             *
*  Name   : critical_events                                                   *
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

#include <basis/astring.h>

namespace loggers {

//! This macro wraps the notion of stopping in the debugger.
//#ifndef _MSC_VER
  #define CAUSE_BREAKPOINT
//hmmm: need a unix equivalent for this, see old below for doze?
/*
#else
  #ifdef __MINGW32__
    extern "C" {
//      #include <ddk/winddi.h>
//for debugbreak.  does this cause other problems?
    }
    #define CAUSE_BREAKPOINT 
//    #define CAUSE_BREAKPOINT __debugbreak()
  #else
    #define CAUSE_BREAKPOINT __debugbreak()
  #endif
#endif
*/

//! Tests the value "check" to ensure that it's not zero.
/*! This can be used instead of an ASSERT macro to check conditions in
builds with ERRORS_ARE_FATAL turned on.  This macro is orthogonal to the
build being built with debugging or release features. */
#ifdef ERRORS_ARE_FATAL
  #define REQUIRE(check) if (!check) CAUSE_BREAKPOINT;
#else
  #define REQUIRE(check)
#endif

// now more dangerous or potent guards...

//! an extra piece of information used, if available, in bounds_halt below.
//#define BH_ERROR_ADDITION ((basis::astring(" in ") + _global_argv[0]).s())

//! Verifies that "value" is between "low" and "high", inclusive.
/*! "Value" must be an object for which greater than and less than are defined.
The static_class_name() method and func definition are used to tag the
complaint that is emitted when problems are detected.  Note that if
CATCH_ERRORS is defined, then the program is _halted_ if the value is out
of bounds.  Otherwise, the "to_return" value is returned. */
//#ifdef CATCH_ERRORS
//  #define bounds_halt(value, low, high, to_return) { 
//    if (((value) < (low)) || ((value) > (high))) { 
//      critical_events::implement_bounds_halt(static_class_name(), func, #value, #low, 
//          #high, BH_ERROR_ADDITION); 
//      return to_return; 
//    } 
//  }
//#else
//  #define bounds_halt(a, b, c, d) bounds_return(a, b, c, d)
//#endif

//////////////

//! Provide some macros that will automatically add the file and line number.
/*! These use the functions below to report different types of error
situations and in some cases, exit the program. */
#define non_continuable_error(c, f, i) \
  critical_events::FL_non_continuable_error(__FILE__, __LINE__, c, f, i, \
      "A Non-Continuable Runtime Problem Has Occurred")
#define continuable_error(c, f, i) \
  critical_events::FL_continuable_error(__FILE__, __LINE__, c, f, i, \
      "Runtime Problem Information")
#define console_error(c, f, i) \
  critical_events::FL_console_error(__FILE__, __LINE__, c, f, i)
#define deadly_error(c, f, i) \
  critical_events::FL_deadly_error(__FILE__, __LINE__, c, f, i)
#define out_of_memory_now(c, f) \
  critical_events::FL_out_of_memory_now(__FILE__, __LINE__, c, f)

//////////////

//! Provides a means of logging events for runtime problems.

class critical_events : public virtual basis::root_object
{
public:
  virtual ~critical_events() {}

  // super handy system inter-operation functions...

  static basis::un_int system_error();
    //!< gets the most recent system error reported on this thread.

  static basis::astring system_error_text(basis::un_int error_to_show);
    //!< returns the OS's string form of the "error_to_show".
    /*!< this often comes from the value reported by system_error(). */

  // some noisemaking methods...

  //! Prints out a message to the standard error file stream.
  static void write_to_console(const char *message);

  //! shows the message in "info", with an optional "title" on the message.
  /*! the message is sent to the program wide logger, if one is expected to
  exist in this program. */
  static void alert_message(const char *info, const char *title = "Alert Message");
  static void alert_message(const basis::astring &info);  // uses default title.
  static void alert_message(const basis::astring &info, const basis::astring &title);  // use "title".

  static void write_to_critical_events(const char *message);
    //!< sends the "message" to the critical events log file.
    /*!< Prints out a message to the specially named file that captures all
    serious events written to error logging functions.  If you use the
    functions in this file, you are likely already writing to this log. */

  static void set_critical_events_directory(const basis::astring &directory);
    //!< sets the internal location where the critical events will be logged.
    /*!< this is postponed to a higher level, although the default
    will work too. */

  static basis::astring critical_events_directory();
    //!< returns the current location where critical events are written.

  // this section implements the error macros.

  //! Prints out an error message and then exits the program.
  /*! The parameters describe the location where the error was detected.  This
  call should only be used in test programs or where absolutely necessary
  because it causes an almost immediate exit from the program!  deadly_error
  should be reserved for when the program absolutely has to exit right then,
  because this function will actually enforce a crash / abort / break into
  debugger action rather than just calling exit(). */
  static void FL_deadly_error(const char *file, int line, const char *classname,
      const char *function_name, const char *info);
  //! A version that takes strings instead of char pointers.
  static void FL_deadly_error(const basis::astring &file, int line,
      const basis::astring &classname, const basis::astring &function_name,
      const basis::astring &info);

  //! Describes an error like deadly_error, but does not exit the program.
  static void FL_continuable_error(const char *file, int line,
      const char *classname, const char *function_name, const char *info,
      const char *title);

  //! A version using astring instead of char pointers.
  static void FL_continuable_error(const basis::astring &file, int line,
      const basis::astring &classname, const basis::astring &function_name,
      const basis::astring &info, const basis::astring &title);

  //! Shows the same information as continuable_error, but causes an exit.
  /*! This is a friendlier program exit than deadly_error.  This version can
  be used when the program must stop, possibly because of a precondition failure
  or a problem in input data or basically whatever.  it is not intended to
  abort or break into the debugger, but to simply exit(). */
  static void FL_non_continuable_error(const char *file, int line,
      const char *classname, const char *function_name, const char *info,
      const char *title);

  //! A version using astring instead of char pointers.
  static void FL_non_continuable_error(const basis::astring &file, int line,
      const basis::astring &classname, const basis::astring &function_name,
      const basis::astring &info, const basis::astring &title);

  //! Causes the program to exit due to a memory allocation failure.
  static void FL_out_of_memory_now(const char *file, int line,
      const char *classname, const char *function_name);
  
  //! Prints out an error message to the standard error file stream.
  static void FL_console_error(const char *file, int line, const char *error_class,
      const char *error_function, const char *info);

  //! Used to build our particular type of error message.
  /*! It writes an error message into "guards_message_space" using our stylized
  object::method formatting. */
  static void make_error_message(const char *file, int line,
      const char *error_class, const char *error_function,
      const char *info, char *guards_message_space);

  //! Provides the real implementation of bounds_halt to save code space.
  static void implement_bounds_halt(const char *the_class_name, const char *func,
      const char *value, const char *low, const char *high,
      const char *error_addition);

private:
  static basis::astring &hidden_critical_events_dir();

  static void FL_continuable_error_real(const char *file, int line,
      const char *error_class, const char *error_function, const char *info,
      const char *title);
};

} //namespace.

#endif

