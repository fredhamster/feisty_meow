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

#include "process_anchor.h"

///#include <application/windoze_helper.h>
#include <basis/array.h>
#include <basis/definitions.h>
#include <basis/utf_conversion.h>
#include <basis/astring.h>
#include <filesystem/filename.h>
#include <loggers/program_wide_logger.h>
#include <structures/static_memory_gremlin.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace structures;
//using namespace timely;

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

#ifdef __WIN32__
const int STARTUP_MESSAGE = WM_USER + 123;
  // the special message we send out on startup of the event loop.

const int TIMER_EVENT_ID = 23;
  // the ID we use to create a timer, if the user wants one.

// an anchor_association tracks the object related to a window handle.
struct anchor_association {
  window_handle _handle;
  process_anchor *_anchor_object;

  anchor_association(window_handle wh = NULL_POINTER, process_anchor *wo = NULL_POINTER)
      : _handle(wh), _anchor_object(wo) {}
};

class anchor_association_list
: public array<anchor_association>, public virtual root_object
{
public:
  DEFINE_CLASS_NAME("anchor_association_list");
};

// the associations list keeps track of anchors that have been created so
// that the window procedure (on win32) can get to the real object.

SAFE_STATIC(anchor_association_list, associations, )
#endif //win32

//////////////

process_anchor::process_anchor()
#ifdef __LINUX__
: shutdown_alerter()
#endif
#ifdef __WIN32__
: _instance(NULL_POINTER),
  _anchor_title(new astring),
  _anchor_class(new astring),
  _class_reg(NULL_POINTER),
  _wind_handle(NULL_POINTER),
  _cycle(0),
  _defunct(false)
#endif
{
}

process_anchor::~process_anchor()
{
  set_defunct();
#ifdef __WIN32__
  WHACK(_anchor_title);
  WHACK(_anchor_class);
  _class_reg = NULL_POINTER;
  // remove the association for our anchor.
  for (int i = 0; i < associations().length(); i++) {
    if (associations()[i]._handle == _wind_handle) {
      associations().zap(i, i);
      break;
    }
  }
#endif
}

void process_anchor::handle_startup() { /* nothing for base class. */ }

void process_anchor::handle_timer() { /* nothing for base class. */ }

void process_anchor::handle_shutdown() { /* nothing for base class. */ }

bool process_anchor::close_this_program()
{
#ifdef __LINUX__
  shutdown_alerter::close_this_program();
  return true;
#endif
#ifdef __WIN32__
  bool to_return = false;
  for (int i = 0; i < associations().length(); i++) {
    window_handle win = associations()[i]._handle;
    int ret = PostMessage(win, WM_CLOSE, NULL_POINTER, NULL_POINTER);
    // if we got a healthy return from any post, then we'll say this worked.
    if (ret) to_return = true;
  }
  return to_return;
#endif
}

bool process_anchor::defunct() const
{
#ifdef __LINUX__
  return shutdown_alerter::is_defunct();
#endif
#ifdef __WIN32__
  return _defunct;
#endif
}

void process_anchor::set_defunct()
{
#ifdef __LINUX__
  return shutdown_alerter::set_defunct();
#endif
#ifdef __WIN32__
  _defunct = true;
#endif
}

bool process_anchor::close_app_anchor(const astring &app_name)
{
  FUNCDEF("close_app_anchor");
#ifdef __LINUX__
  return shutdown_alerter::close_application(app_name);
#endif
#ifdef __WIN32__
  astring title = process_anchor::make_well_known_title(app_name);
#ifdef DEBUG_PROCESS_MANAGER
  LOG(astring("title is: ") + title);
#endif

//hmmm: add support for linux...
#ifdef __WIN32__
  window_handle win_found = FindWindow(NULL_POINTER, to_unicode_temp(title));
#ifdef DEBUG_PROCESS_MANAGER
  LOG(astring(astring::SPRINTF, "found window %lx", win_found));
#endif
  if (!win_found) {
    LOG(astring("Failed to find special window for [") + app_name
      + astring("]"));
    return false;
  }

//hmmm:
//why would we find only one window if there were multiple apps under that
//name?  how does windows deal with that?  is there a find window iterator?

  int ret = PostMessage(win_found, WM_CLOSE, NULL_POINTER, NULL_POINTER);
  if (!ret) {
    LOG(astring("Failed to send close message to [") + app_name
        + astring("]"));
    return false;
  }

  LOG(astring("Sent close message to [") + app_name + astring("]"));
#else
  #ifdef DEBUG_PROCESS_MANAGER
    LOG("graceful shutdown not implemented for this OS.");
  #endif
  return false;
#endif
  return true;
#endif //win32
}

