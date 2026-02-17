/*
*
*  Name   : callstack_tracker
*  Author : Chris Koeritz
*
*
* Copyright (c) 2007-$now By Author.  This program is free software; you can
* redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either version 2 of
* the License or (at your option) any later version.  This is online at:
*     http://www.fsf.org/copyleft/gpl.html
* Please send any updates to: fred@gruntose.com
*/

#ifdef ENABLE_CALLSTACK_TRACKING

// note: this object cannot be constructed when the memory_checker is still
// tracking memory leaks.  it must be disabled so that this object can
// construct without being tracked, which causes an infinite loop.  the code
// in the basis extern support takes care of that for us.

#include "callstack_tracker.h"

#include <basis/functions.h>

#include <malloc.h>
#include <stdio.h>
#include <string.h>

using namespace basis;

namespace application {

const int MAX_STACK_DEPTH = 2000;
  // beyond that many stack frames, we will simply refuse to add any more.

const int MAX_TEXT_FIELD = 1024;
  // the most space we allow the class, function, and file to take up.

const char *emptiness_note = "Empty Stack\n";
  //!< what we show when the stack is empty.

//////////////

basis::mutex &callstack_tracker::__callstack_tracker_synchronizer()
{
  thread_local basis::mutex __global_synch_callstacks;
  return __global_synch_callstacks;
}

//////////////

//! the single instance of callstack_tracker.
/*!
  this is also an ultra low-level object, although it's not as far down
  as the memory checker.  it can allocate c++ objects and that kind of thing
  just fine.
*/
callstack_tracker &thread_wide_stack_trace()
{
  auto_synchronizer l(callstack_tracker::__callstack_tracker_synchronizer());

  thread_local callstack_tracker *_hidden_trace = NULL_POINTER;
  if (!_hidden_trace) {
#ifdef ENABLE_MEMORY_HOOK
    program_wide_memories().disable();
      /* we don't want infinite loops tracking the call stack during this object's construction. */

//hmmm: does that disable the progwide memories for the whole program or just for this thread?
//      and what does that entail exactly?
//      did it actually fix the problem we saw?

#endif
    _hidden_trace = new callstack_tracker;
#ifdef ENABLE_MEMORY_HOOK
    program_wide_memories().enable();
#endif
  }
  return *_hidden_trace;
}

//////////////

class callstack_records
{
public:
  frame_tracking_instance _records[MAX_STACK_DEPTH + 2];  // fudging room.
};

//////////////

/*
  our current depth gives us our position in the array.  we define our
  stack as starting at element zero, which is a null stack entry and
  corresponds to a depth of zero also.  then, when the stack depth is one,
  we actually have an element in place and it resides at index 1.  this
  scheme allows us to have an update to the line number just do nothing when
  there is no current stack.
*/

callstack_tracker::callstack_tracker()
: _bt(new callstack_records),
  _depth(0),
  _frames_in(0),
  _frames_out(0),
  _highest(0),
  _unusable(false)
{
}

callstack_tracker::~callstack_tracker()
{
  _unusable = true;
  WHACK(_bt);
}

bool callstack_tracker::push_frame(const char *class_name, const char *func,
    const char *file, int line)
{
  auto_synchronizer l(callstack_tracker::__callstack_tracker_synchronizer());

//printf("callstack pushframe depth=%d in\n", _depth);
  if (_unusable) return false;
  if (_depth >= MAX_STACK_DEPTH) {
    // too many frames already.
    printf("callstack_tracker::push_frame: past limit at class=%s func=%s "
        "file=%s line=%d\n", class_name, func, file, line);
    return false;
  }
  _depth++;
  if (_depth > _highest) _highest = _depth;
  _frames_in += 1;
  _bt->_records[_depth].assign(class_name, func, file, line);
//printf("callstack pushframe depth=%d out\n", _depth);
  return true;
}

bool callstack_tracker::pop_frame()
{
  auto_synchronizer l(callstack_tracker::__callstack_tracker_synchronizer());

//printf("callstack popframe depth=%d in\n", _depth);
  if (_unusable) return false;
  if (_depth <= 0) {
    // how inappropriate of them; we have no frames.
    _depth = 0;  // we don't lose anything useful by forcing it to be zero.
    printf("callstack_tracker::pop_frame stack underflow!\n");
    return false;
  }
  _bt->_records[_depth].clean();
  _depth--;
  _frames_out += 1;
//printf("callstack popframe depth=%d out\n", _depth);
  return true;
}

bool callstack_tracker::update_line(int line)
{
  auto_synchronizer l(callstack_tracker::__callstack_tracker_synchronizer());

  if (_unusable) return false;
  if (!_depth) return false;  // not as serious, but pretty weird.
  _bt->_records[_depth]._line = line;
  return true;
}

// helpful macro makes sure we stay within our buffer for string storage of the stack trace.
#define CHECK_SPACE_IN_BUFFER(desired_chunk) \
  /* being slightly paranoid about the space, but we don't want any buffer overflows. */ \
  if (space_used_in_buffer + desired_chunk >= full_size_needed - 4) { \
    printf("callstack_tracker::full_trace: failure in size estimation--we would have blown out of the buffer"); \
    return to_return; \
  } else { \
    space_used_in_buffer += desired_chunk; \
  }
  
char *callstack_tracker::full_trace() const
{
  auto_synchronizer l(callstack_tracker::__callstack_tracker_synchronizer());

  if (_unusable) return strdup("");
  int full_size_needed = full_trace_size();
  char *to_return = (char *)malloc(full_size_needed);
//printf("fulltrace allocated %d bytes for trace.\n", full_size_needed);
  to_return[0] = '\0';
  if (!_depth) {
    strcat(to_return, emptiness_note);
    return to_return;
  }

  const int initial_len = MAX_TEXT_FIELD + 8;
  char temp[initial_len];
  int space_left_in_line;  // the space provided for one text line.

  int space_used_in_buffer = 0;
    /* tracks whether we're getting close to the buffer limit.  technically, this
    should not happen, since we calculated the space ahead of time... but it is good
    to ensure we don't overflow the buffer in case we were not accurate. */

  // start at top most active frame and go down towards bottom most.
  for (int i = _depth; i >= 1; i--) {
    CHECK_SPACE_IN_BUFFER(1);
    strcat(to_return, "\t");  // we left space for this and \n at end.
    space_left_in_line = initial_len;  // reset our counter per line now.
    temp[0] = '\0';
    int len_class = strlen(_bt->_records[i]._class);
    int len_func = strlen(_bt->_records[i]._func);
    if (space_left_in_line > len_class + len_func + 6) {
      space_left_in_line -= len_class + len_func + 6;
      sprintf(temp, "\"%s::%s\", ", _bt->_records[i]._class,
          _bt->_records[i]._func);
      CHECK_SPACE_IN_BUFFER(strlen(temp));
      strcat(to_return, temp);
    }

    temp[0] = '\0';
    int len_file = strlen(_bt->_records[i]._file);
    if (space_left_in_line > len_file + 4) {
      space_left_in_line -= len_file + 4;
      sprintf(temp, "\"%s\", ", _bt->_records[i]._file);
      CHECK_SPACE_IN_BUFFER(strlen(temp));
      strcat(to_return, temp);
    }

    temp[0] = '\0';
    sprintf(temp, "\"line=%d\"", _bt->_records[i]._line);
    int len_line = strlen(temp);
    if (space_left_in_line > len_line) {
      space_left_in_line -= len_line;
      CHECK_SPACE_IN_BUFFER(strlen(temp));
      strcat(to_return, temp);
    }

    CHECK_SPACE_IN_BUFFER(1);
    strcat(to_return, "\n");  // we left space for this already.
  }

  return to_return;
}

int callstack_tracker::full_trace_size() const
{
  auto_synchronizer l(callstack_tracker::__callstack_tracker_synchronizer());

  if (_unusable) return 0;
  if (!_depth) return strlen(emptiness_note) + 14;  // liberal allocation.
  int to_return = 28;  // another hollywood style excess.
  for (int i = _depth; i >= 1; i--) {
    int this_line = 0;  // add up parts for just this item.

    // all of these additions are completely dependent on how it's done above.

    int len_class = strlen(_bt->_records[i]._class);
    int len_func = strlen(_bt->_records[i]._func);
    this_line += len_class + len_func + 6;

    int len_file = strlen(_bt->_records[i]._file);
    this_line += len_file + 4;

    this_line += 32;  // extra space for line number and such.

    // limit it like we did above; we will use the lesser size value.
    if (this_line < MAX_TEXT_FIELD + 8) to_return += this_line;
    else to_return += MAX_TEXT_FIELD + 8;
  }
  return to_return;
}

//////////////

frame_tracking_instance::frame_tracking_instance(const char *class_name,
    const char *func, const char *file, int line, bool add_frame)
: _frame_involved(add_frame),
  _class(class_name? strdup(class_name) : NULL_POINTER),
  _func(func? strdup(func) : NULL_POINTER),
  _file(file? strdup(file) : NULL_POINTER),
  _line(line)
{
  if (_frame_involved) {
//printf("frametrackinst ctor in class=%s func=%s\n", class_name, func);
    thread_wide_stack_trace().push_frame(class_name, func, file, line);
//printf("frametrackinst ctor out\n");
  }
}

frame_tracking_instance::frame_tracking_instance
    (const frame_tracking_instance &to_copy)
: _frame_involved(false),  // copies don't get a right to this.
  _class(to_copy._class? strdup(to_copy._class) : NULL_POINTER),
  _func(to_copy._func? strdup(to_copy._func) : NULL_POINTER),
  _file(to_copy._file? strdup(to_copy._file) : NULL_POINTER),
  _line(to_copy._line)
{
}

frame_tracking_instance::~frame_tracking_instance() { clean(); }

void frame_tracking_instance::clean()
{
  if (_frame_involved) {
//printf("frametrackinst clean\n");
    thread_wide_stack_trace().pop_frame();
  }
  _frame_involved = false;
  free(_class); _class = NULL_POINTER;
  free(_func); _func = NULL_POINTER;
  free(_file); _file = NULL_POINTER;
  _line = 0;
}

frame_tracking_instance &frame_tracking_instance::operator =
    (const frame_tracking_instance &to_copy)
{
//printf("frametrackinst tor = in\n");
  if (this == &to_copy) return *this;
  assign(to_copy._class, to_copy._func, to_copy._file, to_copy._line);
//printf("frametrackinst tor = out\n");
  return *this;
}

void frame_tracking_instance::assign(const char *class_name, const char *func,
    const char *file, int line)
{
  clean();
  _frame_involved = false;  // copies don't get a right to this.
  _class = class_name? strdup(class_name) : NULL_POINTER;
  _func = func? strdup(func) : NULL_POINTER;
  _file = file? strdup(file) : NULL_POINTER;
  _line = line;
}

void update_current_stack_frame_line_number(int line)
{
//printf("frametrackinst updatelinenum in\n");
  thread_wide_stack_trace().update_line(line);
//printf("frametrackinst updatelinenum out\n");
}

} // namespace

#endif // ENABLE_CALLSTACK_TRACKING

