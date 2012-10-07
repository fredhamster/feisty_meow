/*****************************************************************************\
*                                                                             *
*  Name   : directory_tree                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2004-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "directory.h"
#include "directory_tree.h"
#include "filename.h"
#include "filename_list.h"
#include "filename_tree.h"

#include <basis/functions.h>
#include <basis/contracts.h>
#include <loggers/program_wide_logger.h>
#include <structures/object_packers.h>
#include <structures/string_array.h>
#include <textual/parser_bits.h>
#include <textual/string_manipulation.h>

#include <stdio.h>

using namespace basis;
using namespace loggers;
using namespace nodes;
using namespace structures;
using namespace textual;

//#define DEBUG_DIRECTORY_TREE
  // uncomment for noisier version.

#undef LOG
#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger::get(), to_print)

//////////////

namespace filesystem {

class dir_tree_iterator : public filename_tree::iterator
{
public:
  filename_tree *_current;

  dir_tree_iterator(const filename_tree *initial,
      tree::traversal_directions dir)
  : filename_tree::iterator(initial, dir), _current(NIL) {}
};

//////////////

directory_tree::directory_tree()
: _scanned_okay(false),
  _path(new astring),
  _pattern(new astring),
  _real_tree(new filename_tree),
  _ignore_files(false),
  _creator(new fname_tree_creator)
{
}

directory_tree::directory_tree(const astring &path, const char *pattern,
    bool ignore_files)
: _scanned_okay(false),
  _path(new astring(path)),
  _pattern(new astring(pattern)),
  _real_tree(NIL),
  _ignore_files(ignore_files),
  _creator(new fname_tree_creator)
{
  reset(path, pattern);
}

directory_tree::~directory_tree()
{
  _scanned_okay = false;
  WHACK(_path);
  WHACK(_pattern);
  WHACK(_real_tree);
  WHACK(_creator);
}

const astring &directory_tree::path() const { return *_path; }

int directory_tree::packed_size() const
{
  return 2 * PACKED_SIZE_INT32
      + _path->packed_size()
      + _pattern->packed_size()
      + _real_tree->recursive_packed_size();
}

void directory_tree::pack(byte_array &packed_form) const
{
  attach(packed_form, int(_scanned_okay));
  attach(packed_form, int(_ignore_files));
  _path->pack(packed_form);
  _pattern->pack(packed_form);
  _real_tree->recursive_pack(packed_form);
}

bool directory_tree::unpack(byte_array &packed_form)
{
  int temp;
  if (!detach(packed_form, temp)) return false;
  _scanned_okay = temp;
  if (!detach(packed_form, temp)) return false;
  _ignore_files = temp;
  if (!_path->unpack(packed_form)) return false;
  if (!_pattern->unpack(packed_form)) return false;
  WHACK(_real_tree);
  _real_tree = (filename_tree *)packable_tree::recursive_unpack
      (packed_form, *_creator);
  if (!_real_tree) {
    _real_tree = new filename_tree;  // reset it.
    return false;
  }
  return true;
}

void directory_tree::text_form(astring &target, bool show_files)
{
  dir_tree_iterator *ted = start(directory_tree::prefix);
    // create our iterator to do a prefix traversal.

  int depth;  // current depth in tree.
  filename curr;  // the current path the iterator is at.
  string_array files;  // the filenames held at the iterator.

  while (current(*ted, curr, files)) {
    // we have a good directory to show.
    directory_tree::depth(*ted, depth);
    target += string_manipulation::indentation(depth * 2) + astring("[")
        + curr.raw() + "]" + parser_bits::platform_eol_to_chars();
    if (show_files) {
      astring names;
      for (int i = 0; i < files.length(); i++) names += files[i] + " ";
      if (names.length()) {
        astring split;
        string_manipulation::split_lines(names, split, depth * 2 + 2);
        target += split + parser_bits::platform_eol_to_chars();
      }
    }

    // go to the next place.
    next(*ted);
  }

  throw_out(ted);
}

void directory_tree::traverse(const astring &path, const char *pattern,
    filename_tree &add_to)
{
  FUNCDEF("traverse");
  // prepare the current node.
  add_to._dirname = filename(path, astring::empty_string());
  add_to._files.reset();
#ifdef DEBUG_DIRECTORY_TREE
  LOG(astring("working on node ") + add_to._dirname);
#endif

  // open the directory.
  directory curr(path, "*");
  if (!curr.good()) return;

  if (!_ignore_files) {
    // add all the files to the current node.
    directory curr_stringent(path, pattern);
    add_to._files = curr_stringent.files();
  }

  // now iterate across the directories here and add a sub-node for each one,
  // and recursively traverse that sub-node also.
  const string_array &dirs = curr.directories();
  for (int i = 0; i < dirs.length(); i++) {
    filename_tree *new_branch = NIL;
    astring new_path = path + filename::default_separator() + dirs[i];
#ifdef DEBUG_DIRECTORY_TREE
    LOG(astring("seeking path: ") + new_path);
#endif
    if (!filename(new_path).is_normal()) {
//#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("bailing on weird dir: ") + new_path);
//#endif
      continue;  // only regular directories please.
    }
    for (int q = 0; q < add_to.branches(); q++) {
      filename_tree *curr_kid = (filename_tree *)add_to.branch(q);
#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("curr kid: ") + curr_kid->_dirname);
#endif
      if (filename(new_path).raw().iequals(filename
          (curr_kid->_dirname).raw())) {
        new_branch = curr_kid;
#ifdef DEBUG_DIRECTORY_TREE
        LOG(astring("using existing branch for ") + new_path);
#endif
        break;
      }
    }
    if (!new_branch) {
#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("adding new branch for ") + new_path);
#endif
      new_branch = new filename_tree;
      add_to.attach(new_branch);
      new_branch->_depth = add_to._depth + 1;
    }
#ifdef DEBUG_DIRECTORY_TREE
    LOG(astring("traversing sub-node ") + new_path);
#endif
    traverse(new_path, pattern, *new_branch);
  }
}

bool directory_tree::reset(const astring &path, const char *pattern)
{
  FUNCDEF("reset");
  _scanned_okay = false;
  WHACK(_real_tree);
//  filename temp_path(path);
//  temp_path.canonicalize();
  *_path = path;
//temp_path.raw();
  *_pattern = pattern;
  _real_tree = new filename_tree;

#ifdef DEBUG_DIRECTORY_TREE
  LOG(astring("dirtree::reset to path: ") + path);
#endif

  // check that the top-level is healthy.
  directory curr(path, "*");
  if (!curr.good()) return false;
    // our only exit condition; other directories might not be accessible
    // underneath, but the top one must be accessible for us to even start
    // the scanning.

  traverse(path, pattern, *_real_tree);
  _scanned_okay = true;;
  return true;
}

dir_tree_iterator *directory_tree::start_at(filename_tree *start,
    traversal_types type) const
{
  // translate to the lower level traversal enum.
  tree::traversal_directions dir = tree::prefix;
  if (type == infix) dir = tree::infix;
  else if (type == postfix) dir = tree::postfix;

  return new dir_tree_iterator(start, dir);
}

dir_tree_iterator *directory_tree::start(traversal_types type) const
{
  // translate to the lower level traversal enum.
  tree::traversal_directions dir = tree::prefix;
  if (type == infix) dir = tree::infix;
  else if (type == postfix) dir = tree::postfix;

  return new dir_tree_iterator(_real_tree, dir);
}

bool directory_tree::jump_to(dir_tree_iterator &scanning,
    const astring &sub_path)
{
  FUNCDEF("jump_to");
  string_array pieces;
  bool rooted;
  filename(sub_path).separate(rooted, pieces);
  for (int i = 0; i < pieces.length(); i++) {
    filename_tree *curr = dynamic_cast<filename_tree *>(scanning.current());
#ifdef DEBUG_DIRECTORY_TREE
    LOG(astring("at ") + curr->_dirname.raw());
#endif
    string_array sub_pieces = pieces.subarray(i, i);
    filename curr_path;
    curr_path.join(rooted, sub_pieces);
    curr_path = filename(curr->_dirname.raw() + filename::default_separator()
        + curr_path.raw());
#ifdef DEBUG_DIRECTORY_TREE
    LOG(astring("made curr path ") + curr_path.raw());
#endif
    if (!curr) return false;
    bool found_it = false;
    for (int j = 0; j < curr->branches(); j++) {
      filename_tree *sub = dynamic_cast<filename_tree *>(curr->branch(j));
#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("looking at ") + sub->_dirname.raw());
#endif
      if (sub->_dirname.compare_prefix(curr_path)) {
        // this part matches!
        scanning.push(sub);
#ifdef DEBUG_DIRECTORY_TREE
        LOG(astring("found at ") + sub->_dirname.raw());
#endif
        found_it = true;
        break;
      }
    }
    if (!found_it) {
#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("could not find ") + curr_path.raw());
#endif
      return false;
    }
  }
  return true;
}

filename_tree *directory_tree::goto_current(dir_tree_iterator &scanning)
{
  if (!scanning._current) {
    // this one hasn't been advanced yet, or it's already over with.
    scanning._current = (filename_tree *)scanning.next();
  }
  // now check that we're healthy.
  if (!scanning._current) return NIL;  // all done.

  // cast the tree to the right type.
  return dynamic_cast<filename_tree *>(scanning._current);
}

bool directory_tree::current_dir(dir_tree_iterator &scanning,
    filename &dir_name)
{
  dir_name = astring::empty_string();
  filename_tree *tof = goto_current(scanning);
  if (!tof) return false;
  dir_name = tof->_dirname;
  return true;
}

bool directory_tree::current(dir_tree_iterator &scanning,
    filename &dir_name, string_array &to_fill)
{
  // clear any existing junk.
  dir_name = astring::empty_string();
  to_fill.reset();

  filename_tree *tof = goto_current(scanning);
  if (!tof) return false;

  // fill in what they wanted.
  dir_name = tof->_dirname;
  tof->_files.fill(to_fill);

  return true;
}

bool directory_tree::current(dir_tree_iterator &scanning,
    filename &dir_name, filename_list &to_fill)
{
  // clear any existing junk.
  dir_name = astring::empty_string();
  to_fill.reset();

  filename_tree *tof = goto_current(scanning);
  if (!tof) return false;

  // fill in what they wanted.
  dir_name = tof->_dirname;
  to_fill = tof->_files;

  return true;
}

filename_list *directory_tree::access(dir_tree_iterator &scanning)
{
  filename_tree *tof = goto_current(scanning);
  if (!tof) return NIL;
  return &tof->_files;
}

bool directory_tree::depth(dir_tree_iterator &scanning, int &depth)
{
  depth = -1;  // invalid as default.
  filename_tree *tof = goto_current(scanning);
  if (!tof) return false;
  depth = tof->_depth;
  return true;
}

bool directory_tree::children(dir_tree_iterator &scanning, int &kids)
{
  kids = -1;  // invalid as default.
  filename_tree *tof = goto_current(scanning);
  if (!tof) return false;
  kids = tof->branches();
  return true;
}

bool directory_tree::next(dir_tree_iterator &scanning)
{
  scanning._current = (filename_tree *)scanning.next();
  return !!scanning._current;
}

void directory_tree::throw_out(dir_tree_iterator * &to_whack)
{
  WHACK(to_whack);
}

filename_tree *directory_tree::seek(const astring &dir_name_in,
    bool ignore_initial) const
{
  FUNCDEF("seek");
  array<filename_tree *> examining;
    // the list of nodes we're currently looking at.

#ifdef DEBUG_DIRECTORY_TREE
  LOG(astring("seeking on root of: ") + *_path);
#endif

  astring dir_name = filename(dir_name_in).raw();
  // set the search path up to have the proper prefix.
  if (ignore_initial)
    dir_name = path() + filename::default_separator()
       + filename(dir_name_in).raw();

#ifdef DEBUG_DIRECTORY_TREE
  LOG(astring("adding root: ") + _real_tree->_dirname);
#endif
  examining += _real_tree;

  astring sequel;  // holds extra pieces from filename comparisons.

  // chew on the list of nodes to examine until we run out.
  while (examining.length()) {
    int posn;
    bool found = false;
    // start looking at all the items in the list, even though we might have
    // to abandon the iteration if we find a match.
    filename_tree *check = NIL;
    for (posn = 0; posn < examining.length(); posn++) {
      check = examining[posn];
      filename current(check->_dirname);
      if (!current.is_normal()) {
//#ifdef DEBUG_DIRECTORY_TREE
        LOG(astring("seek: skipping abnormal dir: \"") + current + "\"");
//#endif
        continue;
      }
#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("looking at ") + current.raw());
#endif
      if (current.compare_prefix(dir_name, sequel)) {
        // we have a match!
#ifdef DEBUG_DIRECTORY_TREE
        LOG(astring("matched! at ") + current.raw());
#endif
        found = true;
        if (!sequel) {
          // whoa!  an exact match.  we're done now.
#ifdef DEBUG_DIRECTORY_TREE
          LOG(astring("exact match at ") + current.raw() + "!  done!!!");
#endif
          return check;
        } else {
#ifdef DEBUG_DIRECTORY_TREE
          LOG(astring("inexact match because sequel=") + sequel);
#endif
        }
        break;
      }
    }
    if (!found) return NIL;  // we found nothing comparable.

    // we found a partial match.  that means we should start looking at this
    // node's children for the exact match.
    if (!check) {
      // this is a serious logical error!
      LOG("serious logical error: tree was not located.");
      return NIL;
    }
    examining.reset();  // clear the existing nodes.
    for (int i = 0; i < check->branches(); i++)
      examining += (filename_tree *)check->branch(i);
  }

  return NIL;  // we found nothing that looked like that node.
}

bool directory_tree::calculate(bool just_size)
{ return calculate(_real_tree, just_size); }

bool directory_tree::calculate(filename_tree *start, bool just_size)
{
  FUNCDEF("calculate");
//#ifdef DEBUG_DIRECTORY_TREE
  LOG(astring("calculate: got tree to start with at ") + start->_dirname.raw());
//#endif
  dir_tree_iterator *ted = start_at(start, directory_tree::postfix);
    // create our iterator to do a postfix traversal.  why postfix?  well,
    // prefix has been used elsewhere and since it doesn't really matter what
    // order we visit the nodes here, it's good to change up.

  int depth;  // current depth in tree.
  filename curr;  // the current path the iterator is at.
  filename_list *files;  // the filenames held at the iterator.

  while (directory_tree::current_dir(*ted, curr)) {
    if (!curr.is_normal()) {
//#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("calculate: skipping abnormal dir: \"") + curr + "\"");
//#endif
      directory_tree::next(*ted);
      continue;  // scary non-simple file type.
    }
    // we have a good directory to show.
#ifdef DEBUG_DIRECTORY_TREE
    LOG(astring("calcing node ") + curr.raw());
#endif
    files = directory_tree::access(*ted);
    directory_tree::depth(*ted, depth);
    for (int i = 0; i < files->elements(); i++) {
      filename curr_file = curr + "/" + *files->borrow(i);
#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("got a curr file: ") + curr_file);
#endif
      if (!curr_file.is_normal()) {
//#ifdef DEBUG_DIRECTORY_TREE
        LOG(astring("skipping abnormal file: \"") + curr + "\"");
//#endif
        continue;
      }
      if (!files->borrow(i)->calculate(curr.raw(), just_size)) {
        LOG(astring("failure to calculate ") + files->get(i)->text_form());
      }
    }

    directory_tree::next(*ted);
  }

  directory_tree::throw_out(ted);
  return true;
}

bool directory_tree::compare_trees(const directory_tree &source,
    const directory_tree &target, filename_list &differences,
    file_info::file_similarity how_to_compare)
{
  return compare_trees(source, astring::empty_string(), target,
      astring::empty_string(), differences, how_to_compare);
}

bool directory_tree::compare_trees(const directory_tree &source,
    const astring &source_start_in, const directory_tree &target,
    const astring &target_start_in, filename_list &differences,
    file_info::file_similarity how_compare)
{
  FUNCDEF("compare_trees");
  differences.reset();  // prepare it for storage.

  // make sure we get canonical names to work with.
  filename source_start(source_start_in);
  filename target_start(target_start_in);

  dir_tree_iterator *ted = source.start(directory_tree::prefix);
    // create our iterator to do a prefix traversal.

  astring real_source_start = source.path();
  if (source_start.raw().t()) {
    // move to the right place.
    real_source_start = real_source_start + filename::default_separator()
        + source_start.raw();
    if (!directory_tree::jump_to(*ted, source_start.raw())) {
      // can't even start comparing.
      LOG(astring("failed to find source start in tree, given as ")
          + source_start.raw());
      return false;
    }
  }

  filename curr;  // the current path the iterator is at.
  filename_list files;  // the filenames held at the iterator.

  // calculate where our comparison point is on the source.
  int source_pieces = 0;
  {
    string_array temp;
    bool rooted_source;
    filename(real_source_start).separate(rooted_source, temp);
    source_pieces = temp.length();
  }

  bool seen_zero_pieces = false;
  while (directory_tree::current(*ted, curr, files)) {
    // we're in a place in the source tree now.  let's compare it with the
    // target's recollection.

#ifdef DEBUG_DIRECTORY_TREE
    LOG(astring("curr dir in tree: ") + curr.raw());
#endif

    string_array pieces;
    bool curr_rooted;
    curr.separate(curr_rooted, pieces);  // get the components of the current location.
#ifdef DEBUG_DIRECTORY_TREE
    LOG(astring("name in pieces:") + pieces.text_form());
#endif
    pieces.zap(0, source_pieces - 1);
      // snap the root components out of there.

    filename corresponding_name;
//hmmm: is that right decision?
    corresponding_name.join(false, pieces);
#ifdef DEBUG_DIRECTORY_TREE
    LOG(astring("computed target name as: ") + corresponding_name);
#endif
    filename original_correspondence(corresponding_name);

    if (!corresponding_name.raw().t()) {
      if (seen_zero_pieces) {
#ifdef DEBUG_DIRECTORY_TREE
        LOG(astring("breaking out now due to empty correspondence"));
#endif
        break;
      }
      seen_zero_pieces = true;
    }
    if (target_start.raw().t()) {
      corresponding_name = filename(target_start.raw()
          + filename::default_separator() + corresponding_name.raw());
/*doesn't help, not right yet.    } else {
      // if they didn't give us a place to start, we start at the top.
      corresponding_name = filename(target.path()
          + filename::default_separator() + corresponding_name.raw());
*/
    }
