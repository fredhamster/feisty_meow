#ifndef DIRECTORY_TREE_CLASS
#define DIRECTORY_TREE_CLASS

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
#include "file_info.h"

#include <basis/byte_array.h>
#include <basis/contracts.h>
#include <basis/outcome.h>
#include <structures/string_array.h>

namespace filesystem {

// forward declarations.
class dir_tree_iterator;
class filename;
class filename_list;
class filename_tree;
class fname_tree_creator;

//! An object that traverses directory trees and provides a view of all files.

class directory_tree : public virtual basis::packable
{
public:
  directory_tree();  //!< constructs an empty tree.

  directory_tree(const basis::astring &path, const char *pattern = "*",
          bool ignore_files = false);
    //!< opens up the "path" specified and scans for files and subdirectories.
    /*!< if the location was accessible, then the good() method returns true.
    note that the "path" should just be a bare directory without any
    wildcards attached.  the "pattern" can be specified if you wish to
    strain out just a subset of the files in the directory.  note that
    unlike the directory object, directory_tree applies the wildcard to
    filenames only--all sub-directories are included.  the pattern must meet
    the same requirements that the operating system places on wildcard
    patterns.  if "ignore_files" is true, then no files are considered and
    only the tree of directories is gathered. */

  ~directory_tree();

  DEFINE_CLASS_NAME("directory_tree");

  bool good() const { return _scanned_okay; }
    //!< returns true if the directory existed and we read its contents.

  const basis::astring &path() const;
    //!< returns the root of the directory tree that we manage.

  bool reset(const basis::astring &path, const char *pattern = "*");
    //!< gets rid of any current files and rescans the directory at "path".
    /*!< a new "pattern" can be specified at this time also.  true is returned
    if the process was started successfully at "path"; there might be
    problems with subdirectories, but at least the "path" got validated. */

  filename_tree *seek(const basis::astring &dir_name, bool ignore_initial) const;
    //!< finds the "dir_name" in our tree.
    /*!< locates the node that corresponds to the directory name contained in
    "dir_name" and returns the filename_tree rooted at that node.  if the
    "ignore_initial" flag is true, then dir_name is expected to omit the
    path() where "this" tree is rooted. */

  virtual int packed_size() const;
    //!< reports the size after packing up the tree.
  virtual void pack(basis::byte_array &packed_form) const;
    //!< packs the directory_tree into a byte_array.
  virtual bool unpack(basis::byte_array &packed_form);
    //!< unpacks the directory_tree from a byte_array.

  bool calculate(bool just_size);
    //!< visits each file in the directory_tree and calculates its attributes.
    /*!< the attributes include file size and checksum.  if "just_size" is
    true, then no checksum is computed. */

  bool calculate(filename_tree *start, bool just_size);
    //!< a calculate method that starts at a specific node rather than the root.

  basis::outcome add_path(const basis::astring &new_item, bool just_size = false);
    //!< adds a "new_item" into the tree.
    /*!< this is useful when one knows that new files exist under the
    directory, but one doesn't want to recalculate the entire tree.  the new
    item will automatically be calculated.  the item can be either a file or
    directory that's under the root.  the root directory name should not be
    included in the "new_item". */

  basis::outcome remove_path(const basis::astring &zap_item);
    //!< removes the "zap_item" from the tree.
    /*!< this only works for cases where one knows that an item has been
    removed in the filesystem.  if the item is still really there, then the
    next rescan will put it back into the tree. */

  basis::outcome make_directories(const basis::astring new_root);
    //!< creates all of the directories in this object, but start at the "new_root".

  static bool compare_trees(const directory_tree &source,
          const directory_tree &target, filename_list &differences,
          file_info::file_similarity how_to_compare);
    //!< compares the tree in "source" with the tree in "target".
    /*!< the two root names may be different, but everything below the root
    in "source" will be checked against "target".  the "differences" between
    the two trees will be compiled.  note that this does not perform any disk
    access; it merely compares the two trees' current contents.
    the "differences" list's members will have a primary filename set to
    the source path and an alternate filename set to the location in the
    target.  the "how_to_compare" value will dictate what aspects of file
    equality are used. */

