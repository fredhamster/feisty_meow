#ifndef SHUTDOWN_ALERTER_CLASS
#define SHUTDOWN_ALERTER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : shutdown_alerter                                                  *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2003-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <basis/definitions.h>
#include <timely/timer_driver.h>

// forward.
//class basis::astring_object;

//! A platform-independent way to alert a program that it should shut down immediately.
/*!
  This can provide a service management feature for graceful shutdown of an
  application, allowing it a chance to close its objects cleanly, rather than
  just being whacked in the middle of whatever it's doing.
    Only one of these objects should be instantiated per program, but the
  static methods can be used from anywhere in the program.
*/
class shutdown_alerter : public virtual basis::root_object, public timely::timeable
{
public:
  shutdown_alerter();
    //!< constructor does very little; setup() is what begins operation.

  virtual ~shutdown_alerter();

  DEFINE_CLASS_NAME("shutdown_alerter");

  bool setup(const basis::astring &app_name, int timer_period = 0);
    //!< constructs a shutdown_alerter for the "app_name" specified.
    /*!< this can be any string, although it might be processed for certain
    operating systems.  also, for close_this_program() to work properly, it
    must be the application's basename.  the "timer_period" specifies how
    frequently to invoke the handle_timer() method during runtime. */

  static bool is_defunct() { return _defunct(); }
    //!< returns true if the object has been marked as defunct.
    /*!< this means that it is either shutting down or soon will be. */

  static void set_defunct();
    //!< used by the derived class to mark that this object is about to exit.
    /*!< note that this can be used anywhere in the program to initiate an
    exit of the program. */

  inline bool saw_interrupt() { return _saw_interrupt(); }
    //!< reports whether the process saw an interrupt from the user.

  // these virtual methods can be overridden by applications derived from the
  // shutdown_alerter.  they support a graceful shutdown process by which
  // applications can be alerted that they must shutdown, allowing them to take
  // care of releasing resources beforehand.

  virtual void handle_startup();
    //!< this function is called once the program has begun operation.

  virtual void handle_shutdown();
    //!< called during the program's shutdown process.
    /*!< this is invoked just prior to the destruction of this class which is
    also just before the shutdown of the program overall.  in this method,
    the derived object must ensure that any threads the program started get
    stopped, that any opened files get closed, and that any other resources
    are released.  this is the application's last chance to clean up. */

  virtual void handle_timer();
    //!< called periodically if a timer period was specified.

  // static methods that can be used by the program for starting up or for
  // graceful shutdown.

  static bool launch_console(shutdown_alerter &alert, const basis::astring &app_name,
          int timer_period = 0);
    //!< this is used to begin execution of a console mode application.
    /*!< this method does not do anything except sit while the extant threads
    are in play.  it will not return until the program must exit, as caused
    by close_this_program() or close_application(). */

#if 0  //not implemented.
#ifdef __WIN32__
  static bool launch_event_loop(shutdown_alerter &alert,
          const basis::astring &app_name, int timer_period = 0);
    //!< launches by starting up a windowing event loop.
    /*!< this is appropriate for programs that are windowed and must
    continually process window events. */
#endif
#endif

  static void close_this_program();
    //!< causes this particular application to begin shutting down.
    /*!< this is a static method available for programs that support the
    shutdown_alerter's graceful shutdown process.  it causes the application
    to begin the shutdown. */

  static bool close_application(const basis::astring &app_name);
    //!< attempts to close the application named "app_name".
    /*!< this can only be done if this program possesses sufficient rights to
    zap that program. */

  // internal methods not to be used by outside objects.

  static void handle_OS_signal(int sig_id);
    //!< processes the signal from the OS when its time to shut down.

private:
  static bool &_saw_interrupt();  //!< did we see a break from the user?
  static basis::astring &_app_name();  //!< the of this application.
  static bool &_defunct();  //!< is the program shutting down?
  static int &_timer_period();  //!< rate at which timer goes off.

  virtual void handle_timer_callback();
    //!< invoked by the timer driver.

  // not appropriate.
  shutdown_alerter(const shutdown_alerter &);
  shutdown_alerter &operator =(const shutdown_alerter &);
};

#endif // outer guard.

