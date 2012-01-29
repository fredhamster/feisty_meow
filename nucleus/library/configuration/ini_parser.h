#ifndef INI_PARSER_CLASS
#define INI_PARSER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : ini_parser                                                        *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "table_configurator.h"

namespace configuration {

//! Parses strings in the fairly well-known INI file format.
/*!
  Description of INI file format:

    The format expected in this parser for initialization files allows for
  three types of entries.  These are section headers, comments and value
  definitions.
    Section headers define the start of a list of value definitions.  A
  section header is a name in brackets, like [startup].
    A comment is a string of text that will be ignored.  Comments are preceded
  by an octothorpe ('#') or a semicolon (';').  The parser will keep comments
  roughly in the same places they were found in the string that was parsed.
  A comment is allowed to follow a section header on the same line.
    Value definitions are a pair of strings separated by an equality operator
  ('=').  The left side of the value definition is referred to here as the
  variable's name while the right side is referred to as the variable's value.
  Note that any line which is not a comment or a section header is considered
  implicitly to be a value definition, even if it does not contain an equals
  operator.  This is required for parsing certain INI files that don't follow
  the established format.  Such lines will have an empty string for their
  value.
    White space (either tab characters or space characters) are allowed before
  and after any of these constructs.  Spaces may also exist before and after
  the equals operator of a value definition, but once the value starts, any
  white space is considered part of the value.  Trailing white space is not
  considered part of a variable name, but white space between the characters
  before the equals operator is signficant.  Any number of carriage returns
  can separate the lines of the INI file.
 
  Here is an example of a valid INI file:
    @code
    # Initialization file for Frootnik Corp.
 
    [common]  ; all of our programs use these.
    magnification=1
 
    text_color=puce
    font=atavata
      ;;; see font/color document in readme.txt.
 
    [sourceburger]  ; sourceburger application specific settings
 
    crashnow = 0
    crashlater = 1
    get clue=0
    windowtitle = Source Burger 5000
 
    danger will robinson
    @endcode
*/

class ini_parser : public table_configurator
{
public:
  ini_parser(const basis::astring &to_parse,
          treatment_of_defaults behavior = RETURN_ONLY);
    //!< constructs an ini_parser by parsing entries out of "to_parse".
    /*!< after construction, the success of parsing can be checked using
    well_formed(). */

  ~ini_parser();

  void reset(const basis::astring &to_parse);
    //!< drops any existing information and processes the string "to_parse".

  void add(const basis::astring &to_parse);
    //!< merges items parsed from "to_parse" into the current set.
    /*!< processes the string "to_parse" as in the reset() method but adds
    any new sections found to our configuration.  if sections are found with
    the same names, then the values found in "to_parse" override the ones
    already listed. */

  bool well_formed() const { return _well_formed; }
    //!< returns true if the ini file's contents were in the format expected.

  bool restate(basis::astring &new_ini, bool add_spaces = false);
    //!< stores a cleaned version of the internal state into "new_ini".
    /*!< if "add_spaces" is true, then the entries will be in the form of
    'x = y' rather than 'x=y'. */

  void merge_section(const basis::astring &section_name, const structures::string_table &to_merge);
    //!< merges the table "to_merge" into the "section_name".
    /*!< any new values from "to_merge" that are not found in the section with
    "section_name" in "this" object are added and any existing values will be
    replaced. */

private:
  bool _well_formed;  //!< true if the ini file had a valid format.
  basis::astring *_preface;  //!< information that comes before the first section.

  void chow_through_eol(basis::astring &to_chow);
    //!< eats up to an EOL character but adds the text to our preface string.

  bool parse_section(basis::astring &to_parse, basis::astring &section_name);
    //!< looks for a section name in the string "to_parse".
    /*!< true is returned on success; success means that a "section_name" was
    found and that "to_parse" has been destructively eaten to remove it. */
};

} //namespace.

#endif

