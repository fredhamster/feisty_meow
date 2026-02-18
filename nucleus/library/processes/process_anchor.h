#ifndef PROCESS_ANCHOR_CLASS
#define PROCESS_ANCHOR_CLASS

/*
*
*  Name   : process_anchor
*  Author : Chris Koeritz
*
****
* Copyright (c) 2000-$now By Author.  This program is free software; you can
* redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either version 2 of
* the License or (at your option) any later version.  This is online at:
*     http://www.fsf.org/copyleft/gpl.html
* Please send any updates to: fred@gruntose.com
*/

#include <application/windoze_helper.h>
#include <basis/astring.h>

#ifdef __LINUX__
  #include "shutdown_alerter.h"
#endif

//! Implements a graceful shutdown procedure for an application.
/*!
  The advantage of the process_anchor (where it's actually a window, which is
  to say, under win32) is that it's hidden and has a well-known name.  This
  allows it to be found through FindWindow to facilitate a graceful shutdown
  for the applications that use it.  On linux, it is implemented with a
  shutdown_alerter object instead and is not implemented using windowing.
*/
class process_anchor
#ifdef __LINUX__
: public shutdown_alerter
#endif
{
public:
  process_anchor();
    //!< constructor does very little; setup() begins operation.

  virtual ~process_anchor();

  DEFINE_CLASS_NAME("process_anchor");

  static bool close_this_program();
    //!< causes this particular program to shut down.
    /*!< this method is available for programs that support the graceful
    shutdown process.  it causes the machinery of the process_anchor to alert
    the application that a graceful shutdown has been started.  the method
    returns true if this program's process_anchor was found and sent the
    close message. */

  static bool close_app_anchor(const basis::astring &app_name);
    //!< closes the anchor object associated with "app_name".
    /*!< note that this closes *ALL* such anchors, so if the application had
    more than one instance running, they will all shut down.
also note: the above doesn't seem to be implemented in the current version.
 */

  bool defunct() const;
    //!< returns true if the object has been marked as defunct.
    /*!< this means that it is either shutting down or soon will be. */

  void set_defunct();
    //!< used by the derived class to mark that this object is about to exit.

  bool setup(application_instance handle, const basis::astring &application_name,
          int timing_cycle = 0);
    //!< constructs a process_anchor for the "application_name" specified.
    /*!< the instance "handle" is the link to the application.  the anchor is
    created (and made hidden on windows) if successful.  the standard here for the
    "application_name" is that it should not have any directory components,
    but it should be the full program name (including extension).  if
    "timing_cycle" is non-zero, then it is used as a timer interval.  every
    time the interval elapses, the handle_timer() method is invoked.
    note that this function is invoked by launch() and does not need to be
    called by a user unless launch() is not being used to control the anchor's
    lifetime. */

  void register_anchor(window_handle anchor);
    //!< this supports the anchor being created elsewhere.
    /*!< when the object is created independently but should still function
    as an process_anchor, then this method can be used to hook in that
    external "window" into our support.  this only makes sense for the
    windows version. */

  static bool launch(process_anchor &anchor, application_instance handle,
          const basis::astring &app, int cycle = 0);
    //!< establishes a process_anchor for the program named "app".
    /*!< on windows, this function will not return until WM_CLOSE is received; it is the
    main message loop for a simple application.  the "handle" parameter should
    be coming from the main function for the program.  note that the "window"
    should be constructed but not have any functions called on it yet.  this
    is important since this function does all the setup.  using a "window"
    derived from process_anchor is also okay.  the "cycle" is passed to the
    setup() method. */

  virtual void handle_startup();
    //!< derived classes can override this to catch the application startup.
    /*!< this function is guaranteed to be called after the event processing
    loop has started, but before much of anything else is done in the
    application. */

  virtual void handle_timer();
    //!< invoked periodically if the anchor was setup() with a timer "cycle".

  virtual void handle_shutdown();
    //!< invoked just prior to the shutdown of this anchor.

  static basis::astring make_well_known_title(const basis::astring &application_name);
    //!< returns the string form of the well-known window title for the process_anchor.
    /*!< this title identifies the "application_name" on ms-windows and is used for our
    hidden window.  this is how the window can be found at runtime.
    note that this approach will only work properly if there is only one
    window by that name on a host at a time (due to how the windows are
    registered). */
  static basis::astring make_well_known_class(const basis::astring &application_name);
    //!< same as above but for the anchor's class name.

private:
  application_instance _instance;  //!< the current instance in win32.
  basis::astring *_anchor_title;  //!< the text for the title bar.
  basis::astring *_anchor_class;  //!< the name for internal registration.
  ATOM _class_reg;  //!< non-zero if the class has been registered.
  window_handle _wind_handle;  //!< the handle for this window's object.
  int _cycle;  //!< timer interval spacing out periodic activity.
  bool _defunct;  //!< is the object shutting down?

  ATOM register_class();
    //!< sets up the window's class entry for hooking into ms-windows events.

#ifdef __WIN32__
  static LRESULT CALLBACK WndProc(HWND wind, UINT msg, WPARAM wp, LPARAM lp);
    //!< the window procedure that handles all events.
#endif

  // not appropriate.
  process_anchor(const process_anchor &);
  process_anchor &operator =(const process_anchor &);
};

#endif // outer guard.

