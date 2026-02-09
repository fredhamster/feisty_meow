/*****************************************************************************\
*                                                                             *
*  Name   : path                                                              *
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

#include "node.h"
#include "path.h"

#include <basis/astring.h>
#include <structures/stack.h>

#include <stdio.h>

using namespace basis;
using namespace structures;

#undef LOG
#define LOG(to_print) printf("%s::%s: %s\n", static_class_name(), func, astring(to_print).s())

namespace nodes {

class path_node_stack : public stack<node *>
{
public:
  path_node_stack() : stack<node *>(0) {}
};

//////////////

path::path(const node *start)
: _stack(new path_node_stack)
{ _stack->push(const_cast<node *>(start)); }

path::path(const path &to_copy)
: _stack(new path_node_stack(*to_copy._stack))
{}

path::~path()
{
  while (_stack->elements()) _stack->pop(); 
  WHACK(_stack);
}

node *path::operator [] (int index) const { return (*_stack)[index]; }

int path::size() const { return _stack->size(); }

node *path::root() const { return (*_stack)[0]; }

node *path::current() const { return _stack->top(); }

node *path::follow() const { return _stack->top(); }

path &path::operator = (const path &to_copy)
{
  if (this == &to_copy) return *this;
  *_stack = *to_copy._stack;
  return *this;
}

node *path::pop()
{
  node *to_return;
  if (_stack->acquire_pop(to_return) != common::OKAY)
    return NULL_POINTER;
  return to_return;
}

outcome path::push(node *to_add)
{ return _stack->push(to_add); }

outcome path::push(int index)
{
  if (!_stack->top()->get_link(index)) return common::NOT_FOUND;
  return _stack->push(_stack->top()->get_link(index));
}

bool path::generate_path(node *to_locate, path &to_follow) const
{
  FUNCDEF("generate_path")

if (to_locate || to_follow.current()) {}
LOG("hmmm: path::generate_path is not implemented.");
return false;
}

} // namespace.

