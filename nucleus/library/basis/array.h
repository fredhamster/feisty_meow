#ifndef ARRAY_CLASS
#define ARRAY_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : array                                                             *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1989-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "common_outcomes.h"
#include "definitions.h"
#include "enhance_cpp.h"
#include "functions.h"
#include "guards.h"
#include "outcome.h"

#define DEBUG_ARRAY
  // uncomment for the noisier debugging version.

namespace basis {

//! Represents a sequential, ordered, contiguous collection of objects.
/*!
  This object manages a contiguous array of memory to hold the objects it
  contains.  The objects to be stored must have a constructor with zero
  parameters, since the objects are stored in a C-style array (and array
  constructors cannot be given arguments to be passed to the objects).
  The objects must also either be flat (containing no pointers) or have an
  assignment operator (=) that correctly copies the deep contents.
    This class also provides an exponential growth mode for memory to reduce
  thrashing; this allows the size pre-allocated to double every time a new
  allocation is required during a resize. This causes the allocation to grow
  very swiftly, speeding up usage of frequently growing arrays, but this may
  not be desired for every array.
  terms used here...
    blank array: a array with some number of elements, but where those
       elements are objects that have been constructed using their default
       parameterless constructor.
    empty array: a array of zero elements.
*/

template <class contents>
class array : public virtual root_object
{
public:
  //! the flags specify how the array treats its contents and its length.
  enum specialc_flags {
    NO_SPECIAL_MODES = 0x0,  //!< do nothing extra; only valid by itself.
    SIMPLE_COPY = 0x1,  //!< the contents can be memcpy'd and are not deep.
    EXPONENTIAL_GROWTH = 0x2,  //!< length is doubled when reallocation happens.
    EXPONE = EXPONENTIAL_GROWTH,  //!< synonym for EXPONENTIAL_GROWTH.
    FLUSH_INVISIBLE = 0x4  //!< blanks out allocated but inaccessible elements.
  };

  DEFINE_CLASS_NAME("array");

  enum how_to_copy { NEW_AT_END, NEW_AT_BEGINNING, DONT_COPY }; 
    //!< An enumeration of ways to shift existing contents in an array.
    /*!< In the context of dynamically resizing an array of data, a range of
    new elements can be added (or removed for that matter) at the end of the
    existing space (NEW_AT_END), at the beginning of the existing space
    (NEW_AT_BEGINNING), or we might not care about the existing data when
    we perform the resize (DONT_COPY).  These methods impose a particular
    shift on the existing contents if we actually already have enough space
    for the new contents.  If there is space before the part of the array
    that's in use and we want NEW_AT_END, then the existing contents are
    jammed up into the front end of the array. */

  array(int number = 0, const contents *init = NIL,
          int flags = EXPONENTIAL_GROWTH | FLUSH_INVISIBLE);
    //!< Constructs an array with room for "number" objects.
    /*!< The initial contents are copied from "init" unless NIL is passed in
    instead.  If "init" is not NIL, then it must point to an array of objects
    that contains at least "number".  The "flags" are a value based on the
    special flags being added bit-wise.  If "flags" contains SIMPLE_COPY, then
    memmove() is used rather than using the C++ object's assignment operator.
    Note that SIMPLE_COPY will NOT work if the templated object has a regular
    constructor or assignment operator, since those methods will not be
    called on copying.  If the "flags" contain EXPONENTIAL_GROWTH, then the
    true allocation size will be doubled every time a new allocation is
    required.  when the FLUSH_INVISIBLE flag is included, then the array
    elements that go out of scope are returned to the state provided by
    the content's default constructor.  this ensures that if they ever come
    back into scope, they do not yet have any contents.  further, if the
    elements had any deep contents, those resources should be released. */

  array(const array<contents> &copy_from);
    //!< copies the contents & sizing information from "copy_from".

  virtual ~array();  //!< destroys the memory allocated for the objects.

