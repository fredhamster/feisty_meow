/*
*  Name   : static_memory_gremlin
*  Author : Chris Koeritz
*
* Copyright (c) 2004-$now By Author.  This program is free software; you can
* redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either version 2 of
* the License or (at your option) any later version.  This is online at:
*     http://www.fsf.org/copyleft/gpl.html
* Please send any updates to: fred@gruntose.com
*/

#include "static_memory_gremlin.h"

#include <basis/functions.h>

#include <basis/array.h>
//temp!  needed for fake continuable error etc

#include <stdio.h>
#include <string.h>

using namespace basis;

namespace structures {

//#define DEBUG_STATIC_MEMORY_GREMLIN
  // comment this out to eliminate debugging print-outs.  otherwise they
  // are controlled by the class interface.

//#define SKIP_STATIC_CLEANUP
  // don't uncomment this unless you want all static objects to be left
  // allocated on shutdown of the program.  that's very sloppy but may
  // sometimes be needed for testing.

//////////////

const int SMG_CHUNKING_FACTOR = 32;
  // we'll allocate this many indices at a time.

//////////////

static bool __global_program_is_dying = false;
  // this is set to true when no more logging or access to static objects
  // should be allowed.

//////////////

class gremlin_object_record
{
public:
  root_object *c_object;
  const char *c_name;
};

//////////////

static_memory_gremlin::static_memory_gremlin()
: c_lock(),
  c_top_index(0),
  c_actual_size(0),
  c_pointers(NULL_POINTER),
  c_show_debugging(false)
{
  ensure_space_exists();
}

static_memory_gremlin::~static_memory_gremlin()
{
  __global_program_is_dying = true;
    // now the rest of the program is on notice; we're practically gone.

#ifdef DEBUG_STATIC_MEMORY_GREMLIN
  if (c_show_debugging)
    printf("SMG: beginning static object shutdown...\n");
#endif

#ifndef SKIP_STATIC_CLEANUP
  // clean up any allocated pointers in reverse order of addition.
  while (c_top_index > 0) {
    // make sure we fixate on which guy is shutting down.  some new ones
    // could be added on the end of the list as a result of this destruction.
    int zapped_index = c_top_index - 1;
    gremlin_object_record *ptr = c_pointers[zapped_index];
    c_pointers[zapped_index] = NULL_POINTER;
      // since we know the one we're zapping, we no longer need that index.
    c_top_index--;
      // this should allow us to keep chewing on items that are newly being
      // added during static shutdown, since we have taken the current object
      // entirely out of the picture.
    if (ptr) {
#ifdef DEBUG_STATIC_MEMORY_GREMLIN
      if (c_show_debugging)
        printf((astring("SMG: deleting ") + ptr->c_object->instance_name()
            + " called " + ptr->c_name
            + a_sprintf(" at index %d.\n", zapped_index) ).s());
#endif
      WHACK(ptr->c_object);
      WHACK(ptr);
    }
  }
#endif
  delete [] c_pointers;
  c_pointers = NULL_POINTER;
}

bool static_memory_gremlin::__program_is_dying() { return __global_program_is_dying; }

mutex &static_memory_gremlin::__memory_gremlin_synchronizer()
{
  static mutex __globabl_synch_mem;
  return __globabl_synch_mem;
}

int static_memory_gremlin::locate(const char *unique_name)
{
  auto_synchronizer l(c_lock);
  for (int i = 0; i < c_top_index; i++) {
    if (!strcmp(c_pointers[i]->c_name, unique_name)) return i;
  }
  return common::NOT_FOUND;
}

root_object *static_memory_gremlin::get(const char *unique_name)
{
  auto_synchronizer l(c_lock);
  int indy = locate(unique_name);
  if (negative(indy)) return NULL_POINTER;
  return c_pointers[indy]->c_object;
}

const char *static_memory_gremlin::find(const root_object *ptr)
{
  auto_synchronizer l(c_lock);
  for (int i = 0; i < c_top_index; i++) {
    if (ptr == c_pointers[i]->c_object)
      return c_pointers[i]->c_name;
  }
  return NULL_POINTER;
}

bool static_memory_gremlin::put(const char *unique_name, root_object *to_put)
{
  auto_synchronizer l(c_lock);
  int indy = locate(unique_name);
  // see if that name already exists.
  if (non_negative(indy)) {
#ifdef DEBUG_STATIC_MEMORY_GREMLIN
    if (c_show_debugging)
      printf((astring("SMG: cleaning out old object ")
          + c_pointers[indy]->c_object->instance_name()
          + " called " + c_pointers[indy]->c_name 
          + " in favor of object " + to_put->instance_name()
          + " called " + unique_name
          + a_sprintf(" at index %d.\n", indy)).s());
#endif
    WHACK(c_pointers[indy]->c_object);
    c_pointers[indy]->c_object = to_put;
    return true;
  }
#ifdef DEBUG_STATIC_MEMORY_GREMLIN
  if (c_show_debugging)
    printf((astring("SMG: storing ") + to_put->instance_name()
        + " called " + unique_name
        + a_sprintf(" at index %d.\n", c_top_index)).s());
#endif
  ensure_space_exists();
  c_pointers[c_top_index] = new gremlin_object_record;
  c_pointers[c_top_index]->c_object = to_put;
  c_pointers[c_top_index]->c_name = unique_name;
  c_top_index++;
  return true;
}

void static_memory_gremlin::ensure_space_exists()
{
  auto_synchronizer l(c_lock);
  if (!c_pointers || (c_top_index + 1 >= c_actual_size) ) {
    // never had any contents yet or not enough space exists.
#ifdef DEBUG_STATIC_MEMORY_GREMLIN
    if (c_show_debugging)
      printf(a_sprintf("SMG: adding space for top at %d.\n", c_top_index).s());
#endif
    c_actual_size += SMG_CHUNKING_FACTOR;
    typedef gremlin_object_record *base_ptr;
    gremlin_object_record **new_ptr = new base_ptr[c_actual_size];
    if (!new_ptr) {
      throw "error: static_memory_gremlin::ensure_space_exists: failed to allocate memory for pointer list";
    }
    for (int i = 0; i < c_actual_size; i++) new_ptr[i] = NULL_POINTER;
    for (int j = 0; j < c_actual_size - SMG_CHUNKING_FACTOR; j++) 
      new_ptr[j] = c_pointers[j];
    if (c_pointers) delete [] c_pointers;
    c_pointers = new_ptr;
  }
}

// this function ensures that the space for the global objects is kept until
// the program goes away.  if it's the first time through, then the gremlin
// gets created; otherwise the existing one is used.  this function should
// always be called by the main program before any attempts to use global
// features like SAFE_STATIC or the program wide logger.  it is crucial that no
// user-level threads have been created in the program before this is called.
static_memory_gremlin &static_memory_gremlin::__hoople_globals()
{
  static bool _initted = false;  // tells whether we've gone through yet.
  static static_memory_gremlin *_internal_gremlin = NULL_POINTER;
    // holds our list of shared pieces...

  if (!_initted) {
#ifdef DEBUG_STATIC_MEMORY_GREMLIN
    printf("%s: initializing HOOPLE_GLOBALS now.\n", _global_argv[0]); 
#endif

#ifdef ENABLE_MEMORY_HOOK
    void *temp = program_wide_memories().provide_memory(1, __FILE__, __LINE__);
      // invoke now to get memory engine instantiated.
    program_wide_memories().release_memory(temp);  // clean up junk.
#endif

#ifdef ENABLE_CALLSTACK_TRACKING
    program_wide_stack_trace().full_trace_size();
      // invoke now to get callback tracking instantiated.
#endif
    FUNCDEF("HOOPLE_GLOBALS remainder");
      // this definition must be postponed until after the objects that would
      // track it actually exist.
    if (func) {}  // shut up the compiler about using it.

    // this simple approach is not going to succeed if the SAFE_STATIC macros
    // are used in a static library which is then used in more than one dynamic
    // library on win32.  this is because each dll in win32 will have a
    // different version of certain static objects that should only occur once
    // per program.  this problem is due to the win32 memory model, but in
    // hoople at least this has been prevented; our only static library that
    // appears in a bunch of dlls is basis and it is not allowed to use the
    // SAFE_STATIC macro.
    _internal_gremlin = new static_memory_gremlin;
    _initted = true;
  }

  return *_internal_gremlin;
}

//////////////

} // namespace.

