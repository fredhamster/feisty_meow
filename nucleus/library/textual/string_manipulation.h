#ifndef STRING_MANIPULATION_CLASS
#define STRING_MANIPULATION_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : string_manipulation                                               *
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

#include <basis/astring.h>

namespace textual {

//! Provides various functions for massaging strings.

class string_manipulation
{
public:

//////////////

  static basis::astring make_random_name(int min = 1, int max = 64);
    //!< creates a random name, where the letters are between 'a' and 'z'.
    /*!< the underscore will also be used occasionally.  the size is random,
    but the minimum size is "min" while the maximum is "max". */

//////////////

  static basis::astring long_line(char line_item = '/', int repeat = 76);
    //!< produces a long line of "line_item" characters.
    /*!< returns a string of text that is somewhat long compared to an 80 column
    output window and which consists of a single character repeated.  the
    character used and the repeat count are both variable. */

  static basis::astring indentation(int spaces);
    //!< Returns a string made of white space that is "spaces" long.

//////////////

  static void carriage_returns_to_spaces(basis::astring &to_strip);
    //!< converts carriage returns in "to_strip" into spaces.
    /*!< processes the string "to_strip" by replacing all single carriage
    returns with spaces and by turning two or more carriage returns into a
    single CR plus spaces. */

//////////////

  static void split_lines(const basis::astring &input, basis::astring &output,
          int min_column = 0, int max_column = 79);
    //!< formats blocks of text for a maximum width.
    /*!< processes the "input" text by splitting any lines that are longer
    than the "max_column".  the "min_column" is used to specify how much
    indentation should be included. */

//////////////

  // numerical manipulation functions:

  static basis::abyte char_to_hex(char to_convert);
    //!< Converts a single character into the corresponding hex nibble.
    /*!< If the character is not valid, an arbitrary value is returned. */
  static char hex_to_char(basis::abyte to_convert);
    //!< Converts a byte between 0 and 15 into a corresponding hexadecimal character.
  
  static basis::byte_array string_to_hex(const basis::astring &character_form);
    //!< Turns a string form of a set of hex numbers into an array of bytes.
    /*!< This functions takes a string in "character_form" and returns an array
    of bytes that is half as long and which contains the hexadecimal
    interpretation of the string.  This is currently geared to even length
    strings... */
  static basis::astring hex_to_string(const basis::byte_array &byte_form);
    //!< The inverse of string_to_hex prints "byte_form" as text.
    /*!< This function takes an array of bytes and converts them into their
    equivalent hexadecimal character representation. */
};

} //namespace.

#endif