  void reset(int number = 0, const contents *initial_contents = NIL);
    //!< Resizes this array and sets the contents from an array of contents.
    /*< If "initial_contents" is not NIL, then it must contain an array of
    "contents" with at least "number" objects in it.  If it is NIL, then
    the size of the array is changed but the contents are not.  note that
    any pre-existing elements that were previously out of range might still
    have their prior contents; the newly available elements are not necessarily
    "blank".  thus, before using them, ensure you store appropriate elements
    in those positions. */

  array &operator = (const array<contents> &copy_from);
    //!< Copies the array in "copy_from" into this.

  int length() const { return c_active_length; }
    //!< Returns the current reported length of the allocated C array.

  int last() const { return c_active_length - 1; }
    //!< Returns the last valid element in the array.

  int flags() const { return c_flags; }
    //!< Provides the raw flags value, without interpreting what it means.

  bool exponential() const { return c_flags & EXPONENTIAL_GROWTH; }
    //!< Returns true if this allocator will grow exponentially on resize.

  bool simple() const { return c_flags & SIMPLE_COPY; }
    //!< Reports whether the templated object is a simple type or not.

  const contents &get(int index) const;
    //!< Accesses individual objects stored in "this" at the "index" position.
    /*!< If the index is out of range, then a bogus reference (to internally
    held garbage) is returned. */
  contents &use(int index);
    //!< A non-constant version of get(); the returned object can be modified.

  const contents &operator [] (int index) const { return get(index); }
    //!< Synonym for get that provides the expected array indexing syntax.
  contents &operator [] (int index) { return use(index); }
    //!< Synonym for use that provides the expected array indexing syntax.

  outcome put(int index, const contents &to_put);
    //!< Stores an object at the index "index" in the array.
    /*!< The outcome is "OUT_OF_RANGE" if the index does not exist. */

  array concatenation(const array &to_concatenate) const;
    //!< Returns the concatenation of "this" and the array "to_concatenate".
  array concatenation(const contents &to_concatenate) const;
    //!< Returns the concatenation of "this" and the object "to_concatenate".

  array &concatenate(const array &to_concatenate);
    //!< Appends the array "to_concatenate" onto "this" and returns "this".
  array &concatenate(const contents &to_concatenate);
    //!< Appends the object "to_concatenate" onto "this" and returns "this".
  array &concatenate(const contents *to_concatenate, int length);
    //!< Concatenates a C-array "to_concatenate" onto "this" and returns "this".
    /*!< There must be at least "length" elements in "to_concatenate". */

  array operator + (const array &to_cat) const
          { return concatenation(to_cat); }
    //!< Synonym for concatenation.
  array operator + (const contents &to_concatenate) const
          { return concatenation(to_concatenate); }
    //!< Synonym for concatenation.
  array &operator += (const array &to_concatenate)
          { return concatenate(to_concatenate); }
    //!< Synonym for concatenate that modifies "this".
  array &operator += (const contents &to_concatenate)
          { return concatenate(to_concatenate); }
    //!< Synonym for concatenate that modifies "this".

  const contents *observe() const { return c_offset; }
    //!< Returns a pointer to the underlying C array of data.
    /*!< The array contains "length()" number of objects in it.  BE CAREFUL.
    This version is a constant peek at that pointer. */
  contents *access() { return c_offset; }
    //!< A non-constant access of the underlying C-array.  BE REALLY CAREFUL.

  void swap_contents(array<contents> &other);
    //!< Exchanges the contents of "this" and "other".
    /*!< No validation is performed but this should always succeed given
    arrays constructed properly. */

  void snarf(array &new_contents);
    //!< Drops "this" array's contents into the dustbin and uses "new_contents".
    /*!< Afterwards, "new_contents" is an empty array and what used to be
    stored there is now in "this" instead. */

  array subarray(int start, int end) const;
    //!< Returns the array segment between the indices "start" and "end".
    /*!< This is all characters from "start" to "end" inclusively, as long as
    those values are valid for this array.   Even then, an intelligent default
    is usually assumed if the indices are out of range. */

  outcome insert(int index, int new_indices);
    //!< Adds "new_indices" new positions for objects into the array at "index".

