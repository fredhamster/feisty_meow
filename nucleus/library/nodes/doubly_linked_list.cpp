/*
*
*  Name   : doubly_linked_list
*  Author : Chris Koeritz
**
* Copyright (c) 1998-$now By Author.  This program is free software; you can
* redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either version 2 of
* the License or (at your option) any later version.  This is online at:
*     http://www.fsf.org/copyleft/gpl.html
* Please send any updates to: fred@gruntose.com
*/

/*
 * POLICIES:
 *
 * + the cursor should never be stored to or deleted; it is merely a scanner that runs through the list.
 * + the cursor can point at the head or tail.  any storage action is taken to mean that it applies to the closest real object, if one exists.  any query action is taken similarly.
 */

#include "doubly_linked_list.h"

#include "node.h"

#include <basis/functions.h>

namespace nodes {

// nice names for the positions of the next link and the previous link in
// our node's indices.
const int PREVIOUS = 0;
const int NEXT = 1;

//////////////

// iterator functions:

void doubly_linked_list::iterator::operator ++()
{
  if (is_tail()) return;  // already at the end.
  _cursor = _cursor->get_link(NEXT);
}

void doubly_linked_list::iterator::operator --()
{
  if (is_head()) return;  // already at the front.
  _cursor = _cursor->get_link(PREVIOUS);
}

bool doubly_linked_list::iterator::operator ==(const iterator &to_compare) const
{ return _cursor == to_compare._cursor; }

const node *doubly_linked_list::iterator::observe()
{
  if (!_manager || _manager->empty()) return NULL_POINTER;
  if (*this == _manager->head()) next();
  if (*this == _manager->tail()) previous();
  return _cursor;
}

node *doubly_linked_list::iterator::access()
{
  if (!_manager || _manager->empty()) return NULL_POINTER;
  if (*this == _manager->head()) next();
  if (*this == _manager->tail()) previous();
  return _cursor;
}

bool doubly_linked_list::iterator::is_head() const
{
  if (!_manager) return false;
  return _cursor == _manager->_head;
}

bool doubly_linked_list::iterator::is_tail() const
{ 
  if (!_manager) return false;
  return _cursor == _manager->_tail;
}

void doubly_linked_list::iterator::jump_head()
{
  if (!_manager) return;
  _cursor = _manager->_head;
}

void doubly_linked_list::iterator::jump_tail()
{
  if (!_manager) return;
  _cursor = _manager->_tail;
}

//////////////

doubly_linked_list::doubly_linked_list()
: _head(NULL_POINTER), _tail(NULL_POINTER)
{
  _head = new node(2);
  _tail = new node(2);
  _head->set_link(NEXT, _tail);
  _tail->set_link(PREVIOUS, _head);
}

doubly_linked_list::~doubly_linked_list()
{
  iterator zapper = head();
  while (!empty())
    zap(zapper);
  WHACK(_head);
  WHACK(_tail);
}

bool doubly_linked_list::empty() const
{
  if (_head->get_link(NEXT) == _tail) return true;
  return false;
}

bool doubly_linked_list::set_index(iterator &where, int new_index)
{
  if (where._manager != this) return false;
  if (empty()) return false;
  node *skipper = _head->get_link(NEXT);
  for (int i = 0; i < new_index; i++) {
    skipper = skipper->get_link(NEXT);
    if (skipper == _tail) return false;  // out of bounds now.
  }
  where._cursor = skipper;
  return true;
}

bool doubly_linked_list::forward(iterator &where, int count)
{ 
  if (where._manager != this) return false;
  if (count <= 0) return true;
  if (items_from_tail(where) < count) return false;
  if (where.is_head()) where.next();  // skip the head guard.
  for (int i = 0; i < count; i++) where.next();
  return true;
}

bool doubly_linked_list::backward(iterator &where, int count)
{
  if (where._manager != this) return false;
  if (count <= 0) return true;
  if (items_from_head(where) < count) return false;
  if (where.is_tail()) where.previous();  // skip the tail guard.
  for (int i = 0; i < count; i++) where.previous();
  return true;
}

int doubly_linked_list::elements() const
{
  if (empty()) return 0;
  int to_return = 0;
  node *skipper = _head->get_link(NEXT);
  while (skipper != _tail) {
    to_return++;
    skipper = skipper->get_link(NEXT);
  }
  return to_return;
}

int doubly_linked_list::items_from_head(const iterator &where) const
{
  if (where._manager != this) return 0;
  if (where.is_head()) return 0;  // make sure it's not there already.
  int index = 0;
  node *skipper = _head->get_link(NEXT);
  while ( (where._cursor != skipper) && (skipper != _tail) ) {
    index++;
    skipper = skipper->get_link(NEXT);
  }
  return index;
}

int doubly_linked_list::items_from_tail(const iterator &where) const
{
  if (where._manager != this) return 0;
  if (where.is_tail()) return 0;  // make sure it's not there already.
  int index = 0;
  node *skipper = _tail->get_link(PREVIOUS);
  while ( (where._cursor != skipper) && (skipper != _head) ) {
    index++;
    skipper = skipper->get_link(PREVIOUS);
  }
  return index;
}

/*
node *list::get()
{
  if (empty()) return NULL_POINTER;  // make sure the list isn't empty.
  if (_cursor == _head) return _head->get_link(NEXT);
    // check special case for pointing at the head.
  if (_cursor == _tail) return _tail->get_link(PREVIOUS);
    // check special case for pointing at the tail.
  return _cursor;
}
*/

node *doubly_linked_list::remove(iterator &where)
{
  if (where._manager != this) return NULL_POINTER;
  if (empty()) return NULL_POINTER;
  if (where._cursor == _head)
    where._cursor = _head->get_link(NEXT);
  if (where._cursor == _tail)
    where._cursor = _tail->get_link(PREVIOUS);
  node *old_cursor = where._cursor;
  node *old_previous = old_cursor->get_link(PREVIOUS);
  node *old_next = old_cursor->get_link(NEXT);
  old_cursor->set_link(PREVIOUS, NULL_POINTER);
  old_cursor->set_link(NEXT, NULL_POINTER);
  old_previous->set_link(NEXT, old_next);
  old_next->set_link(PREVIOUS, old_previous);
  where._cursor = old_next;
  return old_cursor;
}

void doubly_linked_list::zap(iterator &where) { delete remove(where); }

void doubly_linked_list::append(iterator &where, node *new_node)
{
  if (where._manager != this) return;
  while (new_node->links() < 2) new_node->insert_link(0, NULL_POINTER);
  if (empty()) where._cursor = _head;
  if (where._cursor == _tail)
    where._cursor = _tail->get_link(PREVIOUS);
    // shift from the tail sentinel to the tail element.
  node *save_next = where._cursor->get_link(NEXT);
  where._cursor->set_link(NEXT, new_node);
  new_node->set_link(PREVIOUS, where._cursor);
  new_node->set_link(NEXT, save_next);
  save_next->set_link(PREVIOUS, new_node);
  where._cursor = new_node;
}

void doubly_linked_list::insert(iterator &where, node *new_node)
{
  if (where._manager != this) return;
  while (new_node->links() < 2) new_node->insert_link(0, NULL_POINTER);
  if (empty()) where._cursor = _tail;
  if (where._cursor == _head)
    where._cursor = _head->get_link(NEXT);
    // shift from head sentinel to the head element.
  node *save_prior = where._cursor->get_link(PREVIOUS);
  where._cursor->set_link(PREVIOUS, new_node);
  new_node->set_link(NEXT, where._cursor);
  new_node->set_link(PREVIOUS, save_prior);
  save_prior->set_link(NEXT, new_node);
  where._cursor = new_node;
}

void doubly_linked_list::zap_all()
{
  iterator zapper = head();
  while (!empty()) zap(zapper);
}

} // namespace.




