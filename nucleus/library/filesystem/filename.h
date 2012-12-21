#ifndef FILENAME_CLASS
#define FILENAME_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : filename                                                          *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1993-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/contracts.h>
#include <structures/string_array.h>

// forward declarations.
class status_info;

//hmmm: this doesn't really belong here, does it...
// define useful constant for filesystem path length.
#ifndef MAX_ABS_PATH 
  #ifdef __WIN32__
    #include <windows.h>
    #define MAX_ABS_PATH MAX_PATH
  #else
    #ifdef __APPLE__
      #include <sys/syslimits.h>
    #else
      #include <limits.h>
    #endif
    #define MAX_ABS_PATH PATH_MAX
  #endif
#endif


namespace filesystem {

//! Provides operations commonly needed on file names.

class filename : public basis::astring, public virtual basis::packable
{
public:
  filename();  //!< blank constructor.
  filename(const basis::astring &name);
    //!< creates a filename from any part of a full pathname, if possible.
    /*!< if the name contains quotes, they are stripped out. */
  filename(const basis::astring &directory, const basis::astring &name_of_file);
    //!< constructs a filename from a "directory" and the "name_of_file".
    /*!< the "name_of_file" can itself be a directory. */
  filename(const filename &to_copy);  //!< copy constructor.

  virtual ~filename();

  bool good() const;
    //!< returns true if the filename seems to be valid.
    /*!< this means that not only was the pathname parsed and found valid,
    but the file actually exists. */

  void reset(const basis::astring &name);
    //!< changes the file name held by the object.

  const basis::astring &raw() const;
    //!< returns the astring that we're holding onto for the path.
  basis::astring &raw();
    //!< accesses the astring that we're holding onto for the path.
    /*!< important note: if you change the string with this non-const raw()
    method, you MUST call canonicalize() on it again afterwards. */

  filename &operator = (const filename &to_copy);
    //!< provides assignment for this object, plus a simple string.
  filename &operator = (const basis::astring &to_copy);
    //!< provides assignment for this object, plus a simple string.
    /*!< the latter version invokes canonicalize to clean the string up. */

  void canonicalize();
    //!< cleans up the filename as needed for the current operating system.
    /*!< reforms the name by replacing any alternate directory separators with
    the operating system's preferred character. */

  bool exists() const;
    //!< returns true if the file exists.

  bool unlink() const;
    //!< actually removes the file, if possible.
    /*!< if the file was successfully deleted, then true is returned. */

  filename parent() const;
    //!< returns the parent filename for this one.

  basis::astring pop();
    //!< removes the deepest component of the pathname.
    /*!< the component might be a file or directory name, but popping beyond
    the top-level directory will not succeed.  the returned string contains
    the component that was removed.  it will be a blank string if nothing
    could be popped. */

  void push(const basis::astring &to_push);
    //!< pushes a new filename onto the current pathname.
    /*!< this only makes sense as a real pathname if this is currently a
    directory name and the component "to_push" is a child of that directory
    (or one intends to create that component as a child).  this is the
    opposite of pop. */

  filename basename() const;
    //!< returns the base of the filename; no directory.
  filename dirname() const;
    //!< returns the directory for the filename.
    /*!< if no directory name can be found in the filename, then "." is
    returned. */
  basis::astring dirname(bool add_slash) const;
    //!< returns the directory for the filename and optionally adds a slash.
    /*!< if "add_slash" is true, then the default directory separator will be
    present on the end of the string. */
  bool had_directory() const { return _had_directory; }
    //!< returns true if the name that we were given had a non-empty directory.
    /*!< this allows one to distinguish between a file with the current
    directory (.) attached and a file with no directory specified. */

  char drive(bool interact_with_fs = false) const;
    //!< returns the drive letter for the file, without the colon.
    /*!< this only makes sense for a fully qualified MS-DOS style name.  if no
    drive letter is found, then '\0' is returned.  if "interact_with_fs" is
    true, then the file system will be checked for the actual drive if no
    drive letter was found in the contents. */

  basis::astring extension() const;
    //!< returns the extension for the file, if one is present.

  basis::astring rootname() const;
    //!< returns the root part of the basename without an extension.

  // status functions return true if the characteristic embodied in
  // the name is also true.

  bool is_directory() const;
  bool is_writable() const;
  bool is_readable() const;
  bool is_executable() const;