  outcome overwrite(int index, const array &write_with, int count = -1);
    //!< Stores the array "write_with" into the current array at the "index".
    /*!< The current contents are overwritten with "write_with".  If the
    index is invalid, then OUT_OF_RANGE is returned.  If the "write_with"
    array cannot fit due to the boundaries of "this" array, then only the
    portion that can fit is used.  If "count" is negative, the whole
    "write_with" array is used; otherwise, only "count" elements are used. */

  outcome stuff(int length, contents *to_stuff) const;
    //!< Copies at most "length" elements from this into the array "to_stuff".
    /*!< This call will fail disastrously if "length" is larger than the
    array "to_stuff"'s allocated length. */

  outcome resize(int new_size, how_to_copy way = NEW_AT_END);
    //!< Changes the size of the C array to "new_size".
    /*!< If "way" is NEW_AT_END and the array grows, then new space is added
    at the end and the prefix of the array is the same as the old array.  If
    the "way" is NEW_AT_END, but the array shrinks, then the new array is a
    prefix of the old array.  If "way" is NEW_AT_BEGINNING and the array
    grows, then the suffix of the array is the same as the old one and the
    space is added at the beginning.  if the "way" is NEW_AT_BEGINNING but the
    array shrinks, then the new array is a suffix of the old array.  if "way"
    is DONT_COPY, then the old contents are not copied.  keep in mind that
    any newly visible elements can be in an arbitrary state and will not
    necessarily be freshly constructed. */

  outcome zap(int start, int end);
    //!< Deletes from "this" the objects inclusively between "start" and "end".
    /*!< C-array conventions are used (0 through length()-1 are valid if
    length() > 0).  If either index is out of range, then a default is
    assumed. */

  outcome shrink();
    //!< Cuts loose any allocated space that is beyond the real length.

  outcome retrain(int new_size, const contents *to_copy);
    //!< Resizes the C array and stuffs it with the contents in "to_copy".

  enum shift_directions { TO_LEFT, TO_RIGHT };
    //!< All the real contents can be shifted to either the left or right.

  void shift_data(shift_directions where);
    //!< The valid portion of the array is moved to the left or right.
    /*!< This means that the unused space (dictated by the offset where the
    data starts) will be adjusted.  This may involve copying the data. */

  // These are gritty internal information methods and should not be used
  // except by appropriately careful code.
  int internal_real_length() const { return c_real_length; }
    //!< Gritty Internal: the real allocated length.
  int internal_offset() const { return int(c_offset - c_mem_block); }
    //!< Gritty Internal: the offset from real start to stored data.
  const contents *internal_block_start() const { return c_mem_block; }
    //!< Gritty Internal: constant peek at the real allocated pointer.
  contents *internal_block_start() { return c_mem_block; }
    //!< Gritty Internal: the real allocated pointer made accessible.
  contents * const *internal_offset_mem() const { return &c_offset; }
    //!< Gritty Internal: the start of the actual stored data.

private:
  int c_active_length; //!< the number of objects reported to be in the array.
  int c_real_length;  // the real number of objects that can be stored.
  contents *c_mem_block;  //!< a pointer to the objects held for this array.
  contents *c_offset;  //!< the beginning of the useful part of the memory block.
  int c_flags;  //!< records the special characteristics of this array.

