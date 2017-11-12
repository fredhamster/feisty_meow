


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

#ifdef ENABLE_CALLSTACK_TRACKING

// note: this object cannot be constructed when the memory_checker is still
// tracking memory leaks.  it must be disabled so that this object can
// construct without being tracked, which causes an infinite loop.  the code
// in the basis extern support takes care of that for us.

#include "callstack_tracker.h"

#include <malloc.h>
#include <stdio.h>

////#undef new
//is this right way to clean that out.

const int MAX_STACK_DEPTH = 2000;
  // beyond that many stack frames, we will simply refuse to add any more.

const int MAX_TEXT_FIELD = 1024;
  // the most space we allow the class, function, and file to take up.

const char *emptiness_note = "Empty Stack\n";
  //!< what we show when the stack is empty.

//////////////

class callstack_records
{
public:
  frame_tracking_instance _records[MAX_STACK_DEPTH + 2];  // fudging room.
};

//////////////

// our current depth gives us our position in the array.  we define our
// stack as starting at element zero, which is a null stack entry and
// corresponds to a depth of zero also.  then, when the stack depth is one,
// we actually have an element in place and it resides at index 1.  this
// scheme allows us to have an update to the line number just do nothing when
// there is no current stack.

callstack_tracker::callstack_tracker()
: _bt(new callstack_records),
  _depth(0),
  _frames_in(0),
  _frames_out(0),
  _highest(0),
  _unusable(false)
{
//printf("callstack ctor\n");
}

callstack_tracker::~callstack_tracker()
{
//printf("!!!!!!!!!!!!!!!!!!   callstack dtor in\n");
  _unusable = true;
  WHACK(_bt);
//printf("!!!!!!!!!!!!!!!!!!   callstack dtor out\n");
}

bool callstack_tracker::push_frame(const char *class_name, const char *func,
    const char *file, int line)
{
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
  if (_unusable) return false;
  if (!_depth) return false;  // not as serious, but pretty weird.
  _bt->_records[_depth]._line = line;
  return true;
}

char *callstack_tracker::full_trace() const
{
  if (_unusable) return strdup("");
//printf("fulltrace in\n");
  char *to_return = (char *)malloc(full_trace_size());
  to_return[0] = '\0';
  if (!_depth) {
    strcat(to_return, emptiness_note);
    return to_return;
  }
  const int initial_len = MAX_TEXT_FIELD + 8;
  char temp[initial_len];
  int allowed_len = initial_len;
    // space provided for one text line.
  // start at top most active frame and go down towards bottom most.
  for (int i = _depth; i >= 1; i--) {
    strcat(to_return, "\t");  // we left space for this and \n at end.
    temp[0] = '\0';
    int len_class = strlen(_bt->_records[i]._class);
    int len_func = strlen(_bt->_records[i]._func);
    if (allowed_len > len_class + len_func + 6) {
      allowed_len -= len_class + len_func + 6;
      sprintf(temp, "\"%s::%s\", ", _bt->_records[i]._class,
          _bt->_records[i]._func);
      strcat(to_return, temp);
    }

    temp[0] = '\0';
    int len_file = strlen(_bt->_records[i]._file);
    if (allowed_len > len_file + 4) {
      allowed_len -= len_file + 4;
      sprintf(temp, "\"%s\", ", _bt->_records[i]._file);
      strcat(to_return, temp);
    }

    temp[0] = '\0';
    sprintf(temp, "\"line=%d\"", _bt->_records[i]._line);
    int len_line = strlen(temp);
    if (allowed_len > len_line) {
      allowed_len -= len_line;
      strcat(to_return, temp);
    }

    strcat(to_return, "\n");  // we left space for this already.
  }

//printf("fulltrace out\n");
  return to_return;
}

int callstack_tracker::full_trace_size() const
{
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
    program_wide_stack_trace().push_frame(class_name, func, file, line);
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
    program_wide_stack_trace().pop_frame();
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
  program_wide_stack_trace().update_line(line);
//printf("frametrackinst updatelinenum out\n");
}

#endif // ENABLE_CALLSTACK_TRACKING