bool process_anchor::setup(application_instance instance,
    const astring &app_name, int cycle)
{
#ifdef __LINUX__
  if (instance) {}
  return shutdown_alerter::setup(app_name, cycle);
#endif
#ifdef __WIN32__
  if (_wind_handle) return true;  // already initialized.
  // simple initializations first...
  _instance = instance;
  _cycle = cycle;
  *_anchor_title = make_well_known_title(app_name);
  *_anchor_class = make_well_known_class(app_name);
  _class_reg = register_class();  // register a new window class for this.
  _wind_handle = CreateWindow(to_unicode_temp(*_anchor_class),
      to_unicode_temp(*_anchor_title), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
      0, CW_USEDEFAULT, 0, NULL_POINTER, NULL_POINTER, _instance, NULL_POINTER);
  if (!_wind_handle) return false;

  register_anchor(_wind_handle);  // hook in our anchor to the list.
  ShowWindow(_wind_handle, SW_HIDE);
  UpdateWindow(_wind_handle);
  PostMessage(_wind_handle, STARTUP_MESSAGE, 0, 0);
#endif  //win32
  return true;
}

void process_anchor::register_anchor(window_handle wind)
{
#ifdef __WIN32__
  // add the new anchor to our list of associations.
  associations() += anchor_association(wind, this);
#else
  if (wind) {}
#endif
}

#ifdef __WIN32__
ATOM process_anchor::register_class()
{
  WNDCLASSEX wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = (WNDPROC)WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = _instance;
  wcex.hIcon = NULL_POINTER;
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  wcex.lpszMenuName = NULL_POINTER;
  to_unicode_persist(hold_class, _anchor_class->s());
  wcex.lpszClassName = hold_class;
  wcex.hIconSm = NULL_POINTER;

  return RegisterClassEx(&wcex);
}
#endif

#ifdef __WIN32__
LRESULT CALLBACK process_anchor::WndProc(HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    case STARTUP_MESSAGE: {
      for (int i = 0; i < associations().length(); i++) {
        if (associations()[i]._handle == hWnd) {
          process_anchor &anch = *associations()[i]._anchor_object;
          // invoke the initial callback so the anchor can initialize.
          anch.handle_startup();

          // set a timer going if they wanted one.
          if (anch._cycle)
            SetTimer(anch._wind_handle, TIMER_EVENT_ID, anch._cycle, 0);
          break;
        }
      }
      break;
    }
    case WM_CLOSE: {
      for (int i = 0; i < associations().length(); i++) {
        if (associations()[i]._handle == hWnd) {
          // invoke the closing callback because we're leaving.
          associations()[i]._anchor_object->handle_shutdown();
          associations()[i]._anchor_object->set_defunct();
          break;
        }
      }
      return DefWindowProc(hWnd, message, wParam, lParam);
      break;
    }
    case WM_DESTROY: {
      PostQuitMessage(0);
      break;
    }
    case WM_TIMER: {
      bool found_window = false;  // records if we dispatched the event.
      for (int i = 0; i < associations().length(); i++)
        if (associations()[i]._handle == hWnd) {
          // always stop the timer since we're suspending time while we're busy.
          // more of a desire than a strategy.
          KillTimer(hWnd, TIMER_EVENT_ID);
          // invoke the timer callback to give the user some action.
          associations()[i]._anchor_object->handle_timer();
          found_window = true;
          // always set the timer going again after handling it.
          SetTimer(hWnd, TIMER_EVENT_ID,
              associations()[i]._anchor_object->_cycle, 0);
          break;
        }
      bool to_return = 0;
      // if the timer wasn't for one of our anchors, we pass it to the default
      // window procedure in hopes it will know what the hell to do with it.
      if (!found_anchor)
        to_return = DefWindowProc(hWnd, message, wParam, lParam);
      return to_return;
    }
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hWnd, &ps);
      // add drawing code here if needed.
      EndPaint(hWnd, &ps);
      break;
    }
    default: return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}
#endif

#ifdef __WIN32__
astring process_anchor::make_well_known_title(const astring &application_name)
{
  filename app_short = application_name;
  return astring("Anchor_for_") + app_short.basename();
}
#endif

#ifdef __WIN32__
astring process_anchor::make_well_known_class(const astring &application_name)
{
  filename app_short = application_name;
  return astring("Dozeclass_for_") + app_short.basename();
}
#endif

bool process_anchor::launch(process_anchor &win, application_instance handle,
    const astring &application_name, int cycle)
{
  // prepare the anchor for its role...
  if (!win.setup(handle, application_name, cycle)) return false;
#ifdef __LINUX__
  return shutdown_alerter::launch_console(win, application_name, cycle);
#endif
#ifdef __WIN32__
  MSG msg;
  msg.hwnd = 0; msg.message = 0; msg.wParam = 0; msg.lParam = 0;
  while (GetMessage(&msg, NULL_POINTER, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
#endif
  return true;
}




