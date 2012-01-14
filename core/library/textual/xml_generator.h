#ifndef XML_GENERATOR_CLASS
#define XML_GENERATOR_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : xml_generator                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2007-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <basis/contracts.h>
#include <structures/string_table.h>

namespace textual {

class tag_info;
class tag_stack;

//! Supports simple XML output with consistency checking.

class xml_generator
{
public:
  enum behavioral_mods {
    HUMAN_READABLE = 0x1,
    CLEAN_ILLEGAL_CHARS = 0x2
  };

  xml_generator(int modifiers = HUMAN_READABLE | CLEAN_ILLEGAL_CHARS);
    //!< creates an xml generator with the specified behavior.

  virtual ~xml_generator();

  DEFINE_CLASS_NAME("xml_generator");

  //! the possible ways that operations here can complete.
  enum outcomes {
    OKAY = basis::common::OKAY,
    NOT_FOUND = basis::common::NOT_FOUND,
    ERRONEOUS_TAG = basis::common::INVALID //temporary until we can shed a compatibility concern.
//    DEF INE_OUTCOME(ERRONEOUS_TAG, -75, "The most recently opened tag must be "
//        "closed before a new tag can be opened and before any other tag can "
//        "be closed"),
  };

  static const char *outcome_name(const basis::outcome &to_name);
    //!< reports the string version of "to_name".

  void reset();  //!< throws out all accumulated information.

  basis::astring generate();
    //!< writes the current state into a string and returns it.
    /*!< if there was an error during generation, the string will be empty.
    note that unclosed tags are not considered an error; they will simply be
    closed.  note that the accumulated string is not cleared after the
    generate() invocation.  use reset() to clear out all prior state. */

  void generate(basis::astring &generated);
    //!< synonym method, writes the current state into "generated".

  basis::outcome add_header(const basis::astring &tag_name, const structures::string_table &attributes);
    //!< adds an xml style header with the "tag_name" and "attributes".
    /*!< headers can be located anywhere in the file. */

  basis::outcome open_tag(const basis::astring &tag_name, const structures::string_table &attributes);
    //!< adds a tag with "tag_name" and the "attributes", if any.
    /*!< this adds an item into the output string in the form: @code
       <tag_name attrib1="value1" attrib2="value2"...> @endcode
    it is required that you close the tag later on, after the tag's contents
    have been added. */

  basis::outcome open_tag(const basis::astring &tag_name);
    //!< adds a tag with "tag_name" without any attributes.

  basis::outcome close_tag(const basis::astring &tag_name);
    //!< closes a previously added "tag_name".
    /*!< this will generate xml code like so: @code
       </tag_name> @endcode
    note that it is an error to try to close any tag but the most recently
    opened one. */

  void close_all_tags();
    //!< a wide-bore method that closes all outstanding tags.

  basis::outcome add_content(const basis::astring &content);
    //!< stores content into the currently opened tag.
    /*!< it is an error to add content when no tag is open. */

  void set_indentation(int to_indent);
    //!< sets the number of spaces to indent for the human readable form.

  static basis::astring clean_reserved(const basis::astring &to_modify,
          bool replace_spaces = false);
    //!< returns a cleaned version of "to_modify" to make it XML appropriate.
    /*!< if "replace_spaces" is true, then any spaces will be turned into
    their html code equivalent; this helps in attribute names. */

  static void clean_reserved_mod(basis::astring &to_modify,
          bool replace_spaces = false);
    //!< ensures that "to_modify" contains only characters valid for XML.
    /*!< this is only different from the other clean method because this
    one modifies the string in place. */

private:
  tag_stack *_tags;  //!< the already opened tags.
  basis::astring *_accumulator;  //!< stores our output.
  bool _human_read;  //!< true if the output should be human readable.
  bool _clean_chars;  //!< true if strings should be validated and repaired.
  int _indentation;  //!< number of spaces per level of xml.

  enum open_types { NORMAL_TAG, HEADER_TAG };
  void print_open_tag(const tag_info &to_print, int type = NORMAL_TAG);
    //!< opens the tag for to_print by showing the tag name and attributes.
  void print_close_tag(const basis::astring &tag_name);
    //!< closes the tag for "tag_name" in the output string.
};

} //namespace.

#endif

