#ifndef SINGLY_LINKED_LIST_CLASS
#define SINGLY_LINKED_LIST_CLASS

/*
*  Name   : singly_linked_list
*  Author : Chris Koeritz
**
* Copyright (c) 1998-$now By Author.  This program is free software; you can
* redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either version 2 of
* the License or (at your option) any later version.  This is online at:
*     http://www.fsf.org/copyleft/gpl.html
* Please send any updates to: fred@gruntose.com
*/
#include "node.h"

namespace nodes {

//class node;  // forward.

//! Implements a singly-linked list structure.

class singly_linked_list : public node
{
public:
  singly_linked_list() : node(1) {}

  //hmmm: clean up all children?
  ~singly_linked_list() {}

  // symbol for the rest of the list linked here.
  static const int NEXT_NODE = 0;

  int elements() const;
    //!< returns the number of items currently in the list, including this node.
    /*!< this is a costly operation. */

  singly_linked_list *next() { return (singly_linked_list *)get_link(NEXT_NODE); }

  void set_next(singly_linked_list *new_next) { set_link(NEXT_NODE, new_next); }

  //! returns true if this list has a cycle in it.
  static bool has_cycle(singly_linked_list *check) {
  	singly_linked_list *a = check;
  	singly_linked_list *b = check;
  	while ((a != NULL_POINTER) && (b!= NULL_POINTER) ) {
  		a = a->next();  // move position of a forward once.
  	  // move position of b forward twice.
  		b = b->next();
  		if (b != NULL_POINTER) b = b->next();

  		if (a == b) {
  			// argh, our single skipper and double skipper have arrived at the same node.
  			// the only way that can happen is if there's a cycle in the list.
  			return true;
  		}
  	}
  	// if we fell out of the list iteration with a null for either pointer,
  	// then there was no cycle in the list.
  	return false;
  }

private:
  // not permitted.
  singly_linked_list(const singly_linked_list &);
  singly_linked_list &operator =(const singly_linked_list &);
};

} // namespace.

#endif

