#ifndef PACKABLE_TREE_CLASS
#define PACKABLE_TREE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : packable_tree                                                     *
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

#include <basis/byte_array.h>

namespace nodes {

// forward.
class packable_tree_factory;

//! A tree object that can be packed into an array of bytes and unpacked again.

class packable_tree : public tree, public virtual basis::packable
{
public:
  packable_tree();
    //!< constructs a new tree with a root and zero branches.

  // the recursive packing methods here will operate on all nodes starting at this one
  // and moving downwards to all branches.

  int recursive_packed_size() const;
    //!< spiders the tree starting at this node to calculate the packed size.

  void recursive_pack(basis::byte_array &packed_form) const;
    //!< packs the whole tree starting at this node into the packed form.
  
  static packable_tree *recursive_unpack(basis::byte_array &packed_form,
           packable_tree_factory &creator);
    //!< unpacks a tree stored in "packed_form" and returns it.
    /*!< if NIL is returned, then the unpack failed.  the "creator" is needed
    for making new derived packable_tree objects of the type stored. */

  // standard pack, unpack and packed_size methods must be implemented by the derived tree.

private:
  static void packit(basis::byte_array &packed_form, const packable_tree *current_node);
    //!< used by our recursive packing methods.
  static void calcit(int &size_accumulator, const packable_tree *current_node);
    //!< used by recursive packed size calculator.
};

//////////////

class packable_tree_factory
{
public:
  virtual ~packable_tree_factory();
  virtual packable_tree *create() = 0;
    //!< a tree factory is needed when we are recreating the packed tree.
    /*!< this is because the real type held is always a derived object.
    this method should just create a blank object of the appropriate type. */
};

} // namespace.

#endif

