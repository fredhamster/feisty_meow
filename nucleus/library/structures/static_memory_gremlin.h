#ifndef STATIC_MEMORY_GREMLIN_CLASS
#define STATIC_MEMORY_GREMLIN_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : static_memory_gremlin                                             *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/contracts.h>
#include <basis/enhance_cpp.h>
#include <basis/mutex.h>

namespace structures {

// forward declarations.
class gremlin_object_record;

//! Holds onto memory chunks that are allocated globally within the program.
/*!
  The objects managed by the gremlin do not get destroyed until after the
  program begins shutting down.  This file also provides the SAFE_STATIC macros
  that can be used for allocating static objects safely in a multi-threaded
  program.
*/

class static_memory_gremlin : public virtual basis::nameable
{
public:
  static_memory_gremlin();

  ~static_memory_gremlin();
    /*!< when destroyed, it is assumed that the program's lifetime is over and
    all objects stored here are now unnecessary.  this implements a
    regenerative scheme for when static shutdowns occur out of order; if an
    object has already been destroyed, it is recreated for the purposes of
    other statics being shutdown.  eventually this should stabilize since
    it's completely unacceptable for static objects to depend on each other
    in a cycle. */

  DEFINE_CLASS_NAME("static_memory_gremlin");

  static bool __program_is_dying();
    //!< Reports whether the program is shutting down currently.
    /*!< If this flag is true, then code should generally just return without
    doing anything where possible.  It's especially important for long
    running loops or active threads to stop if they see this, although
    the normal program exit processes usually make it unnecessary. */

  static static_memory_gremlin &__hoople_globals();
    //!< Holds onto objects that have been allocated in a program-wide scope.
    /*!< These objects will have a lifetime up to the point of normal static
    object destruction and then they will be cleaned up. */

  static basis::mutex &__memory_gremlin_synchronizer();
    //!< private object for static_memory_gremlin's use only.

  void enable_debugging(bool verbose) { c_show_debugging = verbose; }
    //!< if "verbose" is true, then the object will produce a noisy log.

  bool put(const char *unique_name, basis::root_object *ptr);
    //!< adds a "ptr" to the set of static objects under the "unique_name".
    /*!< the name must really be unique or objects will collide.  we recommend
    using an identifier based on a line number and filename where the static
    is going to be placed (see the safe static implementation below). */

  basis::root_object *get(const char *unique_name);
    //!< locates the pointer held for the "unique_name", if any.
    /*!< if no pointer exists, then NULL_POINTER is returned.  NOTE: the returned
    pointer must not be destroyed, since the object could be used at any time
    during the program's lifetime. */

  const char *find(const basis::root_object *ptr);
    //!< locates the name for "ptr" in our objects.
    /*!< if it does not exist, then NULL_POINTER is returned. */

  void ensure_space_exists();
    /*!< makes sure that the list of objects is large enough to contain all of
    the identifiers that have been issued. */

private:
  basis::mutex c_lock;  //!< protects object's state.
  int c_top_index;  //!< top place that's occupied in the list.
  int c_actual_size;  //!< the real number of indices in list.
  gremlin_object_record **c_pointers;  //!< storage for the static pointers.
  bool c_show_debugging;  //!< if true, then the object will log noisily.

