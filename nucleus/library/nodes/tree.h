#ifndef TREE_CLASS
#define TREE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : tree                                                              *
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

#include <basis/enhance_cpp.h>

namespace nodes {

//! A dynamically linked tree with an arbitrary number of branches.
/*!
  A tree is defined as a node with n branch nodes, where n is dynamic.
  Each branch is also a tree.  Branch numbers range from 0 through n-1 in
  the methods below.  Trees can be self-cleaning, meaning that the tree
  will destroy all of its children when it is destroyed.

  NOTE: the node indices are not numbered completely obviously; it is better
  to use the tree functions for manipulating the node rather than
  muddling with it directly.  the branch to the tree node's parent is
  stored as node zero, in actuality, rather than node zero being the
  first branch.
*/

class tree : public node
{
public:
  tree();
    //!< constructs a new tree with a root and zero branches.

  virtual ~tree();
    //!< destroys the tree by recursively destroying all child tree nodes.

  DEFINE_CLASS_NAME("tree");

  virtual tree *branch(int branch_number) const;
    //!< Returns the specified branch of this tree.
    /*!< NIL is returned if the "branch_number" refers to a branch that
    does not exist. */

  virtual int which(tree *branch_to_find) const;
    //!< Returns the branch number for a particular branch in this tree.
    /*!< common::NOT_FOUND if the branch is not one of the child nodes. */

  virtual int branches() const;
    //!< Returns the number of branches currently connected to this tree.

  virtual tree *parent() const;
    //!< Returns the tree node that is the immediate ancestor of this one.
    /*!< if this is the root node, then NIL is returned. */

  virtual tree *root() const;
    //!< Locates and returns the absolute root of the tree containing this tree.
    /*!< If this tree IS the absolute root, then "this" is returned. */

  virtual int depth() const;
    //!< Returns the distance of "this" from the root.  The root's depth is 0.

  virtual void attach(tree *new_branch);
    //!< Attaches the specified branch to the current tree.

  virtual void insert(int branch_place, tree *new_branch);
    //!< inserts "new_branch" before the branches starting at "branch_place".
    /*!< Places a branch at a particular index and pushes the branches at that
    index (and after it) over by one branch. */

  virtual basis::outcome prune(tree *branch_to_cut);
    //!< Removes the specified branch from this tree.
    /*!< note that the pruning does not affect the branch being removed; it
    just detaches that branch from the tree.  if one wants to get rid of the
    branch, it should be deleted.  if this cannot find the tree specified in
    the available branches then the branches of this tree are not touched and
    common::NOT_FOUND is returned. */
  virtual basis::outcome prune_index(int branch_to_cut);
    //!< Removes the branch at the specified index from this tree.
    /*!< if this is given an invalid branch number, then the available
    branches then the branches of this tree are not touched and
    common::NOT_FOUND is returned. */

  enum traversal_directions { prefix, infix, postfix, to_branches,
          reverse_branches };
    //!< these are the different kinds of tree traversal that are supported.
    /*!< "prefix" means that tree nodes will be visited as soon as they are
    seen; the deepest nodes have to wait the longest to be seen by the
    traversal.  "postfix" means that tree nodes are not visited until all of
    their ancestors have been visited; the nodes nearer the start of traversal
    will wait the longest to be visited.  the "infix" direction visits
    the left branch, then the starting node, then the right branch.
    "infix" is only valid for binary or unary trees; it is an error to
    apply "infix" to a tree containing a node that has more than 2 branches.
    the direction "to_branches" visits each of the branches in the order
    defined by the branch() method.  "to_branches" does not visit this
    tree's node.  "reverse_branches" operates in the opposite direction
    of traversal from "to_branches".  "reverse_branches" also does not
    visit this node.  "reverse_branches" can be used to prune off subtrees
    during iteration without changing the ordering of the branches; this
    is valuable because a pruning operation applied in "to_branches" order
    would keep reducing the index of the branches. */

  enum elevator_directions { TOWARD_ROOT, AWAY_FROM_ROOT };
    //!< movement in the tree is either towards or away from the root.
    /*!< distinguishes between motion towards the root node of the tree and
    motion away from the root (towards one's children). */

  class iterator : public path
  {
  public:
    iterator(const tree *initial, traversal_directions direction);
    ~iterator();

    tree *next();
      //!< Returns a pointer to the next tree in the direction of traversal.
      /*!< If the traversal is finished, NIL is returned. */

    void whack(tree *to_whack);
      //!< destroys the tree "to_whack".
      /*!< whacks the node "to_whack" by patching this iterator so that future
      iterations will be correct.  it is required that the "to_whack" node
      was just returned from a call to next().
      NOTE: this has only been tested with postfix so far. */

    traversal_directions _order;
    elevator_directions _aim;

  private:
    bool next_node(tree *&to_return);
      //!< sets "to_return" to the next tree in the direction of tree traversal.
      /*!< if the next node could not be found in one invocation of next_node,
      then "to_return" is set to NIL.  the function returns a boolean which
      is true only if the iteration process can be continued by another call
      to next_node.  if the function returns false, the iteration is
      complete and "to_return" will always be NIL. */
  };
  
  iterator start(traversal_directions direction) const;
    //!< Returns a fresh iterator positioned at this tree node.

  virtual bool generate_path(path &to_follow) const;
    //!< Returns the path to "this" path_tree from its root.

private:
  // unavailable.
  tree(const tree &);
  tree &operator =(const tree &);
};

} // namespace.

#endif

