#ifndef SET_CLASS
#define SET_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : set                                                               *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1996-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "object_packers.h"
#include "string_array.h"

#include <basis/astring.h>
#include <basis/contracts.h>
#include <basis/definitions.h>
#include <basis/guards.h>

namespace structures {

//! Emulates a mathematical set, providing several standard set operations.
/*!
  Note: this is not an efficient object and it should not be used for
  sets of non-trivial sizes.
*/

template <class contents>
class set : public basis::array<contents>
{
public:
  //! Constructs a set with "num" elements, copying them from "init".
  /*! Be very careful to ensure that the array "init" has sufficient length
  for "num" elements to be copied from it. */
  set(int num = 0, const contents *init = NIL,
      basis::un_short flags = basis::array<contents>::EXPONE)
  : basis::array<contents>(num, init, flags) {}

  ~set() {}  //!< Destroys any storage held for the set.

  int elements() const { return this->length(); }
    //!< Returns the number of elements in this set.

  bool empty() const { return elements() == 0; }
    //!< Returns true if the set has no elements.
  bool non_empty() const { return elements() != 0; }
    //!< Returns true if the set has some elements.

  void clear() { this->reset(); }  //!< Empties out this set.

  bool member(const contents &to_test) const;
    //!< Returns true if the item "to_test" is a member of this set.
  
  bool add(const contents &to_add);
    //!< Adds a new element "to_add" to the set.
    /*!< This always succeeds, but will return true if the item was not
    already present. */

  set &operator += (const contents &to_add)
      { add(to_add); return *this; }
    //!< An algebraic operator synonym for add() that operates on the contents.
  set &operator += (const set &to_add)
      { unionize(to_add); return *this; }
    //!< An algebraic operator synonym for add() that operates on a set.

  bool remove(const contents &to_remove);
    //!< Removes the item "to_remove" from the set.
    /*!< If it was not present, false is returned and the set is unchanged. */

  set &operator -= (const contents &to_zap)
      { remove(to_zap); return *this; }
    //!< An algebraic operator synonym for remove that operates on the contents.
  set &operator -= (const set &to_zap)
      { differentiate(to_zap); return *this; }
    //!< An algebraic operator synonym for remove that operates on a set.

  set set_union(const set &union_with) const;
    //!< Implements the set union of "this" with "union_with".
    /*!< This returns the set formed from the union of "this" set with the set
    specified in "union_with".  (unfortunately, the name "set_union" must
    be used to distinguish from the C keyword "union".) */

  void unionize(const set &union_with);
    //!< Makes "this" set a union of "this" and "union_with".

  set operator + (const set &uw) const { return set_union(uw); }
    //!< A synonym for set_union.

  set intersection(const set &intersect_with) const;
    //!< Returns the intersection of "this" with the set in "intersect_with".

  set operator * (const set &iw) const { return intersection(iw); }
    //!< A synonym for intersection.

  set difference(const set &differ_with) const;
    //!< Returns the difference of this with "differ_with".
    /*!< Difference is defined as the subset of elements in "this" that are not
    also in "differ_with". */

  void differentiate(const set &differ_with);
    //!< Makes "this" set equal to the difference of "this" and "differ_with".
    /*!< That is, after the call, "this" will only contain elements that were
    not also in "differ_with". */

  set operator - (const set &dw) const { return difference(dw); }
    //!< A synonym for difference.

  int find(const contents &to_find) const;
    //!< Returns the integer index of the item "to_find" in this set.
    /*!< This returns a negative number if the index cannot be found.  Note
    that this only makes sense within our particular implementation of set as
    an array. */

  //! Zaps the entry at the specified "index".
  /*! This also treats the set like an array.  The index must be within the
  bounds of the existing members. */
  bool remove_index(int index)
      { return this->zap(index, index) == basis::common::OKAY; }
};

//////////////

//! provides a way to pack any set that stores packable objects.
template <class contents>
void pack(basis::byte_array &packed_form, const set<contents> &to_pack)
{
  obscure_attach(packed_form, to_pack.elements());
  for (int i = 0; i < to_pack.elements(); i++) to_pack[i].pack(packed_form);
}

//! provides a way to unpack any set that stores packable objects.
template <class contents>
bool unpack(basis::byte_array &packed_form, set<contents> &to_unpack)
{
  to_unpack.clear();
  basis::un_int len;
  if (!obscure_detach(packed_form, len)) return false;
  contents to_fill;
  for (int i = 0; i < (int)len; i++) {
    if (!to_fill.unpack(packed_form)) return false;
    to_unpack.add(to_fill);
  }
  return true;
}

//////////////

//! A simple object that wraps a templated set of ints.
class int_set : public set<int>, public virtual basis::root_object
{
public:
  int_set() {}
    //!< Constructs an empty set of ints.
  int_set(const set<int> &to_copy) : set<int>(to_copy) {}
    //!< Constructs a copy of the "to_copy" array.

