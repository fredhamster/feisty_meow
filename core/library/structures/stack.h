#ifndef STACK_CLASS
#define STACK_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : stack                                                             *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1990-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/array.h>

namespace structures {

//! An abstraction that represents a stack data structure.
/*!
  This behaves like a standard stack of objects, but it additionally allows
  access to any item in the stack via an array style bracket operator.
*/

template <class contents>
class stack
{
public:
  enum stack_kinds { BOUNDED, UNBOUNDED };

  stack(int elements = 0);
    //!< Creates a stack with room for the specified number of "elements".
    /*!< If "elements" is zero, then the stack is an UNBOUNDED stack that
    has no set limit on the number of elements it can contain (besides the
    amount of memory available).  on an unbounded stack, a result of IS_FULL
    will never be returned--instead a memory allocation failure would occur.
    If "elements" is greater than zero, then the stack is a BOUNDED stack
    which can hold at maximum "elements" number of objects.  for bounded
    stacks, if there is to be an allocation failure, it will happen at the
    time of stack construction, rather than during execution. */

  stack(const stack &to_copy);
    //!< constructs a stack as a copy of "to_copy".

  ~stack();
    //!< destroys anything left on the stack.

  void reset();
    //!< throws out all contents on the stack.

  stack_kinds kind() const { return _kind; }
    //!< returns the type of stack that was constructed.

  basis::outcome push(const contents &element);
    //!< Enters a new element onto the top of the stack.
    /*!< if the stack is too large to add another element, then IS_FULL is
    returned.  if the element to push is nil, the stack is unchanged and
    IS_EMPTY is returned. */

  basis::outcome pop();
    //!< Removes the top element on the stack.
    /*!< If the stack has no elements to be popped off, then IS_EMPTY is
    returned.  The element that was popped is destroyed. */

  contents &top();
    //!< Returns the top element from the stack but doesn't change the stack.
    /*!< This method does not pop the element!  If the stack is empty, then a
    bogus contents object is returned. */

  basis::outcome acquire_pop(contents &to_stuff);
    //!< Used to grab the top off of the stack.
    /*!< this is basically a call to top() followed by a pop().  if there was
    no top, then IS_EMPTY is returned. */

  int size() const;
    //!< returns the size of the stack.
    /*!< if the stack is empty, then 0 is returned. */

  stack &operator =(const stack &to_copy);
    //!< makes this stack a copy of "to_copy".

  contents &operator [](int index);
    //!< Accesses the item at position "index" in the stack.
    /*!< Allows access to the stack in an impure fashion; elements other than
    the top can be examined.  Efforts to access elements that do not exist
    are ignored.  The range for the element numbers is as in C and runs
    from 0 to size() - 1. */

  void invert();
    //!< Inverts this stack, meaning that the old bottom is the new top.

  int elements() const;
    //!< Returns the number of elements used by the stack.
    /*!< For a bounded stack, this returns the number of elements the stack
    was constructed to hold.  For an unbounded stack, it returns the current
    number of elements (which is the same as size()).  Note though that it is
    different from size() for a bounded size stack! */

private:
  basis::array<contents> _store;  //!< holds the contents of the stack.
  stack_kinds _kind;  //!< the type of stack we've got.
  int _valid_fields;  //!< count of the number of items actually in use here.
};

//////////////

// implementations below...

template <class contents>
stack<contents>::stack(int elements)
: _store(elements >= 0? elements : 0),
  _kind(_store.length()? BOUNDED : UNBOUNDED),
  _valid_fields(0)
{}

template <class contents>
stack<contents>::stack(const stack &to_copy)
: _store(0), _valid_fields(0)
{ operator = (to_copy); }

template <class contents> stack<contents>::~stack() {}

template <class contents>
int stack<contents>::size() const { return _valid_fields; }

template <class contents>
void stack<contents>::reset()
{
  while (pop() == basis::common::OKAY) {}  // pop off elements until all are gone.
}

template <class contents>
int stack<contents>::elements() const { return _store.length(); }

template <class contents>
basis::outcome stack<contents>::push(const contents &element)
{
  if (_kind == BOUNDED) {
    if (_valid_fields >= elements()) return basis::common::IS_FULL;
    basis::outcome result = _store.put(_valid_fields, element);
    if (result != basis::common::OKAY) return basis::common::IS_FULL;
  } else _store.concatenate(element);
  _valid_fields++;
  return basis::common::OKAY;
}

template <class contents>
basis::outcome stack<contents>::pop()
{
  if (_valid_fields < 1) return basis::common::IS_EMPTY;
  if (_kind == UNBOUNDED)
    _store.zap(_store.length() - 1, _store.length() - 1);
  _valid_fields--;
  return basis::common::OKAY;
}

template <class contents>
contents &stack<contents>::top()
{ return _store[_valid_fields - 1]; }

template <class contents>
stack<contents> &stack<contents>::operator = (const stack &to_copy)
{
  if (this == &to_copy) return *this;
  reset();
  _kind = to_copy._kind;
  _store.reset(to_copy._store.length());
  for (int i = 0; i < to_copy._store.length(); i++)
    _store.put(i, to_copy._store[i]);
  _valid_fields = to_copy._valid_fields;
  return *this;
}

template <class contents>
void stack<contents>::invert()
{
  for (int i = 0; i < _store.length() / 2; i++) {
    contents hold = _store.get(i);
    int exchange_index = _store.length() - i - 1;
    _store.put(i, _store.get(exchange_index));
    _store.put(exchange_index, hold);
  }
}

template <class contents>
contents &stack<contents>::operator [](int index)
{
  if (index >= _valid_fields) index = -1;  // force a bogus return.
  return _store[index];
}

template <class contents>
basis::outcome stack<contents>::acquire_pop(contents &to_stuff)
{
  if (!_valid_fields) return basis::common::IS_EMPTY;
  to_stuff = _store[_valid_fields - 1];
  if (_kind == UNBOUNDED) _store.zap(elements()-1, elements()-1);
  _valid_fields--;
  return basis::common::OKAY;
}

} //namespace.

#endif

