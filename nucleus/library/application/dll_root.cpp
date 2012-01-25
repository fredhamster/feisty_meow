/*****************************************************************************\
*                                                                             *
*  Name   : DLL Main Root Support                                             *
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

// Thanks to Andy Tan for some research into MFC extension dlls.

#include <application/base_application.h>
#include <basis/utf_conversion.h>
#include <structures/static_memory_gremlin.h>

//HOOPLE_STARTUP_CODE_DLL;
  // initialize objects needed by the hoople libs.

#ifdef _AFXDLL

#ifdef DEBUG
  #define TRACE_PRINTER(s) TRACE_PRINT(s)
#else
  #define TRACE_PRINTER(s) 
#endif

// base for AFX dlls.

#include <afxext.h>  // MFC extensions.
#include <afxdllx.h>  // Extension DLL declarations; include only once!

bool check_DLL_versions();
  // supplied by the library's version of dllmain.cpp.

static AFX_EXTENSION_MODULE SomeDLL = { NULL, NULL };

#ifndef DLL_NAME
  #define DLL_NAME "unnamed DLL"
#endif

extern "C" int APIENTRY
DllMain(application_instance instance, DWORD reason, LPVOID reserved)
{
  SET_INSTANCE_HANDLE(instance);
  // Remove this if you use lpReserved.
  UNREFERENCED_PARAMETER(reserved);

  char *dll_name = DLL_NAME;
    // mainly for debugging purposes.  having the value for DLL_NAME actually
    // stored should allow us to know which dll is being debugged.

  static CDynLinkLibrary *dll_link = NIL;

  static int dll_entry_count = 0;

  switch (reason) {
    case DLL_PROCESS_ATTACH: {
      char *message = DLL_NAME " Initializing!\n";
      TRACE_PRINTER(message);

      if (!check_DLL_versions()) return 0;
    
      // Extension DLL one-time initialization
      if (!AfxInitExtensionModule(SomeDLL, instance)) return 0;

      // Insert this DLL into the resource chain.
      dll_link = new CDynLinkLibrary(SomeDLL);

      // NOTE: If this Extension DLL is being implicitly linked to by an MFC
      // Regular DLL (such as an ActiveX Control) instead of an MFC
      // application, then you will want to remove this line from DllMain and
      // put it in a separate function exported from this Extension DLL.  The
      // Regular DLL that uses this Extension DLL should then explicitly call
      // that function to initialize this Extension DLL.  Otherwise, the
      // CDynLinkLibrary object will not be attached to the Regular DLL's
      // resource chain, and serious problems will result.
      ++dll_entry_count;
      break;
    }
    case DLL_PROCESS_DETACH: {
      --dll_entry_count;
      char *message = DLL_NAME " Terminating!\n";
      TRACE_PRINTER(message);
      // clean up our other stuff.
      WHACK(dll_link);
      // Terminate the library before destructors are called.
      AfxTermExtensionModule(SomeDLL);
      break;
    }
    case DLL_THREAD_ATTACH:
        ++dll_entry_count;
        break;
    case DLL_THREAD_DETACH:
        --dll_entry_count;
        break;
    default:
// do nothing.
      break;
  }

  return 1;
}

#elif defined(__WIN32__)

// regular dll base.

#include <application/windoze_helper.h>  // base windows stuff.

bool check_DLL_versions();
  // supplied by the library's version of dllmain.cpp.

BOOL APIENTRY DllMain(HANDLE module, DWORD ul_reason_for_call, LPVOID reserved)
{
  SET_INSTANCE_HANDLE((application_instance)module);
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
      if (!check_DLL_versions()) return 0;
      break;
    
    // these are currently not processed.
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return true;
}

#endif