  static bool compare_trees(const directory_tree &source,
          const basis::astring &source_start, const directory_tree &target,
          const basis::astring &target_start, filename_list &differences,
          file_info::file_similarity how_to_compare);
    // compares the trees but not at their roots.  the location on the source
    // side is specified by "source_start", which must be a path found under
    // the "source" tree.  similarly, the "target_start" will be the location
    // compared with the "source" + "source_start".  the "diffs" will still
    // be valid with respect to "source" rather than "source_start".

  void text_form(basis::astring &tree_dump, bool show_files = true);
    //!< provides a visual representation of the tree in "tree_dump".
    /*!< if "show_files" is not true, then only the directories will be
    shown. */

  // Note on the iterator functions: the iterator becomes invalid if the
  // directory tree is reset.  the only valid operation on the iterator
  // at that point is to call throw_out().

  enum traversal_types {
    prefix,  //!< prefix means that subnodes are processed after their parent.
    infix,  //!< infix (for binary trees) goes 1) left, 2) current, 3) right.
    postfix  //!< postfix means that subnodes are traversed first (depth first).
  };

  dir_tree_iterator *start(traversal_types type) const;
    //!< starts an iterator on the directory tree.  

  dir_tree_iterator *start_at(filename_tree *start,
          traversal_types type) const;
    //!< starts the iterator at a specific "start" node.

  static bool jump_to(dir_tree_iterator &scanning, const basis::astring &sub_path);
    //!< seeks to a "sub_path" below the iterator's current position.
    /*!< tries to take the iterator "scanning" down to a "sub_path" that is
    underneath its current position.  true is returned on success. */

  static bool current_dir(dir_tree_iterator &scanning, filename &dir_name);
    //!< sets "dir_name" to the directory name at the "scanning" location.

  static bool current(dir_tree_iterator &scanning, filename &dir_name,
          structures::string_array &to_fill);
    //!< retrieves the information for the iterator's current location.
    /*!< fills the "to_fill" array with filenames that are found at the
    "scanning" iterator's current position in the tree.  the "dir_name"
    for that location is also set.  if the iterator has ended, then false
    is returned. */

  static bool current(dir_tree_iterator &scanning, filename &dir_name,
          filename_list &to_fill);
    //!< similar to the above but provides a list of the real underlying type.

  static filename_list *access(dir_tree_iterator &scanning);
    //!< more dangerous operation that lets the actual list be manipulated.
    /*!< NULL_POINTER is returned if there was a problem accessing the tree
    at the iterator position. */

  static bool depth(dir_tree_iterator &scanning, int &depth);
    //!< returns the current depth of the iterator.
    /*!< a depth of zero means the iterator is at the root node for the tree. */

  static bool children(dir_tree_iterator &scanning, int &children);
    //!< returns the number of children for the current node.

  static bool next(dir_tree_iterator &scanning);
    //!< goes to the next filename in the "scanning" iterator.
    /*!< true is returned if there is an entry there. */

  static void throw_out(dir_tree_iterator * &to_whack);
    //!< cleans up an iterator that was previously opened with start().

private:
  bool _scanned_okay;  //!< did this directory work out?
  basis::astring *_path;  //!< the directory we're looking at.
  basis::astring *_pattern;  //!< the pattern used to find the files.
  filename_tree *_real_tree;  //!< the tree of directory contents we build.
  bool _ignore_files;  //!< true if they don't care about the files.
  fname_tree_creator *_creator;  //!< creates blank trees during unpacking.

  static filename_tree *goto_current(dir_tree_iterator &scanning);
    //!< goes to the current node for "scanning" and returns the tree there.
    /*!< if there are no nodes left, NULL_POINTER is returned. */

  void traverse(const basis::astring &path, const char *pattern,
          filename_tree &add_to);
    //!< recursively adds a "path" given the filename "pattern".
    /*!< assuming that we want to add the files at "path" using the "pattern"
    into the current node "add_to", we will also scoot down all sub-dirs
    and recursively invoke traverse() to add those also. */

  basis::outcome find_common_root(const basis::astring &path, bool exists,
          filename_tree * &common_root, basis::astring &common_path,
          structures::string_array &pieces, int &match_place);
    //!< locates the node where this tree and "path" have membership in common.
    /*!< if "exists" is true, then the "path" is tested for existence and
    otherwise it's assumed that the path no longer exists.  the "common_root"
    is the last node that's in both places, the "common_path" is the name of
    that location, the list of "pieces" is "path" broken into its components,
    and the "match_place" is the index in "pieces" of the common node. */
};

} //namespace.

#endif

