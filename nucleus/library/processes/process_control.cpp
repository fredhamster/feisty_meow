
//NOTE:
//
//  this thing is showing bad behavior on win32 when unicode is enabled.
//  therefore unicode is currently disabled for win32, which is a shame.
//  but something needs to be fixed in our unicode conversion stuff; the unicode versions
//  of the file names were not getting correctly back-converted into the ascii counterpart,
//  and *that* is broken.
//  ** this may be a widespread issue in the win32 code related to unicode right now!
//  ** needs further investigation when a reason to better support ms-windows emerges.
//
//  => but don't panic...  right now things work as long as you do a utf-8 / ascii build,
//     which is how everything's configured.


/*
*  Name   : process_control
*  Author : Chris Koeritz
**
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include "process_entry.h"
#include "process_control.h"

#include <application/windoze_helper.h>
#include <basis/astring.h>
#include <basis/utf_conversion.h>
#include <configuration/application_configuration.h>
#include <filesystem/filename.h>
#include <loggers/program_wide_logger.h>
#include <loggers/standard_log_base.h>
#include <mathematics/chaos.h>
#include <structures/set.h>
#include <structures/version_record.h>

#include <stdio.h>
#include <stdlib.h>
#ifdef __UNIX__
  #include <unistd.h>
#endif

using namespace basis;
using namespace configuration;
using namespace filesystem;
using namespace loggers;
using namespace mathematics;
using namespace structures;

namespace processes {

#ifdef __WIN32__
  #include <tlhelp32.h>
  const astring NTVDM_NAME = "ntvdm.exe";
    // the umbrella process that hangs onto 16 bit tasks for NT.
  #ifdef _MSCVER
    #include <vdmdbg.h>
  #endif
#endif
#ifdef __UNIX__
  #include <signal.h>
  #include <stdio.h>
#endif

//#define DEBUG_PROCESS_CONTROL
  // uncomment for noisier debugging.

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

//////////////

class process_implementation_hider
{
public:
#ifdef __WIN32__
  // psapi members:
  application_instance psapi_dll;
  application_instance vdm_dll;
  BOOL (WINAPI *enumerate_processes)(basis::un_int *, basis::un_int cb, basis::un_int *);
  BOOL (WINAPI *enumerate_modules)(HANDLE, HMODULE *, basis::un_int, basis::un_int *);
  basis::un_int (WINAPI *get_module_name)(HANDLE, HMODULE, LPTSTR, basis::un_int);
#ifdef _MSCVER
  INT (WINAPI *tasker_16bit)(basis::un_int, TASKENUMPROCEX  fp, LPARAM);
#endif

  // toolhelp members:
  application_instance kernel32_dll;
  HANDLE (WINAPI *create_snapshot)(basis::un_int,basis::un_int);
  BOOL (WINAPI *first_process)(HANDLE,LPPROCESSENTRY32);
  BOOL (WINAPI *next_process)(HANDLE,LPPROCESSENTRY32);

  // get an atomic view of the process list, which rapidly becomes out of date.
///  HANDLE hSnapShot;

  process_implementation_hider()
    : psapi_dll(NIL), vdm_dll(NIL), enumerate_processes(NIL),
      enumerate_modules(NIL), get_module_name(NIL),
#ifdef _MSCVER
      tasker_16bit(NIL),
#endif
      kernel32_dll(NIL), create_snapshot(NIL), first_process(NIL),
      next_process(NIL) {}

  ~process_implementation_hider() {
    if (psapi_dll) FreeLibrary(psapi_dll);
    if (vdm_dll) FreeLibrary(vdm_dll);
    if (kernel32_dll) FreeLibrary(kernel32_dll);
    psapi_dll = NIL;
    vdm_dll = NIL;
    kernel32_dll = NIL;
  }
#endif
};

//////////////

class process_info_clump
{
public:
  basis::un_int _process_id;
  process_entry_array &_to_fill;  // where to add entries.

  process_info_clump(basis::un_int id, process_entry_array &to_fill)
      : _process_id(id), _to_fill(to_fill) {}
};

//////////////

// top-level functions...

process_control::process_control()
: _ptrs(new process_implementation_hider),
#ifdef __WIN32__
  _use_psapi(true),
#endif
#ifdef __UNIX__
  _rando(new chaos),
#endif
  _healthy(false)
{
  // Check to see if were running under Windows95 or Windows NT.
  version osver = application_configuration::get_OS_version();

#ifdef __WIN32__
  if (osver.v_revision() == VER_PLATFORM_WIN32_WINDOWS) {
    // we're on Windows 95, so use the toolhelp API for the processes.
    _use_psapi = false;
  } else if (osver.v_major() >= 5) {
    // w2k and onward can do the toolhelp instead of psapi.
    _use_psapi = false;
  }
  if (_use_psapi)
    _healthy = initialize_psapi_support();
  else
    _healthy = initialize_toolhelp_support();
#endif
#ifdef __UNIX__
  _healthy = true;
#endif
}

process_control::~process_control()
{
  WHACK(_ptrs);
#ifdef __UNIX__
  WHACK(_rando);
#endif
}

void process_control::sort_by_name(process_entry_array &v)
{
  process_entry temp;
  for (int gap = v.length() / 2; gap > 0; gap /= 2)
    for (int i = gap; i < v.length(); i++)
      for (int j = i - gap; j >= 0
          && (filename(v[j].path()).basename().raw()
             > filename(v[j + gap].path()).basename().raw());
          j = j - gap)
      { temp = v[j]; v[j] = v[j + gap]; v[j + gap] = temp; }
}

void process_control::sort_by_pid(process_entry_array &v)
{
  process_entry temp;
  for (int gap = v.length() / 2; gap > 0; gap /= 2)
    for (int i = gap; i < v.length(); i++)
      for (int j = i - gap; j >= 0 && (v[j]._process_id
          > v[j + gap]._process_id); j = j - gap)
      { temp = v[j]; v[j] = v[j + gap]; v[j + gap] = temp; }
}

bool process_control::query_processes(process_entry_array &to_fill)
{
  if (!_healthy) return false;
#ifdef __WIN32__
  if (!_use_psapi) {
    // we're on Windows 95 or something, so use the toolhelp API for the
    // processes.
    return get_processes_with_toolhelp(to_fill);
  } else {
    // we're on Windows NT and so on; use the process API (PSAPI) to get info.
    return get_processes_with_psapi(to_fill);
  }
#endif
#ifdef __UNIX__
  return get_processes_with_ps(to_fill);
#endif
}

#ifdef __WIN32__
bool process_control::initialize_psapi_support()
{
  // create an instance of the PSAPI dll for querying 32-bit processes and
  // an instance of the VDM dll support just in case there are also some
  // 16-bit processes.
  _ptrs->psapi_dll = LoadLibraryA("psapi.dll");
  if (!_ptrs->psapi_dll) return false; 
  _ptrs->vdm_dll = LoadLibraryA("vdmdbg.dll");
  if (!_ptrs->vdm_dll) return false;

  // locate the functions that we want to call.
  _ptrs->enumerate_processes = (BOOL(WINAPI *)(basis::un_int *,basis::un_int,basis::un_int*))
        GetProcAddress(_ptrs->psapi_dll, "EnumProcesses");
  _ptrs->enumerate_modules
      = (BOOL(WINAPI *)(HANDLE, HMODULE *, basis::un_int, basis::un_int *))
        GetProcAddress(_ptrs->psapi_dll, "EnumProcessModules");
  _ptrs->get_module_name
      = (basis::un_int (WINAPI *)(HANDLE, HMODULE, LPTSTR, basis::un_int))
        GetProcAddress(_ptrs->psapi_dll, "GetModuleFileNameExA");
#ifdef _MSCVER
  _ptrs->tasker_16bit = (INT(WINAPI *)(basis::un_int, TASKENUMPROCEX, LPARAM))
        GetProcAddress(_ptrs->vdm_dll, "VDMEnumTaskWOWEx");
#endif
  if (!_ptrs->enumerate_processes || !_ptrs->enumerate_modules
      || !_ptrs->get_module_name
#ifdef _MSCVER
      || !_ptrs->tasker_16bit
#endif
      ) return false;

  return true;
}

bool process_control::initialize_toolhelp_support()
{
  // get hooked up with the kernel dll so we can use toolhelp functions.
  _ptrs->kernel32_dll = LoadLibraryA("Kernel32.DLL");
  if (!_ptrs->kernel32_dll) return false;

  // create pointers to the functions we want to invoke.
  _ptrs->create_snapshot = (HANDLE(WINAPI *)(basis::un_int,basis::un_int))
        GetProcAddress(_ptrs->kernel32_dll, "CreateToolhelp32Snapshot");
  _ptrs->first_process = (BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32))
        GetProcAddress(_ptrs->kernel32_dll, "Process32First");
  _ptrs->next_process = (BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32))
        GetProcAddress(_ptrs->kernel32_dll, "Process32Next");
  if (!_ptrs->next_process || !_ptrs->first_process
      || !_ptrs->create_snapshot) return false;
  return true;
}

#endif

bool process_control::zap_process(basis::un_int to_zap)
{
#ifdef DEBUG_PROCESS_CONTROL
  FUNCDEF("zap_process");
#endif
  if (!_healthy) return false;
#ifdef __UNIX__
  int ret = kill(to_zap, 9);
    // send the serious take-down signal to the process.
  return !ret;
#endif
#ifdef __WIN32__
  HANDLE h = OpenProcess(PROCESS_TERMINATE, false, to_zap);
  if (!h) {
#ifdef DEBUG_PROCESS_CONTROL
    int err = critical_events::system_error();
    LOG(a_sprintf("error zapping process %d=", to_zap)
        + critical_events::system_error_text(err));
#endif
    return false;
  }
  int exit_code = 0;
  BOOL ret = TerminateProcess(h, exit_code);
  CloseHandle(h);
  return !!ret;
#endif
}

process_entry process_control::query_process(basis::un_int to_query)
{
//  FUNCDEF("query_process");
  process_entry to_return;

  process_entry_array to_fill;
  bool got_em = query_processes(to_fill);
  if (!got_em) return to_return;

  for (int i = 0; i < to_fill.length(); i++) {
    if (to_fill[i]._process_id == to_query)
      return to_fill[i];
  }

//hmmm: implement more specifically.
#ifdef __UNIX__
//put in the single process grabber deal.
#endif
#ifdef __WIN32__
//grab the entry from the list.
#endif

  return to_return;
}

bool process_control::find_process_in_list(const process_entry_array &processes,
    const astring &app_name_in, int_set &pids)
{
#ifdef DEBUG_PROCESS_CONTROL
  FUNCDEF("find_process_in_list");
#endif
  pids.clear();
  astring app_name = app_name_in.lower();

  version os_ver = application_configuration::get_OS_version();

  bool compare_prefix = (os_ver.v_major() == 5) && (os_ver.v_minor() == 0);
    // we only compare the first 15 letters due to a recently noticed bizarre
    // bug where w2k only shows (and reports) the first 15 letters of file
    // names through toolhelp.

  bool found = false;  // was it seen in process list?
  for (int i = 0; i < processes.length(); i++) {
    filename path = processes[i].path();
    astring base = path.basename().raw().lower();
    // a kludge for w2k is needed--otherwise we will miss seeing names that
    // really are running due to the toolhelp api being busted and only
    // reporting the first 15 characters of the name.
    if ( (compare_prefix && (base.compare(app_name, 0, 0, 15, false)))
        || (base == app_name) ) {
      found = true;
      pids.add(processes[i]._process_id);
    }
  }
#ifdef DEBUG_PROCESS_CONTROL
  if (!found)
    LOG(astring("failed to find the program called ") + app_name);
#endif
  return found;
}

//////////////

#ifdef __WIN32__
// this section is the PSAPI version of the query.

// called back on each 16 bit task.
BOOL WINAPI process_16bit(basis::un_int dwThreadId, WORD module_handle16, WORD hTask16,
    PSZ pszModName, PSZ pszFileName, LPARAM lpUserDefined)
{
  process_info_clump *to_stuff = (process_info_clump *)lpUserDefined;
  process_entry to_add;
  to_add._process_id = to_stuff->_process_id;
  to_add._module16 = hTask16;
  to_add.path(pszFileName);
//threads, etc?
  to_stuff->_to_fill += to_add;
  return true;
}

bool process_control::get_processes_with_psapi(process_entry_array &to_fill)
{
  // prepare the result object.
  to_fill.reset();

  // loop over the process enumeration function until we are sure that we
  // have allocated a large enough space for all existing processes.
  bool got_all = false;
  basis::un_int *pid_list = NIL;
  basis::un_int max_size = 428 * sizeof(basis::un_int);
  basis::un_int actual_size = 0;
  while (!got_all) {
    pid_list = (basis::un_int *)HeapAlloc(GetProcessHeap(), 0, max_size);
    if (!pid_list) return false;
    if (!_ptrs->enumerate_processes(pid_list, max_size, &actual_size)) {
      HeapFree(GetProcessHeap(), 0, pid_list);
      return false;
    }
    if (actual_size == max_size) {
      // there were too many to store, so whack the partial list.
      HeapFree(GetProcessHeap(), 0, pid_list);
      max_size *= 2;  // try with twice as much space.
    } else got_all = true;
  }

  // calculate the number of process ids that got stored.
  basis::un_int ids = actual_size / sizeof(basis::un_int);

  // examine each process id that we found.
  for (basis::un_int i = 0; i < ids; i++) {
    // get process information if security permits.
//turn chunk below into "scan process" or something.
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        false, pid_list[i]);
    flexichar process_name[MAX_ABS_PATH + 1] = { '\0' };
    if (hProcess) {
      // go over the modules for the process.  the first will be the main
      // application module and the others will be threads.  ???
      basis::un_int max_size = 1 * sizeof(HMODULE);
//hmmm: could do a rescan loop here if too many.
      basis::un_int actual_size = 0;
      HMODULE *module_handles = new HMODULE[max_size + 1];
      if (!module_handles) {
        CloseHandle(hProcess);
        HeapFree(GetProcessHeap(), 0, pid_list);
        return false;
      }
      if (_ptrs->enumerate_modules(hProcess, module_handles, max_size,
          &actual_size)) {
        // we want the name of the first module.
        if (!_ptrs->get_module_name(hProcess, *module_handles, process_name,
            sizeof(process_name)))
          process_name[0] = 0;
      }
      WHACK(module_handles);
      CloseHandle(hProcess);
    }

    // we add whatever information we were able to find about this process.
    process_entry new_entry;
    new_entry._process_id = pid_list[i];
    astring converted_name = from_unicode_temp(process_name);
    new_entry.path(converted_name);

//how to get?  performance data helper?
///    new_entry._threads = threads;
    to_fill += new_entry;

    // if we're looking at ntvdm, then there might be 16 bit processes
    // attached to it.
    if (new_entry.path().length() >= NTVDM_NAME.length()) {
      astring temp = new_entry.path().substring
          (new_entry.path().end() - NTVDM_NAME.length() + 1,
          new_entry.path().end());
      temp.to_lower();
#ifdef _MSCVER
//hmmm: pull this back in for mingw when it seems to be supported, if ever.
      if (temp == NTVDM_NAME) {
        // set up a callback stampede on the 16 bit processes.
        process_info_clump info(pid_list[i], to_fill);
        _ptrs->tasker_16bit(pid_list[i], (TASKENUMPROCEX)process_16bit,
             (LPARAM)&info);
      }
#endif
    }
  }

  if (pid_list) HeapFree(GetProcessHeap(), 0, pid_list);
  return true;
}

//////////////

// this is the toolhelp version of the query.

bool process_control::get_processes_with_toolhelp(process_entry_array &to_fill)
{
  // prepare the result object.
  to_fill.reset();

  // get an atomic view of the process list, which rapidly becomes out of date.
  HANDLE hSnapShot;
  hSnapShot = _ptrs->create_snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapShot == INVALID_HANDLE_VALUE) return false;

  // start iterating through the snapshot by getting the first process.
  PROCESSENTRY32 entry;
  entry.dwSize = sizeof(PROCESSENTRY32);
  BOOL keep_going = _ptrs->first_process(hSnapShot, &entry);

  // while we see valid processes, iterate through them.
  while (keep_going) {
    // add an entry for the current process.
    process_entry new_entry;
    new_entry._process_id = entry.th32ProcessID;
    new_entry._references = entry.cntUsage;
    new_entry._threads = entry.cntThreads;
    new_entry._parent_process_id = entry.th32ParentProcessID;
    astring exe_file = from_unicode_temp(entry.szExeFile);
    new_entry.path(exe_file);
    to_fill += new_entry;
    entry.dwSize = sizeof(PROCESSENTRY32);  // reset struct size.
    keep_going = _ptrs->next_process(hSnapShot, &entry);
  }

  CloseHandle(hSnapShot);
  return true;
}
#endif  // __WIN32__

#ifdef __UNIX__

#define CLOSE_TEMPORARY_FILE { \
/*  continuable_error("process_control", "get_processes_with_ps", error); */ \
  if (output) { \
    fclose(output); \
    unlink(tmpfile.s()); \
  } \
}