#ifdef DEBUG_DIRECTORY_TREE
    LOG(astring("target with start is: ") + corresponding_name);
#endif

    filename_tree *target_now = target.seek(corresponding_name.raw(), true);
    if (!target_now) {
      // that entire sub-tree is missing.  add all of the files here into
      // the list.
//#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("could not find dir in target for ") + curr.raw()
          + " which we computed corresp as " + corresponding_name.raw());
//#endif
    }

    // now scan across all the files that are in our source list.
    for (int i = 0; i < files.elements(); i++) {
      if (!target_now  // there was no node, so we're adding everything...
          || !target_now->_files.member_with_state(*files[i], how_compare) ) {
        // ... or we need to add this file since it's missing.
#ifdef DEBUG_DIRECTORY_TREE
        LOG(astring("adding record: ") + files[i]->text_form());
#endif

        file_info *new_record = new file_info(*files[i]);
        // record the file time for use later in saving.
        new_record->calculate(curr, true);
        astring original = new_record->raw();
#ifdef DEBUG_DIRECTORY_TREE
        LOG(astring("current: ") + new_record->raw());
#endif

        astring actual_name = source_start.raw();
#ifdef DEBUG_DIRECTORY_TREE
        if (actual_name.t()) LOG(astring("sname=") + actual_name);
#endif
        if (actual_name.length()) actual_name += filename::default_separator();
        actual_name += original_correspondence.raw();
        if (actual_name.length()) actual_name += filename::default_separator();
        actual_name += new_record->raw();
#ifdef DEBUG_DIRECTORY_TREE
        if (actual_name.t()) LOG(astring("sname=") + actual_name);
#endif
        (filename &)(*new_record) = filename(actual_name);

        astring targ_name = corresponding_name.raw();
#ifdef DEBUG_DIRECTORY_TREE
        if (targ_name.t()) LOG(astring("tname=") + targ_name);
#endif
        if (targ_name.length()) targ_name += filename::default_separator();
        targ_name += original;
#ifdef DEBUG_DIRECTORY_TREE
        if (targ_name.t()) LOG(astring("tname=") + targ_name);
#endif

        new_record->secondary(targ_name);

        differences += new_record;
#ifdef DEBUG_DIRECTORY_TREE
        LOG(astring("came out as: ") + new_record->text_form());
#endif
      }
    }
    
    // go to the next place.
    directory_tree::next(*ted);
  }

  directory_tree::throw_out(ted);

  return true;
}

