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

#include "tree.h"

#include <basis/common_outcomes.h>
#include <basis/functions.h>
#include <basis/guards.h>

//#define DEBUG_TREE
  // uncomment if you want lots of debugging info.

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

using namespace basis;

namespace nodes {

const int BACKWARDS_BRANCH = 0;
  // BACKWARDS_BRANCH is the branch from this tree to its parent.  this is
  // steeped in the perspective that the root is the backwards direction (where
  // we came from, in a sense) and that the children of this node are the
  // forwards direction.

//////////////

// iterator methods:

tree::iterator::iterator(const tree *initial, traversal_directions direction)
: path(initial), _order(direction), _aim(AWAY_FROM_ROOT)
{
}

tree::iterator::~iterator() { while (size()) pop(); }

bool tree::iterator::next_node(tree *&to_return)
{
#ifdef DEBUG_TREE
  FUNCDEF("next_node");
#endif
  to_return = NIL;
#ifdef DEBUG_TREE
  if ( (_order != to_branches)
      && (_order != reverse_branches) ) {
    if (_aim == AWAY_FROM_ROOT) LOG("going down")
    else LOG("going up");
  }
#endif
  switch (_order) {
    case prefix: {
      if (_aim == AWAY_FROM_ROOT) {
        // going down means this is the first time we have seen the current top
        // node on the stack.
        to_return = (tree *)(*this)[size() - 1];
#ifdef DEBUG_TREE
//        LOG(a_sprintf("[%s] ", to_return->get_contents()->held().s());
        if (to_return->branches()) LOG("pushing 0")
        else LOG("switching direction");
#endif
        if (to_return->branches())
          push(to_return->branch(0));
        else
          _aim = TOWARD_ROOT;
      } else {
        // going up means that we need to get rid of some things before we
        // start seeing new nodes again.
        if (size() == 1) return false;
          // the last node has been seen....
        tree *last = (tree *)pop();
        tree *current_tree = (tree *)current();
        int lastnum = current_tree->which(last);
#ifdef DEBUG_TREE
        if (lastnum < current_tree->branches() - 1)
          LOG(a_sprintf("going down %d", lastnum+1))
        else LOG("still going up");
#endif
        if (lastnum < current_tree->branches() - 1) {
          _aim = AWAY_FROM_ROOT;
          push(current_tree->branch(lastnum + 1));
        }  // else still going up.
      }
      break;
    }
    case infix: {
      if (_aim == AWAY_FROM_ROOT) {
        // going down means starting on the left branch.
        tree *temp = (tree *)current();
#ifdef DEBUG_TREE
        if (temp->branches()) LOG("pushing 0")
        else LOG("switching direction");
#endif
        if (temp->branches()) push(temp->branch(0));
        else {
          _aim = TOWARD_ROOT;
          to_return = (tree *)current();
#ifdef DEBUG_TREE
//          LOG(a_sprintf("[%s] ", to_return->get_contents()->held().s()));
#endif
        }
      } else {
        // going up means that the left branch is done and we need to either
        // keep going up or go down the right branch.
        if (size() == 1) return false;
          // the last node has been seen....
        tree *last = (tree *)pop();
        tree *current_tree = (tree *)current();
        int lastnum = current_tree->which(last);
#ifdef DEBUG_TREE
        if (lastnum < 1) LOG(a_sprintf("going down %d", lastnum+1))
        else LOG("still going up");
#endif
        if (lastnum < 1) {
          _aim = AWAY_FROM_ROOT;
          to_return = (tree *)current();
#ifdef DEBUG_TREE
///          LOG(a_sprintf("[%s] ", to_return->get_contents()->held().s()));
#endif
          push(current_tree->branch(lastnum + 1));
        }  // else still going up.
      }
      break;
    }
    case to_branches: {
      if (_aim == TOWARD_ROOT) return false;
      else {
        if (size() == 1) {
          tree *temp = (tree *)current();
          if (!temp->branches())
            _aim = TOWARD_ROOT;
          else
            push(temp->branch(0));
        } else {
          tree *last = (tree *)pop();
          tree *curr = (tree *)current();
          int lastnum = curr->which(last);
          if (lastnum < curr->branches() - 1)
            push(curr->branch(lastnum + 1));
          else _aim = TOWARD_ROOT;
          to_return = last;
        }
      }
      break;
    }
    case reverse_branches: {
      if (_aim == TOWARD_ROOT) return false;
      else {
        if (size() == 1) {
          tree *temp = (tree *)current();
          if (!temp->branches()) _aim = TOWARD_ROOT;
          else push(temp->branch(temp->branches() - 1));
        } else {
          tree *last = (tree *)pop();
          tree *curr = (tree *)current();
          int lastnum = curr->which(last);
          if (lastnum > 0) push(curr->branch(lastnum - 1));
          else _aim = TOWARD_ROOT;
          to_return = last;
        }
      }
      break;
    }
    default:   // intentional fall-through to postfix.
    case postfix: {
      if (_aim == AWAY_FROM_ROOT) {
        // going down means that the bottom is still being sought.
        tree *temp = (tree *)current();
#ifdef DEBUG_TREE
        if (temp->branches()) LOG("pushing 0")
        else LOG("switching direction");
#endif
        if (temp->branches()) push(temp->branch(0));
        else _aim = TOWARD_ROOT;
      } else {
        // going up means that all nodes below current have been hit.
        if (!size()) return false;  // the last node has been seen...
        else if (size() == 1) {
          to_return = (tree *)pop();
            // this is the last node.
          return true;
        }
        tree *last = (tree *)pop();
        to_return = last;
#ifdef DEBUG_TREE
///        LOG(a_sprintf("[%s] ", to_return->get_contents()->held()));
#endif
        tree *current_tree = (tree *)current();
        int lastnum = current_tree->which(last);
#ifdef DEBUG_TREE
        if (lastnum < current_tree->branches() - 1)
          LOG(a_sprintf("going down %d", lastnum+1))
        else LOG("still going up");
#endif
        if (lastnum < current_tree->branches() - 1) {
          _aim = AWAY_FROM_ROOT;
          push(current_tree->branch(lastnum + 1));
        }  // else still going up.
      }
      break;
    }
  }
  return true;
    // it is assumed that termination conditions cause a return earlier on.
}

void tree::iterator::whack(tree *to_whack)
{
#ifdef DEBUG_TREE
  FUNCDEF("whack");
#endif
  if (!to_whack) return;  // that's a bad goof.
  if (size()) {
    if (to_whack == current()) {
      // we found that this is the one at the top right now.
      pop();
#ifdef DEBUG_TREE
      LOG("found node in current top; removing it there.");
#endif
    } else if (to_whack->parent() == current()) {
      // the parent is the current top.  make sure we don't mess up traversal.
#ifdef DEBUG_TREE
      LOG("found node's parent as current top; don't know what to do.");
#endif
    } else {
#ifdef DEBUG_TREE
      LOG("found no match for either node to remove or parent in path.");
#endif
    }
  }
  tree *parent = to_whack->parent();
  if (!parent) {
#ifdef DEBUG_TREE
    LOG("no parent node for the one to whack!  would have whacked "
        "root of tree!");
#endif
  } else {
    parent->prune(to_whack);
    WHACK(to_whack);
  }
}

tree *tree::iterator::next()
{
#ifdef DEBUG_TREE
  FUNCDEF("next");
#endif
  tree *to_return = NIL;
  bool found_tree = false;
  while (!found_tree) {
    bool still_running = next_node(to_return);
    if (to_return || !still_running) found_tree = true;
  }
  return to_return;
}

//////////////

// tree methods:

tree::tree()
: node(1)
{ set_link(BACKWARDS_BRANCH, NIL); }

tree::~tree()
{
  // must at least unhook ourselves from the parent so we don't become a lost
  // cousin.
  tree *my_parent = parent();
  if (my_parent) my_parent->prune(this);

  // iterate over the child nodes and whack each individually.
  while (branches()) delete branch(0);
    // this relies on the child getting out of our branch list.
}

tree *tree::parent() const { return (tree *)get_link(BACKWARDS_BRANCH); }

int tree::branches() const { return links() - 1; }

tree *tree::branch(int branch_number) const
{
  branch_number++;
  bounds_return(branch_number, 1, branches(), NIL);
  return (tree *)get_link(branch_number);
}

int tree::which(tree *branch_to_find) const
{ return node::which((node *)branch_to_find) - 1; }

tree *tree::root() const
{
  const tree *traveler = this;
  // keep looking at the backwards branch until it is a NIL.  the tree with
  // a NIL BACKWARDS_BRANCH is the root.  return that tree.
  while (traveler->get_link(BACKWARDS_BRANCH))
    traveler = (tree *)traveler->get_link(BACKWARDS_BRANCH);
  return const_cast<tree *>(traveler);
}

void tree::attach(tree *new_branch)
{
  if (!new_branch) return;
  insert_link(links(), new_branch);
  new_branch->set_link(BACKWARDS_BRANCH, this);
}

void tree::insert(int branch_place, tree *new_branch)
{
  branch_place++;
  insert_link(links(), NIL);
  if (branch_place >= links())
    branch_place = links() - 1;
  for (int i = links() - 1; i > branch_place; i--)
    set_link(i, get_link(i-1));
  set_link(branch_place, new_branch);
  new_branch->set_link(BACKWARDS_BRANCH, this);
}

outcome tree::prune(tree *branch_to_cut)
{
  int branch_number = which(branch_to_cut);
  if (branch_number == basis::common::NOT_FOUND) return basis::common::NOT_FOUND;
  return prune_index(branch_number);
}

outcome tree::prune_index(int branch_to_cut)
{
  branch_to_cut++;
  bounds_return(branch_to_cut, 1, branches(), basis::common::NOT_FOUND);
  tree *that_branch = (tree *)get_link(branch_to_cut);
  that_branch->set_link(BACKWARDS_BRANCH, NIL);
  zap_link(branch_to_cut);
  return basis::common::OKAY;
}

int tree::depth() const
{
  tree *my_root = root();
  const tree *current_branch = this;
  int deep = 0;
  while (current_branch != my_root) {
    current_branch = current_branch->parent();
    deep++;
  }
  return deep;
}

//probably okay; we want to use this function rather than non-existent
//node base function which isn't implemented yet.
bool tree::generate_path(path &to_follow) const
{
if (to_follow.size()) {} 
/*
  tree *traveller = this;
  path to_accumulate(root());
  while (traveller->parent() != NIL) {
//    int branch_number = traveller->parent()->which(traveller);
//    if (branch_number == BRANCH_NOT_FOUND) non_continuable_error
//      (class_name(), "generate_path", "branch not found during path construction");
//    chunk *to_stuff = new chunk
//      (SELF_OWNED, (byte *)&branch_number, sizeof(int));
    to_accumulate.push(traveller);
    traveller = traveller->parent();
  }
  // the order of things on the stack needs to be reversed now.
//  path to_return = new stack(*to_accumulate.invert());
//  return to_return;
  to_accumulate.invert();
  return to_accumulate;
*/
return false;//temp.
}

//hmmm: need text form!

tree::iterator tree::start(traversal_directions direction) const
{ return iterator(this, direction); }

}  // namespace.