  outcome allocator_reset(int initial_elements, int blocking);
    //!< Allocates space for the "initial_elements" plus the "blocking" factor.
    /*!< This resets the C-array to have "initial_elements" indices, plus an
    extra pre-allocation of "blocking" that leaves room for expansion.  Any
    prior contents are whacked. */
};

//////////////

//! A simple object that wraps a templated array of ints.
class int_array : public array<int>
{
public:
  int_array(int number = 0, const int *initial_contents = 0)
      : root_object(),
        array<int>(number, initial_contents, SIMPLE_COPY | EXPONE) {}
    //!< Constructs an array of "number" integers.
    /*!< creates a list of ints based on an initial "number" of entries and
    some "initial_contents", which should be a regular C array of ints
    with at least as many entries as "number". */
};

//////////////

//! An array of double floating point numbers.
class double_array : public array<double>
{
public:
  double_array(int len = 0, double *data = NIL)
      : root_object(),
        array<double>(len, data, SIMPLE_COPY | EXPONE) {}
  double_array(const array<double> &to_copy) : array<double>(to_copy) {}
};

//////////////

// implementation code, much longer methods below...

// GOALS:
//
// 1) provide a slightly smarter allocation method for C arrays and other
//    contiguous-storage container classes with better speed and reduced memory
//    fragmentation through pre-allocation.  this can reduce memory thrashing
//    when doing appends and inserts that can be granted with previously
//    allocated, but unused, space.
// 2) clean-up bounds failure cases in functions that return a reference by
//    always having at least one bogus element in the array for returns.  this
//    really just requires that we never allow our hidden real length of the
//    array to be zero.

template <class contents>
array<contents>::array(int num, const contents *init, int flags)
: root_object(), c_active_length(0), c_real_length(0), c_mem_block(NIL), c_offset(NIL), c_flags(flags)
{
  if (c_flags > 7) {
#ifdef DEBUG_ARRAY
    throw "error: array::constructor: error in parameters!  still passing a block size?";
#endif
    c_flags = EXPONE | FLUSH_INVISIBLE;
      // drop simple copy, since the caller doesn't know what they're doing.
  }

  allocator_reset(num, 1);  // get some space.
  retrain(num, init);  // plug in their contents.
}

template <class contents>
array<contents>::array(const array<contents> &cf)
: root_object(), c_active_length(0), c_real_length(0), c_mem_block(NIL), c_offset(NIL), c_flags(cf.c_flags)
{
  allocator_reset(cf.c_active_length, 1);  // get some space.
  operator = (cf);  // assignment operator does the rest.
}

template <class contents>
array<contents>::~array()
{
  c_offset = NIL;
  if (c_mem_block) delete [] c_mem_block;
  c_mem_block = NIL;
  c_active_length = 0;
  c_real_length = 0;
}

template <class contents>
void array<contents>::reset(int num, const contents *init)
{ retrain(num, init); }

template <class contents>
array<contents> &array<contents>::operator =(const array &cf)
{
  if (this == &cf) return *this;
  c_flags = cf.c_flags;  // copy the flags coming in from the other object.
  // prepare the array for retraining...
  c_offset = c_mem_block;  // slide the offset back to the start.
  c_active_length = 0;  // the length gets reset also.
  retrain(cf.c_active_length, cf.observe());
  return *this;
}

template <class contents>
contents &array<contents>::use(int index)
{
  bounds_return(index, 0, this->last(), bogonic<contents>());
  return this->access()[index];
}

template <class contents>
const contents &array<contents>::get(int index) const
{
  bounds_return(index, 0, this->last(), bogonic<contents>());
  return this->observe()[index];
}

template <class contents>
array<contents> &array<contents>::concatenate(const array &s1)
{
  // check whether there's anything to concatenate.
  if (!s1.length()) return *this;
  if (this == &s1) {
    // make sure they don't concatenate this array to itself.
    return concatenate(array<contents>(*this)); 
  }
  int old_len = this->length();
  resize(this->length() + s1.length(), NEW_AT_END);
  overwrite(old_len, s1);
  return *this;
}

template <class contents>
array<contents> &array<contents>::concatenate(const contents &to_concatenate)
{
  resize(this->length() + 1, NEW_AT_END);
  if (!this->simple())
    this->access()[this->last()] = to_concatenate;
  else
    memcpy(&(this->access()[this->last()]), &to_concatenate, sizeof(contents));
  return *this;
}

template <class contents>
array<contents> &array<contents>::concatenate(const contents *to_concatenate,
    int length)
{
  if (!length) return *this;  // nothing to do.
  const int old_len = this->length();
  resize(this->length() + length, NEW_AT_END);
  if (!this->simple())
    for (int i = 0; i < length; i++)
      this->access()[old_len + i] = to_concatenate[i];
  else
    memcpy(&(this->access()[old_len]), to_concatenate,
        length * sizeof(contents));
  return *this;
}

template <class contents>
array<contents> array<contents>::concatenation(const array &s1) const
{
  // tailor the return array to the new size needed.
  array<contents> to_return(this->length() + s1.length(), NIL, s1.c_flags);
  to_return.overwrite(0, *this);  // put the first part into the new array.
  to_return.overwrite(this->length(), s1);  // add the second segment.
  return to_return;
}

template <class contents>
array<contents> array<contents>::concatenation(const contents &s1) const
{
  array<contents> to_return(this->length() + 1, NIL, c_flags);
  to_return.overwrite(0, *this);
  if (!this->simple())
    to_return.access()[to_return.last()] = s1;
  else
    memcpy(&(to_return.access()[to_return.last()]), &s1, sizeof(contents));
  return to_return;
}

template <class contents>
array<contents> array<contents>::subarray(int start, int end) const
{
  bounds_return(start, 0, this->last(), array<contents>(0, NIL, c_flags));
  bounds_return(end, 0, this->last(), array<contents>(0, NIL, c_flags));
  if (start > end) return array<contents>(0, NIL, c_flags);
  return array<contents>(end - start + 1, &(this->observe()[start]), c_flags);
}

template <class contents>
void array<contents>::swap_contents(array &other)
{
  if (this == &other) return;  // already swapped then, i suppose.
  swap_values(this->c_active_length, other.c_active_length);
  swap_values(this->c_real_length, other.c_real_length);
  swap_values(this->c_offset, other.c_offset);
  swap_values(this->c_mem_block, other.c_mem_block);
  swap_values(this->c_flags, other.c_flags);
}

template <class contents>
outcome array<contents>::shrink()
{
  if (!c_mem_block) return common::OUT_OF_MEMORY;
  if (c_active_length == c_real_length) return common::OKAY;  // already just right.
  array new_holder(*this);
    // create a copy of this object that is just the size needed.
  swap_contents(new_holder);
    // swap this object with the copy, leaving the enlarged version behind
    // for destruction.
  return common::OKAY;
}

template <class contents>
outcome array<contents>::stuff(int lengthx, contents *to_stuff) const
{
  if (!lengthx || !this->length()) return common::OKAY;
  int copy_len = minimum(lengthx, this->length());
  if (!this->simple()) {
    for (int i = 0; i < copy_len; i++)
      to_stuff[i] = this->observe()[i];
  } else {
    memcpy(to_stuff, this->observe(), copy_len * sizeof(contents));
  }
  return common::OKAY;
}

template <class contents>
outcome array<contents>::overwrite(int position,
    const array<contents> &write_with, int count)
{
  if (!count) return common::OKAY;
  if ( (this == &write_with) || !this->length() || !write_with.length())
    return common::BAD_INPUT;
  bounds_return(position, 0, this->last(), common::OUT_OF_RANGE);
  if ( negative(count) || (count > write_with.length()) )
    count = write_with.length();
  if (position > this->length() - count)
    count = this->length() - position;
  if (!this->simple()) {
    for (int i = position; i < position + count; i++)
      this->access()[i] = write_with.observe()[i - position];
  } else {
    memcpy(&(this->access()[position]), write_with.observe(), 
        count * sizeof(contents));
  }
  return common::OKAY;
}

template <class contents>
outcome array<contents>::allocator_reset(int initial, int blocking)
{
//  FUNCDEF("allocator_reset")
  if (blocking < 1) {
#ifdef DEBUG_ARRAY
    throw "error: array::allocator_reset: has bad block size";
#endif
    blocking = 1;
  }
  if (initial < 0) initial = 0;  // no antimatter arrays.
  if (c_mem_block) {
    // remove old contents.
    delete [] c_mem_block;
    c_mem_block = NIL;
    c_offset = NIL;
  }
  c_active_length = initial;  // reset the length to the reporting size.
  c_real_length = initial + blocking;  // compute the real length.
  if (c_real_length) {
    c_mem_block = new contents[c_real_length];
    if (!c_mem_block) {
      // this is an odd situation; memory allocation didn't blow out an
      // exception, but the memory block is empty.  let's consider that
      // a fatal error; we can't issue empty objects.
      throw common::OUT_OF_MEMORY;
    }
    c_offset = c_mem_block;  // reset offset to start of array.
  }
  return common::OKAY;
}

template <class contents>
void array<contents>::shift_data(shift_directions where)
{
  if (where == TO_LEFT) {
    // we want to end up with the data jammed up against the left edge.  thus
    // we need the offset to be zero bytes from start.
    if (c_offset == c_mem_block)
      return;  // offset already at start, we're done.
    // well, we need to move the data.
    if (simple()) {
      memmove(c_mem_block, c_offset, c_active_length * sizeof(contents));
    } else {
      for (contents *ptr = c_offset; ptr < c_offset + c_active_length; ptr++)
        c_mem_block[ptr - c_offset] = *ptr;
    }
    c_offset = c_mem_block;  // we've ensured that this is correct.
    if (c_flags & FLUSH_INVISIBLE) {
      // we need to clean up what might have had contents previously.
//      for (contents *p = c_mem_block + c_active_length; p < c_mem_block + c_real_length; p++)
//        *p = contents();
    }
  } else {
    // we want to move the data to the right, so the offset should be the
    // difference between the real length and the length.
    if (c_offset == c_mem_block + c_real_length - c_active_length)
      return;  // the offset is already the right size.
    if (simple()) {
      memmove(&c_mem_block[c_real_length - c_active_length], c_offset, c_active_length * sizeof(contents));
    } else {
      for (int i = c_real_length - 1; i >= c_real_length - c_active_length; i--)
        c_mem_block[i] = c_offset[i - c_real_length + c_active_length];
    }
    c_offset = c_mem_block + c_real_length - c_active_length;  // we've now ensured this.
    if (c_flags & FLUSH_INVISIBLE) {
      // we need to clean up the space on the left where old contents might be.
//      for (contents *p = c_mem_block; p < c_offset; p++)
//        *p = contents();
    }
  }
}

template <class contents>
outcome array<contents>::resize(int new_size, how_to_copy way)
{
///  FUNCDEF("resize");
  if (new_size < 0) new_size = 0;  // we stifle this.
  if (new_size == c_active_length) {
    // nothing much to do.
    return common::OKAY;
  }
  // okay, now we at least know that the sizes are different.  we save all the
  // old information about the array prior to this resizing.
  contents *old_s = c_mem_block;  // save the old contents...
  const int old_len = c_active_length;  // and length.
  contents *old_off = c_offset;  // and offset.
  bool delete_old = false;  // if true, old memory is whacked once it's copied.

//hmmm: wasn't there a nice realization that we could bail out early in
//      the case where the size is already suffcient?  there seems to be
//      an extraneous copy case here.
//      also it would be nice to have better, more descriptive names for the
//      variables here since they are not lending themselves to understanding
//      the algorithm.

  // we check whether there's easily enough space in the array already.
  // if not, then we have some more decisions to make.
  if (c_real_length - (old_off - old_s) < new_size) {
    // well, there's not enough space with the current space and offset.
    if (c_real_length < new_size) {
      // there's really not enough space overall, no fooling.  we now will
      // create a new block.
      c_mem_block = NIL;  // zero out the pointer so reset doesn't delete it.
      delete_old = true;
      int blocking = 1;
      if (exponential()) blocking = new_size + 1;
      outcome ret = allocator_reset(new_size, blocking);
      if (ret != common::OKAY) {
        // failure, but don't forget to whack the old glob.
#ifdef DEBUG_ARRAY
        throw "error: array::resize: saw array reset failure";
#endif
        delete [] old_s;
        return ret;
      }
      // fall out to the copying phase, now that we have some fresh memory.
    } else {
      // there is enough space if we shift some things around.
      const int size_difference = new_size - c_active_length;
        // we compute how much space has to be found in the array somewhere
        // to support the new larger size.
      if (way == DONT_COPY) {
        // simplest case; just reset the offset appropriately so the new_size
        // will fit.
        c_offset = c_mem_block;
        c_active_length = new_size;
      } else if (way == NEW_AT_BEGINNING) {
        // if the new space is at the beginning, there are two cases.  either
        // the new size can be accomodated by the current position of the
        // data or the data must be shifted to the right.
        if (c_offset - c_mem_block < size_difference) {
          // we need to shift the data over to the right since the offset isn't
          // big enough for the size increase.
          shift_data(TO_RIGHT);  // resets the offset appropriately.
        }
        // now we know that the amount of space prior to the real data
        // is sufficient to hold what new space is needed.  we just need to
        // shift the offset back somewhat.
        c_offset -= size_difference;
        c_active_length = new_size;
      } else {
        // better only be three ways to do this; we're now assuming the new
        // space should be at the end (NEW_AT_END).
        // now that we're here, we know there will be enough space if we shift
        // the block to the left, but we DO NEED to do this.  if we didn't need
        // to shift the data, then we would find that:
        //     c_real_length - old_off >= new_size
        // which is disallowed by the guardian conditional around this area.
        shift_data(TO_LEFT);  // resets the offset for us.
        c_active_length = new_size;
      }
      // we have ensured that we had enough space and we have already shifted
      // the data around appropriately.  we no longer need to enter the next
      // block where we would copy data around if we had to.  it has become
      // primarily for cases where either we have to copy data because we
      // have new storage to fill or where we are shrinking the array.
      return common::OKAY;
    }
  }

  // the blob of code below is offset invariant.  by the time we reach here,
  // the array should be big enough and the offset should be okay.
  c_active_length = new_size;  // set length to the new reporting size.
  if (way != DONT_COPY) {
    int where = 0;  // offset for storing into new array.
    bool do_copy = false;  // if true, then we need to move the data around.
    contents *loopc_offset_old = old_off;  // offset into original object.
    // we should only have to copy the memory in one other case besides our
    // inhabiting new memory--when we are asked to resize with the new stuff
    // at the beginning of the array.  if the new space is at the end, we're
    // already looking proper, but if the new stuff is at the beginning, we
    // need to shift existing stuff downwards.
    if (way == NEW_AT_BEGINNING) {
      where = new_size - old_len;  // move up existing junk.
      if (where) do_copy = true;  // do copy, since it's not immobile.
      if (where < 0) {
        // array shrank; we need to do the loop differently for starting
        // from the beginning.  we skip to the point in the array that our
        // suffix needs to start at.
        loopc_offset_old -= where;
        where = 0;  // reset where so we don't have negative offsets.
      }
    }
    const int size_now = minimum(old_len, c_active_length);
    if (delete_old || do_copy) {
      contents *offset_in_new = c_offset + where;
      contents *posn_in_old = loopc_offset_old;
      if (simple()) {
        // memmove should take care of intersections.
        memmove(offset_in_new, posn_in_old, size_now * sizeof(contents));
      } else {
        // we need to do the copies using the object's assignment operator.
        if (new_size >= old_len) {
          for (int i = size_now - 1; i >= 0; i--)
            offset_in_new[i] = posn_in_old[i];
        } else {
          for (int i = 0; i < size_now; i++)
            offset_in_new[i] = posn_in_old[i];
        }
      }

      // we only want to flush the obscured elements when we aren't already
      // inhabiting new space.
      if ( (c_flags & FLUSH_INVISIBLE) && !delete_old) {
        // clear out the space that just went out of scope.  we only do this
        // for the flushing mode and when we aren't starting from a fresh
        // pointer (i.e., delete_old isn't true).
        if (new_size < old_len) {
//          for (contents *p = posn_in_old; p < offset_in_new; p++)
//            *p = contents();
        }
      }
    }
  }
  if (delete_old) delete [] old_s;
  return common::OKAY;
}

template <class contents>
outcome array<contents>::retrain(int new_len, const contents *to_set)
{
///  FUNCDEF("retrain")
  if (new_len < 0) new_len = 0;  // stifle that bad length.
#ifdef DEBUG_ARRAY
  if (to_set && (c_mem_block >= to_set) && (c_mem_block < to_set + new_len) ) {
    throw "error: array::retrain: ranges overlap in retrain!";
  }
#endif
  outcome ret = resize(new_len, DONT_COPY);
  if (ret != common::OKAY) return ret;
#ifdef DEBUG_ARRAY
  if (new_len != c_active_length) {
    throw "error: array resize set the wrong length";
  }
#endif
  if (to_set) {
    if (simple()) 
      memcpy(c_offset, to_set, c_active_length * sizeof(contents));
    else
      for (int i = 0; i < c_active_length; i++)
        c_offset[i] = to_set[i];
  } else {
    if (c_flags & FLUSH_INVISIBLE) {
      // no contents provided, so stuff the space with blanks.
//      for (int i = 0; i < c_active_length; i++) c_offset[i] = contents();
    }
  }
  if (c_flags & FLUSH_INVISIBLE) {
//    for (contents *ptr = c_mem_block; ptr < c_offset; ptr++)
//      *ptr = contents();
//    for (contents *ptr = c_offset + c_active_length; ptr < c_mem_block + c_real_length; ptr++)
//      *ptr = contents();
  }
  return common::OKAY;
}

template <class contents>
outcome array<contents>::zap(int position1, int position2)
{
  if (position1 > position2) return common::OKAY;
  bounds_return(position1, 0, c_active_length - 1, common::OUT_OF_RANGE);
  bounds_return(position2, 0, c_active_length - 1, common::OUT_OF_RANGE);
  if (!position1) {
    // if they're whacking from the beginning, we just reset the offset.
    c_offset += position2 + 1;
    c_active_length -= position2 + 1;
    return common::OKAY;
  }
  const int difference = position2 - position1 + 1;
  // copy from just above position2 down into position1.
  if (simple()) {
    if (c_active_length - difference - position1 > 0)
      memmove(&c_offset[position1], &c_offset[position1 + difference],
          (c_active_length - difference - position1) * sizeof(contents));
  } else {
    for (int i = position1; i < c_active_length - difference; i++)
      c_offset[i] = c_offset[i + difference];
  }

  outcome ret = resize(c_active_length - difference, NEW_AT_END);
    // chop down to new size.
#ifdef DEBUG_ARRAY
  if (ret != common::OKAY) {
    throw "error: array::zap: resize failure";
    return ret;
  }
#endif
  return ret;
}

template <class contents>
outcome array<contents>::insert(int position, int elem_to_add)
{
  if (position < 0) return common::OUT_OF_RANGE;
  if (position > this->length())
    position = this->length();
  if (elem_to_add < 0) return common::OUT_OF_RANGE;
  how_to_copy how = NEW_AT_END;
  if (position == 0) how = NEW_AT_BEGINNING;
  resize(this->length() + elem_to_add, how);

  // if the insert wasn't at the front, we have to copy stuff into the new
  // locations.
  if (how == NEW_AT_END) {
    const contents simple_default_object = contents();
    if (!this->simple()) {
      for (int i = this->last(); i >= position + elem_to_add; i--)
        this->access()[i] = this->observe()[i - elem_to_add];
      for (int j = position; j < position + elem_to_add; j++)
        this->access()[j] = simple_default_object;
    } else {
      memmove(&(this->access()[position + elem_to_add]),
          &(this->observe()[position]), (this->length() - position
          - elem_to_add) * sizeof(contents));
      for (int j = position; j < position + elem_to_add; j++)
        memcpy(&this->access()[j], &simple_default_object, sizeof(contents));
    }
  }
  return common::OKAY;
}

template <class contents>
outcome array<contents>::put(int index, const contents &to_put)
{
  bounds_return(index, 0, this->last(), common::OUT_OF_RANGE);
  if (!this->simple())
    this->access()[index] = to_put;
  else
    memcpy(&(this->access()[index]), &to_put, sizeof(contents));
  return common::OKAY;
}

template <class contents>
void array<contents>::snarf(array<contents> &new_contents)
{
  if (this == &new_contents) return;  // no blasting own feet off.
  reset();  // trash our current storage.
  swap_contents(new_contents);
}

/*
//! a simple wrapper of an array of char *, used by portable::break_line().
class char_star_array : public array<char *>
{
public:
  char_star_array() : array<char *>(0, NIL, SIMPLE_COPY | EXPONE
      | FLUSH_INVISIBLE) {}
  ~char_star_array() {
    // clean up all the memory we're holding.
    for (int i = 0; i < length(); i++) {
      delete [] (use(i));
    }
  }
};
*/

} //namespace

#undef static_class_name

#endif

