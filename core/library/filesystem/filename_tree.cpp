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

#include "filename_tree.h"

#include <structures/object_packers.h>

using namespace basis;
using namespace nodes;
using namespace structures;

namespace filesystem {

nodes::packable_tree *fname_tree_creator::create() { return new filename_tree; }

//////////////

filename_tree::filename_tree() : _depth(0) {}

filename_tree::~filename_tree() { _dirname = ""; _files.reset(); }

int filename_tree::packed_size() const {
  return PACKED_SIZE_INT32 + _dirname.packed_size() + _files.packed_size();
}

void filename_tree::pack(byte_array &packed_form) const {
  structures::attach(packed_form, _depth);
  _dirname.pack(packed_form);
  _files.pack(packed_form);
}

bool filename_tree::unpack(byte_array &packed_form) {
  if (!structures::detach(packed_form, _depth)) return false;
  if (!_dirname.unpack(packed_form)) return false;
  if (!_files.unpack(packed_form)) return false;
  return true;
}

} //namespace.


