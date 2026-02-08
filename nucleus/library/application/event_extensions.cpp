#ifndef EVENT_EXTENSIONS_IMPLEMENTATION_FILE
#define EVENT_EXTENSIONS_IMPLEMENTATION_FILE

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

#ifdef __WIN32__

#include "event_extensions.h"

#include <basis/guards.h>
#include <basis/portable.h>
#include <mechanisms/time_stamp.h>

#include <mmsystem.h>

bool event_extensions::poll()
{
  MSG message;
  return poll(message);
}

bool event_extensions::poll(MSG &message)
{
  return windoze_helper::event_poll(message);
/*
  message.hwnd = 0;
  message.message = 0;
  message.wParam = 0;
  message.lParam = 0;
  if (PeekMessage(&message, NIL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
  return true;
*/
}

bool event_extensions::poll(int wait)
{
  time_stamp start_time;
  do {
    MSG message;
    if (!poll(message)) return false;
  } while (wait && (time_stamp().value() - start_time.value() <= wait));
  return true;
}

bool event_extensions::poll_on_message(window_handle handle, UINT look_for,
    int wait, MSG &message)
{
  time_stamp start_time;
  do {
    bool okay = poll(message);
    if (!okay) return false;
    if ( (message.hwnd == handle) && (message.message == look_for) )
      return true;
  } while (time_stamp().value() - start_time.value() <= wait);
    // keep going while time is left.
  return false;
}

bool event_extensions::poll_on_message_and_wparam(window_handle handle,
    UINT look_for, WPARAM look_also, int wait, MSG &message)
{
  time_stamp start_time;
  do {
    bool okay = poll(message);
    if (!okay) return false;
    if ( (message.hwnd == handle) && (message.wParam == look_also)
          && (message.message == look_for) )
      return true;
  } while (time_stamp().value() - start_time.value() <= wait);
    // keep going while time is left.
  return false;
}

#endif


#endif //EVENT_EXTENSIONS_IMPLEMENTATION_FILE