  int locate(const char *unique_name);
    //!< returns the index number of the "unique_name".
};

//////////////

//! Statically defines a singleton object whose scope is the program's lifetime.
/*!
  SAFE_STATIC is a macro that dynamically creates a function returning a particular
  data type.  the object is allocated statically, so that the same object will be
  returned ever after until the program is shut down.  the allocation is guarded so
  that multiple threads cannot create conflicting static objects.

  note: in ms-win32, if you use this macro in a static library (rather than
  a DLL), then the heap context is different.  thus you could actually have
  multiple copies of the underlying object.  if the object is supposed to
  be global and unique, then that's a problem.  relocating the definitions
  to a dll while keeping declarations in the static lib works (see the
  program wide logger approach in <application/program_wide_logger.h>).
    "type" is the object class to return.
    "func_name" is the function to be created.
    "parms" must be a list of parameters in parentheses or nothing.

  example usage: @code
  SAFE_STATIC(connection_table, conntab, (parm1, parm2)) @endcode
  will define a function: connection_table &conntab()
  that returns a connection table object which has been created safely,
  given that the main synchronizer was called from the main thread at least
  once.  @code
  SAFE_STATIC(astring, static_string_doodle, ) @endcode
  will define a static astring object named "static_string_doodle" that uses
  the default constructor for astrings.
*/
#define SAFE_STATIC(type, func_name, parms) \
  type &func_name() { SAFE_STATIC_IMPLEMENTATION(type, parms, __LINE__); }

//! this version returns a constant object instead.
#define SAFE_STATIC_CONST(type, func_name, parms) \
  const type &func_name() \
  { SAFE_STATIC_IMPLEMENTATION(type, parms, __LINE__); }

//! Statically defines a string for the rest of the program's life.
/*! This macro can be used to make functions returning a string a little
simpler.  This can only be used when the string is constant.  The usual way
to use this macro is in a function that returns a constant reference to a
string.  The macro allocates the string to be returned statically so that all
future calls will refer to the stored string rather than recreating it again. */
#define STATIC_STRING(str) \
  SAFE_STATIC_IMPLEMENTATION(astring, (str), __LINE__)

//////////////

//! this macro creates a unique const char pointer based on file location.
#define UNIQUE_NAME_BASED_ON_SOURCE(name, linenum) \
  static const char *name = "file:" __FILE__ ":line:" #linenum

//! this blob is just a chunk of macro implementation for SAFE_STATIC...
/*! if the static object isn't initialized yet, we'll create it and store it
in the static_memory_gremlin.  we make sure that the program isn't shutting
down, because that imposes a new requirement--previously created statics might
have already been destroyed.  thus, during the program shutdown, we carefully
recreate any objects that were already toast. */
#define SAFE_STATIC_IMPLEMENTATION(type, parms, linenum) \
/*hmmm: bring all this back.*/ \
/*  const char *func = "allocation"; */ \
/*  frame_tracking_instance __trail_of_function("safe_static", func, \
      __FILE__, __LINE__, true); */ \
  UNIQUE_NAME_BASED_ON_SOURCE(__uid_name, linenum); \
/*  program_wide_memories(); */ \
  static basis::root_object *_hidden = NULL_POINTER; \
  /* if haven't initialized yet, then definitely need to lock carefully. */ \
  if (structures::static_memory_gremlin::__program_is_dying() || !_hidden) { \
    basis::auto_synchronizer l(structures::static_memory_gremlin::__memory_gremlin_synchronizer()); \
    if (structures::static_memory_gremlin::__program_is_dying()) { \
      /* we can't rely on the pointer since we're shutting down currently. */ \
      _hidden = structures::static_memory_gremlin::__hoople_globals().get(__uid_name); \
    } \
    if (!_hidden) {  /* make sure no one scooped us. */ \
      /* try to retrieve an existing one first and use it if there. */ \
      _hidden = structures::static_memory_gremlin::__hoople_globals().get(__uid_name); \
      if (!_hidden) { \
        _hidden = new type parms ; /* create the object finally. */ \
        /* store our object using the unique name for it. */ \
        if (!structures::static_memory_gremlin::__hoople_globals().put(__uid_name, _hidden)) { \
          /* we failed to allocate space.  this is serious. */ \
          throw __uid_name; \
        } \
      } \
    } \
  } \
  if (!_hidden) { \
    /* grab the pointer that was stored, in case we're late getting here. */ \
    /* another thread might have scooped the object creation. */ \
    _hidden = structures::static_memory_gremlin::__hoople_globals().get(__uid_name); \
  } \
  return *dynamic_cast<type *>(_hidden)

// historical note: the SAFE_STATIC approach has existed since about 1998.
// however, the static_memory_gremlin's role in this started much later.

} //namespace.

#endif

