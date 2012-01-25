#ifndef TOKENIZER_CLASS
#define TOKENIZER_CLASS

/*
*  Name   : variable_tokenizer
*  Author : Chris Koeritz
**
* Copyright (c) 1997-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <basis/contracts.h>
#include <structures/string_table.h>

namespace configuration {

//! Manages a bank of textual definitions of variables.
/*!
  Manipulates strings containing variable definitions where a variable
  is syntactically defined as a name, an assignment operator, and a value.
  The string can optionally define many variables by placing a separator
  character between the definitions.  The assignment and separator are
  referred to as sentinels in the following docs.
  This class also supports quoted values if the appropriate constructor
  is used.
*/

class variable_tokenizer : public virtual basis::root_object
{
public:
  enum constraints { DEFAULT_MAX_BITS = 7 };

  variable_tokenizer(int max_bits = DEFAULT_MAX_BITS);
    //!< creates a variable_tokenizer with the default characters.
    /*!< this will not look for quote characters.  the "max_bits" establishes
    the hashing width for the internal table of strings; there will be
    2 ^ "max_bits" of space in the table.  the default assignment operator
    is '=' and the default separator is ','. */

  variable_tokenizer(const basis::astring &separator, const basis::astring &assignment,
          int max_bits = DEFAULT_MAX_BITS);
    //!< creates an empty list of tokens and uses the specified sentinel chars.
    /*!< the character that is expected to be between name/value pairs is
    "separator".  the "assignment" character is expected to be between each
    name and its value.  note that if the "separator" or "assignment" are more
    than one character long, these will be taken as a set of valid characters
    that can be used for those purposes. */

  variable_tokenizer(const basis::astring &separator, const basis::astring &assignment,
          const basis::astring &quotes, bool nesting = true,
          int max_bits = DEFAULT_MAX_BITS);
    //!< similar to the constructor above, but supports quoting.
    /*!< if the "quotes" list is not empty, then those characters will be
    treated as quoting characters that must be matched in pairs.  inside a
    quote, separators are ignored.  if "nesting" is not true, then only one
    level of quotes will be considered; the occurrence of other types of
    quotes will be ignored until the original type is completed. */

  variable_tokenizer(const variable_tokenizer &to_copy);
    //!< builds a variable_tokenizer that is identical to "to_copy".

  virtual ~variable_tokenizer();

  DEFINE_CLASS_NAME("variable_tokenizer");

  void set_comment_chars(const basis::astring &comments);
    //!< establishes a set of characters in "comments" as the comment items.
    /*!< comments will be specially handled by being added to the string table
    with the comment prefix.  this allows them to be regenerated uniquely
    later. */

  variable_tokenizer &operator =(const variable_tokenizer &to_copy);
    //!< makes this variable_tokenizer identical to "to_copy".

  int symbols() const;
    //!< returns the number of entries in the variable_tokenizer.

  void reset();
    //!< clears all of the entries out.

  const structures::string_table &table() const;
    //!< provides a constant peek at the string_table holding the values.
  structures::string_table &table();
    //!< provides direct access to the string_table holding the values.

//fix these docs.
  bool parse(const basis::astring &to_tokenize);
    //!< parses the string using our established sentinel characters.
    /*!< attempts to snag as many value/pairs from "to_tokenize" as are
    possible by using the current separator and assignment characters.  
    E.G.: if the separator is ';' and the assignment character
    is '=', then one's string would look something like: @code
      TEMP=c:\tmp; GLOB=c:\glob.exe; ....  @endcode
    whitespace is ignored if it's found (1) after a separator and before
    the next variable name, (2) after the variable name and before the
    assignment character, (3) after the assignment character and before the
    value.  this unfortunately implies that white space cannot begin or end
    a value.
    NOTE: unpredictable results will occur: if one's variables are
    improperly formed, if assignment operators are missing or misplaced,
    or if the separator character is used within the value.
    NOTE: carriage returns are considered white-space and can exist in the
    string as described above.
    NOTE: parse is additive; if multiple calls to parse() occur, then the
    symbol_table will be built from the most recent values found in the
    parameters to parse().  if this is not desired, the symbol table's
    reset() function can be used to empty out all variables. */

  basis::astring find(const basis::astring &name) const;
    //!< locates the value for a variable named "name" if it exists.
    /*!< if "name" doesn't exist, then it returns an empty string.  note that
    an empty string might also indicate that the value is blank; locate is the
    way to tell if a field is really missing.  also note that when a variable
    name is followed by an assignment operator and an empty value (e.g.,
    "avversione=" has no value), then a value of a single space character
    will be stored.  this ensures that the same format is used on the
    output side, but it also means that if you access the table directly,
    then you will get a space as the value.  however, this function returns
    an empty string for those entries to keep consistent with expectations. */

  bool exists(const basis::astring &name) const;
    //!< returns true if the "name" exists in the variable_tokenizer.

  basis::astring text_form() const;
    //!< creates a new token list as a string of text.
    /*!< the first separator and assignment characters in each set are used
    to generate it.  note that the whitespace that existed in the original
    parsed string might not be exactly the same in the generated string. */
  void text_form(basis::astring &to_fill) const;
    //!< like text_form() above, but stores into "to_fill".

  // dictates whether the output will have spaces between the assignment
  // character and the key name and value.  default is to not add them.
  bool add_spaces() const { return _add_spaces; }
  void add_spaces(bool add_them) { _add_spaces = add_them; }

  bool okay_for_variable_name(char to_check) const;
    //!< true if "to_check" is a valid variable name character.
    /*!< this includes any characters besides separators and assignments. */

  const basis::astring &assignments() const;
    //!< provides a peek at the assignments list.
  const basis::astring &separators() const;
    //!< provides a peek at the separators list.
  const basis::astring &quotes() const;
    //!< provides a peek at the quotes list.

  bool assignment(char to_check) const;
    //!< true if "to_check" is a valid assignment operator.

  bool separator(char to_check) const;
    //!< true if "to_check" is a valid separator.

  bool comment_char(char to_check) const;
    //!< true if "to_check" is a registered comment character.

  bool is_eol_a_separator() const;
    //!< reports whether any of the separators are an EOL character.

  bool quote_mark(char to_check) const;
    //!< true if "to_check" is a member of the quotes list.

private:
  structures::string_table *_implementation;  //!< holds the parsed values.
  basis::astring *_assignments;  //!< separates name from value.
  basis::astring *_separators;  //!< separates name/value pairs from other pairs.
  basis::astring *_quotes;  //!< the characters that are used for quoting.
  bool _nesting;  //!< if true, we nest arbitrary levels of quotes.
  basis::astring *_comments;  //!< if non-empty, characters that begin comments.
  int _comment_number;  //!< automatically incremented for use in comment tags.
  bool _add_spaces;  //!< records whether we add spaces around the assignment.
};

} //namespace.

#endif

