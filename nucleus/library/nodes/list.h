#ifndef LIST_CLASS
#define LIST_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : list                                                              *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1998-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/



namespace nodes {

class node;  // forward.

//! Implements a guarded, doubly linked list structure.
/*!
  The list is viewed one element at a time, through the monocular of an
  iterator, which keeps track of the current position in the list.  the
  iterator's cursor can be shifted around at will.  nodes can be added to
  the list before or after the cursor, and the node pointed at by the cursor
  can be queried or modified.  Using multiple iterators is fine as long as
  you guarantee that they either are all just reading the list or that you
  have a valid access pattern of reads and writes such that no iterator's
  cursor is affected.  Note that this list is not thread safe.
*/

class list
{
public:
  list();
    //!< constructs a blank list.

  ~list();
    //!< invalidates all contents of the list and destroys all child nodes.

  int elements() const;
    //!< returns the number of items currently in the list.
    /*!< this is a costly operation. */

  bool empty() const;
    //!< returns true if the list is empty.
    /*!< this is really quick compared to checking the number of elements. */

  //! iterators allow the list to be traversed.
  /*! NOTE: it is an error to use an iterator on one list with a different
  list; the methods will do nothing or return empty results in this
  situation. */

  class iterator {
  public:
    iterator(const list *mgr, node *start) : _cursor(start), _manager(mgr) {}
      //!< opens up an iterator on a list.
      /*!< the preferred method to construct an iterator is to use the
      head/tail functions in list. */

    void operator ++();  //!< go to next item.
    void operator --();  //!< go to previous item.
    void operator ++(int) { ++(*this); }  //!< post-fix synonym for ++.
    void operator --(int) { --(*this); }  //!< post-fix synonym for --.

  
    void next() { (*this)++; }  //!< synonym for ++.
    void previous() { (*this)--; }  //!< synonyms for --.

    bool operator ==(const iterator &to_compare) const;
      //!< returns true if the two iterators are at the same position.

    bool is_head() const;
      //!< returns true if the cursor is at the head of the list.
      /*!< Note: head() and tail() only return true if the iterator is
      actually positioned _at_ the head or tail guard.  for example, if the
      cursor is pointing at the last actual element in the list (but not at
      the guard itself), then is_tail() returns false.  so, in iteration,
      your iterator will actually go past the last valid element before
      is_tail() returns true.  thus it is very important to check is_tail()
      *before* looking at the node with observe() or access(), since those
      two will shift the list position to the nearest valid node and away
      from the guard. */
    bool is_tail() const;
      //!< returns true if the cursor is at the tail of the list.

    void jump_head();  //!< set the iterator to the head.
    void jump_tail();  //!< set the iterator to the tail.

    const node *observe();  //!< peek at the current element.
      /*!< Note: observe() and access() will return the first element if the
      list is positioned at the head (or the last if at the tail), and will
      not return NULL_POINTER for these two positions as long as the list has some
      elements in it.  the cursor will also have been moved to point at the
      element that is returned.
      Another Note: it is extremely important that you do not mess with the
      links owned by the node (or at least the first two links).
      A Third Note: these two functions can return NULL_POINTER if the list is empty. */
    node *access();  //!< obtain access to the current element.

  private:
    node *_cursor;  //!< the current position.
    friend class list;  //!< lists have full access to this object.
    const list *_manager;  //!< our friendly manager.
  };

  iterator head() const { return iterator(this, _head); }
    //!< returns an iterator located at the head of the list.
  iterator tail() const { return iterator(this, _tail); }
    //!< returns an iterator located at the tail of the list.

  int index(const iterator &it) const { return items_from_head(it); }
    //!< returns the zero-based index of the cursor's position from the head.
    /*!< this is a synonym for items_from_head(). */

  bool set_index(iterator &to_set, int new_index);
    //!< positions the iterator "to_set" at the index specified.

  // storage and retrieval actions.
  // Note: all of these methods shift the iterator onto the next valid node
  // if it is positioned at the beginning or end of the list.

  void append(iterator &where, node *new_node);
    //!< adds a "new_node" into the list immediately after "where".
    /*!< the nodes previously following the current node (if any) will follow
    after the "new_node".  this does not change the current position.
    ownership of "new_node" is taken over by the list. */

  void insert(iterator &where, node *new_node);
    //!< places a "new_node" immediately before the current node in the list.
    /*!< the "new_node" will follow the prior precursor to the current node.
    this does not change the current position.  ownership of "new_node"
    is taken over by the list.  after the call, the iterator points at
    the new node. */

  node *remove(iterator &where);
    //!< extracts the current item from "where" and repairs the hole.
    /*!< NULL_POINTER is returned if the list was empty.  the current position is set
    to the node that used to be after the node that has been removed.  after
    the call, the iterator points at the new node. */

  void zap(iterator &where);
    //!< wipes out the item at "where", including its contents.
    /*!< the current position is reset to the item after the now defunct
    item (or the tail). */

  void zap_all();
    //!< destroys all the list elements and makes the list empty.
    /*!< any existing iterators will be invalidated by this. */

  // the following positioning functions could fail if the request is out of
  // bounds.  for example, forward cannot go beyond the end of the list.  in
  // such cases, false is returned and the list pointer is not moved.

  bool forward(iterator &where, int count);
    //!< moves the list pointer "count" items towards the tail.
    /*!< Note: forward and backward operate on elements; the head and tail
    guard are not included in the count.  Also, negative values of "count"
    are ignored. */
  bool backward(iterator &where, int count);
    //!< moves the list pointer "count" items towards the head.

  int items_from_head(const iterator &where) const;
    //!< Returns the number of elements between the current item and the head.
    /*!< zero is returned if this is at the first element or the head. */
  int items_from_tail(const iterator &where) const;
    //!< Returns the number of elements between the current item and the tail.
    /*!< zero is returned if this is at the last element or the tail. */

private:
  friend class iterator;
  node *_head;  //!< pointer to the first item.
  node *_tail;  //!< pointer to the last item.

  bool skip_or_ignore(iterator &where, int count);
    //!< zips the list to the position indicated by "count", if it can.

  // not permitted.
  list(const list &);
  list &operator =(const list &);
};

} // namespace.

#endif

