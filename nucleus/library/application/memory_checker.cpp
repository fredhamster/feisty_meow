


/*****************************************************************************\
*                                                                             *
*  Name   : memory_checker                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1998-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

// note: parts of this have been around since at least 1998, but this code was
// newly revised for memory checking in february of 2007.  --cak

#ifdef ENABLE_MEMORY_HOOK

#include "definitions.h"
#include "log_base.h"
#include "memory_checker.h"
#include "mutex.h"
#include "utility.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int MAXIMUM_HASH_SLOTS = 256 * KILOBYTE;
  // that's a whole lot of slots.  this number is basically multiplied by
  // the sizeof(memory_bin) to get full memory footprint.

const int SINGLE_LINE_SIZE_ESTIMATE = 200;
  // we are guessing that the average line of memory printout will take
  // this many characters.  that includes the size, the pointer value,
  // and the location and line number.

const int RESERVED_AREA = 1000;
  // we reserve this much space in the string generated for the memory bin
  // dumps.  it will be used for adding more information to the string.

#define CLEAR_ALLOCATED_MEMORY
  // uncomment this to ensure that new memory gets its contents set to zero.
  // neither new nor malloc does this, but it can help finding bugs from
  // people re-using deallocated memory.

#define MEMORY_CHECKER_STATISTICS
  // uncomment to enable code that analyzes how many allocations were new and
  // so forth.  this will make the object run a bit slower.

//#define DEBUG_MEMORY_CHECKER
  // uncomment for super noisy version.

//////////////

// define the replacement new and delete operators.

#include <basis/trap_new.addin>
void *operator new(size_t size, char *file, int line) throw (std::bad_alloc)
{ return program_wide_memories().provide_memory(size, file, line); }
#include <basis/untrap_new.addin>

void operator delete(void *ptr) throw ()
{ program_wide_memories().release_memory(ptr); }

//////////////

// memlink is one link in a chain of memories.  it's singly linked, so our
// algorithms have to explicitly remember the parent.

class memlink
{
public:
  void *_chunk;  //!< the chunk of memory held (not real address).
    /*!< NOTE: we store the chunk as it looks to the outside world, rather
    than using its real address.  this eliminates a bit of ambiguity in the
    code. */
  memlink *_next;  //!< the next memory wrapper in the list.
  int _size;  //!< the size of the chunk delivered.
  char *_where;  //!< the name of the place that created it.
  int _line;  //!< the line number where the item was allocated.
#ifdef ENABLE_CALLSTACK_TRACKING
  char *_stack;  //!< records the stack seen at time of allocation.
#endif

  void construct(void *ptr, int size, char *where, int line) {
    _next = NULL_POINTER;
    _chunk = ptr;
    _size = size;
    _where = strdup(where);  // uses malloc, not new, so we're safe.
    if (strlen(_where) > SINGLE_LINE_SIZE_ESTIMATE - 40) {
      // if we will not have room for the full line, we crop it.
      _where[SINGLE_LINE_SIZE_ESTIMATE - 40] = '\0';
    }
    _line = line;
#ifdef ENABLE_CALLSTACK_TRACKING
    _stack = program_wide_stack_trace().full_trace();
///printf("stack here:\n%s", _stack);
#endif
  }

  void destruct() {
    free(_chunk); _chunk = NULL_POINTER;
    free(_where); _where = NULL_POINTER; 
    _next = NULL_POINTER;
    _size = 0;
    _line = 0;
#ifdef ENABLE_CALLSTACK_TRACKING
    free(_stack); _stack = NULL_POINTER;
#endif
  }
};

//////////////

//pretty lame here so far.
#ifdef MEMORY_CHECKER_STATISTICS
  // simple stats: the methods below will tweak these numbers if memory_checker
  // statistics are enabled.  ints won't do here, due to the number of
  // operations in a long-running program easily overflowing that size.

  // this bank of statistics counts the number of times memory was treated
  // a certain way.
  double _stat_new_allocations = 0;  // this many new allocations occurred.
  double _stat_freed_allocations = 0;  // number of freed blocks.
  // next bank of stats are the sizes of the memory that were stowed, etc.
  double _stat_new_allocations_size = 0;  // this many bytes got allocated.
  double _stat_freed_allocations_size = 0;  // this many bytes were freed.
#endif

//////////////

//! the memory bin holds a list of chunks of memory in memlink objects.

