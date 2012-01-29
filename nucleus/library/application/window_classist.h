#ifndef WINDOW_CLASSIST_CLASS
#define WINDOW_CLASSIST_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : window_classist                                                   *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2007-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

//! An add-in file providing window class registration and a window procedure.
/*!
  This file makes it easier to add a very simple window to any console or
  win32 application that might need it (possibly because the app does not
  create any windows itself, but for crazy insane reasons, a window is still
  needed by an external agent, ahem installshield).  It implements a very
  important part of this process, which is setting a window procedure and
  registering a window class.  Sometime in 2005 or 2006, a windows update
  came through that made these formerly optional practices mandatory (and
  broke many of our applications that created windows without a window
  procedure or class registration).  That occurrence prompted the creation
  of this class which tries to provide the bare minimum needed to make
  things work again.

  Example Usage:

  // include our code file to embed the window procedure and register class
  // methods in whoever needs them.  this should only be needed once per
  // program.

  #include <application/windows_classist.h>

  // create our simple window...

  basis::astring window_title = "my_freaky_window";
  basis::astring class_name = "jumbo_stompy_update_crudburger";

  window_handle f_window = create_simplistic_window(window_title, class_name);

  // and then much later, after the window is no longer needed...

  whack_simplistic_window(f_window);

*/

#include "windoze_helper.h"

#include <basis/utf_conversion.h>

namespace application {

#ifndef __WIN32__

// this is a placeholder implementation for other platforms.
window_handle create_simplistic_window(const basis::astring &formal(window_title),
    const basis::astring &formal(class_name)) { return NIL; }
void whack_simplistic_window(window_handle formal(f_window)) {}

#else

//! this is the very simple window procedure used by register_class() below.

LRESULT CALLBACK window_procedure(HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam)
{

  switch (message) {
    case WM_COMMAND: {
      int identifier, event;
      identifier = LOWORD(wParam);
      event = HIWORD(wParam);
      return DefWindowProc(hWnd, message, wParam, lParam);
      break;
    }
    case WM_PAINT: {
      HDC hdc;
      PAINTSTRUCT ps;
      hdc = BeginPaint(hWnd, &ps);
      // hmmm: Add any drawing code here...
      EndPaint(hWnd, &ps);
      break;
    }
    case WM_DESTROY: {
      PostQuitMessage(0);
      break;
    }
    default: {
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
  }
  return 0;
}

//! returns the registered class as a windows atom.

ATOM register_class(const basis::astring &name)
{
  WNDCLASSEX wcex;
  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = (WNDPROC)window_procedure;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = GET_INSTANCE_HANDLE();
  wcex.hIcon = 0;
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  wcex.lpszMenuName = 0;
  basis::to_unicode_persist(temp_class, name);
  wcex.lpszClassName = temp_class;
  wcex.hIconSm = 0;

  return RegisterClassEx(&wcex);
}

window_handle create_simplistic_window(const basis::astring &window_title,
    const basis::astring &class_name)
{
  register_class(class_name);
  window_handle f_window = CreateWindow(basis::to_unicode_temp(class_name), 
      basis::to_unicode_temp(window_title), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
      0, CW_USEDEFAULT, 0, NIL, NIL, GET_INSTANCE_HANDLE(), NIL);
  ShowWindow(f_window, SW_HIDE);
  UpdateWindow(f_window);
  return f_window;
}

void whack_simplistic_window(window_handle f_window)
{
  SendMessage(f_window, WM_CLOSE, NIL, NIL);
//hmmm: is this enough?
}

#endif // win32

} // namespace.

#endif // outer guard

