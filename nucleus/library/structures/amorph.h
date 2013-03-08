#ifndef AMORPH_CLASS
#define AMORPH_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : amorph                                                            *
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

#include "object_packers.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/contracts.h>
#include <basis/guards.h>

//! A dynamic container class that holds any kind of object via pointers.
/*!
  The object is considered owned by the amorph unless re-acquired from it,
  and will be deleted when the amorph is destroyed.

  An amorph has a specified number of fields at any one time, but the number
  can be changed with "zap", "insert" and "adjust".  Fields in the amorph
  are either full or empty, where an empty field in the amorph has NIL as
  its content.  "put" adds a new field to the amorph.  "get" retrieves the
  contents of a field as a constant.  "acquire" is used to check a field
  out of the amorph, meaning that the amorph no longer possesses that field.
  The legal range of indices in an amorph is from 0 through
  "elements() - 1".  In general, a range violation for an index is
  treated as an invalid request and is ignored (although BAD_INDEX is
  returned by functions with compatible return values).

  Model:

  The policy followed in amorph is that once an object is checked in with
  "put" or "append", then the amorph owns that object.  The object must not
  be destroyed externally, because the amorph will automatically destroy
  the object when either: 1) the amorph itself is destroyed, or 2) another
  object is entered into the same field.  If the stored object must be
  destroyed externally, then it should be checked out of the amorph with
  "acquire"; after that, the pointer may be deleted.  "get" and "borrow"
  return a pointer to the stored item while leaving it checked in to the
  amorph.  it is safe to modify what was "borrow"ed or to look at what one
  "get"s, but do not destroy the pointers returned from either method.
*/

namespace structures {

template <class contents>
class amorph : private basis::array<contents *>
{
public:
  amorph(int elements = 0);
    //!< constructs an amorph capable of holding "elements" pointers.

  ~amorph();

  int elements() const { return this->length(); }
    //!< the maximum number of elements currently allowed in this amorph.

  int valid_fields() const { return _fields_used; }
    //!< Returns the number of fields that have non-NIL contents.
    /*!< This might be different from the number of total elements. */

  void adjust(int new_max);
    //!< Changes the maximum number of elements for this amorph.
    /*!< If the new number is smaller than the original, then the fields at
    index "new_maximum" and upwards are thrown away.  existing fields are
    kept. */

  void resize(int new_maximum = 0);
    //!< like adjust but doesn't keep existing contents.

  void reset() { this->resize(0); }
  	//!< cleans out all of the contents.

  basis::outcome put(int field, const contents *data);
    //!< Enters an object into the field at index "field" in the amorph.
    /*!< If "data" is NIL, then the field is cleared.  The amorph considers the
    pointer "data" to be its own property after put is invoked; "data"
    should not be destructed since the amorph will automatically do so.
    This restriction does not hold if the object is checked back out of
    the amorph with acquire(). */

  basis::outcome append(const contents *data);
    //!< puts "data" on the end of this amorph.
    /*!< adds an element to the end of the amorph by increasing the amorph size
    (with "adjust") and putting the element into the new spot (with "put"). */

  basis::outcome operator += (const contents *data) { return append(data); }
    //!< a synonym for append.

  //! Returns a constant pointer to the information at the index "field".
  /*! If no information is stored or the field is out range, then NIL is
  returned. */
  const contents *get(int field) const;
  //! Returns a pointer to the information at the index "field".
  /*! Also returns NIL for invalid indexes.  DO NOT destroy the returned
  pointer; it is still owned by the amorph. */
  contents *borrow(int field);

  //! synonym for get.
  const contents *operator [] (int field) const { return get(field); }
  //! synonym for borrow.
  contents *operator [] (int field) { return borrow(field); }

  contents *acquire(int field);
    //!< Retrieves a "field" from the amorph, taking responsibility for it back.
    /*!< This function is similar to get, except that the contents are pulled
    out of the amorph.  The contents will no longer be destroyed when the
    amorph is destroyed.  To store the modified contents again, use put,
    and then the amorph will take over management of the contents again.
    Note that the index is not zapped with this function; the acquired
    "field" will simply hold a null pointer. */

  basis::outcome clear(int field);
    //!< Clears the contents of the field specified.
    /*!< Clearing an empty field has no effect.  Clearing an invalid field has
    no effect.  NOTE: clearing the contents means that the contents are
    destroyed, not just disconnected. */

