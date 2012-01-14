#ifndef NODE_CLASS
#define NODE_CLASS

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

#include <basis/definitions.h>

namespace nodes {

// forward:
class node_link_amorph;

//! An object representing the interstitial cell in most linked data structures.
/*!
  The node is intended as an extensible base class that provides general
  connectivity support.  Nodes can be connected to each other in
  arbitrary ways, but often a derived data type provides more structured
  organization.  When a node's link is zapped, only the connection is
  cut; no destruction is performed.

  Examples: A linked list can be represented as a node with one link that
  connects to the succeeding item in the list.  A doubly linked list can
  be represented by a node with two links; one to the previous node and
  the other to the next node.  The most general structure might be an
  arbitrary graph that can connect nodes to any number of other nodes.
*/

class node : public virtual basis::root_object
{
public:
  node(int number_of_links = 0);
    //!< the constructor provides for "number_of_links" links initially.
    /*!< the table below gives some common numbers for links for a variety of
    data structures: @code
    
    Links   Data Structure             Purpose of Link(s)
    ------  -------------------------  ----------------------------------
      1     singly linked list         points to next node in list
      2     doubly linked list         one to previous node, one to next
      2     binary tree                point to the two children
      n     n-ary tree                 point to the n children
     n+1    n-ary doubly linked tree   point to n children and 1 parent
      m     m-ary graph node           point to m relatives.
    @endcode */

  virtual ~node();
    //!< the destructor simply invalidates the node.
    /*!< this does not rearrange the links as would be appropriate for a
    data structure in which only the node to be destroyed is being eliminated.
    policies regarding the correct management of the links must be made in
    objects derived from node. */

  int links() const;
    //!< Returns the number of links the node currently holds.

  void set_link(int link_number, node *new_link);
    //!< Connects the node "new_link" to this node.
    /*!< the previous link is lost, but not modified in any way.  the address
    of the new link is used directly--no copy of the node is made.  setting a
    link to a null node pointer clears that link. */

  node *get_link(int link_number) const;
    //!< Returns the node that is connected to the specified "link_number".
    /*!< if the link is not set, then NIL is returned. */

  void zap_link(int link_number);
    //!< the specified link is removed from the node.

  void insert_link(int where, node *to_add = NIL);
    //!< adds a new link prior to the position specified in "where".
    /*!< thus a "where" value of less than or equal to zero will add a new
    link as the first element.  a "where" value greater than or equal to
    links() will add a new link after the last element.  the node "to_add"
    will be stored in the new link. */

  int which(node *to_find) const;
    //!< locates the index where "to_find" lives in our list of links.
    /*!< returns the link number for a particular node that is supposedly
    connected to this node or common::NOT_FOUND if the node is not one
    of the children. */

private:
  node_link_amorph *_links;  //!< the list of connections to other nodes.

  void set_empty(int link_number);
    //!< clears the link number specified.

  // disallowed:
  node(const node &);
  node &operator =(const node &);
};

//////////////

//! the basket class holds an object and supports connecting them as nodes.
/*!
  the templated object is required to provide both a default constructor
  and a copy constructor.
*/

template <class contents>
class basket : public node
{
public:
  basket(int links, const contents &to_store = contents())
          : node(links), _storage(to_store) {}

  basket(const basket &to_copy) { *this = to_copy; }

  basket &operator = (const contents &to_copy)
          { if (&to_copy != this) _storage = to_copy; return *this; }

  const contents &stored() const { return _storage; }
    //!< allows a peek at the stored object.
  contents &stored() { return _storage; }
    //!< provides access to the stored object.

private:
  contents _storage;
};

} // end namespace.

#endif

