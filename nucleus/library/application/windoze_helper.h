#ifndef WINDOZE_HELPER_GROUP
#define WINDOZE_HELPER_GROUP

/*
*  Name   : windoze_helper definitions
*  Author : Chris Koeritz
* Copyright (c) 1994-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

//! @file windoze_helper.h Aids in achievement of platform independence.
/*! @file windoze_helper.h
  These definitions, inclusions and types are aimed at allowing our source
  code to remain independent of the underlying windowing and operating
  systems.  Specifically, windows puts a lot of burden on the developer,
  and this file exists to hide all that malarkey.
*/

// gnarly headers that are needed for certain types of compilation...

//unix headers not needed in here for new purpose of file.
#ifndef _MSC_VER
  #include <unistd.h>
  #ifdef __GNU_WINDOWS__
    #include <sys/unistd.h>
  #endif
#endif
#ifndef NO_XWINDOWS
  #ifdef __XWINDOWS__
    #include <Intrinsic.h>
    #include <StringDefs.h>
    #include <Xm/Xm.h>
    #include <Xlib.h>
  #endif
#endif
#ifdef __WIN32__
  #ifndef STRICT
    #define STRICT
  #endif
  // winsock support...
//  #undef FD_SETSIZE
//  #define FD_SETSIZE 1000
    // if you don't set this, you can only select on a default of 64 sockets.
//  #include <winsock2.h>

  // windows headers...
//noooo  #define _WINSOCKAPI_  // make windows.h happy about winsock.
  #ifndef _AFXDLL
    // include ms-windows headers only if we're not doing mfc; mfc has its own
    // special way of including the headers.
    #include <windows.h>
  #else
    #include <afx.h>
    #include <afxwin.h>
  #endif
#endif

// forward.
//class version;

// wrapper classes defined in the sequel...
//
//   application_instance -- this program's application object; used mainly
//       in ms-windows.
//
//   window_handle -- a wrapper for the root of all objects that can be
//       referred to in the relevant windowing system.

#include <basis/definitions.h>

#ifdef __UNIX__
  // the application_instance class is implemented very simply for unix;
  // it's a stand-in since unix apps don't refer to an instance handle like
  // ms-windows does.
  typedef void *application_instance;

  // some definitions to quiet complaints from win32-based code.
  #ifndef LONGINT_SIZE
    #if defined(__alpha) || (defined(__HOS_AIX__) && defined(_LP64)) \
        || defined(__sparcv9) || defined(__LP64__) 
      #define LONGINT_SIZE 8
    #else
      #define LONGINT_SIZE 4
    #endif
  #endif
  #if (LONGINT_SIZE == 4)
    // be compatible with linux's definitions on this subject.
    typedef unsigned long DWORD;
  #else
    typedef unsigned int DWORD;
  #endif

  typedef void *HANDLE;

  //temp; these just mimic function prototypes coming from non-portable code.
  typedef void *ATOM;
  typedef void *BITMAPINFO;
  typedef void *HBITMAP;
  typedef void *HDC;
  typedef void *RGBQUAD;
  #define LoadBitmap(a, b) (a)
  #define __stdcall 
  #define afx_msg

  // windoze_helper definitions for the X windowing system.
  #ifndef NO_XWINDOWS  // protects regions with similar names.
  #ifdef __XWINDOWS__
    typedef Widget window_handle;
      //!< main way to access a windowing system object.  should be a pointer.

    typedef Colormap window_colormap;
      //!< the windowing system representation of a pallette or colormap.

    typedef XColor window_color;
      //!< the system representation of color.

    typedef XmString window_string;
      //!< a special windowing system dependent kind of character string.

//is that really fixed?
    const int MAXIMUM_COLOR_INTENSITY = 65535;
      // largest valid value a color component (R, G or B) can have.
  #else
    // no x-windows.
    typedef void *window_handle;
  #endif
  #endif
#endif

#ifdef __WIN32__
  typedef HINSTANCE application_instance;
    // our moniker for an application's guts.

  typedef HWND window_handle;
    // our alias for window handles in win32.
#endif