  // is_normal makes sure that the file or directory is not a named pipe or other
  // special type of file.  symbolic links are considered normal.
  bool is_normal() const;

  enum write_modes {
    ALLOW_NEITHER = 0x0,
    ALLOW_READ = 0x1, ALLOW_WRITE = 0x2,
    ALLOW_BOTH = ALLOW_READ | ALLOW_WRITE
  };

  enum ownership_modes {
    NO_RIGHTS = 0x0,
    USER_RIGHTS = 0x1, GROUP_RIGHTS = 0x2, OTHER_RIGHTS = 0x4,
    ALL_RIGHTS = USER_RIGHTS | GROUP_RIGHTS | OTHER_RIGHTS
  };

  bool chmod(int write_mode, int owner_mode) const;
    //!< changes the access rights on the file.

  //! the default separator for directories per operating system.
  /*! the PC uses the backward slash to separate file and directory names from
  each other, while Unix uses the forward slash. */
  enum directory_separator { pc_separator = '\\', unix_separator = '/' };

  static bool separator(char is_it);
    //!< returns true if the character "is_it" in question is a separator.

  static basis::astring default_separator();
    //!< returns the default separator character for this OS.

  static bool legal_character(char to_check);
    //!< returns true if "to_check" is a valid character in a filename.
    /*!< this does not consider separator characters; it only looks at the
    the name components.  also, it is appropriate for the union of the
    operating systems we support. */

  static void detooth_filename(basis::astring &to_clean, char replacement = '_');
    //!< takes any known illegal file system characters out of "to_clean".
    /*!< this prepares "to_clean" for use as a component in a larger filename
    by ensuring that the file system will not reject the name (as long as a
    suitable directory path is prepended to the name and permissions allow
    the file to be created or accessed).  the "replacement" is used as the
    character that is substituted instead of illegal characters. */

  void separate(bool &rooted, structures::string_array &pieces) const;
    //!< breaks the filename into its component parts.
    /*!< this returns an array containing the component names for the path in
    this filename object.  if the "rooted" flag is set to true, then the path
    was absolute (i.e. started at '/' in unix.  this notion is not needed for
    dos/windoze, as the first component will be something like 'a:'). */

  void join(bool rooted, const structures::string_array &pieces);
    //!< undoes a separate() operation to get the filename back.
    /*!< "this" is set to a filename made from each of the "pieces".  if there
    are any directory separators inside the pieces themselves, then they will
    be removed by canonicalize().  if separate() said the path was rooted,
    then join needs to be told that. */

  // these implement the packing functionality.
  virtual void pack(basis::byte_array &packed_form) const;
  virtual bool unpack(basis::byte_array &packed_form);
  virtual int packed_size() const;

  bool compare_prefix(const filename &to_compare, basis::astring &sequel);
    //!< examines "this" filename to see if it's a prefix of "to_compare".
    /*!< this returns true if all of "this" is the same as the first portion
    of "to_compare".  that is, if "this" is a prefix of "to_compare", then
    true is returned.  this will always fail if there are fewer components in
    "to_compare".  it will always succeed if the two filenames are identical.
    on success, the "sequel" is set to the portion of "to_compare" that's
    not included in this filename. */

  bool compare_prefix(const filename &to_compare);
    //!< this simpler form doesn't bother with computing the sequel.

  bool compare_suffix(const filename &to_compare, basis::astring &prequel);
    //!< compares the back end of a filename to this.
    /*!< this is similar to compare_prefix() but it checks to see if the
    back end of "this" filename is the same as "to_compare".  if "this" is
    longer than "to_compare", then failure occurs.  only if all of the bits
    in "this" are seen in the back of "to_compare" is true returned. */

  bool compare_suffix(const filename &to_compare);

  static basis::astring null_device();
    //!< returns the name for the black hole device that consumes all input, i.e. /dev/null.

private:
  bool _had_directory;  //!< true if _some_ directory was specified on init.
///  basis::astring *_contents;  //!< the full path is held here.

  int find_last_separator(const basis::astring &look_at) const;
    //!< locates the last separator character in the filename.

  bool get_info(status_info *to_fill) const;
    //!< returns information for the filename.

  // helper functions do the real work for comparing.
  bool base_compare_prefix(const filename &to_compare, structures::string_array &first,
          structures::string_array &second);
  bool base_compare_suffix(const filename &to_compare, structures::string_array &first,
          structures::string_array &second);
};

} //namespace.

#endif

