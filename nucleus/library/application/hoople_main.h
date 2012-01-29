#ifndef HOOPLE_MAIN_GROUP
#define HOOPLE_MAIN_GROUP

/*****************************************************************************\
*                                                                             *
*  Name   : HOOPLE_MAIN group
*  Author : Chris Koeritz
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

//! @file "hoople_main.h" Provides macros that implement the 'main' program of an application.

#include "application_shell.h"
#include "command_line.h"
#include "windoze_helper.h"

#include <basis/contracts.h>
#include <loggers/critical_events.h>
#include <loggers/combo_logger.h>
#include <structures/static_memory_gremlin.h>
#include <loggers/program_wide_logger.h>

namespace application {

// The following versions of main programs are provided for different operating
// systems and compilation environments.  These support the requirements of the
// HOOPLE startup code and program-wide features, assuming an object derived
// from base_application is available.
// The object derived from base_application must be provided in "obj_name".
// The "obj_args" are any arguments that need to be passed to the object's
// constructor; this list can be empty.
// Note that the default logger used for unix and console mode win32 programs
// is the console logger; that can be changed in the startup code for the
// "obj_name".

#define HOOPLE_STARTUP_CODE \
  DEFINE_INSTANCE_HANDLE;

#ifdef __WXWIDGETS__
  //! main program for applications using WxWidgets library.
  #define HOOPLE_MAIN(obj_name, obj_args) \
    HOOPLE_STARTUP_CODE; \
    int main(int argc, char *argv[]) { \
      SET_ARGC_ARGV(argc, argv); \
      SETUP_COMBO_LOGGER; \
      obj_name to_run_obj obj_args; \
      return to_run_obj.execute_application(); \
    }

//////////////

#elif defined(__UNIX__)
  //! options that should work for most unix and linux apps.
  #define HOOPLE_MAIN(obj_name, obj_args) \
    HOOPLE_STARTUP_CODE; \
    int main(int argc, char *argv[]) { \
      SET_ARGC_ARGV(argc, argv); \
      SETUP_COMBO_LOGGER; \
      obj_name to_run_obj obj_args; \
      return to_run_obj.execute_application(); \
    }

//////////////

#elif defined(__WIN32__)
  // for win32 we need to support four different environments--console mode,
  // borland compilation, MFC programs and regular windows programs.
  #ifdef _CONSOLE
    //! console mode programs can easily write to a command shell.
    #define HOOPLE_MAIN(obj_name, obj_args) \
      HOOPLE_STARTUP_CODE; \
      int main(int argc, char *argv[]) { \
        SETUP_COMBO_LOGGER; \
        SET_ARGC_ARGV(argc, argv); \
        obj_name to_run_obj obj_args; \
        return to_run_obj.execute_application(); \
      }
  #elif defined(_AFXDLL)
    //! MFC applications generally use a tiny shell which hooks up logging.
    #define HOOPLE_MAIN(obj_name, obj_args)  \
      HOOPLE_STARTUP_CODE; \
      SET_ARGC_ARGV(__argc, __argv); \
      tiny_application<obj_name> theApp;
  #elif defined(__WIN32__)
    //! standard win32 applications have no console, so we just log to a file.
    #define HOOPLE_MAIN(obj_name, obj_args)  \
      HOOPLE_STARTUP_CODE; \
      int WINAPI WinMain(application_instance instance, \
          application_instance prev_instance, LPSTR lpCmdLine, \
          int nCmdShow) { \
        SET_ARGC_ARGV(__argc, __argv); \
        SET_INSTANCE_HANDLE(instance); \
        SETUP_FILE_LOGGER; \
        obj_name to_run_obj obj_args; \
        return to_run_obj.execute_application(); \
      }
  #endif

//////////////

#else  // not __UNIX__ or __WIN32__
  //! just guessing this might work; otherwise we have no idea.
  #define HOOPLE_MAIN(obj_name, obj_args)  \
    HOOPLE_STARTUP_CODE; \
    int main(int argc, char *argv[]) { \
      SETUP_CONSOLE_LOGGER; \
      obj_name to_run_obj obj_args; \
      return to_run_obj.execute_application(); \
    }
#endif

} //namespace.

#endif // outer guard.

