#ifndef VERSION_STRUCTURE_GROUP
#define VERSION_STRUCTURE_GROUP

/*****************************************************************************\
*                                                                             *
*  Name   : version & version_record                                          *
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

/*! @file version_record.h
  Note that this header is tuned for forward declaration of the version
  and version_record objects; this is a better approach than including this
  somewhat heavy header file in other header files.
*/

#include "string_array.h"

#include <basis/astring.h>
#include <basis/contracts.h>

namespace structures {

//! Holds a file's version identifier.
/*!
  The version structures can be used in any of our components because
  they're not platform specific.  They maintain information about a file
  that is part of the released product.
*/

class version : public virtual basis::packable, public virtual basis::orderable
{
public:
  version();  //!< constructs a blank version.

  version(const structures::string_array &version_info);
    //!< constructs a version from a list of strings that form the components.
    /*!< note that if a component is an empty string, it is changed to be a
    zero ("0"). */

  version(const basis::astring &formatted_string);
    //!< the version components will be parsed from the "formatted_string".
    /*!< the version component separator is the period ('.') or the comma (',')
    character. */
 
  version(int major, int minor, int rev = 0, int build = 0);
    //!< constructs a win32 style version structure from the information.

  version(const version &to_copy);
    //!< constructs a copy of "to_copy".

  virtual ~version();

  version &operator =(const version &to_copy);
    //!< assigns this to the "to_copy".

  DEFINE_CLASS_NAME("version");

  virtual basis::astring text_form() const;

  bool equal_to(const equalizable &to_test) const;
    //!< compares two versions for exact equality.
    /*!< to perform a check of win32 build compatibility, use the
    compatible() method. */

  bool less_than(const orderable &to_test) const;
    //!< reports if this version is less than "to_test".
    /*!< supplies the other operator needed for the full set of comparisons
    (besides equality).  the basis namespace provides templates for the rest
    of the comparison operators in <basis/functions.h>. */

  int components() const;
    //!< reports the number of components that make up this version.

  basis::astring get_component(int index) const;
    //!< returns the component at the specified index.
    /*!< note that if an entry is an empty string, then a string with zero
    in it is returned ("0"). */

  void set_component(int index, const basis::astring &to_set);
    //!< sets the component at "index" to "to_set".
    /*!< an empty string for "to_set" is turned into a zero. */

  enum version_places { MAJOR, MINOR, REVISION, BUILD };
    //!< these are names for the first four components of the version.
    /*!< there may be more components than four on some platforms. */

  enum version_style { DOTS, COMMAS, DETAILED };
    //!< different ways that a version object can be displayed.
    /*!< DOTS is in the form "1.2.3.4"
    COMMAS is in the form "1, 2, 3, 4"
    DETAILED is in the form "major 1, minor 2, ..."
    */

  basis::astring flex_text_form(version_style style = DOTS, int including = -1) const;
    //!< returns a textual form of the version number.
    /*!< the place passed in "including" specifies how much of the version
    to print, where a negative number means all components.  for example, if
    "including" is MINOR, then only the first two components (major and minor
    components) are printed. */

  static version from_text(const basis::astring &to_convert);
    //!< returns a version structure parsed from "to_convert".

  virtual int packed_size() const;
  virtual void pack(basis::byte_array &target) const;
  virtual bool unpack(basis::byte_array &source);

  //////////////

  // the following methods mainly help on win32 platforms, where the version
  // components are always simple short integers.  there are only ever four
  // of these.  if one adheres to that same scheme on other platforms, then
  // these functions may be helpful.  otherwise, with the mixed alphanumeric
  // versions one sees on unix, these are not so great.

  int v_major() const;
    //!< major version number.
    /*!< major & minor are the most significant values for a numerical version.
    these are the familiar numbers often quoted for software products, like
    "jubware version 8.2". */
  int v_minor() const;
    //!< minor version number.
  int v_revision() const;
    //!< revision level.
    /*!< in the hoople code and the clam system, this number is changed for
    every new build.  when two versions of a file are the same in major,
    minor and revision numbers, then they are said to be compatible.  for
    those using this version scheme, it asserts that dll compatibility has not
    been broken if one swaps those two files in an installation.  after the
    swap, any components that are dependent on the dll must all link properly
    against the replacement file.  when in doubt, increment the version number.
    some folks automatically increment the revision level every week. */
  int v_build() const;
    //!< build number.
    /*!< this number is not considered important when comparing file
    compatibility.  the compatible() method always returns true if two files
    differ only in the "build" number (rather than major, minor or revision).
    this allows patches to be created with a newer (larger) build number, but
    still link fine with existing dlls.  since the file is distinguished by
    more than just its time stamp, it allows changes to an installation to be
    tracked very precisely.  some folks keep a catalog of patched components
    for each software release and index the patch details by the different
    build numbers. */

  bool compatible(const version &that) const;
    //!< returns true if this is compatible with "that" version on win32.
    /*!< that means that all version components are the same except for the
    last one, the build number.  we allow the build numbers to fluctuate so
    that patched components can be installed without causing version
    complaints. */

  bool bogus() const;
    //!< returns true if the version held here is clearly bogus.
    /*!< this means that all four numbers are zero. */

//////////////

  static void *__global_module_handle();
    //!< a static resource required to identify the actual win32 module that this lives in.
    /*!< this handle is stored statically when the libraries are started up.  it records
    the handle of the module they belong to for later use in checking versions. */
    
//hmmm: storage here is still missing!

private:
  structures::string_array *_components;  //!< our list of version components.
};

//////////////

//! Holds all information about a file's versioning.
/*! Not all of these fields are meaningful on every platform. */

class version_record : public virtual basis::root_object
{
public:
  virtual ~version_record();

  DEFINE_CLASS_NAME("version_record");

  basis::astring text_form() const;
    // returns a view of the fields in this record. 

  // these describe a particular file:
  basis::astring description;  // the purpose of this file.
  version file_version;  // the version number for this file.
  basis::astring internal_name;  // the internal name of the file.
  basis::astring original_name;  // name of file before possible renamings.

  // these describe the project that the file belongs to:
  basis::astring product_name;  // the product this file belongs to.
  version product_version;  // the version of the product.

  // these describe the creators of the file:
  basis::astring company_name;  // name of the company that created the file.

  // legal matters:
  basis::astring copyright;  // copyright info for this file.
  basis::astring trademarks;  // trademarks related to the file.

  // extra pieces not stored in record (yet).
  basis::astring web_address;  // the location of the company on the web.
};

//////////////

} //namespace.

#endif

