/*****************************************************************************\
*                                                                             *
*  Name   : node                                                              *
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

#include <basis/functions.h>
#include <basis/guards.h>
#include <structures/amorph.h>

using namespace basis;
using namespace structures;

namespace nodes {

// the internal_link class anonymously hangs onto a pointer to the object.
struct internal_link {
  node *_connection;
  internal_link(node *destination = NIL) : _connection(destination) {}
  virtual ~internal_link() { _connection = NIL; }
};

class node_link_amorph : public amorph<internal_link>
{
public:
  node_link_amorph(int num) : amorph<internal_link>(num) {}
};

//////////////

node::node(int number_of_links)
: _links(new node_link_amorph(number_of_links))
{ for (int i = 0; i < number_of_links; i++) set_empty(i); }

node::~node()
{
  _links->reset();
  WHACK(_links);
}

int node::links() const { return _links->elements(); }

// set_empty: assumes used correctly by internal functions--no bounds return.
void node::set_empty(int link_num)
{
  internal_link *blank_frank = new internal_link(NIL);
  _links->put(link_num, blank_frank);
}

#define test_arg(link_num) bounds_return(link_num, 0, _links->elements()-1, );

void node::set_link(int link_number, node *new_link)
{
  test_arg(link_number);
  (*_links)[link_number]->_connection = new_link;
}

void node::zap_link(int link_number)
{
  test_arg(link_number);
  _links->zap(link_number, link_number);
}

void node::insert_link(int where, node *to_insert)
{
  // make sure that the index to insert at will not be rejected by the
  // amorph insert operation.
  if (where > links())
    where = links();
  _links->insert(where, 1);
  set_empty(where);
  set_link(where, to_insert);
}

node *node::get_link(int link_number) const
{
  bounds_return(link_number, 0, _links->elements()-1, NIL);
  return (*_links)[link_number]->_connection;
}

int node::which(node *branch_to_find) const
{
  int to_return = common::NOT_FOUND;
  for (int i = 0; i <= links() - 1; i++)
    if (branch_to_find == get_link(i)) {
      to_return = i;
      break;
    }
  return to_return;
}

} // namespace.

