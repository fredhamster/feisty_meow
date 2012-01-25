#ifndef CALLSTACK_TRACKER_CLASS
#define CALLSTACK_TRACKER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : callstack_tracker                                                 *
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

#include "definitions.h"

#ifdef ENABLE_CALLSTACK_TRACKING

#include "build_configuration.h"
#include "root_object.h"

namespace application {

// forward.
class callstack_records;
class callstack_tracker;

//////////////

callstack_tracker BASIS_EXTERN &program_wide_stack_trace();
  //!< a global object that can be used to track the runtime callstack.

//////////////

//! This object can provide a backtrace at runtime of the invoking methods.
/*!
  The callstack tracking is hooked in through the FUNCDEF macros used to
  set function names for logging.  Thus it will only be visible if those
  macros are used fairly carefully or if people invoke the stack frame addition
  method themselves.
*/

class callstack_tracker
{
public:
  callstack_tracker();
  virtual ~callstack_tracker();

  DEFINE_CLASS_NAME("callstack_tracker");

  bool push_frame(const char *class_name, const char *func, const char *file,
          int line);
    //!< adds a new stack from for the "class_name" in "function" at the "line".
    /*!< this function should be invoked when entering a new stack frame.  the
    "file" can be gotten from the __FILE__ macro and the "line" number can come
    from __LINE__, but the "class_name" and "func" must be tracked some other
    way.  we recommend the FUNCDEF macro.  this function might return false if
    there is no longer any room for tracking more frames; that is a serious
    issue that might indicate a runaway recursion or infinite loop. */

  bool pop_frame();
    //!< removes the last callstack frame off from our tracking.

  bool update_line(int line);
    //!< sets the line number within the current stack frame.
    /*!< the current frame can reside across several line numbers, so this
    allows the code to be more specific about the location of an invocation. */

  char *full_trace() const;
    //!< provides the current stack trace in a newly malloc'd string.
    /*!< the user *must* free() the string returned. */

  int full_trace_size() const;
    //!< this returns the number of bytes needed for the above full_trace().

  int depth() const { return _depth; }
    //!< the current number of frames we know of.

  double frames_in() const { return _frames_in; }
    //!< reports the number of call stack frames that were added, total.

  double frames_out() const { return _frames_out; }
    //!< reports the number of call stack frames that were removed, total.

  double highest() const { return _highest; }
    //!< reports the maximum stack depth seen during the runtime so far.

private:
  callstack_records *_bt;  //!< the backtrace records for current program.
  int _depth;  //!< the current number of frames we know of.
  double _frames_in;  //!< number of frame additions.
  double _frames_out;  //!< number of frame removals.
  double _highest;  //!< the most number of frames in play at once.
  bool _unusable;  //!< object has already been destroyed.
};

//////////////

//! a small object that represents a stack trace in progress.
/*! the object will automatically be destroyed when the containing scope
exits.  this enables a users of the stack tracker to simply label their
function name and get the frame added.  if they want finer grained tracking,
they should update the line number periodically through their function,
especially when memory is about to be allocated or where something might go
wrong. */

class frame_tracking_instance
{
public:
  // these are not encapsulated, but be careful with the contents.
  bool _frame_involved;  //!< has this object been added to the tracker?
  char *_class, *_func, *_file;  //!< newly allocated copies.
  int _line;

  frame_tracking_instance(const char *class_name = "", const char *func = "",
      const char *file = "", int line = 0, bool add_frame = false);
    //!< as an automatic variable, this can hang onto frame information.
    /*!< if "add_frame" is true, then this actually adds the stack frame in
    question to the tracker.  thus if you use this class at the top of your
    function, such as via the FUNCDEF macro, then you can forget about having
    to pop the frame later. */

  frame_tracking_instance(const frame_tracking_instance &to_copy);

  ~frame_tracking_instance();
    //!< releases the information *and* this stack frame in the tracker.

  frame_tracking_instance &operator =(const frame_tracking_instance &to_copy);

  void assign(const char *class_name, const char *func, const char *file,
          int line);
    //!< similar to assignment operator but doesn't require an object.

  void clean();
    //!< throws out our accumulated memory and pops frame if applicable.
};

void update_current_stack_frame_line_number(int line);
  //!< sets the line number for the current frame in the global stack trace.

#else // ENABLE_CALLSTACK_TRACKING
  // bogus replacements for most commonly used callstack tracking support.
  #define frame_tracking_instance
  #define __trail_of_function(p1, p2, p3, p4, p5) if (func) {}
    // the above actually trades on the name of the object we'd normally
    // define.  it must match the object name in the FUNCDEF macro.
  #define update_current_stack_frame_line_number(line)
#endif // ENABLE_CALLSTACK_TRACKING

} //namespace.

#endif // outer guard.