outcome directory_tree::find_common_root(const astring &finding, bool exists,
    filename_tree * &found, astring &reassembled, string_array &pieces,
    int &match_place)
{
  FUNCDEF("find_common_root");
  // test the path to find what it is.
  filename adding(finding);
  if (exists && !adding.good())
    return common::BAD_INPUT;  // not a good path.
  int file_subtract = 0;  // if it's a file, then we remove last component.
  if (exists && !adding.is_directory()) file_subtract = 1;

  // break up the path into pieces.
  pieces.reset();
  bool rooted;
  adding.separate(rooted, pieces);

  // break up our root into pieces; we must take off components that are
  // already in the root.
  string_array root_pieces;
  bool root_rooted;
  filename temp_file(path());
  temp_file.separate(root_rooted, root_pieces);

  // locate the last place where the path we were given touches our tree.
  // it could be totally new, partially new, or already contained.
  filename_tree *last_match = _real_tree;  // where the common root is located.
  int list_length = pieces.length() - file_subtract;
  reassembled = "";

  // we must put all the pieces in that already come from the root.
  for (int i = 0; i < root_pieces.length() - 1; i++) {
    bool add_slash = false;
    if (reassembled.length() && (reassembled[reassembled.end()] != '/') )
      add_slash = true;
    if (add_slash) reassembled += "/";
    reassembled += pieces[i];
    if (reassembled[reassembled.end()] == ':') {
#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("skipping drive component ") + reassembled);
#endif
      continue;
    }
  }

#ifdef DEBUG_DIRECTORY_TREE
  LOG(astring("after pre-assembly, path is ") + reassembled);
#endif

  outcome to_return = common::NOT_FOUND;

  for (match_place = root_pieces.length() - 1; match_place < list_length;
      match_place++) {
    // add a slash if there's not one present already.
    bool add_slash = false;
    if (reassembled.length() && (reassembled[reassembled.end()] != '/') )
      add_slash = true;
    // add the next component in to our path.
    if (add_slash) reassembled += "/";
    reassembled += pieces[match_place];
    // special case for dos paths.
    if (reassembled[reassembled.end()] == ':') {
#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("skipping drive component ") + reassembled);
#endif
      continue;
    }
    reassembled = filename(reassembled).raw();  // force compliance with OS.
#ifdef DEBUG_DIRECTORY_TREE
    LOG(astring("now seeking ") + reassembled);
#endif
    filename_tree *sought = seek(reassembled, false);
    if (!sought) {
#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("couldn't find ") + reassembled);
#endif
      if (!exists && (match_place == list_length - 1)) {
        // see if we can get a match on a file rather than a directory, but
        // only if we're near the end of the compare.
        if (last_match->_files.member(pieces[match_place])) {
          // aha!  a file match.
          to_return = common::OKAY;
          match_place--;
          break;
        }
      }
      match_place--;
      break;
    } else {
      // record where we last had some success.
#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("found subtree for ") + reassembled);
#endif
      last_match = sought;
    }
  }
  // this is a success, but our loop structure can put us one past the right
  // place.
  if (match_place >= list_length) {
    match_place = list_length - 1;
    to_return = common::OKAY;
  }

  found = last_match;
  return to_return;
}

