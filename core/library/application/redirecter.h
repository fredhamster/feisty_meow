#ifndef STDIO_REDIRECTER_CLASS
#define STDIO_REDIRECTER_CLASS

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

#include <basis/contracts.h>
#include <basis/mutex.h>
#include <processes/ethread.h>

namespace application {

//! Redirects I/O for a newly launched application.

class stdio_redirecter : public virtual basis::root_object
{
public:
  stdio_redirecter(const basis::astring &command, const basis::astring &parameters);
    //!< controls the I/O for a program started with "command" and "parameters".
    /*!< this creates an io redirecter that will trap the inputs and outputs
    of a program called "command" that is launched with the "parameters".
    the program can be communicated with via the read and write methods
    below. */

  stdio_redirecter();
    //!< creates a blank redirecter which should be started up with reset().

  ~stdio_redirecter();
    //!< shuts down the program that was launched, if it's still running.

  basis::outcome reset(const basis::astring &command, const basis::astring &parameters);
    //!< shuts down the active program and starts with new parameters.

  DEFINE_CLASS_NAME("stdio_redirecter");

  enum outcomes {
    OKAY = basis::common::OKAY,
    NOT_FOUND = basis::common::NOT_FOUND,  //!< the file to launch was not found.
    NONE_READY = basis::common::NONE_READY,  //!< there was no data available.
    PARTIAL = basis::common::PARTIAL,  //!< only some of the bytes could be stored.
    ACCESS_DENIED = basis::common::ACCESS_DENIED  //!< the OS told us we could not.
  };

  basis::outcome health() const { return _persistent_result; }
    //!< if the object constructed successfully, this returns OKAY.
    /*!< otherwise an error occurred during the pipe creation or process fork.
    */

  bool running();
    //!< returns true if the application is still running.
    /*!< if it exited on its own, then this will return false. */

  int exit_value() const { return _exit_value; }
    //!< returns the exit value from the app after it was launched.
    /*!< this is only valid if the launch succeeded, the app ran, and now the
    running() method is returning false because the application exited. */

  int process_id() const { return _process_id; }
    //!< the process id of the launched application.
    /*!< this is only valid if the app is still running. */

  void zap_program();
    //!< attempts to force a shutdown of the launched application.
    /*!< this also closes out all of our pipes and handles. */

  basis::outcome read(basis::byte_array &received);
    //!< attempts to read bytes from the program's standard output.
    /*!< if any bytes were found, OKAY is returned and the bytes are stored
    in "received". */

  basis::outcome write(const basis::byte_array &to_write, int &written);
    //!< writes the bytes in "to_write" into the program's standard input.
    /*!< OKAY is returned if all were successfully written.  the number of
    bytes that were actually sent to the program is put in "written".  if only
    a portion of the bytes could be written, then PARTIAL is returned. */

  basis::outcome write(const basis::astring &to_write, int &written);
    //!< sends a string "to_write" to the launched program's standard input.

  basis::outcome read_stderr(basis::byte_array &received);
    //!< reads from the program's standard error stream similarly to read().

  void close_input();
    //!< shuts down the standard input to the program.
    /*!< this simulates when the program receives an end of file on its
    main input. */

  // internal use only...

  void std_thread_action(bool is_stdout);
    //!< invoked by our threads when data becomes available.
    /*!< if "is_stdout" is true, then that's the pipe we get data from.
    otherwise we will try to get data from stderr. */

private:
#ifdef __UNIX__
  int _output_fds[2];  //!< file descriptors for parent's output.
  int _input_fds[2];  //!< file descriptors for parent's input.
  int _stderr_fds[2];  //!< file descriptors for standard error from child.
#endif
#ifdef __WIN32__
  void *_child_in, *_child_out, *_child_err;  //!< child process pipes replace io.
  void *_parent_in, *_parent_out, *_parent_err;  //!< our access to the pipes.
  void *_app_handle;  //!< refers to the launched application.
#endif
  basis::astring *_command;  //!< the application that we'll start and switch I/O on.
  basis::astring *_parms;  //!< the parameters for the app we will launch.
  basis::outcome _persistent_result;  //!< this records failures during construction.
  processes::ethread *_stdout_reader;  //!< gets data from process's stdout.
  processes::ethread *_stderr_reader;  //!< gets data from process's stderr.
  basis::byte_array *_stdout_queue;  //!< holds data acquired from stdout.
  basis::byte_array *_stderr_queue;  //!< holds data acquired from stderr.
  basis::mutex *_lock;  //!< protects our queues.
  int _process_id;  //!< pid for the launched program.
  int _exit_value;  //!< stored once when we notice app is gone.

  basis::outcome create_pipes();
    //!< creates the three pipes that the child will be given as stdin, stdout and stderr.

  basis::outcome launch_program(int &new_process_id);
    //!< launches the application using the previously established pipes.
};

} //namespace.

#endif