class memory_bin
{
public:
  void construct() {
    _head = NULL_POINTER;
    _count = 0;
    _lock = (mutex_base *)malloc(sizeof(mutex_base));
    _lock->construct();
  }
  void destruct() {
    _lock->destruct();
    free(_lock);
  }

  int count() const { return _count; }

  int record_memory(void *ptr, int size, char *where, int line) {
    memlink *new_guy = (memlink *)malloc(sizeof(memlink));
    new_guy->construct(ptr, size, where, line);
    _lock->lock();
    // this code has the effect of putting more recent allocations first.
    // if they happen to get cleaned up right away, that's nice and fast.
    new_guy->_next = _head;
    _head = new_guy;
    _count++;
    _lock->unlock();
    return common::OKAY;  // seems to have worked fine.
  }

  int release_memory(void *to_release) {
    _lock->lock();
    // search the bin to locate the item specified.
    memlink *current = _head;  // current will scoot through the list.
    memlink *previous = NULL_POINTER;  // previous remembers the parent node, if any.
    while (current) {
      if (current->_chunk == to_release) {
#ifdef MEMORY_CHECKER_STATISTICS
        // record that it went away.
        _stat_freed_allocations += 1.0;
        _stat_freed_allocations_size += current->_size;
#endif
#ifdef DEBUG_MEMORY_CHECKER
        printf("found %p listed, removing for %s[%d]\n", to_release,
            current->_where, current->_line);
#endif
        // unlink this one and clean up; they don't want it now.
        if (!previous) {
          // this is the head we're modifying.
          _head = current->_next;
        } else {
          // not the head, so there was a valid previous element.
          previous->_next = current->_next;
        }
        // now trash that goner's house.
        current->destruct();
        free(current);
        _count--;
        _lock->unlock();
        return common::OKAY;
      }
      // the current node isn't it; jump to next node.
      previous = current;
      current = current->_next;
    }
#ifdef DEBUG_MEMORY_CHECKER
    printf("failed to find %p listed.\n", to_release);
#endif
    _lock->unlock();
    return common::NOT_FOUND;
  }

  void dump_list(char *add_to, int &curr_size, int max_size) {
    int size_alloc = 2 * SINGLE_LINE_SIZE_ESTIMATE;  // room for one line.
    char *temp_str = (char *)malloc(size_alloc);
    memlink *current = _head;  // current will scoot through the list.
    while (current) {
      temp_str[0] = '\0';
      sprintf(temp_str, "\n\"%s[%d]\", \"size %d\", \"addr %p\"\n",
          current->_where, current->_line, current->_size, current->_chunk);
      int len_add = strlen(temp_str);
      if (curr_size + len_add < max_size) {
        strcat(add_to, temp_str);
        curr_size += len_add;
      }
#ifdef ENABLE_CALLSTACK_TRACKING
      len_add = strlen(current->_stack);
      if (curr_size + len_add < max_size) {
        strcat(add_to, current->_stack);
        curr_size += len_add;
      }
#endif
      current = current->_next;
    }
    free(temp_str);
  }

private:
  memlink *_head;  // our first, if any, item.
  mutex_base *_lock;  // protects our bin from concurrent access.
  int _count;  // current count of items held.
};

//////////////

class allocation_memories
{
public:
  void construct(int num_slots) {
    _num_slots = num_slots;
    _bins = (memory_bin *)malloc(num_slots * sizeof(memory_bin));
    for (int i = 0; i < num_slots; i++)
      _bins[i].construct();
  }

  void destruct() {
    // destroy each bin in our list.
    for (int i = 0; i < _num_slots; i++) {
      _bins[i].destruct();
    }
    free(_bins);
    _bins = NULL_POINTER;
  }

  int compute_slot(void *ptr) {
    return utility::hash_bytes(&ptr, sizeof(void *)) % _num_slots;
  }

  void *provide_memory(int size_needed, char *file, int line) {
    void *new_allocation = malloc(size_needed);
    // slice and dice pointer to get appropriate hash bin.
    int slot = compute_slot(new_allocation);
#ifdef DEBUG_MEMORY_CHECKER
    printf("using slot %d for %p\n", slot, new_allocation);
#endif
    _bins[slot].record_memory(new_allocation, size_needed, file, line);
#ifdef MEMORY_CHECKER_STATISTICS
    _stat_new_allocations += 1.0;
    _stat_new_allocations_size += size_needed;
#endif
    return new_allocation;
  }