  void clear_all();
    //!< Clears every field in the amorph.

  basis::outcome zap(int start, int end);
    //!< Removes a range of indices from the amorph.
    /*!< This does not just clear the field associated with the specified index
    as "clear" does, it actually changes the number of total elements by
    removing the indices from the amorph.  The new amorph contains the old
    elements up to just before the "start" and from the "end" + 1 through the
    end of the amorph.  AMORPH_BAD_INDEX is returned if either index is out of
    range.  If the zap succeeds, then AMORPH_OKAY is returned, even if the
    "end" is less than the "start". */

  basis::outcome insert(int position, int lines_to_add);
    //!< Adds "lines_to_add" indices to the amorph at the index "position".
    /*!< If "lines_to_add" is non-positive, the request is ignored.  Inserting
    at a position beyond the bounds of the array is ignored, but a position AT
    elements() is allowed (it is an append...). */

  int find_empty(basis::outcome &o) const;
    //!< Returns the index of a free field if there are any.
    /*!< The returned index is invalid if the "o" is IS_FULL. */

  const contents *next_valid(int &field) const;
    //!< Returns the contents of the next valid element at or after "field".
    /*!< "field" is set to the location where an entry was found, if one was
    actually found.  If none exists at "field" or after it, then NIL is
    returned. */

  int find(const contents *to_locate, basis::outcome &o);
    //!< Searches the amorph for the contents specified.
    /*!< Since only pointers to the contents are maintained, the search is
    based on finding a pointer in the amorph that is identical to "to_locate".
    if "o" is OKAY, then the index of the entry is returned.  If
    "o" is NOT_FOUND, then the contents are not present. */

  void swap_contents(amorph<contents> &other);
    //!< Exchanges the contents of "this" and "other".
    /*!< No validation is performed but this should always succeed given
    amorphs that are constructed properly. */

private:
  int _fields_used;  //!< The number of fields currently full of info.

  void check_fields(const char *where) const;
    //!< Crashes out if the field count is wrong.
    /*!< This is only used for the debugging version. */

  void set_nil(int start, int end);
    // Puts NIL in the indices between start and end.
    /*!< Does not delete whatever used to reside there; it just sets the
    pointers to NIL. */

