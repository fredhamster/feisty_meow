#ifndef PARSER_BITS_CLASS
#define PARSER_BITS_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : parser_bits                                                       *
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

//! Warehouses some functions that are often useful during text parsing.

class parser_bits
{
public:
  //! Line endings is an enumeration of the separator character(s) used for text files.
  /*!  on unix, every line in a text file has a line feed (LF) character appended to the line.
  on ms-dos and ms-windows, each line has a carriage return (CR) and line feed (LF) appended
  instead.  a synonym for the line_ending is "eol" which stands for "end of line".  */
  enum line_ending {
    LF_AT_END = -15,  //!< Unix standard is LF_AT_END ("\n").
    CRLF_AT_END,  //!< DOS standard is CRLF_AT_END ("\r\n").
    NO_ENDING  //!< No additional characters added as line endings.
  };

  static const char *eol_to_chars(line_ending ending);
    //!< returns the C string form for the "ending" value.

  static line_ending platform_eol();
    //!< provides the appropriate ending on the current OS platform.

  static const char *platform_eol_to_chars();
    //!< provides the characters that make up this platform's line ending.

  static void translate_CR_for_platform(basis::astring &to_translate);
    //!< flips embedded EOL characters for this platform's needs.
    /*!< runs through the string "to_translate" and changes any CR or CRLF
    combinations into the EOL (end-of-line) character that's appropriate
    for this operating system. */

  static basis::astring substitute_env_vars(const basis::astring &text,
          bool leave_unknown = true);
    //!< resolves embedded environment variables in "text".
    /*!< replaces the names of any environment variables in "text" with the
    variable's value and returns the resulting string.  the variable names
    are marked by a single dollar before an alphanumeric identifier
    (underscores are valid), for example:  $PATH  if the "leave_unknown" flag
    is true, then any unmatched variables are left in the text with a question
    mark instead of a dollar sign.  if it's false, then they are simply
    replaced with nothing at all. */

  static bool is_printable_ascii(char to_check);
    //!< returns true if "to_check" is a normally visible ASCII character.
    /*!< this is defined very simply by it being within the range of 32 to
    126.  that entire range should be printable in ASCII.  before 32 we have
    control characters.  after 126 we have potentially freakish looking
    characters.  this is obviously not appropriate for utf-8 or unicode. */

  static bool white_space_no_cr(char to_check);
    //!< reports if "to_check" is white space but not a carriage return.
    /*!< returns true if the character "to_check" is considered a white space,
    but is not part of an end of line combo (both '\n' and '\r' are
    disallowed).  the allowed set includes tab ('\t') and space (' ') only. */

  static bool is_eol(char to_check);
    //!< returns true if "to_check" is part of an end-of-line sequence.
    /*!< this returns true for both the '\r' and '\n' characters. */

  static bool white_space(char to_check);
    //!< returns true if the character "to_check" is considered a white space.
    /*!< this set includes tab ('\t'), space (' '), carriage return ('\n'),
    and line feed ('\r'). */

  static bool is_alphanumeric(char look_at);
    //!< returns true if "look_at" is one of the alphanumeric characters.
    /*!< This includes a to z in either case and 0 to 9. */
  static bool is_alphanumeric(const char *look_at, int len);
    //!< returns true if the char ptr "look_at" is all alphanumeric characters.
  static bool is_alphanumeric(const basis::astring &look_at, int len);
    //!< returns true if the string "look_at" is all alphanumeric characters.

  static bool is_alpha(char look_at);
    //!< returns true if "look_at" is one of the alphabetical characters.
    /*!< This includes a to z in either case. */
  static bool is_alpha(const char *look_at, int len);
    //!< returns true if the char ptr "look_at" is all alphabetical characters.
  static bool is_alpha(const basis::astring &look_at, int len);
    //!< returns true if the string "look_at" is all alphabetical characters.

  static bool is_numeric(char look_at);
    //!< returns true if "look_at" is a valid numerical character.
    /*! this allows the '-' character for negative numbers also (but only for
    first character if the char* or astring versions are used).  does not
    support floating point numbers or exponential notation yet. */
  static bool is_numeric(const char *look_at, int len);
    //!< returns true if "look_at" is all valid numerical characters.
  static bool is_numeric(const basis::astring &look_at, int len);
    //!< returns true if the "look_at" string has only valid numerical chars.

  static bool is_hexadecimal(char look_at);
    //!< returns true if "look_at" is one of the hexadecimal characters.
    /*!< This includes a to f in either case and 0 to 9. */
  static bool is_hexadecimal(const char *look_at, int len);
    //!< returns true if "look_at" is all hexadecimal characters.
  static bool is_hexadecimal(const basis::astring &look_at, int len);
    //!< returns true if the string "look_at" is all hexadecimal characters.

  static bool is_identifier(char look_at);
    //!< returns true if "look_at" is a valid identifier character.
    /*!< this just allows alphanumeric characters and underscore. */
  static bool is_identifier(const char *look_at, int len);
    //!< returns true if "look_at" is composed of valid identifier character.
    /*!< additionally, identifiers cannot start with a number. */
  static bool is_identifier(const basis::astring &look_at, int len);
    //!< like is_identifier() above but operates on a string.
};

} //namespace.

#endif

