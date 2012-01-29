#ifndef SYSTEM_VALUES_CLASS
#define SYSTEM_VALUES_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : system_values                                                     *
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

#include <basis/astring.h>
#include <basis/contracts.h>
//#include <structures/static_memory_gremlin.h>

namespace configuration {

// forward.
class system_values_lookup_list;

//! This class provides a way to look up generated values used in the code base.
/*!
  The type of value here includes outcomes, events and filters so far.  These
  items are reported in the build manifest that is included with every release
  of the compiled software.  The system_values class provides a lookup
  interface to the extensible system of unique identifiers which is mapped by
  the manifest file.  The manifest file is processed like an initialization
  file to retrieve the descriptions and names of symbols when given their
  numerical identifiers.
*/

class system_values : public virtual basis::root_object
{
public:
  system_values(const basis::astring &section_tag);
    //!< provides a lookup on values found in the section named "section_tag".
    /*!< the "section_tag" indicates what kind of asset is being looked up in
    the manifest.  it is always assumed that the manifest file is the main
    one defined here in DEFAULT_MANIFEST.  it must be a file created by
    the value_tagger during the indexing of all value assets defined in the
    build.
    the valid values for the section names so far are "DEFINE_ OUTCOME",
    "DEFINE_ FILTER" and "DEFINE_ EVENT" (with no spaces), but it is better
    to use the defined "VALUES" methods below (such as OUTCOME_VALUES()).
    outcomes are used by functions to describe how the operation completed.
    filters are used to enable different types of logging.  events are sent
    between information source and sink to indicate the occurrence of an
    activity. */
  
  virtual ~system_values();

  DEFINE_CLASS_NAME("system_values");

  // these provide symbolic versions of the section tag used in the
  // constructor.  these are preferable to using the string constants
  // directly.
  static const char *OUTCOME_VALUES();
    //!< values that define the outcomes of operations.
  static const char *FILTER_VALUES();
    //!< values that define filters used in logging.
  static const char *EVENT_VALUES();
    //!< values that define event objects used in the program.

  static const char *DEFAULT_MANIFEST;
    //!< the default manifest file.
    /*!< it is expected to live in the folder where the applications are. */

  bool use_other_manifest(const basis::astring &manifest_file);
    //!< supports using a different manifest file than the default.
    /*!< the values will now come from the "manifest_file" instead of the
    default manifest name shipped with the software release. */

  virtual basis::astring text_form() const;
    //!< shows all items in the table.

  bool lookup(int value, basis::astring &symbolic_name, basis::astring &description,
          basis::astring &file_location);
    //!< locates a "value" and finds its name, description and location.
    /*!< this looks up one of the enumerated values defined for our type of
    value.  true is returned if the value is meaningful.  the "symbolic_name"
    is set to the item's name as used inside the source code (i.e., its enum
    member's name), the "description" is set to the full textual description
    of what that value means, and the "file_location" is set to the name of
    the source file that defines the value. */

  bool lookup(const basis::astring &symbolic_name, int &value, basis::astring &description,
          basis::astring &file_location);
    //!< similar to the above lookup, but seeks on the "symbolic_name".
    /*!< this lookup tries to associate from the textual name of the value
    in order to find the integer actual "value" of it.  the textual
    "description" and "file_location" for that item are also included. */

  int elements() const;
    //!< returns how many items are listed for the types of values specified.

  bool get(int index, basis::astring &symbolic_name, int &value,
           basis::astring &description, basis::astring &file_location);
    //!< accesses the "index"th item in the list.

private:
  basis::astring *_tag;  //!< the name of our section.
  system_values_lookup_list *_list;  //!< the values and text that we found.
  basis::astring *_file;  //!< the manifest filename.

  bool open_values();  //!< retrieves the information from the file.
};

} //namespace.

#endif