  // not to be used: amorphs should not be copied because it is intended that
  // they support storing heavyweight objects that either have no copy
  // constructors or have high-cost copy constructors.
  amorph(const amorph &to_copy) {_fields_used = 0;}  //!< not to be used.
  amorph &operator = (const amorph &to_copy) { return *this; }
    //!< not to be used.
};

//////////////

// these extensions are phrased as macros to avoid including the headers
// necessary for compiling them.  to use them, just put the macro name in
// the file where the template is needed.

//////////////

//! This can be used when the templated object has a copy constructor.
/*! this makes the "to_assign" into a copy of the "to_copy" amorph. */
template <class contents>
void amorph_assign(amorph<contents> &to_assign,
    const amorph<contents> &to_copy);

//////////////

//! support for packing an amorph into an array of bytes.
/*!
  this can be used when the underlying object is based on packable or
  supports the same pack/unpack methods.  note that if there are empty spots in
  the amorph, they are not packed.  when the packed form is unpacked again, it will
  have only the first N elements set, where N is the number of non-empty items.
*/
template <class contents>
void amorph_pack(basis::byte_array &packed_form, const amorph<contents> &to_pack);

//! unpacks the amorph from an array of bytes.
template <class contents>
bool amorph_unpack(basis::byte_array &packed_form, amorph<contents> &to_unpack);

//! reports how large the packed form will be.
template <class contents>
int amorph_packed_size(const amorph<contents> &to_pack);

// implementation for longer methods...

//#define DEBUG_AMORPH
  // uncomment to enable more testing, as well as complaints on errors.

#undef static_class_name
#define static_class_name() "amorph"
  // used in bounds_halt macro.

#undef AMO_ALERT
#ifdef DEBUG_AMORPH
  #include <basis/astring.h>
  #define AMO_ALERT(a, b, c) basis::throw_error(basis::astring(a), basis::astring(func), basis::astring(b) + " -- " + c)
  #define CHECK_FIELDS check_fields(func)
#else
  #define AMO_ALERT(a1, a2, a3) {}
  #define CHECK_FIELDS { if (!func) {} }
#endif

//////////////

template <class contents>
amorph<contents>::amorph(int elements)
: basis::array<contents *>(elements, NIL, basis::array<contents *>::SIMPLE_COPY
      | basis::array<contents *>::EXPONE | basis::array<contents *>::FLUSH_INVISIBLE),
  _fields_used(0)
{
  FUNCDEF("constructor");
  set_nil(0, elements - 1);
  CHECK_FIELDS;
}

template <class contents>
amorph<contents>::~amorph()
{
  FUNCDEF("destructor");
  CHECK_FIELDS;
  clear_all();
}

template <class contents>
void amorph<contents>::set_nil(int start, int end)
{
  for (int i = start; i <= end; i++)
    basis::array<contents *>::put(i, (contents *)NIL);
}

template <class contents>
void amorph<contents>::check_fields(const char *where) const
{
  FUNCDEF("check_fields");
  int counter = 0;
  for (int i = 0; i < elements(); i++)
    if (basis::array<contents *>::get(i)) counter++;
  if (_fields_used != counter) {
    AMO_ALERT("amorph", basis::a_sprintf("check_fields for %s", where),
        "error in _fields_used count");
  }
}

template <class contents>
void amorph<contents>::resize(int new_maximum)
{
  FUNCDEF("reset");
  CHECK_FIELDS;
  adjust(new_maximum);
  clear_all();
}

template <class contents>
void amorph<contents>::clear_all()
{
  FUNCDEF("clear_all");
  CHECK_FIELDS;
  for (int i = 0; i < elements(); i++) clear(i);
}

template <class contents>
basis::outcome amorph<contents>::append(const contents *data)
{
  FUNCDEF("append");
  CHECK_FIELDS;
  adjust(elements() + 1);
  return put(elements() - 1, (contents *)data);
}

template <class contents>
const contents *amorph<contents>::get(int field) const
{
  FUNCDEF("get");
  CHECK_FIELDS;
  bounds_return(field, 0, elements() - 1, NIL);
  return basis::array<contents *>::observe()[field];
}

template <class contents>
void amorph<contents>::adjust(int new_maximum)
{
  FUNCDEF("adjust");
  CHECK_FIELDS;
  if (new_maximum < 0) return;  // bad input here.
  int old_max = elements();
  if (new_maximum == old_max) return;  // nothing to do.
  if (new_maximum < old_max) {
    // removes the elements beyond the new size of the amorph.
    zap(new_maximum, old_max - 1);
    // we're done tuning it.
    return;
  }

  // we get to here if we need more space than we used to.
  int new_fields = new_maximum - old_max;

  basis::array<contents *>::insert(old_max, new_fields);
  for (int i = old_max; i < new_maximum; i++) {
    basis::array<contents *>::put(i, NIL);
  }
}

template <class contents>
basis::outcome amorph<contents>::insert(int position, int lines_to_add)
{
  FUNCDEF("insert");
  CHECK_FIELDS;
  bounds_return(position, 0, elements(), basis::common::OUT_OF_RANGE);
  basis::outcome o = basis::array<contents *>::insert(position, lines_to_add);
  if (o != basis::common::OKAY) return basis::common::OUT_OF_RANGE;
  set_nil(position, position + lines_to_add - 1);
  return basis::common::OKAY;
}

template <class contents>
basis::outcome amorph<contents>::zap(int start_index, int end_index)
{
  FUNCDEF("zap");
  CHECK_FIELDS;
  bounds_return(start_index, 0, elements() - 1, basis::common::OUT_OF_RANGE);
  bounds_return(end_index, 0, elements() - 1, basis::common::OUT_OF_RANGE);
  if (end_index < start_index) return basis::common::OKAY;
  for (int i = start_index; i <= end_index;  i++) clear(i);
  basis::outcome o = basis::array<contents *>::zap(start_index, end_index);
  return (o == basis::common::OKAY? basis::common::OKAY : basis::common::OUT_OF_RANGE);
}

template <class contents>
basis::outcome amorph<contents>::put(int field, const contents *data)
{
  FUNCDEF("put");
  CHECK_FIELDS;
  bounds_return(field, 0, elements() - 1, basis::common::OUT_OF_RANGE);
  contents *to_whack = acquire(field);
  delete to_whack;
  if (data) {
    basis::array<contents *>::access()[field] = (contents *)data;
    _fields_used++; 
  }
  return basis::common::OKAY;
}

template <class contents>
int amorph<contents>::find_empty(basis::outcome &o) const
{
  FUNCDEF("find_empty");
  CHECK_FIELDS;
  if (_fields_used == elements()) { o = basis::common::IS_FULL; return 0; }
  for (int i = 0; i < elements(); i++)
    if (!basis::array<contents *>::get(i)) { o = basis::common::OKAY; return i; }
  AMO_ALERT("amorph", "empty", "_fields_used is incorrect");
  return basis::common::IS_FULL;
}

template <class contents>
const contents *amorph<contents>::next_valid(int &field) const
{
  FUNCDEF("next_valid");
  CHECK_FIELDS;
  bounds_return(field, 0, elements() - 1, NIL);
  for (int i = field; i < elements(); i++)
    if (basis::array<contents *>::get(i)) {
      field = i;
      return basis::array<contents *>::get(i);
    }
  return NIL;
}

template <class contents>
basis::outcome amorph<contents>::clear(int field)
{
  FUNCDEF("clear");
  CHECK_FIELDS;
  return this->put(field, NIL);
}

template <class contents>
contents *amorph<contents>::acquire(int field)
{
  FUNCDEF("acquire");
  CHECK_FIELDS;
  contents *to_return = borrow(field);
  if (to_return) {
    _fields_used--;
    basis::array<contents *>::access()[field] = NIL;
  }
  return to_return;
}

template <class contents>
int amorph<contents>::find(const contents *to_locate, basis::outcome &o)
{
  FUNCDEF("find");
  CHECK_FIELDS;
  if (!_fields_used) { o = basis::common::NOT_FOUND; return 0; }
  for (int i = 0; i < elements(); i++) {
    if (basis::array<contents *>::get(i) == to_locate) {
      o = basis::common::OKAY;
      return i; 
    }
  }
  o = basis::common::NOT_FOUND;
  return 0;
}

template <class contents>
contents *amorph<contents>::borrow(int field)
{
  FUNCDEF("borrow");
  CHECK_FIELDS;
  bounds_return(field, 0, elements() - 1, NIL);
  return basis::array<contents *>::access()[field];
}

template <class contents>
void amorph<contents>::swap_contents(amorph<contents> &other)
{
  FUNCDEF("swap_contents");
  CHECK_FIELDS;
  int hold_fields = _fields_used;
  _fields_used = other._fields_used;
  other._fields_used = hold_fields;
  this->basis::array<contents *>::swap_contents(other);
}

template <class contents>
int amorph_packed_size(const amorph<contents> &to_pack)
{
  int parts_size = 0;
  for (int i = 0; i < to_pack.elements(); i++) {
    const contents *current = to_pack.get(i);
    if (current) parts_size += current->packed_size();
  }
  return PACKED_SIZE_INT32 + parts_size;
}

template <class contents>
void amorph_pack(basis::byte_array &packed_form, const amorph<contents> &to_pack)
{
  structures::attach(packed_form, to_pack.elements());
  for (int i = 0; i < to_pack.elements(); i++) {
    const contents *current = to_pack.get(i);
    if (current) current->pack(packed_form);
  }
}

template <class contents>
bool amorph_unpack(basis::byte_array &packed_form, amorph<contents> &to_unpack)
{
  to_unpack.reset();
  int elem = 0;
  if (!structures::detach(packed_form, elem)) return false;
  for (int i = 0; i < elem; i++) {
    contents *to_add = new contents;
    if (!to_add->unpack(packed_form)) { delete to_add; return false; }
    to_unpack.append(to_add);
  }
  return true;
}

template <class contents>
void amorph_assign(amorph<contents> &to_assign,
    const amorph<contents> &to_copy)
{
  if (&to_assign == &to_copy) return;
  to_assign.clear_all();
  if (to_assign.elements() != to_copy.elements()) {
    to_assign.zap(0, to_assign.elements() - 1);
    to_assign.insert(0, to_copy.elements());
  }
  for (int i = 0; i < to_assign.elements(); i++) {
    if (to_copy.get(i)) to_assign.put(i, new contents(*to_copy.get(i)));
    else to_assign.put(i, (contents *)NIL);
  }
}

#undef static_class_name

} //namespace.

#endif

