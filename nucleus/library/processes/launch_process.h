#ifndef LAUNCH_PROCESS_CLASS
#define LAUNCH_PROCESS_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : launch_process
*  Author : Chris Koeritz
*                                                                             *
*******************************************************************************
* Copyright (c) 1994-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/array.h>
#include <basis/astring.h>
#include <basis/definitions.h>

// forward.
struct tagMSG;

namespace processes {

//! a simple wrapper of an array of char *, used by launch_process::break_line().
class char_star_array : public basis::array<char *>
{
public:
  char_star_array() : basis::array<char *>(0, NULL_POINTER, SIMPLE_COPY | EXPONE | FLUSH_INVISIBLE) {}
  ~char_star_array() {
    // clean up all the memory we're holding.
    for (int i = 0; i < length(); i++) {
      delete [] (use(i));
    }
  }
};

//////////////

//! Provides the capability to start processes in a variety of ways to run other applications.

class launch_process : public virtual basis::nameable
{
public:
  DEFINE_CLASS_NAME("launch_process");

  virtual ~launch_process() {}

  enum launch_flags {
    HIDE_APP_WINDOW = 0x1,
      //!< launches the application invisibly if possible.
    AWAIT_APP_EXIT = 0x2,
      //!< stays in the function until the launched application has exited.
    RETURN_IMMEDIATELY = 0x4,
      //!< starts the application and comes right back to the caller.
    AWAIT_VIA_POLLING = 0x8,
      //!< launches the app but polls and doesn't block on its exit.
    SHELL_EXECUTE = 0x10
      //!< only valid on windows--uses ShellExecute instead of CreateProcess.
  };

  static basis::un_int run(const basis::astring &app_name, const basis::astring &command_line,
          int flag, basis::un_int &child_id);
    //!< starts an application using the "app_name" as the executable to run.
    /*!< the "command_line" is the set of parameters to be passed to the app.
    the return value is OS specific but can be identified using
    system_error_text().  usually a zero return means success and non-zero
    codes are errors.  the "flag" is an XORed value from the process launch
    flags that dictates how the app is to be started.  in practice, only the
    HIDE_APP_WINDOW flag can be combined with other values.  if either AWAIT
    flag is used, then the return value will be the launched process's own
    exit value.  the thread or process_id of the launched process is stored
    in "child_id" if appropriate. */

  static char_star_array break_line(basis::astring &app, const basis::astring &parameters);
    //!< prepares an "app" to launch with the "parameters" (via exec).
    /*!< this breaks the strings for an application named "app" and its
    "parameters" into an array of char * that is appropriate for the execv
    function. */

private:
#ifndef _MSC_VER
  static void exiting_child_signal_handler(int sig_num);
    //!< awaits the child processes rather than leaving process handles willy nilly.
#else
  static bool event_poll(tagMSG &message);
    //!< tries to process one win32 event and retrieve the "message" from it.
    /*!< this is a very general poll and will retrieve any message that's
    available for the current thread.  the message is actually processed
    here also, by calling translate and dispatch.  the returned structure
    is mainly interesting for knowing what was done. */
#endif
  
};

} // namespace.

#endif // outer guard.

