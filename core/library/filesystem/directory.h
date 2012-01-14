#ifndef DIRECTORY_CLASS
#define DIRECTORY_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : directory                                                         *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2001-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <basis/contracts.h>
#include <structures/string_array.h>

namespace filesystem {

//! Implements a scanner that finds all filenames in the directory specified.

class directory : public virtual basis::root_object
{
public:
  directory(const basis::astring &path, const char *pattern = "*");
    //!< opens up the "path" specified and scans for files and subdirectories.
    /*!< if the location was accessible, then the good() method returns true.
    note that the "path" should just be a bare directory without any
    wildcards attached.  the "pattern" can be specified if you wish to
    strain out just a subset of the files in the directory.  it must meet
    the same requirements that the operating system places on wildcard
    patterns. */

  directory(const directory &to_copy);

  virtual ~directory();

  directory &operator =(const directory &to_copy);

  DEFINE_CLASS_NAME("directory");

  bool good() const { return _scanned_okay; }
    //!< true if the directory existed and its contents were readable.

  const basis::astring &path() const;
    //!< returns the directory that we manage.

  const basis::astring &pattern() const;
    //!< returns the pattern that the directory class scans for.

  static basis::astring absolute_path(const basis::astring &relative_path);
    //!< returns the absolute path to a file with "relative_path".
    /*!< an empty string is returned on failure. */

  bool reset(const basis::astring &path, const char *pattern = "*");
    //!< gets rid of any current files and rescans the directory at "path".
    /*!< a new "pattern" can be specified at this time also. */

  bool move_up(const char *pattern = "*");
    //!< resets the directory to be its own parent.

  bool move_down(const basis::astring &subdir, const char *pattern = "*");
    //!< changes down into a "subdir" of this directory.
    /*!< the "subdir" should be just the file name component to change into.
    absolute paths will not work.  for example, if a directory "/l/jed" has
    a subdirectory named "clampett", then use: @code
      my_dir->move_down("clampett") @endcode
    */

  bool rescan();
    //!< reads our current directory's contents over again.

  const structures::string_array &files() const;
    //!< returns the list of files that we found in this directory.
    /*!< these are all assumed to be located in our given path.  to find out
    more information about the files themselves, construct a filename object
    with the path() and the file of interest. */
    
  const structures::string_array &directories() const;
    //!< these are the directory names from the folder.
    /*!< they can also be examined using the filename object.  note that
    this does not include the entry for the current directory (.) or the
    parent (..). */

  // static methods of general directory-related interest.

  static basis::astring current();
    //!< returns the current directory, as reported by the operating system.

  static bool make_directory(const basis::astring &path);
    //!< returns true if the directory "path" could be created.

  static bool remove_directory(const basis::astring &path);
    //!< returns true if the directory "path" could be removed.

  static bool recursive_create(const basis::astring &directory_name);
    //!< returns true if the "directory_name" can be created or already exists.
    /*!< false returns indicate that the operating system wouldn't let us
    make the directory, or that we didn't have sufficient permissions to
    access an existing directory to view it or move into it. */

private:
  bool _scanned_okay;  //!< did this directory work out?
  basis::astring *_path;  //!< the directory we're looking at.
  structures::string_array *_files;  //!< the list of files.
  structures::string_array *_folders;  //!< the list of directories.
  basis::astring *_pattern;  //!< the pattern used to find the files.
};

} //namespace.

#endif

