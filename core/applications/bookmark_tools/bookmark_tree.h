#ifndef BOOKMARK_TREE_CLASS
#define BOOKMARK_TREE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : bookmark_tree                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Parses a link database in HOOPLE format into tree structure.             *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <nodes/symbol_tree.h>
#include <structures/amorph.h>
#include <structures/string_array.h>
#include <structures/string_table.h>

// forward.
class inner_mark_tree;
class link_record;
class listo_links;
class symbol_int;

////////////////////////////////////////////////////////////////////////////

class bookmark_tree
{
public:
  bookmark_tree();
  virtual ~bookmark_tree();
  DEFINE_CLASS_NAME("bookmark_tree");

  int read_csv_file(const basis::astring &input_filename);
    // reads the file contents of "input_filename" into this tree.

  static void break_name(const basis::astring &to_break, basis::astring &name,
          basis::astring &nick);
    // breaks a category name into the two components, if they exist.

  static bool magic_category_comparison(const basis::astring &a, const basis::astring &b);
    // compares the two strings "a" and "b" and returns true if either the
    // main name or the nickname matches either.

  static basis::astring prune_link_down(const basis::astring &to_prune);
    // reduces a URL to its bare bones.  it will strip out the "http://" and "www." and such.

  static bool excellent_link_comparator(const basis::astring &a, const basis::astring &b);
    // a string comparator that handles how links are often formed.  it uses the link pruner
    // to decide whether the links are equal at their root.

  inner_mark_tree *process_category(const structures::string_array &items);
    // handles category declarations and adds the new category to our list.
    // this tries to do the intelligent thing if the category is already
    // found to exist, meaning that the file has a duplicate category
    // definitions.

  void process_link(const structures::string_array &items);

  void process_comment(const basis::astring &current_line_in);

  inner_mark_tree *find_parent(const basis::astring &parent_name);
    // locates the parent called "parent_name" given the context that
    // we've saved about the last parent.

  static bool advance(int &index, const basis::astring &check, const basis::astring &finding);
    //!< moves the "index" forward if the "finding" string is the head of "check".

  static int find_prune_point(const basis::astring &to_prune);
    //!< attempts to locate the real start of the root URL in "to_prune".

  // these provide access to the information held about the tree...

  inner_mark_tree &access_root();  // allows access to the root of the tree.

  int link_count() const { return _link_count; }

  int category_count() const { return _category_count; }

// public data members...  currently this is used outside the class.
  int _line_number;  // the current line in the database.

private:
  inner_mark_tree *_mark_tree;  // our tree of categories.
  int _link_count;  // number of links.
  int _category_count;  // number of categories.
  inner_mark_tree *_last_parent;  // the last parent we saw.
  inner_mark_tree *_last_node;  // the last node we touched.
  symbol_int *_links_seen;  // URLs we've seen.
  structures::string_table *_category_names;  // used to enforce uniqueness of categories.
};

////////////////////////////////////////////////////////////////////////////

class link_record
{
public:
  basis::astring _description;
  basis::astring _url;
  int _uid;

  link_record(const basis::astring &description, const basis::astring &url, int uid)
      : _description(description), _url(url), _uid(uid) {}
};

////////////////////////////////////////////////////////////////////////////

class listo_links : public structures::amorph<link_record>
{
public:
  listo_links();

  void add(link_record *new_rec, bool sort = true);

private:
  int _next_index;  // tracks where we've added unsorted items.
};

////////////////////////////////////////////////////////////////////////////

class inner_mark_tree : public nodes::symbol_tree
{
public:
  listo_links _links;  // the list held at this node.
  int _uid;  // the unique identifier of this node.

  inner_mark_tree(const basis::astring &node_name, int uid, int max_bits = 2)
  : nodes::symbol_tree(node_name, max_bits), _uid(uid) {}

};

////////////////////////////////////////////////////////////////////////////

#endif

