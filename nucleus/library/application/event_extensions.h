#ifndef EVENT_EXTENSIONS_CLASS
#define EVENT_EXTENSIONS_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : event_extensions                                                  *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1995-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

//! Provides some useful extensions to event handling in windows.

#ifdef __WIN32__

#include "opsysdll.h"

#include <basis/istring.h>
#include <basis/portable.h>

class OPSYSTEM_CLASS_STYLE event_extensions
{
public:
  static bool poll();
    //!< allows one event to be processed.
    /*!< this will possibly yield control to other programs also.  if an event
    is processed, then true is returned.  if no event was found, then false
    is returned. */

  static bool poll(MSG &message);
    //!< processes one event like poll, but returns what the event was.
    /*!< if no message was processed, then "message" is reset to zeros. */

  static bool poll(int wait);
    //!< polls for events until the "wait" time (in milliseconds) has elapsed.
    /*!< this does not block other windows in the program because events are
    still being handled.  the return value is true if the poll completed
    successfully and false if it had to abort for some reason. */

  static bool poll_on_message(window_handle handle, UINT msg, int wait,
        MSG &found);
    //!< polls awaiting the message "msg" for the window "win".
    /*!< if the message is found within "wait" milliseconds, then it is
    stored in "found" and true is returned.  otherwise, false is returned. */

  static bool poll_on_message_and_wparam(window_handle handle, UINT msg,
        WPARAM wparam, int wait, MSG &found);
    //!< like poll_on_message, but also requires the wParam to equal "wparam".
};

#endif  // win32.

#endif  // outer guard.

