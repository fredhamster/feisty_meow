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

#include "packable_tree.h"

#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/guards.h>
#include <structures/object_packers.h>
#include <structures/stack.h>

using namespace basis;
using namespace structures;

#define DEBUG_PACKABLE_TREE
  // uncomment for noisy debugging.

#undef LOG
#ifdef DEBUG_PACKABLE_TREE
  #include <stdio.h>
  #define LOG(to_print) printf("%s\n", astring(to_print).s());
#else
  #define LOG(s) { if (!!s) {} }
#endif

namespace nodes {

// tree commands are used to tell the unpacker what to do with the blobs
// it finds.  BRANCHES_FOLLOW indicates that there are a few branches stored
// at the next few contiguous memory positions.  ATTACH_BRANCHES means that
// the next branch should be the parent of some number of previous branches.
// FINISH means that the tree is done being stored (or reconstructed).
enum tree_commands { BRANCHES_FOLLOW, ATTACH_BRANCHES, FINISH };

//////////////

packable_tree_factory::~packable_tree_factory() {}

//////////////

//! the TCU stores a command about this packed unit's purpose, the number of branches held, and the size of the contents at this node.
struct tree_command_unit : public virtual packable
{
  tree_commands command;
  int number;
  int size;

  virtual ~tree_command_unit() {}

  virtual int packed_size() const { return 3 * PACKED_SIZE_INT32; }

  virtual void pack(byte_array &packed_form) const {
    attach(packed_form, int(command));
    attach(packed_form, number);
    attach(packed_form, size);
  }

  virtual bool unpack(byte_array &packed_form) {
    int cmd;
    if (!detach(packed_form, cmd)) return false;
    command = (tree_commands)cmd;
    if (!detach(packed_form, number)) return false;
    if (!detach(packed_form, size)) return false;
    return true;
  }
};

//////////////

packable_tree::packable_tree() : tree() {}

void packable_tree::calcit(int &size_accumulator, const packable_tree *current_node)
{
LOG(a_sprintf("calcing node %x", current_node));
  FUNCDEF("calcit");
  if (!current_node) throw_error(static_class_name(), func, "current node is nil");
  tree_command_unit temp;
  size_accumulator += current_node->packed_size() + temp.packed_size();
LOG(a_sprintf("len A %d", size_accumulator));
}

void packable_tree::packit(byte_array &packed_form, const packable_tree *current_node)
{
//LOG(a_sprintf("packing node %x", current_node));
//LOG(a_sprintf("size A %d", packed_form.length()));
  FUNCDEF("packit");
  if (!current_node) throw_error(static_class_name(), func, "current node is nil");

  byte_array temp_store;

int guess = current_node->packed_size();

  current_node->pack(temp_store);

if (temp_store.length() != guess)
throw_error(current_node->class_name(), func, "failure calculating size");

  tree_command_unit command;
  command.size = temp_store.length();
//hmmm: do we still need a packed size?
  if (current_node->branches() == 0) {
    command.command = BRANCHES_FOLLOW;
    // the branches following are always just one branch.
    command.number = 1;
  } else {
    command.command = ATTACH_BRANCHES;
    command.number = current_node->branches();
  }
  // stuff the command unit.
  command.pack(packed_form);
//LOG(a_sprintf("size B %d", packed_form.length()));
  packed_form += temp_store;  // main chunk is not packed, just added.
//LOG(a_sprintf("size C %d", packed_form.length()));
}

int packable_tree::recursive_packed_size() const
{
  packable_tree *curr = NIL;
  int accum = 0;  // where we accumulate the length of the packed form.
  for (iterator zip2 = start(postfix); (curr = (packable_tree *)zip2.next()); )
    calcit(accum, curr);
  tree_command_unit end_command;
  accum += end_command.packed_size();
  return accum;
}

void packable_tree::recursive_pack(byte_array &packed_form) const
{
  packable_tree *curr = NIL;
  for (iterator zip2 = start(postfix); (curr = (packable_tree *)zip2.next()); )
    packit(packed_form, curr);

  tree_command_unit end_command;
  end_command.number = 1;
  end_command.command = FINISH;
  end_command.size = 0;
  // end command is stored at end.
  end_command.pack(packed_form);
}

packable_tree *packable_tree::recursive_unpack(byte_array &packed_form,
    packable_tree_factory &creator)
{
  stack<packable_tree *> accumulated_trees(0);  // unbounded.
  tree_command_unit cmd;
  // get the first command out of the package.
  if (!cmd.unpack(packed_form)) {
//complain.
    return NIL;
  }

  packable_tree *new_branch = NIL;
  bool failure = false;  // set to true if errors occurred.

  // the packed tree is traversed by grabbing a command and then doing what
  // it says as far as pulling in children or adding a new branch.
  while (cmd.command != FINISH) {
    new_branch = creator.create();

    new_branch->unpack(packed_form);

    if (cmd.command == ATTACH_BRANCHES) {
      if (cmd.number > accumulated_trees.size()) {
//log instead: "badly formed packed tree"
        failure = true;
        break;
      }
      for (int i = cmd.number; i > 0; i--) {
        packable_tree *to_add = (packable_tree *)accumulated_trees
            [accumulated_trees.size()-i];
        new_branch->attach(to_add);
      }
      packable_tree *junk;
      for (int j = 0; j < cmd.number; j++)
        accumulated_trees.acquire_pop(junk);
      accumulated_trees.push(new_branch);
    } else if (cmd.command == BRANCHES_FOLLOW) {
      accumulated_trees.push(new_branch);
    } else {
//log instead: "invalid command in packed tree"
      failure = true;
      break;
    }
    if (!cmd.unpack(packed_form)) {
//complain.
      failure = true;
      break;
    }
  }

  if (accumulated_trees.size() != 1) {
//log instead: "not all branches were claimed"
    failure = true;
  } else if (!failure) {
    packable_tree *junk;
    accumulated_trees.acquire_pop(junk);
  }

  // clean up the allocated objects if we saw a failure.
  if (failure) {
    while (true) {
      packable_tree *to_whack;
      outcome ret = accumulated_trees.acquire_pop(to_whack);
      if (ret == common::IS_EMPTY) break;
      if (to_whack != new_branch)
        WHACK(to_whack);
    }
    WHACK(new_branch);
  }

  return new_branch;
}

}  // namespace.