bool process_control::get_processes_with_ps(process_entry_array &to_fill)
{
  FUNCDEF("get_processes_with_ps");
  to_fill.reset();
  // we ask the operating system to give us a list of processes.
  a_sprintf tmpfile("/tmp/proc_list_%d_%d.txt", application_configuration::process_id(),
      _rando->inclusive(1, 400000));
  a_sprintf cmd("ps wax --format \"%%p %%a\" >%s", tmpfile.s());
//hmmm: add more info as we expand the process entry.
  FILE *output = NIL;  // initialize now to establish variable for our macro.
  int sysret = system(cmd.s());
  if (negative(sysret)) {
LOG("got negative return from system()!");
    CLOSE_TEMPORARY_FILE;
    return false;
  }
  output = fopen(tmpfile.s(), "r");
  if (!output) {
LOG("failed to open process list file!");
    CLOSE_TEMPORARY_FILE;
    return false;
  }
  const int max_buff = 10000;
  char buff[max_buff];
  size_t size_read = 1;
  astring accumulator;
  while (size_read > 0) {
    // read bytes from the file.
    size_read = fread(buff, 1, max_buff, output);
    // if there was anything, store it in the string.
    if (size_read > 0)
      accumulator += astring(astring::UNTERMINATED, buff, size_read);
  }
  CLOSE_TEMPORARY_FILE;
  // parse the string up now.
  bool first_line = true;
  while (accumulator.length()) {
    // eat any spaces in front.
    if (first_line) {
      // we toss the first line since it's a header with columns.
      int cr_indy = accumulator.find('\n');
      accumulator.zap(0, cr_indy);
      if (accumulator[accumulator.end()] == '\r')
        accumulator.zap(accumulator.end(), accumulator.end());
      first_line = false;
      continue;
    }
    while (accumulator.length() && (accumulator[0] == ' '))
      accumulator.zap(0, 0);
    // look for the first part of the line; the process id.
    int num_indy = accumulator.find(' ');
    if (negative(num_indy)) break;
    basis::un_int p_id = accumulator.substring(0, num_indy).convert(0);
    accumulator.zap(0, num_indy);
    int cr_indy = accumulator.find('\n');
    if (negative(cr_indy))
      cr_indy = accumulator.end() + 1;
    astring proc_name = accumulator.substring(0, cr_indy - 1);
    if (proc_name[proc_name.end()] == '\r')
      proc_name.zap(proc_name.end(), proc_name.end());
    accumulator.zap(0, cr_indy);
    int space_indy = proc_name.find(' ');
//hmmm: this is incorrect regarding names that do have spaces in them.
    if (negative(space_indy))
      space_indy = proc_name.end() + 1;
    process_entry to_add;
    to_add._process_id = p_id;
    astring path = proc_name.substring(0, space_indy - 1);
    // patch the pathname if we see any bracketed items.
    int brackets_in = 0;
    for (int i = 0; i < path.length(); i++) {
      if (path[i] == '[') brackets_in++;
      else if (path[i] == ']') brackets_in--;
      if (brackets_in) {
        // if we see a slash inside brackets, then we patch it so it doesn't
        // confuse the filename object's directory handling.
        if ( (path[i] == '/') || (path[i] == '\\') )
          path[i] = '#';
      }
    }
    to_add.path(path);
    to_fill += to_add;
  }
  return true;
}
#endif  // __UNIX__

} //namespace.