  int release_memory(void *to_drop) {
    int slot = compute_slot(to_drop);  // slice and dice to get bin number.
#ifdef DEBUG_MEMORY_CHECKER
    printf("removing mem %p from slot %d.\n", to_drop, slot);
#endif
    return _bins[slot].release_memory(to_drop);
  }

  //! this returns a newly created string with all current contents listed.
  /*! this returns a simple char * pointer that was created with malloc.
  the invoker *must* free the returned pointer.  we use malloc/free here to
  avoid creating a wacky report that contains a lot of new allocations for
  the report itself.  that seems like it could become a problem. */
  char *report_allocations() {
    // count how many allocations we have overall.
    int full_count = 0;
    for (int i = 0; i < _num_slots; i++) {
///if (_bins[i].count()) printf("%d adding count %d\n", i, _bins[i].count());
      full_count += _bins[i].count();
    }
///printf("full count is %d\n", full_count);
    // calculate a guess for how much space we need to show all of those.
    int alloc_size = full_count * SINGLE_LINE_SIZE_ESTIMATE + RESERVED_AREA;
    char *to_return = (char *)malloc(alloc_size);
    to_return[0] = '\0';
    if (full_count) {
      strcat(to_return, "===================\n");
      strcat(to_return, "Unfreed Allocations\n");
      strcat(to_return, "===================\n");
    }
    int curr_size = strlen(to_return);  // how much in use so far.
    for (int i = 0; i < _num_slots; i++) {
      _bins[i].dump_list(to_return, curr_size, alloc_size - RESERVED_AREA);
    }
    return to_return;
  }

  // this is fairly resource intensive, so don't dump the state out that often.
  char *text_form(bool show_outstanding) {
    char *to_return = NULL_POINTER;
    if (show_outstanding) {
      to_return = report_allocations();
    } else {
      to_return = (char *)malloc(RESERVED_AREA);
      to_return[0] = '\0';
    }
#ifdef MEMORY_CHECKER_STATISTICS
    char *temp_str = (char *)malloc(4 * SINGLE_LINE_SIZE_ESTIMATE);
    
    sprintf(temp_str, "=================\n");
    strcat(to_return, temp_str);
    sprintf(temp_str, "Memory Statistics\n");
    strcat(to_return, temp_str);
    sprintf(temp_str, "=================\n");
    strcat(to_return, temp_str);
    sprintf(temp_str, "Measurements taken across entire program runtime:\n");
    strcat(to_return, temp_str);
    sprintf(temp_str, "  %.0f new allocations.\n", _stat_new_allocations);
    strcat(to_return, temp_str);
    sprintf(temp_str, "  %.4f new Mbytes.\n",
        _stat_new_allocations_size / MEGABYTE);
    strcat(to_return, temp_str);
    sprintf(temp_str, "  %.0f freed deallocations.\n",
        _stat_freed_allocations);
    strcat(to_return, temp_str);
    sprintf(temp_str, "  %.4f freed Mbytes.\n",
        _stat_freed_allocations_size / MEGABYTE);
    strcat(to_return, temp_str);

    free(temp_str);
#endif
    return to_return;
  }

private:
  memory_bin *_bins;  //!< each bin manages a list of pointers, found by hash.
  int _num_slots;  //!< the number of hash slots we have.
};

//////////////

void memory_checker::construct()
{
  _mems = (allocation_memories *)malloc(sizeof(allocation_memories));
  _mems->construct(MAXIMUM_HASH_SLOTS);
  _unusable = false;
  _enabled = true;
}

void memory_checker::destruct()
{
  if (_unusable) return;  // already gone.
if (!_mems) printf("memory_checker::destruct being invoked twice!\n");

  // show some stats about memory allocation.
  char *mem_state = text_form(true);
  printf("%s", mem_state);
///uhhh  free(mem_state);
//the above free seems to totally die if we allow it to happen.

  _unusable = true;

  _mems->destruct();
  free(_mems);
  _mems = NULL_POINTER;
}

void *memory_checker::provide_memory(size_t size, char *file, int line)
{
  if (_unusable || !_enabled) return malloc(size);
  return _mems->provide_memory(size, file, line);
}

int memory_checker::release_memory(void *ptr)
{
  if (_unusable || !_enabled) {
    free(ptr);
    return common::OKAY;
  }
  return _mems->release_memory(ptr);
}

char *memory_checker::text_form(bool show_outstanding)
{
  if (_unusable) return strdup("already destroyed memory_checker!\n");
  return _mems->text_form(show_outstanding);
}

//////////////

#endif  // enable memory hook



