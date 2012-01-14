


/*****************************************************************************\
*                                                                             *
*  Name   : list                                                              *
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

// POLICIES:
//
// the cursor should never be stored to or deleted; it is merely a scanner that
// runs through the list.
//
// the cursor can point at the head or tail.  any storage action is taken to
// mean that it applies to the closest real object, if one exists.  any query
// action is taken similarly.

#include "list.h"
#include "node.h"

#include <basis/functions.h>

namespace nodes {

// nice names for the positions of the next link and the previous link in
// our node's indices.
const int PREVIOUS = 0;
const int NEXT = 1;

//////////////

// iterator functions:

void list::iterator::operator ++()
{
  if (is_tail()) return;  // already at the end.
  _cursor = _cursor->get_link(NEXT);
}

void list::iterator::operator --()
{
  if (is_head()) return;  // already at the front.
  _cursor = _cursor->get_link(PREVIOUS);
}

bool list::iterator::operator ==(const iterator &to_compare) const
{ return _cursor == to_compare._cursor; }

const node *list::iterator::observe()
{
  if (!_manager || _manager->empty()) return NIL;
  if (*this == _manager->head()) next();
  if (*this == _manager->tail()) previous();
  return _cursor;
}

node *list::iterator::access()
{
  if (!_manager || _manager->empty()) return NIL;
  if (*this == _manager->head()) next();
  if (*this == _manager->tail()) previous();
  return _cursor;
}

bool list::iterator::is_head() const
{
  if (!_manager) return false;
  return _cursor == _manager->_head;
}

bool list::iterator::is_tail() const
{ 
  if (!_manager) return false;
  return _cursor == _manager->_tail;
}

void list::iterator::jump_head()
{
  if (!_manager) return;
  _cursor = _manager->_head;
}

void list::iterator::jump_tail()
{
  if (!_manager) return;
  _cursor = _manager->_tail;
}

//////////////

list::list()
: _head(NIL), _tail(NIL)
{
  _head = new node(2);
  _tail = new node(2);
  _head->set_link(NEXT, _tail);
  _tail->set_link(PREVIOUS, _head);
}

list::~list()
{
  iterator zapper = head();
  while (!empty())
    zap(zapper);
  WHACK(_head);
  WHACK(_tail);
}

bool list::empty() const
{
  if (_head->get_link(NEXT) == _tail) return true;
  return false;
}

bool list::set_index(iterator &where, int new_index)
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

bool list::forward(iterator &where, int count)
{ 
  if (where._manager != this) return false;
  if (count <= 0) return true;
  if (items_from_tail(where) < count) return false;
  if (where.is_head()) where.next();  // skip the head guard.
  for (int i = 0; i < count; i++) where.next();
  return true;
}

bool list::backward(iterator &where, int count)
{
  if (where._manager != this) return false;
  if (count <= 0) return true;
  if (items_from_head(where) < count) return false;
  if (where.is_tail()) where.previous();  // skip the tail guard.
  for (int i = 0; i < count; i++) where.previous();
  return true;
}

int list::elements() const
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

int list::items_from_head(const iterator &where) const
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

int list::items_from_tail(const iterator &where) const
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
  if (empty()) return NIL;  // make sure the list isn't empty.
  if (_cursor == _head) return _head->get_link(NEXT);
    // check special case for pointing at the head.
  if (_cursor == _tail) return _tail->get_link(PREVIOUS);
    // check special case for pointing at the tail.
  return _cursor;
}
*/

node *list::remove(iterator &where)
{
  if (where._manager != this) return NIL;
  if (empty()) return NIL;
  if (where._cursor == _head)
    where._cursor = _head->get_link(NEXT);
  if (where._cursor == _tail)
    where._cursor = _tail->get_link(PREVIOUS);
  node *old_cursor = where._cursor;
  node *old_previous = old_cursor->get_link(PREVIOUS);
  node *old_next = old_cursor->get_link(NEXT);
  old_cursor->set_link(PREVIOUS, NIL);
  old_cursor->set_link(NEXT, NIL);
  old_previous->set_link(NEXT, old_next);
  old_next->set_link(PREVIOUS, old_previous);
  where._cursor = old_next;
  return old_cursor;
}

void list::zap(iterator &where) { delete remove(where); }

void list::append(iterator &where, node *new_node)
{
  if (where._manager != this) return;
  while (new_node->links() < 2) new_node->insert_link(0, NIL);
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

void list::insert(iterator &where, node *new_node)
{
  if (where._manager != this) return;
  while (new_node->links() < 2) new_node->insert_link(0, NIL);
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

void list::zap_all()
{
  iterator zapper = head();
  while (!empty()) zap(zapper);
}

} // namespace.