  DEFINE_CLASS_NAME("int_set");
};

//! A simple object that wraps a templated set of strings.
class string_set : public set<basis::astring>, public virtual basis::packable
{
public:
  string_set() {}
    //!< Constructs an empty set of strings.
  string_set(const set<basis::astring> &to_copy)
      : set<basis::astring>(to_copy) {}
    //!< Constructs a copy of the "to_copy" array.
  string_set(const string_array &to_copy) {
    for (int i = 0; i < to_copy.length(); i++)
      add(to_copy[i]);
  }

  DEFINE_CLASS_NAME("string_set");

  bool operator == (const string_set &compare) const {
	  for (int i = 0; i < elements(); i++)
	    if (get(i) != compare.get(i)) return false;
    return true;
  }

  operator string_array() const {
    string_array to_return;
    for (int i = 0; i < length(); i++)
      to_return += get(i);
    return to_return;
  }

  virtual void pack(basis::byte_array &packed_form) const
      { structures::pack(packed_form, *this); }
  virtual bool unpack(basis::byte_array &packed_form)
      { return structures::unpack(packed_form, *this); }
  virtual int packed_size() const {
    int to_return = sizeof(int) * 2;  // length packed in, using obscure.
    for (int i = 0; i < length(); i++)
      to_return += get(i).length() + 1;
    return to_return;
  }
};

//! A set of pointers that hides the platform's pointer size.
class pointer_set : public set<void *>
{
public:
  pointer_set() {}
    //!< Constructs an empty set of void pointers.
  pointer_set(const set<void *> &to_copy) : set<void *>(to_copy)
      {}
    //!< Constructs a copy of the "to_copy" array.
};

//////////////

// implementation for longer functions is below...

template <class contents>
bool set<contents>::member(const contents &to_test) const
{
  for (int i = 0; i < elements(); i++)
    if (to_test == this->get(i))
      return true;
  return false;
}

template <class contents>
bool set<contents>::add(const contents &to_add)
{
  if (member(to_add)) return false; 
  concatenate(to_add);
  return true;
}

template <class contents>
int set<contents>::find(const contents &to_find) const
{
  for (int i = 0; i < elements(); i++)
    if (to_find == this->get(i))
      return i;
  return basis::common::NOT_FOUND;
}

template <class contents>
bool set<contents>::remove(const contents &to_remove)
{
  for (int i = 0; i < elements(); i++)
    if (to_remove == this->get(i)) {
      this->zap(i, i);
      return true;
    }
  return false;
}

template <class contents>
set<contents> set<contents>::intersection(const set &intersect_with) const
{
  set<contents> created(0, NIL, this->flags());
  const set *smaller = this;
  const set *larger = &intersect_with;
  if (elements() > intersect_with.elements()) {
    // switch the smaller one into place.
    smaller = &intersect_with;
    larger = this;
  }
  for (int i = 0; i < smaller->length(); i++)
    if (larger->member(smaller->get(i)))
      created.concatenate(smaller->get(i));
  return created;
}

template <class contents>
set<contents> set<contents>::set_union(const set &union_with) const
{
  set<contents> created = *this;
  for (int i = 0; i < union_with.elements(); i++)
    created.add(union_with.get(i));
  return created;
}

template <class contents>
void set<contents>::unionize(const set &union_with)
{
  for (int i = 0; i < union_with.elements(); i++)
    add(union_with.get(i));
}

template <class contents>
set<contents> set<contents>::difference(const set &differ_with) const
{
  set<contents> created = *this;
  for (int i = 0; i < differ_with.elements(); i++) {
    if (created.member(differ_with[i]))
      created.remove(differ_with[i]);
  }
  return created;
}

template <class contents>
void set<contents>::differentiate(const set &differ_with)
{
  for (int i = 0; i < differ_with.elements(); i++) {
    if (member(differ_with[i]))
      remove(differ_with[i]);
  }
}

}  // namespace.

#endif

