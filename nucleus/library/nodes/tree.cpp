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
#include <loggers/program_wide_logger.h>
#include <structures/stack.h>

//#define DEBUG_TREE
  // uncomment if you want lots of debugging info.

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), astring(s) + a_sprintf(" [obj=%p]", this))

using namespace basis;
using namespace loggers;
using namespace structures;

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
  to_return = NULL_POINTER;
#ifdef DEBUG_TREE
  if ( (_order != to_branches)
      && (_order != reverse_branches) ) {
    if (_aim == AWAY_FROM_ROOT) LOG("going down...")
    else LOG("going up...");
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
  tree *to_return = NULL_POINTER;
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
{ set_link(BACKWARDS_BRANCH, NULL_POINTER); }

tree::~tree()
{
  FUNCDEF("destructor");
#ifdef DEBUG_TREE
  LOG("entry to tree destructor");
#endif
  {
    // must at least unhook ourselves from the parent so we don't become a lost
    // cousin.
    tree *my_parent = parent();
    if (my_parent) my_parent->prune(this);
  }

#if 0
  //hmmm: clean this code when it's been examined long enough.  maybe already.
LOG("old code for tree destructor activating...");

  //original version suffers from being too recursive on windoze,
  //which blows out the stack.  linux never showed this problem.  so, i
  //guess i'm glad i tested on windows.
  //anyway, it's a general problem for a degenerate tree like the one
  //i've constructed.  current version has ~40000 direct descendants of
  //the root in a single line, so the stack has to support 40000 frames
  //for the delete implementation below to work.

  // iterate over the child nodes and whack each individually.
  while (branches()) delete branch(0);
    // this relies on the child getting out of our branch list.
#endif

#if 0
  LOG("new code for tree destructor activating...");

//hmmm: this new code stinks on ice.  keeps doing something awful, either double deleting or deleting wrong thing.
//      why didn't we just use our own iterator to traverse the tree depth first and clean that way??

  // newer version of delete doesn't recurse; it just iterates instead,
  // which avoids the massive recursive depth of the original approach.
  while (branches()) {
    // we know we have a branch to work on, due to conditional above.
    tree *stop_node = branch(0);  // don't whack the node we're at until we're done with its kids.
    tree *curr_node = stop_node;
    while (curr_node != NULL_POINTER) {
      // make a breadcrumb for getting back to 'here' in the tree.
      tree *way_back = curr_node;
      // our main operation here is to go down a node without using any
      // stack frames.  so we just pick the first kid; it's either valid
      // or there are no kids at all.
      curr_node = curr_node->branch(0);

      if (curr_node == NULL_POINTER) {
        // wayback has no children, so we can take action.
LOG("tree dtor: can take action on childless node...");

        // if wayback is the same as stop_node, then we exit from iterations since
        // we've cleaned all the kids out.
        if (way_back == stop_node) {
LOG("tree dtor: stopping since at wayback node...");
          // we can actually delete wayback here, because we are at the stopping point.
          // and since a tree node whack removes it from its parent, we are clean one branch now.
          WHACK(way_back);
LOG("tree dtor: whacked wayback before inner break.");
          break;
        }

        // we want to whack the wayback node at this point, since it's a parent
        // with no kids, i.e. a leaf.  we've guaranteed we're not going beyond
        // our remit, since wayback is not the same node as the top level one
        // in the destructor (as long as there are no cycles in the tree...).
        curr_node = way_back->parent();  // go up in tree on next iteration.

LOG("tree dtor: reset currnode, about to whack wayback...");

        if (curr_node == stop_node) {
LOG("tree dtor: seeing curr_node hits our stop_node!");
///          break;
        } else if (curr_node == stop_node->parent()) {
LOG("tree dtor: seeing curr_node ABOVE our stop_node!");
        }

        WHACK(way_back);  // whack a node, finally.

LOG("tree dtor: after whacking wayback...");
      } else {
        // okay, there's a node below here.  we will spider down to it.
LOG("tree dtor: spidering to node below...");
        continue;
      }
    }
  }
#endif

#if 1
#ifdef DEBUG_TREE
  LOG("newest code for tree destructor (stack based) activating...");
#endif

  // this version of delete doesn't recurse; it just iterates instead,
  // which avoids the massive recursive depth of the original approach.
  // it does use a stack object however, but that will use main memory,
  // which is usually in greater supply than stack/heap space.

  stack<tree *> sleestak;  // accumulates nodes that we still need to clean up.

  // iterate across this node first, to feed the stack.
  // we feed it in reverse branch order, so that the first branch will be dealt with first (popped off stack first).
  while (branches()) {
    // get our current last branch...
    tree *bran = branch(branches() - 1);
    // pull that branch off the tree (non-destructively)...
    prune(bran);
    // and add that branch to the stack.
#ifdef DEBUG_TREE
    LOG(a_sprintf("tree dtor: adding child %p on stack.", bran));
#endif
    sleestak.push(bran);
  }

  // now iterate across our stack until we have dealt with every node in it.
  while (sleestak.size()) {

    tree *popper = NULL_POINTER;
    sleestak.acquire_pop(popper);
#ifdef DEBUG_TREE
    LOG(a_sprintf("tree dtor: popped a tree %p off stack.", popper));
#endif
    
    // familiar scheme below; push last branch first, then we'll pop first branch first.
    while (popper->branches()) {
      tree *bran = popper->branch(popper->branches() - 1);
      popper->prune(bran);
#ifdef DEBUG_TREE
    LOG(a_sprintf("tree dtor: inner, adding child %p on stack.", bran));
#endif
      sleestak.push(bran);
    }

    // now the popper gets cleaned up; this should be totally safe since all its kids
    // have been pruned and put into the stack.
#ifdef DEBUG_TREE
    LOG(a_sprintf("tree dtor: about to zap a tree %p.", popper));
#endif
    WHACK(popper);
#ifdef DEBUG_TREE
    LOG("tree dtor: whacked that tree.");
#endif
  }
#endif
}

tree *tree::parent() const { return (tree *)get_link(BACKWARDS_BRANCH); }

int tree::branches() const { return links() - 1; }

tree *tree::branch(int branch_number) const
{
  branch_number++;
  bounds_return(branch_number, 1, branches(), NULL_POINTER);
  return (tree *)get_link(branch_number);
}

int tree::which(tree *branch_to_find) const
{ return node::which((node *)branch_to_find) - 1; }

tree *tree::root() const
{
  const tree *traveler = this;
  // keep looking at the backwards branch until it is a NULL_POINTER.  the tree with
  // a NULL_POINTER BACKWARDS_BRANCH is the root.  return that tree.
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
  insert_link(links(), NULL_POINTER);
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
  FUNCDEF("prune_index");
  branch_to_cut++;
  bounds_return(branch_to_cut, 1, branches(), basis::common::NOT_FOUND);
#ifdef DEBUG_TREE
  LOG(a_sprintf("got legit branch %d to cut", branch_to_cut));
#endif
  tree *that_branch = (tree *)get_link(branch_to_cut);
#ifdef DEBUG_TREE
  LOG(a_sprintf("whacking off branch %p", that_branch));
#endif
  that_branch->set_link(BACKWARDS_BRANCH, NULL_POINTER);
#ifdef DEBUG_TREE
  LOG(a_sprintf("zapping link branch %d", branch_to_cut));
#endif
  zap_link(branch_to_cut);
#ifdef DEBUG_TREE
  LOG("returning, finished.");
#endif
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
  while (traveller->parent() != NULL_POINTER) {
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

