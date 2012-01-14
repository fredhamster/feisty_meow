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

#include "symbol_tree.h"

#include <basis/functions.h>
#include <structures/symbol_table.h>
#include <textual/parser_bits.h>
#include <textual/string_manipulation.h>

//#define DEBUG_SYMBOL_TREE
  // uncomment for totally noisy version.

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

using namespace basis;
using namespace structures;
using namespace textual;

namespace nodes {

class symbol_tree_associations : public symbol_table<symbol_tree *>
{
public:
  symbol_tree_associations(int estimated_elements)
      :  symbol_table<symbol_tree *>(estimated_elements) {}
};

//////////////

symbol_tree::symbol_tree(const astring &node_name, int estimated_elements)
: tree(),
  _associations(new symbol_tree_associations(estimated_elements)),
  _name(new astring(node_name))
{
}

symbol_tree::~symbol_tree()
{
  WHACK(_name);
  WHACK(_associations);
}

int symbol_tree::children() const { return _associations->symbols(); }

const astring &symbol_tree::name() const { return *_name; }

int symbol_tree::estimated_elements() const { return _associations->estimated_elements(); }

void symbol_tree::rehash(int estimated_elements) { _associations->rehash(estimated_elements); }

void symbol_tree::hash_appropriately(int estimated_elements)
{ _associations->hash_appropriately(estimated_elements); }

bool symbol_tree::add(symbol_tree *to_add)
{
#ifdef DEBUG_SYMBOL_TREE
  FUNCDEF("add");
  LOG(astring("adding node for ") + to_add->name());
#endif
  attach(to_add);  // add to tree.
  _associations->add(to_add->name(), to_add);  // add to associations.
  return true;
}

outcome symbol_tree::prune(tree *to_zap_in)
{
#ifdef DEBUG_SYMBOL_TREE
  FUNCDEF("prune");
#endif
  symbol_tree *to_zap = dynamic_cast<symbol_tree *>(to_zap_in);
  if (!to_zap)
    throw("error: symbol_tree::prune: wrong type of node in prune");
#ifdef DEBUG_SYMBOL_TREE
  LOG(astring("zapping node for ") + to_zap->name());
#endif
  int found = which(to_zap);  // find the node in the tree.
  if (negative(found)) return common::NOT_FOUND;  // not found.
#ifdef DEBUG_SYMBOL_TREE
  int kids = _associations->symbols();
#endif
  _associations->whack(to_zap->name());  // remove from associations.
#ifdef DEBUG_SYMBOL_TREE
  if (kids - 1 != _associations->symbols())
    throw("error: symbol_tree::prune: failed to crop kid in symtab");
#endif
  tree::prune(to_zap);  // remove from tree.
  return common::OKAY;
}

symbol_tree *symbol_tree::branch(int index) const
{ return dynamic_cast<symbol_tree *>(tree::branch(index)); }

// implementation snagged from basis/shell_sort.
void symbol_tree::sort()
{
  int n = branches();
  symbol_tree *temp;
  int gap, i, j;
  for (gap = n / 2; gap > 0; gap /= 2) {
    for (i = gap; i < n; i++) {
      for (j = i - gap; j >= 0 && branch(j)->name() > branch(j + gap)->name();
          j = j - gap) {
        // swap the elements that are disordered.
        temp = branch(j + gap);
        prune_index(j + gap);
        insert(j, temp);
        temp = branch(j + 1);
        prune_index(j + 1);
        insert(j + gap, temp);
      }
    }
  }
}

symbol_tree *symbol_tree::find(const astring &to_find, find_methods how,
    string_comparator_function *comp)
{
#ifdef DEBUG_SYMBOL_TREE
  FUNCDEF("find");
#endif
  if (comp == NIL) comp = astring_comparator;
#ifdef DEBUG_SYMBOL_TREE
  LOG(astring("finding node called ") + to_find);
#endif
  // ensure that we compare the current node.
  if (comp(name(), to_find)) return this;

  // perform the upward recursion first, since it's pretty simple.
  if (how == recurse_upward) {
    symbol_tree *our_parent = dynamic_cast<symbol_tree *>(parent());
    if (!our_parent) return NIL;  // done recursing.
    return our_parent->find(to_find, how, comp);
  }

  // see if our branches match the search term.
  symbol_tree **found = _associations->find(to_find, comp);
#ifdef DEBUG_SYMBOL_TREE
  if (!found) LOG(to_find + " was not found.")
  else LOG(to_find + " was found successfully.");
#endif
  if (!found) {
    if (how == recurse_downward) {
      // see if we can't find that name in a sub-node.
      symbol_tree *answer = NIL;
      for (int i = 0; i < branches(); i++) {
        // we will try each branch in turn and see if it has a child named
        // appropriately.
        symbol_tree *curr = dynamic_cast<symbol_tree *>(branch(i));
#ifdef DEBUG_SYMBOL_TREE
        LOG(astring("recursing to ") + curr->name());
#endif
        if (curr)
          answer = curr->find(to_find, how, comp);
        if (answer)
          return answer;
      }
    }
    return NIL;
  }
  return *found;
}

//hmmm: is this useful elsewhere?
astring hier_prefix(int depth, int kids)
{
  astring indent = string_manipulation::indentation( (depth - 1) * 2);
  if (!depth) return "";
  else if (!kids) return indent + "|--";
  else return indent + "+--";
}

astring symbol_tree::text_form() const
{
#ifdef DEBUG_SYMBOL_TREE
  FUNCDEF("text_form");
#endif
  astring to_return;

  tree::iterator ted = start(prefix);
    // create our iterator to do a prefix traversal.

  tree *curr = (tree *)ted.next();

//hmmm: this cast assumes that the tree only contains trees.  for more
//      safety, we might want a dynamic cast here also.
  while (curr) {
    // we have a good directory to show.
    symbol_tree *curr_cast = dynamic_cast<symbol_tree *>(curr);
    if (!curr_cast) {
      // something very bad with that...
#ifdef DEBUG_SYMBOL_TREE
      LOG("logic error: unknown type in symbol tree.");
#endif
      ted.next();
      continue;
    }
    astring name_to_log = curr_cast->name();
    if (!ted.size()) name_to_log = "";
#ifdef DEBUG_SYMBOL_TREE
    LOG(a_sprintf("depth %d kids %d name %s", ted.size(), curr_cast->branches(),
        name_to_log.s()));
#endif
    to_return += hier_prefix(curr->depth(), curr_cast->branches());
    to_return += name_to_log;
    to_return += parser_bits::platform_eol_to_chars();

    curr = (tree *)ted.next();
  }

  return to_return;
}

} // namespace.

