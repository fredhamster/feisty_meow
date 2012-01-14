#ifndef LIST_PARSING_CLASS
#define LIST_PARSING_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : list_parsing                                                      *
*  Author : Chris Koeritz                                                     *
*  Author : Gary Hardley                                                      *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/contracts.h>
#include <structures/set.h>
#include <structures/string_array.h>
#include <structures/string_table.h>

namespace textual {

//! A set of functions that help out with parsing lists of things.

class list_parsing
{
public:
  virtual ~list_parsing();
  DEFINE_CLASS_NAME("list_parsing");

  static bool get_ids_from_string(const basis::astring &string, structures::int_set &ids);
    //!< returns true if a set of unique ids can be extracted from "string".
    /*!< valid separators are spaces, commas, hyphens.  note that this
    returns an int_set although the integers will all be non-negative.
    e.g. "1-4,5 6 7,30-25" is a valid string. */

  static bool get_ids_from_string(const basis::astring &string, basis::int_array &ids);
    //!< same as above except result is an array -- to preserve order.
    /*!< this also retains duplicates. */

  static basis::astring put_ids_in_string(const structures::int_set &ids, char separator = ',');
    //!< returns a string containing a "separator" separated list of ids.
    /*!< spaces are also used between entries for readability. */
  static basis::astring put_ids_in_string(const basis::int_array &ids, char separator = ',');
    //!< operates on an array instead of a set.

  static basis::astring emit_quoted_chunk(const basis::astring &to_emit);
     //!< ensures that quotes inside the string "to_emit" are escaped.

  static bool parse_csv_line(const basis::astring &to_parse, structures::string_array &fields);
    //!< examines the string "to_parse" which should be in csv format.
    /*!< the "fields" list is set to the entries found on the line.  true is
    returned if the line parsed without any errors.  this method will accept
    a backslash as an escape character if it is immediately followed by a
    quote character or another backslash character.  no other escapes are
    currently supported; backslashes will be taken literally otherwise. */

  static void create_csv_line(const structures::string_array &to_csv, basis::astring &target);
  static void create_csv_line(const structures::string_table &to_csv, basis::astring &target);
    //!< writes a CSV line of text into the "target" from the items in "to_csv".
    /*!< the "target" is reset before the line is stored there; thus, this is
    not cumulative.  further, the end of line character is not appended.  this
    will escape quote and backslash characters with a prepended backslash. */
};

} //namespace.

#endif

