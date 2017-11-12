#ifndef PATH_CLASS
#define PATH_CLASS

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

#include <basis/contracts.h>
#include <basis/enhance_cpp.h>
#include <basis/outcome.h>

namespace nodes {

// forward:
class node;
class path_node_stack;

//! A method for tracing a route from a tree's root to a particular node.
/*!
  A path has a starting point at a particular node and a list of links to
  take from that node to another node.  For the path to remain valid, none
  of the links contained within it may be destroyed.
*/

class path : public virtual basis::nameable
{
public:
  path(const node *root);
    //!< the path is relative to the "root" node.
    /*!< this will be the first object pushed onto the stack that we use
    to model the path. */

  path(const path &to_copy);

  ~path();

  DEFINE_CLASS_NAME("path");

  path &operator =(const path &to_copy);

  int size() const;
    //!< returns the number of items in the path.

  node *root() const;
    //!< returns the relative root node for this path.

  node *current() const;
  node *follow() const;
    //!< Returns the node specified by this path.
    /*!< if the path is not valid, NULL_POINTER is returned. */

  node *pop();
    //!< returns the top node on the path stack.
    /*!< this returns the node at the farthest distance from the relative
    root node and removes it from this path. */

  basis::outcome push(node *to_add);
    //!< puts the node "to_add" on the top of the stack.
    /*!< adds a node to the path as long as "to_add" is one of the current
    node's descendants. */

  basis::outcome push(int index);
    //!< indexed push uses the current top node's index'th link as new top.
    /*!< adds the node at "index" of the current top node to the path,
    providing that such a link number exists on the current node. */

  bool generate_path(node *to_locate, path &to_follow) const;
    //!< finds the way to get from the root to the "to_locate" node.
    /*!< returns true if there is a path between the relative root of
    this path and the node "to_locate".  if there is such a path,
    "to_follow" is set to one of the possible paths. */

  node *operator [] (int index) const;
    //!< returns the node stored at "index", or NULL_POINTER if "index" is invalid.

private:
  path_node_stack *_stack;  //!< implementation of our pathway.
};

} // namespace.

#endif

