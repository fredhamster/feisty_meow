#ifndef BYTE_FORMATTER_CLASS
#define BYTE_FORMATTER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : byte_formatter                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <basis/definitions.h>

namespace textual {

//! Provides functions for manipulating arrays of bytes.

class byte_formatter : public virtual basis::root_object
{
public:
  virtual ~byte_formatter() {}

  DEFINE_CLASS_NAME("byte_formatter");

  static void print_char(basis::abyte to_print, basis::astring &out,
        char replace = '_');
    //!< prints the byte "to_print" into "out" as long as "to_print" is readable.
    /*!< if it's not readable, then the "replace" is printed. */

  static void print_chars(const basis::abyte *to_print, int length,
          basis::astring &out, char replace = '_');
    //!< sends the bytes in "to_print" of "length" bytes into the string "out".

  static void text_dump(basis::astring &output, const basis::abyte *location,
          basis::un_int length, basis::un_int label = 0, const char *eol = "\n");
    //!< prints out a block of memory in a human readable form.
    /*!< it is stored in the "output" string.  the "location" is where to dump,
    the "length" is the number of bytes to dump, and the "label" is where to
    start numbering the location label on the first line.  the "eol" supplies
    the line ending sequence to be used for the output file.  this should be
    "\r\n" for win32. */

  static basis::astring text_dump(const basis::abyte *location, basis::un_int length,
          basis::un_int label = 0, const char *eol = "\n");
    //!< this is a less efficient version of text_dump that returns a string.
    /*!< it's easier to use when combining astrings. */

  static void text_dump(basis::astring &output,
          const basis::byte_array &to_dump, basis::un_int label = 0, const char *eol = "\n");
    //!< a version that operates on a byte_array and stores into a string.
  static basis::astring text_dump(const basis::byte_array &to_dump,
          basis::un_int label = 0, const char *eol = "\n");
    //!< a version that operates on a byte_array and returns a string.

  //! this operation performs the inverse of a text_dump.
  static void parse_dump(const basis::astring &dumped_form,
          basis::byte_array &bytes_found);

//////////////

  static void bytes_to_string(const basis::byte_array &to_convert,
          basis::astring &as_string, bool space_delimited = true);
    //!< converts a byte_array into a string.
    /*!< takes an array of bytes "to_convert" and spits out the equivalent form
    "as_string".  if "space_delimited" is true, then the bytes are separated
    by spaces. */

  static void string_to_bytes(const basis::astring &to_convert,
          basis::byte_array &as_array);
    //!< wrangles the string "to_convert" into an equivalent byte form "as_array".
    /*!< this is a fairly forgiving conversion; it will accept any string and
    strip out the hexadecimal bytes.  spacing is optional, but every two
    hex nibbles together will be taken as a byte.  if there are an odd number
    of nibbles, then the odd one will be taken as the least significant half
    of a byte. */

  static void bytes_to_string(const basis::abyte *to_convert, int length,
          basis::astring &as_string, bool space_delimited = true);
    //!< a version that accepts a byte pointer and length, rather than byte_array.

  static void string_to_bytes(const char *to_convert,
          basis::byte_array &as_array);
    //!< a version that works with the char pointer rather than an astring.

//////////////

  static void bytes_to_shifted_string
          (const basis::byte_array &to_convert, basis::astring &as_string);
    //!< this is a special purpose converter from bytes to character strings.
    /*!< it merely ensures that the "as_string" version has no zero bytes
    besides the end of string null byte.  this packs 7 bits of data into each
    character, resulting in an 87.5% efficient string packing of the array.
    the resulting string is not readable.  the "as_string" parameter is not
    reset; any data will be appended to it. */

  static void shifted_string_to_bytes(const basis::astring &to_convert,
          basis::byte_array &as_array);
    //!< unshifts a string "to_convert" back into a byte_array.
    /*!< converts a string "to_convert" created by bytes_to_shifted_string() into
    the original array of bytes and stores it in "as_array".  the "as_array"
    parameter is not reset; any data will be appended to it. */

//////////////

  // utility methods to help building the formatted strings.

  static void make_eight(basis::un_int num, basis::astring &out);

  static bool in_hex_range(char to_check);
};

} //namespace.

#endif

