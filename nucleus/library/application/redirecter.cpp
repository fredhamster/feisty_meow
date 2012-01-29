/*****************************************************************************\
*                                                                             *
*  Name   : stdio_redirecter                                                  *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "redirecter.h"

#include <application/windoze_helper.h>
#include <basis/byte_array.h>
#include <basis/utf_conversion.h>
#include <basis/functions.h>
#include <basis/mutex.h>
#include <configuration/application_configuration.h>
#include <configuration/ini_configurator.h>
#include <loggers/program_wide_logger.h>
#include <processes/ethread.h>
#include <processes/launch_process.h>
#include <textual/byte_formatter.h>

#include <stdlib.h>
#ifdef __UNIX__
  #include <unistd.h>
  #include <sys/wait.h>
#endif

using namespace application;
using namespace basis;
using namespace configuration;
using namespace loggers;
using namespace processes;
using namespace textual;

namespace application {

const int IO_PAUSE_PERIOD = 50;  // sleep for this long between read attempts.

const int BUFFER_SIZE = 4096;  // maximum we will read at once.

#undef LOG
#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger::get(), to_print)

const char *REDIRECTER_INI = "redirecter.ini";
  // used to report process ids, since there are users that need this
  // info currently.

const char *PROCESS_SECTION = "process_id";
  // the section in the ini file where we store our process ids.
//hmmm: above should be removed and pushed into stdio wrapper.

//////////////

class reader_thread : public ethread
{
public:
  reader_thread(stdio_redirecter &parent, bool is_stdout)
  : ethread(), _is_stdout(is_stdout), _parent(parent) {
  }

  virtual ~reader_thread() {
  }

  virtual void perform_activity(void *formal(ptr)) {
    while (!should_stop()) {
      _parent.std_thread_action(_is_stdout);
    }
  }

private:
  bool _is_stdout;  // if true, then stdout, if false, then stderr.
  stdio_redirecter &_parent;
};

//////////////

stdio_redirecter::stdio_redirecter()
:
#ifdef __WIN32__
  _child_in(NIL), _child_out(NIL), _child_err(NIL),
  _parent_in(NIL), _parent_out(NIL), _parent_err(NIL),
  _app_handle(NIL),
#endif
  _command(new astring),
  _parms(new astring),
  _persistent_result(OKAY),
  _stdout_reader(new reader_thread(*this, true)),
  _stderr_reader(new reader_thread(*this, false)),
  _stdout_queue(new byte_array),
  _stderr_queue(new byte_array),
  _lock(new mutex),
  _process_id(0),
  _exit_value(0)
{
}

stdio_redirecter::stdio_redirecter(const astring &command,
    const astring &parameters)
:
#ifdef __WIN32__
  _child_in(NIL), _child_out(NIL), _child_err(NIL),
  _parent_in(NIL), _parent_out(NIL), _parent_err(NIL),
  _app_handle(NIL),
#endif
  _command(new astring(command)),
  _parms(new astring(parameters)),
  _persistent_result(OKAY),
  _stdout_reader(new reader_thread(*this, true)),
  _stderr_reader(new reader_thread(*this, false)),
  _stdout_queue(new byte_array),
  _stderr_queue(new byte_array),
  _lock(new mutex),
  _process_id(0),
  _exit_value(0)
{
  outcome ret = create_pipes();
  if (ret != OKAY) { _persistent_result = ret; return; }
  ret = launch_program(_process_id);
  if (ret != OKAY) { _persistent_result = ret; return; }
}

stdio_redirecter::~stdio_redirecter()
{
  zap_program();
  _process_id = 0;

  WHACK(_stdout_queue);
  WHACK(_stderr_queue);
  WHACK(_stdout_reader);
  WHACK(_stderr_reader);
  WHACK(_command);
  WHACK(_parms);
  WHACK(_lock);
}

outcome stdio_redirecter::reset(const astring &command,
    const astring &parameters)
{
  zap_program();
  _process_id = 0;

  *_command = command;
  *_parms = parameters;
  _persistent_result = OKAY;

  outcome ret = create_pipes();
  if (ret != OKAY) { _persistent_result = ret; return ret; }
  ret = launch_program(_process_id);
  if (ret != OKAY) { _persistent_result = ret; return ret; }
  return ret;
}

outcome stdio_redirecter::create_pipes()
{
  FUNCDEF("create_pipes");
#ifdef __UNIX__
  // the input and output here are from the perspective of the parent
  // process and not the launched program.
  if (pipe(_input_fds)) {
    LOG("failure to open an unnamed pipe for input.");
    return ACCESS_DENIED;
  }
  if (pipe(_output_fds)) {
    LOG("failure to open an unnamed pipe for output.");
    return ACCESS_DENIED;
  }
  if (pipe(_stderr_fds)) {
    LOG("failure to open an unnamed pipe for stderr.");
    return ACCESS_DENIED;
  }
#elif defined (__WIN32__)
  // set up the security attributes structure that governs how the child
  // process is created.
  SECURITY_ATTRIBUTES sa;
  ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
  sa.nLength= sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor = NIL;
  sa.bInheritHandle = true;

  HANDLE in_temp = NIL, out_temp = NIL, err_temp = NIL;

  // create pipes that we will hook up to the child process.  these are
  // currently inheritable based on the security attributes.
  if (!CreatePipe(&_child_in, &in_temp, &sa, 0)) return ACCESS_DENIED;
  if (!CreatePipe(&out_temp, &_child_out, &sa, 0)) return ACCESS_DENIED;
  if (!CreatePipe(&err_temp, &_child_err, &sa, 0)) return ACCESS_DENIED;

  HANDLE process_handle = GetCurrentProcess();
    // retrieve process handle for use in system calls below.  since it's
    // a pseudo handle, we don't need to close it.

  // create new handles for the parent process (connected to this object) to
  // use.  the false indicates that the child should not inherit the properties
  // on these because otherwise it cannot close them.
  if (!DuplicateHandle(process_handle, in_temp, process_handle, &_parent_in,
      0, false, DUPLICATE_SAME_ACCESS)) return ACCESS_DENIED;
  if (!DuplicateHandle(process_handle, out_temp, process_handle, &_parent_out,
      0, false, DUPLICATE_SAME_ACCESS)) return ACCESS_DENIED;
  if (!DuplicateHandle(process_handle, err_temp, process_handle, &_parent_err,
      0, false, DUPLICATE_SAME_ACCESS)) return ACCESS_DENIED;

  // close out the handles that we're done with and don't want the child to
  // inherit.
  CloseHandle(in_temp);
  CloseHandle(out_temp);
  CloseHandle(err_temp);
#endif

  return OKAY;
}

outcome stdio_redirecter::launch_program(int &new_process_id)
{
//  FUNCDEF("launch_program");
  new_process_id = 0;
#ifdef __UNIX__
  int fork_ret = fork();
  if (fork_ret == 0) {
    // this is the child.
    close(_output_fds[1]);  // close our *input* pipe's output fd.
    dup2(_output_fds[0], 0);  // close our stdin and replace with input pipe.
    close(_input_fds[0]);  // close our *output* pipe's input fd.
    dup2(_input_fds[1], 1);  // close our stdout and replace with output pipe.
    close(_stderr_fds[0]);  // close stderr input fd.
    dup2(_stderr_fds[1], 2);  // close our stderr and pipe it to parent.
    // now we want to launch the program for real.
    processes::char_star_array parms = launch_process::break_line(*_command, *_parms);
    execv(_command->s(), parms.observe());
    // oops.  failed to exec if we got to here.
    exit(1);
  } else {
    // this is the parent.
    _process_id = fork_ret;  // save the child's process id.
    new_process_id = _process_id;  // set the returned id.
    close(_output_fds[0]);  // close our *output* pipe's input fd.
    close(_input_fds[1]);  // close our *input* pipe's output fd.
    close(_stderr_fds[1]);  // close the child's stderr output side.
    // now we should have a set of pipes that talk to the child.
  }
#elif defined (__WIN32__)
  // set up the startup info struct.
  STARTUPINFO si;
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);
  si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
  si.hStdInput  = _child_in;
  si.hStdOutput = _child_out;
  si.hStdError  = _child_err;
  si.wShowWindow = SW_HIDE;  // we'll hide the console window.

  // setup the security attributes for the new process.
  SECURITY_DESCRIPTOR *sec_desc = (SECURITY_DESCRIPTOR *)GlobalAlloc
      (GPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
  InitializeSecurityDescriptor(sec_desc, SECURITY_DESCRIPTOR_REVISION);
  SetSecurityDescriptorDacl(sec_desc, -1, 0, 0);
  LPSECURITY_ATTRIBUTES sec_attr = (LPSECURITY_ATTRIBUTES)GlobalAlloc(GPTR,
      sizeof(SECURITY_ATTRIBUTES));
  sec_attr->nLength = sizeof(SECURITY_ATTRIBUTES);
  sec_attr->lpSecurityDescriptor = sec_desc;
  sec_attr->bInheritHandle = true;

  astring cmd = *_command;
  if (cmd[0] != '"')
    cmd.insert(0, "\"");
  if (cmd[cmd.end()] != '"')
    cmd += "\"";
  cmd += " ";
  cmd += *_parms;

  // fork off the process.
  PROCESS_INFORMATION pi;
  BOOL success = CreateProcess(NIL, to_unicode_temp(cmd), sec_attr, NIL,
      true, CREATE_NEW_CONSOLE, NIL, NIL, &si, &pi);

  // cleanup junk we allocated.
  if (sec_attr != NIL) GlobalFree(sec_attr);
  if (sec_desc != NIL) GlobalFree(sec_desc);

  if (success) {
    // toss out the thread handle since we don't use it.
    CloseHandle(pi.hThread);
    // track the important handle, for our application.
    _app_handle = pi.hProcess;
//hmmm: boot this stuff out into the stdio_wrapper class, which is the only
//      thing that should do this.
    ini_configurator ini(REDIRECTER_INI, ini_configurator::RETURN_ONLY,
            ini_configurator::APPLICATION_DIRECTORY);
    ini.store(PROCESS_SECTION, a_sprintf("%d", application_configuration::process_id()),
        a_sprintf("%d", pi.dwProcessId));
    _process_id = pi.dwProcessId;
    new_process_id = _process_id;
  } else {
    return NOT_FOUND;
  }
#endif

  _stdout_reader->start(NIL);
  _stderr_reader->start(NIL);

  return OKAY;
}

bool stdio_redirecter::running()
{
  if (!_process_id) return false;  // nothing to check.
  if (_exit_value != 0) return false;  // gone by now.
#ifdef __UNIX__
  int status;
  pid_t pid = waitpid(_process_id, &status, WNOHANG);
  if (!pid) return true;  // still going.
  if (!_exit_value) {
//hmmm: is that all we need from it?  unprocessed exit value?
    _exit_value = status;
  }
  if (WIFEXITED(status)) {
    // the child exited on its own.
    _process_id = 0;
    return false;
  } else if (WIFSIGNALED(status)) {
    // the child was zapped by a signal.
    _process_id = 0;
    return false;
  }
  return true;
#elif defined (__WIN32__)
  DWORD exit_value = 0;
  // see if there's an exit code yet.  if this fails with false, then the
  // process is maybe long gone or something?
  BOOL ret = GetExitCodeProcess(_app_handle, &exit_value);
  if (ret) {
    // store it if we had no previous version.
    if (exit_value != STILL_ACTIVE) {
      _exit_value = exit_value;
      return false;
    }
    return true;
  } else {
    // this one seems to still be going.
    return true;
  }
#endif
}

void stdio_redirecter::close_input()
{
#ifdef __UNIX__
  close(_output_fds[1]);  // shut down input to the child program.
#elif defined(__WIN32__)
  if (_child_in) { CloseHandle(_child_in); _child_in = NIL; }
  if (_parent_in) { CloseHandle(_parent_in); _parent_in = NIL; }
#endif
}

void stdio_redirecter::zap_program()
{
  FUNCDEF("zap_program");
  _stdout_reader->cancel();
  _stderr_reader->cancel();

#ifdef __UNIX__
  close_input();
  close(_stderr_fds[0]);
  close(_input_fds[0]);

  if (_process_id) {
    kill(_process_id, 9);  // end the program without any doubt.
  }
  _process_id = 0;
#elif defined(__WIN32__)
  if (_app_handle) {
    // none of the handle closing works if the app is still running.
    // microsoft hasn't really got a clue, if you cannot close a file handle
    // when you want to, but that's apparently what's happening.
    TerminateProcess(_app_handle, 1);
  }

  close_input();

  if (_child_out) { CloseHandle(_child_out); _child_out = NIL; }
  if (_parent_out) { CloseHandle(_parent_out); _parent_out = NIL; }

  if (_child_err) { CloseHandle(_child_err); _child_err = NIL; }
  if (_parent_err) { CloseHandle(_parent_err); _parent_err = NIL; }

  // shut down the child process if it's still there.
  if (_app_handle) {
    DWORD ret;

//hmmm: also should only be in the stdio wrapper program.
//hmmm: remove this in favor of the stdio wrapper or whomever tracking their
//      own process id.
    ini_configurator ini(REDIRECTER_INI, ini_configurator::RETURN_ONLY,
            ini_configurator::APPLICATION_DIRECTORY);
    ini.delete_entry(PROCESS_SECTION, a_sprintf("%d", application_configuration::process_id()));

    GetExitCodeProcess(_app_handle, &ret);
    if (ret == STILL_ACTIVE) {
      // it's still bumbling along; let's drop it.
      TerminateProcess(_app_handle, 1);
      if (WaitForSingleObject(_app_handle, 1000) == WAIT_TIMEOUT) {
///problem!
        LOG("hmmm, we timed out waiting for the process to exit.");
      }
    }
    CloseHandle(_app_handle);
    _app_handle = NIL;
  }
#endif

  _stdout_reader->stop();
  _stderr_reader->stop();
}

outcome stdio_redirecter::read(byte_array &received)
{
  received.reset();
  if (_persistent_result != OKAY) return _persistent_result;
  auto_synchronizer l(*_lock);
  if (!_stdout_queue->length()) return NONE_READY;
//hmmm: signal eof too!
  received = *_stdout_queue;
  _stdout_queue->reset();
  return common::OKAY;
}

outcome stdio_redirecter::write(const astring &to_write, int &written)
{
  byte_array real_write(to_write.length(), (abyte *)to_write.observe());
  return write(real_write, written);
}

outcome stdio_redirecter::write(const byte_array &to_write, int &written)
{
//  FUNCDEF("write");
  written = 0;
  if (_persistent_result != OKAY) return _persistent_result;
#ifdef __UNIX__
  int writ = ::write(_output_fds[1], to_write.observe(), to_write.length());
  if (writ < 0) return ACCESS_DENIED;
  written = writ;
  return OKAY;
#elif defined(__WIN32__)
  DWORD writ = 0;
  BOOL ret = WriteFile(_parent_in, to_write.observe(), to_write.length(),
      &writ, NIL);
  written = writ;
  if (ret) return OKAY;
  else return ACCESS_DENIED;
#endif
}

outcome stdio_redirecter::read_stderr(byte_array &received)
{
  received.reset();
  if (_persistent_result != OKAY) return _persistent_result;
  auto_synchronizer l(*_lock);
  if (!_stderr_queue->length()) return NONE_READY;
//signal eof too!
  received = *_stderr_queue;
  _stderr_queue->reset();
  return common::OKAY;
}

void stdio_redirecter::std_thread_action(bool is_stdout)
{
//  FUNCDEF("std_thread_action");
  byte_array buff(BUFFER_SIZE + 1);
#ifdef __UNIX__
  bool ret = false;
  int fd = _input_fds[0];
  if (!is_stdout) fd = _stderr_fds[0];
  if (!fd) return;  // nothing to read from.
  int bytes_read = ::read(fd, buff.access(), BUFFER_SIZE);
  if (!bytes_read) {
//indicates end of file; set flags!
  } else if (bytes_read > 0) {
    ret = true;  // there's new data in our buffer.
  }
#elif defined(__WIN32__)
  HANDLE where = _parent_out;
  if (!is_stdout) where = _parent_err;
  if (!where) return;  // nothing to read from.
  // read some data from the file.  the function will return when a write
  // operation completes or we get that much data.
  DWORD bytes_read = 0;
  BOOL ret = ReadFile(where, buff.access(), BUFFER_SIZE, &bytes_read, NIL);
//hmmm:    if (ret && !bytes_read) {///set eof!!! }
#endif
  if (ret && bytes_read) {
    auto_synchronizer l(*_lock);
    byte_array *queue = _stdout_queue;
    if (!is_stdout) queue = _stderr_queue;
    *queue += buff.subarray(0, bytes_read - 1);
  }
}

} //namespace.