outcome directory_tree::add_path(const astring &new_item, bool just_size)
{
  FUNCDEF("add_path");
  // test the path to find out what it is.
  filename adding(new_item);
  if (!adding.good()) {
    LOG(astring("non-existent new item!  ") + new_item);
    return common::NOT_FOUND;  // not an existing path.
  }
  if (!adding.is_normal()) {
//#ifdef DEBUG_DIRECTORY_TREE
    LOG(astring("abnormal new item:  ") + new_item);
//#endif
    return common::BAD_INPUT;  // not a good path.
  }
  int file_subtract = 0;  // if it's a file, then we remove last component.
  if (!adding.is_directory()) file_subtract = 1;
#ifdef DEBUG_DIRECTORY_TREE
  if (file_subtract) { LOG(astring("adding a file ") + new_item); }
  else { LOG(astring("adding a directory ") + new_item); }
#endif

  // find the common root, break up the path into pieces, and tell us where
  // we matched.
  string_array pieces;
  filename_tree *last_match = NIL;
  int comp_index;
  astring reassembled;  // this will hold the common root.
  outcome ret = find_common_root(new_item, true, last_match, reassembled,
      pieces, comp_index);
  if (!last_match) {
    LOG(astring("serious error finding common root for ") + new_item
        + ", got NIL tree.");
    return common::FAILURE;  // something serious isn't right.
  }

  if (!file_subtract) {
    if (ret != common::OKAY) {
      // if it's a new directory, we add a new node for traverse to work on.
#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("now adding node for ") + reassembled);
#endif
      filename_tree *new_branch = new filename_tree;
      new_branch->_depth = last_match->_depth + 1;
      last_match->attach(new_branch);
      last_match = new_branch;
    } else {
#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("matched properly.  reassembled set to ") + reassembled);
#endif
    }
  }

  if (file_subtract) {
    if (ret != common::OKAY) {
#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("common gave us posn of: ") + reassembled);
#endif
      // handle the case for files now that we have our proper node.
      string_array partial_pieces;
      bool partial_rooted;
      filename(reassembled).separate(partial_rooted, partial_pieces);
      int levels_missing = pieces.length() - partial_pieces.length();

      // we loop over all the pieces that were missing in between the last
      // common root and the file's final location.
      for (int i = 0; i < levels_missing; i++) {
#ifdef DEBUG_DIRECTORY_TREE
        LOG(astring("adding intermediate directory: ") + reassembled);
#endif
        filename_tree *new_branch = new filename_tree;
        new_branch->_depth = last_match->_depth + 1;
        new_branch->_dirname = filename(reassembled).raw();
        last_match->attach(new_branch);
        last_match = new_branch;
        reassembled += astring("/") + pieces[partial_pieces.length() + i];
        reassembled = filename(reassembled).raw();  // canonicalize.
      }
    }

    if (!last_match->_files.find(pieces[pieces.last()])) {
#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("adding new file ") + pieces[pieces.last()]
        + " at " + reassembled);
#endif
      file_info *to_add = new file_info(pieces[pieces.last()], 0);
      to_add->calculate(reassembled, just_size);
      last_match->_files += to_add;
    } else {
#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("not adding existing file ") + pieces[pieces.last()]
          + " at " + reassembled);
#endif
    }
  } else {
    // handle the case for directories.
#ifdef DEBUG_DIRECTORY_TREE
    LOG(astring("doing traverse in ") + last_match->_dirname
        + " to add " + reassembled);
#endif
    traverse(reassembled, "*", *last_match);
//hmmm: maybe provide pattern capability instead of assuming all files.
    calculate(last_match, just_size);
  }

  return common::OKAY;
}