namespace application {

//  istring module_name(const void *module_handle = NULL_POINTER);
    //!< returns the name of the module for the "module_handle" where supported.
    /*!< if the handle is NULL_POINTER, then the program name is returned. */


//  u_int system_error();
    //!< gets the most recent system error reported on this thread.

//  istring system_error_text(u_int error_to_show);
    //!< returns the OS's string form of the "error_to_show".
    /*!< this often comes from the value reported by system_error(). */

//  istring null_device();
    //!< returns the name of the system's NULL device.
    /*!< this device is a black hole where output can be sent, never to be
    seen again.  this is /dev/null on unix and null: on win32. */

//  timeval fill_timeval_ms(int milliseconds);
    //!< returns a timeval system object that represents the "milliseconds".
    /*!< if "milliseconds" is zero, then the returned timeval will
    represent zero time passing (rather than infinite duration as some
    functions assume). */

  // this only really helps for win32.
  extern application_instance _i_handle;
    //!< dll_root.cpp defines this for all dlls.
  #define DEFINE_INSTANCE_HANDLE application_instance application::_i_handle = 0
    //!< some applications may need this to use rc_string and others.
    /*!< if this macro is used, then the code is impervious to future
    changes in how the instance handle works. */
  #define SET_INSTANCE_HANDLE(value) application::_i_handle = value
    //!< use this to change the instance handle for this dll or exe.
    /*!< note that this should be done only once and by the main thread. */
  #define GET_INSTANCE_HANDLE() application::_i_handle
    //!< a handy macro that frees one from knowing the name of the handle.

////////////////////////////////////////////////////////////////////////////

  // Unix and Linux specific items are included here.
  #ifdef __UNIX__
////    istring get_cmdline_from_proc();
      //!< returns the command line that /proc has recorded for our process.

//    char *itoa(int to_convert, char *buffer, int radix);
      //!< turns the integer "to_convert" into a string stored in the "buffer".
      /*!< this is needed on linux since there doesn't seem to be a builtin
      version of it.  the buffer must be long enough to hold the number's
      textual representation!  the buffer's length must also account for the
      chosen "radix" (the base for the number system, where 10 is decimal,
      16 is hexadecimal, etc). */

    #define RGB(r, g, b) (b + (g << 8) + (r << 16))
      //!< no idea if that's even approximately right.
  #endif

  // ms-windows of more modern types, i.e. win32.
  #ifdef __WIN32__

//    bool event_poll(MSG &message);
      //!< tries to process one win32 event and retrieve the "message" from it.
      /*!< this is a very general poll and will retrieve any message that's
      available for the current thread.  the message is actually processed
      here also, by calling translate and dispatch.  the returned structure
      is mainly interesting for knowing what was done. */

//hmmm: is there an equivalent to this for unix?
//    bool is_address_valid(const void *address, int size_expected,
//            bool writable = false);
      //!< checks that the address specified is a valid pointer.
      /*! also tests that the address's allocated space has the
      "size_expected".  if "writable" is true, then the pointer is
      checked for being writable as well as readable. */

    #define BROADCAST_HANDLE HWND_BROADCAST

    enum known_operating_systems {
      WIN_95, WIN_NT, WIN_2K, WIN_XP, WIN_SRV2K3, WIN_VISTA, 
///WIN_SRV2K8,
      WIN_7, WIN_8, WIN_10,
      UNKNOWN_OS
    };
    const char *opsystem_name(known_operating_systems which);
      //!< returns the textual form of the known_operating_systems enum.

    known_operating_systems determine_OS();
      //!< returns the operating system that seems to be running currently.
      /*!< note that WIN_95 also covers windows 98 and windows ME. */

//    istring rc_string(u_int id, application_instance instance);
      //!< acquires a string from a particular "instance".
      /*!< the "id" is the resource identifier for the desired string in
      that "instance". */

//    istring rc_string(u_int id);
      //!< simplified from above function by not needing an "instance".
      /*!< the "current" dynamic library or executable's resources are sought
      for the "id". */

//    void show_wait_cursor();
      //!< changes the cursor to the "wait" look, which is usually an hourglass.
//    void show_normal_cursor();
      //!< changes the cursor to "normal", which is usually a pointing arrow.

  #endif // win32.

} // namespace.

#endif // outer guard.

