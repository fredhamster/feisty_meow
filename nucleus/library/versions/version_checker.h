#ifndef VERSION_CHECKER_CLASS
#define VERSION_CHECKER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : check_version                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1996-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <basis/contracts.h>
#include <structures/version_record.h>

namespace versions {

//! Provides version checking for shared libraries.

class version_checker : public virtual basis::root_object
{
public:
  version_checker(const basis::astring &library_file_name, const structures::version &expected,
          const basis::astring &version_complaint);
    //!< Constructs a checking object and ensures the version is appropriate.
    /*!< the version checking (a call to the good_version() function) will
    succeed if the library with the "library_file_name" (such as
    "basis32.dll") has the "expected" version.  the simplest way to check
    if the version is correct is probably similar to: @code
      if (!version_checker("my.dll", version(1.2.3.4)).good_version()) {
        ...program exit or version failure management...
      } @endcode
    the "version_complaint" is the message that will be displayed on a
    failure in version checking (with noisy mode enabled).  it should
    describe that a version failure occurred and include contact information
    for the customer to get the most recent versions.  for example: @code
    astring version_grievance = "Please contact Floobert Corporation for "
        "the latest DLL and Executable files (http://www.floobert.com).";
    @endcode */

  virtual ~version_checker();  //!< Destructor releases any resources.

  bool good_version() const;
    //!< Performs the actual version check.
    /*!< If the version check is unsuccessful, then a message that
    describes the problem is shown to the user and false is returned.
    NOTE: the version check will also fail if the version information
    structure cannot be found for that library. */

  static basis::astring module_name(const void *module_handle);
    //!< returns the module name where this object resides; only sensible on win32.

  // base requirements.
  DEFINE_CLASS_NAME("version_checker");
  basis::astring text_form() const;

  static bool loaded(const basis::astring &library_file_name);
    //!< returns true if the "library_file_name" is currently loaded.
  static void *get_handle(const basis::astring &library_file_name);
    //!< retrieves the module handle for the "library_file_name".
    /*!< This returns zero if the library cannot be located.  the returned
    pointer wraps a win32 HMODULE currently, or it is meaningless. */
  static basis::astring get_name(const void *to_find);
    //!< returns the name of the HMODULE specified by "to_find".
    /*!< If that handle cannot be located, then an empty string is returned. */

  static structures::version retrieve_version(const basis::astring &pathname);
    //!< Returns the version given a "pathname" to the DLL or EXE file.
    /*!< If the directory component is not included, then the search path
    will be used. */

  static bool get_record(const basis::astring &pathname, structures::version_record &to_fill);
    //!< Retrieves a version record for the file at "pathname".
    /*!< Returns the full version record found for a given "pathname" to
    the DLL or EXE file in the record "to_fill".  if the directory component
    of the path is not included, then the search path will be used.  false is
    returned if some piece of information could not be located; this does not
    necessarily indicate a total failure of the retrieval. */

  static bool retrieve_version_info(const basis::astring &filename,
          basis::byte_array &to_fill);
    //!< Retrieves the version info for the "filename" into the array "to_fill".

  static bool get_language(basis::byte_array &version_chunk, basis::un_short &high,
          basis::un_short &low);
    //!< Gets the language identifier out of the "version_chunk".
    /*!< Returns true if the language identifier for the dll's version chunk
    could be stored in "high" and "low".  This is a win32-only method. */

  void complain_wrong_version(const basis::astring &library_file_name,
          const structures::version &expected_version,
          const structures::version &version_found) const;
    //!< Reports that the file has the wrong version.

  void complain_cannot_load(const basis::astring &library_file_name) const;
    //!< Reports that the dll could not be loaded.

private:
  basis::astring *_library_file_name;
  structures::version *_expected_version;
  basis::astring *_version_complaint;

  // forbidden.
  version_checker(const version_checker &);
  version_checker &operator =(const version_checker &);
};

} //namespace.

#endif