outcome directory_tree::remove_path(const astring &zap_item)
{
  FUNCDEF("remove_path");
  // find the common root, if one exists.  if not, we're not going to do this.
  string_array pieces;
  filename_tree *last_match = NIL;
  int comp_index;
  astring reassembled;
  outcome ret = find_common_root(zap_item, false, last_match, reassembled,
      pieces, comp_index);
  if (!last_match) return common::NOT_FOUND;
  // if we didn't actually finish iterating to the file, then we're not
  // whacking anything.
  if (ret != common::OKAY) {
#ifdef DEBUG_DIRECTORY_TREE
    LOG(astring("got error seeking ") + zap_item + " of "
        + common::outcome_name(ret));
#endif
    return ret;
  }

  if (comp_index == pieces.last()) {
    // if the names match fully, then we're talking about a directory.
#ifdef DEBUG_DIRECTORY_TREE
    LOG(astring("found directory match for ") + zap_item);
#endif
  } else {
#ifdef DEBUG_DIRECTORY_TREE
    LOG(astring("may have found file match for ") + zap_item);
#endif
    filename to_seek(pieces[pieces.last()]);
    if (!last_match->_files.member(to_seek)) {
      // this file is not a member, so we must say it's not found.
#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("couldn't find file match in common root for ") + zap_item);
#endif
      return common::NOT_FOUND;
    } else {
      int indy = last_match->_files.locate(to_seek);
#ifdef DEBUG_DIRECTORY_TREE
      LOG(astring("found match to remove for ") + zap_item);
#endif
      last_match->_files.zap(indy, indy);
      return common::OKAY;  // done!
    }
  }

#ifdef DEBUG_DIRECTORY_TREE
  LOG(astring("going to whack node at: ") + last_match->_dirname.raw());
#endif

  // we're whacking directories, so we need to take out last_match and below.
  filename_tree *parent = (filename_tree *)last_match->parent();
  if (!parent || (last_match == _real_tree)) {
    // this seems to be matching the whole tree.  we disallow that.
#ifdef DEBUG_DIRECTORY_TREE
    LOG("there's a problem whacking this node; it's the root.");
#endif
    return common::BAD_INPUT;
  }
#ifdef DEBUG_DIRECTORY_TREE
  LOG(astring("pruning tree at ") + last_match->_dirname.raw());
#endif
  parent->prune(last_match);
  WHACK(last_match);

  return common::OKAY;
}

} //namespace.


