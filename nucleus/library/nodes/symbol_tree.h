#ifndef SYMBOL_TREE_CLASS
#define SYMBOL_TREE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : symbol_tree                                                       *
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

#include "tree.h"

#include <basis/astring.h>
#include <basis/definitions.h>

namespace nodes {

// forward.
class symbol_tree_associations;

//! A symbol table that supports scope nesting and/or trees of symbol tables.
/*!
  Note: although the symbol_tree is a tree, proper functioning is only
  guaranteed if you stick to its own add / find methods rather than calling
  on the base class's methods... but the tree's iterator support should be
  used for traversing the symbol_tree and prune should work as expected.
*/

class symbol_tree : public tree
{
public:
  symbol_tree(const basis::astring &node_name, int estimated_elements = 100);
    //!< creates a symbol_tree node with the "node_name".
    /*!< presumably this could be a child of another symbol tree also.  the "estimated_elements"
    is used to choose a size for the table holding the names.  */

  virtual ~symbol_tree();
    //!< all child nodes will be deleted too.
    /*!< if the automatic child node deletion is not good for your purposes,
    be sure to unhook the children before deletion of the tree and manage them
    separately. */

  DEFINE_CLASS_NAME("symbol_tree");

  int children() const;  //!< returns the number of children of this node.

  const basis::astring &name() const;  //!< returns the name of this node.

  int estimated_elements() const;  //!< returns the number of bits in this node's table.

  symbol_tree *branch(int index) const;  //!< returns the "index"th branch.

  void rehash(int estimated_elements);
    //!< resizes the underlying symbol_table for this node.

  void hash_appropriately(int estimated_elements);
    //!< resizes the hashing parameter to limit bucket sizes.
    /*!< rehashes the name table so that there will be no more (on average)
    than "max_per_bucket" items per hashing bucket.  this is the max that will
    need to be crossed to find an item, so reducing the number per bucket
    speeds access but also requires more memory. */

  bool add(symbol_tree *to_add);
    //!< adds a child to this symbol_tree.

  virtual basis::outcome prune(tree *to_zap);
    //!< removes a sub-tree "to_zap".
    /*!< the "to_zap" tree had better be a symbol_tree; we are just matching
    the lower-level virtual function prototype.  note that the tree node
    "to_zap" is not destroyed; it is just plucked from the tree. */

  enum find_methods { just_branches, recurse_downward, recurse_upward };

  symbol_tree *find(const basis::astring &to_find,
          find_methods how,
///= just_branches,
          basis::string_comparator_function *comp = NIL);
    //!< returns the node specified by "to_find" or NIL.
    /*!< this should be fairly quick due to the symbol table's hashing.
    the "how" method specifies the type of recursion to be used in searching
    if any.  if "how" is passed as "just_branches", then only the branches are
    checked and no recursion upwards or downwards is performed.  if "how" is
    "recurse_downward", then all sub-trees under the branches are checked
    also.  if "how" is given as "recurse_upward", then "this" node and parent
    nodes are checked.  the "comp" parameter will be used for comparing the
    strings if it's passed as non-NIL. */

  void sort();
    //!< sorts the sub-nodes of this symbol_tree.

  basis::astring text_form() const;
    //!< traverses the tree to build a textual list of the nodes.

private:
  symbol_tree_associations *_associations;  //!< the link from names to nodes.
  basis::astring *_name;  //!< the name of this symbol tree node.
};

} // namespace.

#endif

