/*****************************************************************************\
*                                                                             *
*  Name   : bookmark_tree                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "bookmark_tree.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <filesystem/byte_filer.h>
#include <filesystem/filename.h>
#include <loggers/critical_events.h>
#include <loggers/file_logger.h>
#include <loggers/program_wide_logger.h>
#include <nodes/symbol_tree.h>
#include <structures/amorph.h>
#include <structures/string_table.cpp>
#include <structures/symbol_table.h>
#include <textual/list_parsing.h>
#include <textual/parser_bits.h>

///#include <stdio.h>//temp

using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace nodes;
using namespace structures;
using namespace textual;

#define DEBUG_MARKS
  // uncomment to have more debugging noise, but a reasonable amount.
//#define DEBUG_MARKS_TREE
  // uncomment to get crazy noisy debug noise about tree traversal.

#undef BASE_LOG
#define BASE_LOG(s) program_wide_logger::get().log(s)
#define SHOW_LINE a_sprintf("line %d: ", _line_number)
#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), SHOW_LINE + s)
#define DEADLY_LINE (astring(func) + a_sprintf(", line %d: ", _line_number))

const int ESTIMATED_ELEMENTS = 100;
  // we're planning for about this many links to be efficiently handled.

const int MAX_LINE_SIZE = 256 * KILOBYTE;
  // the largest line we'll process in the links database.

// used to compare two strings while ignoring case; we use this to find
// our categories in the symbol table.
bool case_insense_compare(const astring &a, const astring &b)
{ return a.iequals(b); }

////////////////////////////////////////////////////////////////////////////

listo_links::listo_links() : amorph<link_record>(), _next_index(0) {}

void listo_links::add(link_record *new_rec, bool sort)
{
  // we don't sort blank lines--they just get dropped in the end of
  // the section.
  if (sort && new_rec->_description.t()) {
    for (int i = _next_index; i < elements(); i++) {
      const astring &desc_cur = borrow(i)->_description;
//this check doesn't make much sense; it only checks if the description is equal?
// if it were really sorting, wouldn't it need to check if the check is greater than current?
      if (desc_cur.iequals(new_rec->_description)
//          || shouldn't there be a case for this being greater than the current???
          || !desc_cur) {
        insert(i + 1, 1);
        put(i + 1, new_rec);
        return;
      }
    }
  }
  append(new_rec);
  if (!sort)
    _next_index = elements();
}

////////////////////////////////////////////////////////////////////////////

class symbol_int : public symbol_table<int>
{
public:
  symbol_int() : symbol_table<int>(10) {}
};

////////////////////////////////////////////////////////////////////////////

bookmark_tree::bookmark_tree()
: _line_number(0),
  _mark_tree(new inner_mark_tree("Root", 0, ESTIMATED_ELEMENTS)),
  _link_count(0),
  _category_count(0),
  _last_parent(_mark_tree),
  _last_node(_mark_tree),
  _links_seen(new symbol_int),
  _category_names(new string_table)
{}

bookmark_tree::~bookmark_tree()
{
  WHACK(_links_seen);
  WHACK(_mark_tree);
  WHACK(_category_names);
}

void bookmark_tree::break_name(const astring &to_break, astring &name,
    astring &nick)
{
  nick = astring::empty_string();
  name = to_break;
  int indy = name.find('[');
  if (negative(indy)) return;
  nick = name.substring(indy + 1, name.end());
  while ( (nick[nick.end()] == ' ') || (nick[nick.end()] == ']') )
    nick.zap(nick.end(), nick.end());
  name.zap(indy, name.end());
  name.strip_spaces();
  nick.strip_spaces();
}

inner_mark_tree &bookmark_tree::access_root() { return *_mark_tree; }

bool bookmark_tree::magic_category_comparison(const astring &a, const astring &b)
{
//  FUNCDEF("magic_category_comparison");
//LOG(astring("compare: a=") + a + " b=" + b);
  if (a.iequals(b)) return true;
  astring a_name, a_nick;
  break_name(a, a_name, a_nick);
  astring b_name, b_nick;
  break_name(b, b_name, b_nick);
  if (a_name.iequals(b_name)) return true;
  if (a_nick.t() && a_nick.iequals(b_name)) return true;
  if (b_nick.t() && a_name.iequals(b_nick)) return true;
  if (a_nick.t() && b_nick.t() && a_nick.iequals(b_nick)) return true;
  return false;
}

const astring &HTTP_HEAD = "http://";
const astring &FTP_HEAD = "ftp://";
const astring &WWW_SITE = "www.";
const astring &FTP_SITE = "ftp.";

bool bookmark_tree::advance(int &index, const astring &check, const astring &finding)
{
  if (check.compare(finding, index, 0, finding.length(), false)) {
    index += finding.length();
    return true;
  }
  return false;
}

int bookmark_tree::find_prune_point(const astring &to_prune)
{
  int to_return = 0;
  advance(to_return, to_prune, HTTP_HEAD);
  advance(to_return, to_prune, FTP_HEAD);
  advance(to_return, to_prune, WWW_SITE);
  advance(to_return, to_prune, FTP_SITE);
  return to_return;
}

astring bookmark_tree::prune_link_down(const astring &to_prune)
{
//printf("%s\n", (astring("pruned=") + to_prune.substring(find_prune_point(to_prune), to_prune.end())).s());
 return to_prune.substring(find_prune_point(to_prune), to_prune.end()); }

bool bookmark_tree::excellent_link_comparator(const astring &a, const astring &b)
{
  int prune_a = find_prune_point(a);
  int prune_b = find_prune_point(b);
  int bigger_len = maximum(a.length() - prune_a, b.length() - prune_b);
  bool to_return = a.compare(b, prune_a, prune_b, bigger_len, false);
//if (to_return) printf("%s and %s are equal.", a.s(), b.s());
  return to_return;
}

inner_mark_tree *bookmark_tree::find_parent(const astring &parent_name)
{
  FUNCDEF("find_parent");
  // first, look for the node above the last parent.
  inner_mark_tree *parent = dynamic_cast<inner_mark_tree *>
      (_last_parent->find(parent_name, inner_mark_tree::recurse_upward,
       magic_category_comparison));

#ifdef DEBUG_MARKS_TREE
  if (parent)
    LOG(astring("trying upwards find for parent node ") + parent_name);
#endif

  if (!parent) {
#ifdef DEBUG_MARKS_TREE
    LOG(astring("upwards find failed seeking on last_parent node ")
        + parent_name);
#endif

    // we didn't find the parent above the last category...
    parent = dynamic_cast<inner_mark_tree *>(_last_node->find(parent_name,
        inner_mark_tree::recurse_upward, magic_category_comparison));
  }

  if (!parent) {
#ifdef DEBUG_MARKS_TREE
    LOG(astring("upwards find failed seeking on last_node ") + parent_name);
#endif

    // last node didn't help either.
    parent = dynamic_cast<inner_mark_tree *>(_mark_tree->find(parent_name,
        inner_mark_tree::recurse_downward, magic_category_comparison));
  }
  if (!parent) {
    // failed to find the parent node, so hook it to the root node.
    LOG(astring("failed to find parent node ") + parent_name);

    // create a parent node and use it for this guy.
    inner_mark_tree *new_node = new inner_mark_tree(parent_name,
        _line_number, ESTIMATED_ELEMENTS);
    _mark_tree->attach(new_node);
    _mark_tree->sort();
    _category_count++;

    parent = new_node;
  } else {
#ifdef DEBUG_MARKS_TREE
    LOG(astring("found parent node ") + parent_name);
#endif
  }

  return parent;
}

inner_mark_tree *bookmark_tree::process_category(const string_array &items)
{
  FUNCDEF("process_category");
  const astring &category_name = items[1];
  const astring &parent_name = items[2];

  if (items.length() > 3) {
    // complain about a possibly malformed category.
    LOG(astring("category ") + category_name + " under " + parent_name
        + " has extra fields!");
  }

//BASE_LOG("CURRENT:");
//BASE_LOG(_mark_tree->text_form());

  // make sure we don't add anything to the tree if this is the root.
  if (!parent_name || magic_category_comparison("Root", category_name)) {
#ifdef DEBUG_MARKS_TREE
    LOG(astring("skipping parent node for ") + category_name);
#endif
    return _mark_tree;
  }

  // ensure that the categories aren't competing with other names.
  astring main_name, nickname;
  break_name(category_name, main_name, nickname);
  astring *found1 = _category_names->find(main_name, case_insense_compare);
  astring *found2 = _category_names->find(nickname, case_insense_compare);
  if (found1 || found2) {
    astring catnames;
    if (found1) catnames = *found1;  // add the first match, if it exists.
    if (found2) {
      if (!!catnames) catnames += " and ";
      catnames += *found2;
    }
    LOG(astring("category name \"") + category_name
        + "\" in conflict with existing: " + catnames);
    inner_mark_tree *fake_it = NIL;

//hmmm: neither of these are right; they need to use a comparator that
//      uses our magic comparison function.

    if (found1) {
#ifdef DEBUG_MARKS
      LOG(astring("found existing category for main name: ") + main_name);
#endif
//      fake_it = (inner_mark_tree *)_mark_tree->find(*found1,
//          symbol_tree::recurse_downward);
      fake_it = dynamic_cast<inner_mark_tree *>(_mark_tree->find
          (*found1, inner_mark_tree::recurse_downward,
           magic_category_comparison));
    }
    if (fake_it) {
#ifdef DEBUG_MARKS
      LOG(astring("returning existing category for main name: ") + main_name
          + " as: " + fake_it->name());
#endif
      return fake_it;
    }
    if (found2) {
#ifdef DEBUG_MARKS
      LOG(astring("found existing category for nickname: ") + nickname);
#endif
///      fake_it = (inner_mark_tree *)_mark_tree->find(*found2,
///          symbol_tree::recurse_downward);
      fake_it = dynamic_cast<inner_mark_tree *>(_mark_tree->find
          (*found2, inner_mark_tree::recurse_downward,
           magic_category_comparison));
    }
    if (fake_it) {
#ifdef DEBUG_MARKS
      LOG(astring("returning existing category for nickname: ") + nickname
          + " as: " + fake_it->name());
#endif
      return fake_it;
    }
    LOG("==> failure to find a match for either category!");
    deadly_error(class_name(), func, "collision resolution code failed; "
        "please fix category error");
    return NIL;
  }
  // now that we know these names are unique, we'll add them into the list
  // so future categories can't reuse these.
  _category_names->add(main_name, main_name);
  if (!!nickname) _category_names->add(nickname, nickname);

  inner_mark_tree *parent = find_parent(parent_name);
  _last_parent = parent;  // set the parent for the next time.

  // see if the category is already present under the parent.
  for (int i = 0; i < parent->branches(); i++) {
    inner_mark_tree *curr = dynamic_cast<inner_mark_tree *>(parent->branch(i));
    if (!curr)
      non_continuable_error(class_name(), DEADLY_LINE, "missing branch in tree");
#ifdef DEBUG_MARKS_TREE
    LOG(astring("looking at branch ") + curr->name());
#endif
    if (magic_category_comparison(curr->name(), category_name)) {
      // it already exists?  argh.
      LOG(astring("category ") + category_name + " already exists under "
          + parent_name + ".");
      _last_node = curr;
      return curr;
    }
  }

  inner_mark_tree *new_node = new inner_mark_tree(category_name,
      _line_number, ESTIMATED_ELEMENTS);
  parent->attach(new_node);
  parent->sort();
  _last_node = new_node;

  _category_count++;

#ifdef DEBUG_MARKS_TREE
  LOG(astring("attaching node ") + category_name + " to parent "
      + parent->name());
#endif
  return new_node;
}

void bookmark_tree::process_link(const string_array &items)
{
  FUNCDEF("process_link");
  astring description = items[1];
  astring parent_name = items[2];
  astring url = "UNKNOWN";
  if (items.length() >= 4) url = items[3];

  // strip any directory slashes that are provided as a suffix.  we don't need
  // them and they tend to confuse the issue when we look for duplicates.
  while (url[url.end()] == '/') {
    url.zap(url.end(), url.end());
  }

  // make some noise if they seem to have a badly formed link.
  if (items.length() < 4) {
    LOG(astring("link ") + description + " under " + parent_name
        + " has no URL!");
  } else if (items.length() > 4) {
    LOG(astring("link ") + description + " under " + parent_name
        + " has extra fields!");
  }

  // find the parent for this link.
  inner_mark_tree *parent = find_parent(parent_name);
  _last_parent = parent;  // set the parent for the next time.

  // see if the link already exists.
  int *found = _links_seen->find(url, excellent_link_comparator);
  if (found) {
    // this is not so great; a duplicate link has been found.
    LOG(a_sprintf("Duplicate Link: line %d already has ", *found) + url);
    return;
  } else {
    _links_seen->add(prune_link_down(url), _line_number);
  }

  // add the link to the parent.
  link_record *new_rec = new link_record(description, url, _line_number);
  parent->_links.add(new_rec);

  _link_count++;
}

void bookmark_tree::process_comment(const astring &current_line_in)
{
///  FUNCDEF("process_comment");
  astring current_line = current_line_in;

  // output the comment as simple text.
//BASE_LOG("comment case");
  if (current_line.contains("http:")) {
    astring hold_curr = current_line;
    int indy = current_line.find("http:");
    hold_curr.zap(0, indy - 1);
    current_line = astring("&nbsp; &nbsp; &nbsp; &nbsp; "
        "<a href=\"") + hold_curr + "\">" + hold_curr + "</a>";
  } else if (current_line.t()) {
    // snap the comment character off of the front.
    current_line.zap(0, 0);
  }

  link_record *new_rec = new link_record(current_line,
      astring::empty_string(), _line_number);
  _last_node->_links.add(new_rec, false);
}

int bookmark_tree::read_csv_file(const astring &input_filename)
{
  FUNCDEF("read_csv_file");
  byte_filer input_file(input_filename, "r");
  if (!input_file.good())
    non_continuable_error(class_name(), DEADLY_LINE,
        "the input file could not be opened");

  string_array items;  // parsed in csv line.
  astring current_line;  // read from input file.

  // read the lines in the file, one at a time.
  while (input_file.getline(current_line, MAX_LINE_SIZE) > 0) {
    _line_number++;  // go to the next line.
    // remove the carriage returns first.
    while (parser_bits::is_eol(current_line[current_line.end()])) {
      current_line.zap(current_line.end(), current_line.end());
    }
    current_line.strip_spaces();
    if (!current_line.length()) {
//      // blank lines get treated as a special case.  they are always added
//      // at the end of the list.
//      process_comment(current_line);
      continue;
    } else if (current_line[0] == '#') {
      // handle a comment in the database.
      process_comment(current_line);
    } else {
      // csv parse the line, since we don't support much else.
      bool parsed = list_parsing::parse_csv_line(current_line, items);
      if (!parsed)
        non_continuable_error(class_name(), DEADLY_LINE,
            astring("the line could not be parsed: ") + current_line);
      if (!items.length()) {
        LOG("bad formatting on this line.");
        continue;
      }
      if (items[0].iequals("C")) {
        inner_mark_tree *node = process_category(items);
        if (!node) {
          LOG(astring("failed to get a node for ") + items[1]);
        }
      } else if (items[0].iequals("L")) {
        process_link(items);
      } else {
        non_continuable_error(class_name(), DEADLY_LINE,
            astring("unknown format in line: ") + current_line);
      }
    }
  }
  return 0;
}

