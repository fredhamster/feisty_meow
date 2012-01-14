#ifndef FILENAME_TREE_CLASS
#define FILENAME_TREE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : filename_tree                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2004-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

//! This is a support class for the directory_tree.
/*!
  This class has been exported from directory_tree's implementation to
  avoid redundant code for iteration and such.
  Note: this is a heavy-weight header that should not be included in other
  headers.
*/

#include "filename_list.h"

#include <nodes/packable_tree.h>

namespace filesystem {

class filename_tree : public nodes::packable_tree
{
public:
  filename _dirname;  //!< the full directory name at this position.
  filename_list _files;  //!< the filenames that are at this node in the tree.
  int _depth;  //!< how far below root node are we.

  filename_tree();

  virtual ~filename_tree();

  virtual int packed_size() const;

  virtual void pack(basis::byte_array &packed_form) const;

  virtual bool unpack(basis::byte_array &packed_form);
};

//! this is the tree factory used in the recursive_unpack.
/*! it meets our needs for regenerating these objects from a streamed form. */
class fname_tree_creator : public nodes::packable_tree_factory
{
public:
////  virtual ~fname_tree_creator() {}
  virtual nodes::packable_tree *create();
    //!< implements the create() method for filename_trees to support packing.
};

} //namespace.

#endif

