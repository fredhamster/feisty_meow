#ifndef ASTRING_CLASS
#define ASTRING_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : astring                                                           *
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

#include "base_string.h"
#include "byte_array.h"
#include "contracts.h"

#include <stdarg.h>

namespace basis {

//! Provides a dynamically resizable ASCII character string.
/*!
  It mimics the standard (char *) type, but provides a slew of helpful
  methods as well as enforcing bounds checking on the underlying array.
*/

class astring
: public virtual base_string,
  public virtual hoople_standard
{
public:
  astring();
    //!< constructs an empty string.

  astring(const char *initial);
    //!< constructs a copy of the string passed in "initial".

  astring(char c, int repeat);
    //!< constructs a string with "repeat" characters of "c" in it.
    /*!< if "c" is the null character (i.e., equal to zero), then the resulting
         string will have "repeat" space characters in it. */

  astring(const astring &s);
    //!< Constructs a copy of the string "s".

  astring(const base_string &initial);
    //!< constructs a string from the base class.

  enum special_flag { UNTERMINATED = 62, SPRINTF = 84 };
  astring(special_flag way, const char *s, ...);
    //!< constructor that sports a few variable parameter constructions.
    /*!<
    For a flag of "UNTERMINATED", the constructor expects the third
    parameter to be an integer, and then it copies that number of
    characters from the C-string "s" without assuming that "s" is zero
    terminated.

    For a flag of "SPRINTF", a string is constructed using the format specifier
    in "s" in a manner similar to the standard library "sprintf" function
    (see the standard library for <string.h>).  If there are no "%" codes in
    "s", then the constructor just copies "s" without modification.  If "%"
    codes are in the character array, then any additional arguments (...) are
    interpreted as they would be by sprintf.  The length of the
    constructed string is tailored to fit the actual contents.  If "s" is
    null, then the resulting string will be empty.  Currently, the "*"
    specifier for variable length fields is not supported. */

  virtual ~astring();
    //!< destroys any storage for the string.

  DEFINE_CLASS_NAME("astring");

  virtual int comparator(const astring &s2) const;
    //!< helps to fulfill orderable contract.

  int length() const;
    //!< Returns the current length of the string.
    /*!< The length returned does not include the terminating null character
    at the end of the string. */

  int end() const { return length() - 1; }
    //!< returns the index of the last (non-null) character in the string.
    /*!< If there is no content in the string, then a negative value is
    returned. */

  bool empty() const { return !length(); }
    //!< empty() reports if the string is empty, that is, of zero length().
  bool non_empty() const { return !empty(); }
    //!< non_empty() reports if the string has some contents.
  bool operator ! () const { return empty(); }
    //!< the negation operator returns true if the string is empty.
    /*!< it can be used in expressions in a readable way, for example:
      if (!my_string) { it_is_empty; } */
  bool t() const { return !empty(); }
    //!< t() is a shortcut for the string being "true", as in non-empty.
    /*!< the logic here is that the string is not false because it's not
    empty.  for example: if (my_string.t()) { it_is_not_empty; } */

  static const astring &empty_string();
    //!< useful wherever empty strings are needed, e.g., function defaults.
    /*!< note that this is implemented in the opsystem library to avoid bad
    issues with static objects mixed into multiple dlls from a static
    library. */

  virtual const char *observe() const;
    //!< observes the underlying pointer to the zero-terminated string.
    /*!< this does not allow the contents to be modified.  this method should
    never return NULL_POINTER. */
  const char *c_str() const { return observe(); }
    //!< synonym for observe.  mimics the STL method name.
  const char *s() const { return observe(); }
    //!< synonym for observe.  the 's' stands for "string", if that helps.

  virtual char get(int index) const;
    //!< a constant peek at the string's internals at the specified index.

  virtual char *access();
    //!< provides access to the actual string held.
    /*!< this should never return NULL_POINTER.  be very careful with the returned
    pointer: don't destroy or corrupt its contents (e.g., do not mess with
    its zero termination). */
  char *c_str() { return access(); }
    //!< synonym for access.  mimics the STL method.
  char *s() { return access(); }
    //!< synonym for access.

  char &operator [] (int position);
    //!< accesses individual characters in "this" string.
    /*!< if the "position" is out of range, the return value is
    not meaningful. */
  const char &operator [] (int position) const;
    //!< observes individual characters in "this" string.
    /*!< if the "position" is out of range, the return value is
    not meaningful. */

  virtual void put(int position, char to_put) { (*this)[position] = to_put; }
    //!< stores the character "to_put" at index "position" in the string.

  astring &sprintf(const char *s, ...);
    //!< similar to the SPRINTF constructor, but works on an existing string.
    /*!< any existing contents in the string are wiped out. */

  int convert(int default_value) const;
    //!< Converts the string into a corresponding integer.
    /*!< The conversion starts at index 0 in "this" string, and stores it in
    "value".  If a valid integer was found, it is returned.  otherwise, the
    "default_value" is returned.  NOTE: be careful of implicit conversions
    here; the "default_value" for any of these functions must either be an
    object of the exact type needed or must be cast to that type. */
  long convert(long default_value) const;
    //!< converts the string to a long integer.
  float convert(float default_value) const;
    //!< converts the string to a floating point number.
  double convert(double default_value) const;
    //!< converts the string to a double precision floating point number.

  bool equal_to(const char *that) const;
    //!< returns true if "that" is equal to this.

  bool iequals(const astring &that) const;
    //!< returns true if this is case-insensitively equal to "that".
  bool iequals(const char *that) const;
    //!< returns true if this is case-insensitively equal to "that".

  bool compare(const astring &to_compare, int start_first,
          int start_second, int count, bool case_sensitive) const;
    //!< Compares "this" string with "to_compare".
    /*!< The "start_first" is where the comparison begins in "this" string,
    and "start_second" where it begins in the "to_compare".  The "count" is
    the number of characters to compare between the two strings.  If either
    index is out of range, or "count"-1 + either index is out of range, then
    compare returns false.  If the strings differ in that range, false is
    returned.  Only if the strings have identical contents in the range is
    true returned. */

  bool begins(const astring &maybe_prefix) const
        { return compare(maybe_prefix, 0, 0, maybe_prefix.length(), true); }
    //!< Returns true if "this" string begins with "maybe_prefix".

  bool ibegins(const astring &maybe_prefix) const
        { return compare(maybe_prefix, 0, 0, maybe_prefix.length(), false); }
    //!< a case-insensitive method similar to begins().

  //! returns true if this string ends with "maybe_suffix".
  bool ends(const astring &maybe_suffix) const {
    const int diff = length() - maybe_suffix.length();
    return (diff >= 0) && compare(maybe_suffix, diff, 0, maybe_suffix.length(), true);
  }
  //!< a case-insensitive method similar to ends().
  bool iends(const astring &maybe_suffix) const {
    const int diff = length() - maybe_suffix.length();
    return (diff >= 0) && compare(maybe_suffix, diff, 0, maybe_suffix.length(), false);
  }

  astring &operator = (const astring &s);
    //!< Sets the contents of this string to "s".
  astring &operator = (const char *s);
    //!< Sets the contents of this string to "s".

  void reset() { zap(0, end()); }
    //!< clears out the contents string.

  void reset(special_flag way, const char *s, ...);
    //!< operates like the constructor that takes a 'special_flag'.

  void copy(char *to_stuff, int count) const;
    //!< Copies a maximum of "count" characters from this into "to_stuff".
    /*!< The target "to_stuff" is a standard C-string.  The terminating zero
    from this string is also copied.  BE CAREFUL: if "count"+1 is greater than
    the allocated length of the C-string "to_stuff", then an invalid memory
    write will occur.  keep in mind that the terminating zero will be put at
    position "count" in the C-string if the full "count" of characters are
    copied. */
  void stuff(char *to_stuff, int count) const { copy(to_stuff, count); }
    //!< a synonym for copy().

  astring operator + (const astring &s) const;
    //!< Returns the concatenation of "this" and "s".

  astring &operator += (const astring &s);
    //!< Modifies "this" by concatenating "s" onto it.

  astring &operator += (const char *s);  // this is efficient.
    //!< synonym for the concatenation operator but uses a char pointer instead.
  astring operator + (const char *s) const { return *this + astring(s); }
    //!< synonym for the concatenation operator but uses a char pointer instead.
    // this method is not efficient.

  astring &operator += (char c);  //!< concatenater for single characters.

  int find(char to_find, int position = 0, bool reverse = false) const;
    //!< Locates "to_find" in "this".
    /*!<  find returns the index of "to_find" or "NOT_FOUND".  find starts
    looking at "position".  find returns "OUT_OF_RANGE" if the position is
    beyond the bounds of "this". */
  int find(const astring &to_find, int posn = 0, bool reverse = false) const;
    //!< finds "to_find" in this string.

  int ifind(char to_find, int position = 0, bool reverse = false) const;
    //!< like the find() methods above, but case-insensitive.
  int ifind(const astring &to_find, int posn = 0, bool reverse = false) const;
    //!< like the find() methods above, but case-insensitive.

  int find_any(const char *to_find, int position = 0,
          bool reverse = false) const;
    //!< searches for any of the characters in "to_find".
    /*!<  the first occurrence of any of those is returned, or a negative
    number is returned if no matches are found. */
  int ifind_any(const char *to_find, int position = 0,
          bool reverse = false) const;
    //!< searches case-insensitively for any of the characters in "to_find".
    /*!< the first occurrence of any of those is returned, or a negative number
    is returned if none are found. */
  int find_non_match(const char *to_find, int position = 0,
          bool reverse = false) const;
    //!< searches for any character that is not in "to_find" and returns index.

  bool contains(const astring &to_find) const;
    //!< Returns true if "to_find" is contained in this string or false if not.

  bool substring(astring &target, int start, int end) const;
    //!< a version that stores the substring in an existing "target" string.

  astring substring(int start, int end) const;
    //!< Returns the segment of "this" between the indices "start" and "end".
    /*!< An empty string is returned if the indices are out of range. */

  // helper methods similar to other string's choppers.
  astring middle(int start, int count);
    //!< returns the middle of the string from "start" with "count" characters.
  astring left(int count);
    //!< returns the left "count" characters from the string.
  astring right(int count);
    //!< returns the right "count" characters from the string.

  void pad(int length, char padding = ' ');
    //!< makes the string "length" characters long.
    /*!< this string is padded with the "padding" character if the string is
    less than that length initially. */
  void trim(int length);
    //!< shortens the string to "length" if it's longer than that.

  void insert(int position, const astring &to_insert);
    //!< Copies "to_insert" into "this" at the "position".
    /*!<  Characters at the index "position" and greater are moved over. */
  virtual void zap(int start, int end);
    //!< Deletes the characters between "start" and "end" inclusively.
    /*!< C++ array conventions are used (0 through length()-1 are valid).  If
    either index is out of bounds, then the string is not modified. */

  void to_lower();
    //!< to_lower modifies "this" by replacing capitals with lower-case.
    /*!< every capital letter is replaced with the corresponding lower case
    letter (i.e., A becomes a). */
  void to_upper();
    //!< to_upper does the opposite of to_lower (that is, q becomes Q).
  astring lower() const;
    //!< like to_lower(), but returns a new string rather than modifying this.
  astring upper() const;
    //!< like to_upper(), but returns a new string rather than modifying this.

  bool replace(const astring &tag, const astring &replacement);
    //!< replaces the first occurrence of "tag" text with the "replacement".
    /*!< true is returned if the "tag" was actually found and replaced. */
  bool replace_all(char to_replace, char new_char);
    //!< changes all occurrences of "to_replace" with "new_char".
  bool replace_all(const astring &to_replace, const astring &new_string);
    //! changes all occurrences of "to_replace" into "new_string".

  void shrink();
    //!< resizes the string to its minimum possible length.
    /*!< this fixes any situations where a null character has been inserted
    into the middle of the string.  the string is truncated after the first
    null charater encountered and its size is corrected.  this also repairs
    any case where the string was originally longer than it is now. */

  enum how_to_strip { FROM_FRONT = 1, FROM_END = 2, FROM_BOTH_SIDES = 3 };
    //!< an enumeration describing the strip operations.

  void strip(const astring &strip_list, how_to_strip way = FROM_BOTH_SIDES);
    //!< strips all chars from "strip_list" out of "this" given the "way".

  void strip_spaces(how_to_strip way = FROM_BOTH_SIDES)
          { strip(" ", way); }
    //!< removes excess space characters from string's beginning, end or both.

  void strip_white_spaces(how_to_strip way = FROM_BOTH_SIDES)
          { strip(" \t", way); }
    //!< like strip_spaces, but includes tabs in the list to strip.

  static bool matches(const astring &match_list, char to_match);
    //!< returns true if "to_match" is found in the "match_list" string.

  int packed_size() const;
    //!< Reports the size required to pack this string into a byte array.

  void pack(byte_array &target) const;
    //!< stores this string in the "target".  it can later be unpacked again.
  bool unpack(byte_array &source);
    //!< retrieves a string (packed with pack()) from "source" into this string.
    /*!< note that the string is grabbed from the array destructively; whatever
    portion of the byte array was used to store the string will be removed from
    the head of the array. */

//hmmm: rename this--it is not a simple icompare, but a strncasecmp analogue.
//  int icompare(const astring &to_compare, int length = -1) const;
    //!< provides a case insensitive comparison routine.
    /*!< this uses the best methods available (that is, it uses a system
    function if one exists).  the string "to_compare" is compared with this
    string.  if the "length" is negative, then this entire string is compared
    with the entire string "to_compare".  otherwise, only "length" characters
    from this string are compared.  if this string is before "to_compare" in
    a lexicographic ordering (basically alphabetical), then a negative number
    is returned.  if this string is after "to_compare", then a positive number
    is returned.  zero is returned if the two strings are equal for the extent
    of interest. */

///  int icompare(const char *to_compare, int length = -1) const;
    //!< a version of the above for raw character pointers.

///  static int slow_strncasecmp(const char *first, const char *second,
///          int length = -1);
    //!< a replacement for strncasecmp on platforms without them.
    /*!< this is slow because it cannot rely on OS methods to perform the
    comparison.  if the "length" is negative, then the entire string "first"
    is compared to "second".  otherwise just "length" characters are compared.
    this follows the standard library strncasecmp method: the return value can
    be in three states: negative, zero and positive.  zero means the strings
    are identical lexicographically , whereas less than zero means
    "this_string" is less than "to_compare" and greater than zero means
    "this_string" is greater than "to_compare". */

  // play-yard for implementing base class requirements.

  // these implement the orderable and equalizable interfaces.
  virtual bool equal_to(const equalizable &s2) const;
  virtual bool less_than(const orderable &s2) const;

  virtual base_string &concatenate_string(const base_string &s);
  virtual base_string &concatenate_char(char c);
  virtual base_string &assign(const base_string &s);
  virtual base_string &upgrade(const char *s);
  virtual bool sub_string(base_string &target, int start, int end) const;
  virtual bool sub_compare(const base_string &to_compare, int start_first,
      int start_second, int count, bool case_sensitive) const;
  virtual void insert(int position, const base_string &to_insert);
  virtual void text_form(base_string &state_fill) const;

private:
  byte_array c_character_manager;
    //!< hides the real object responsible for implementing much of the class.

  // the real find methods.
  int char_find(char to_find, int position, bool reverse,
          bool case_sense) const;
  // if "invert_find" is true, then non-matches are reported instead of matches.
  int char_find_any(const astring &to_find, int position, bool reverse,
          bool case_sense, bool invert_find = false) const;
  int str_find(const astring &to_find, int posn, bool reverse,
          bool case_s) const;

  // the functions below are used in the formatting string constructor.
public:  // only for base_sprintf.
  astring &base_sprintf(const char *s, va_list &args);
private:
  char *const *c_held_string;  //!< peeks into the actual pointer for debugging.

  void seek_flag(const char *&traverser, char *flag_chars, bool &failure);
    //!< looks for optional flag characters.
  void seek_width(const char *&traverser, char *width_chars);
    //!< looks for optional width characters.
  void seek_precision(const char *&traverser, char *precision_chars);
    //!< looks for optional precision characters.
  void seek_modifier(const char *&traverser, char *modifier_char);
    //!< looks for optional modifier characters.
  void get_type_character(const char *&traverser, va_list &args,
        astring &output_string, const char *flag_chars,
        const char *width_chars, const char *precision_chars,
        const char *modifier_chars);
    /*!< the required character in a format specifier is either grabbed here or
    the other characters are put into the ouput string without formatting.
    the "X"_char variables should have been previously gathered by the
    seek_"X" functions. */

  public: byte_array &get_implementation(); private: // for test programs only....
};

//////////////

//! a_sprintf is a specialization of astring that provides printf style support.
/*! it makes it much easier to call the SPRINTF style constructor but is
otherwise identical to an astring. */

class a_sprintf : public astring
{
public:
  a_sprintf();
  a_sprintf(const char *initial, ...);
  a_sprintf(const astring &s);
};

//////////////

typedef bool string_comparator_function(const astring &a, const astring &b);
  //!< returns true if the strings "a" and "b" are considered equal.
  /*!< this provides a prototype for the equality operation, which allows the
  notion of equality to be redefined according to a particular function's
  implementation. */

bool astring_comparator(const astring &a, const astring &b);
  //!< implements a string comparator that just does simple astring ==.

//////////////

void attach(byte_array &packed_form, const char *to_attach);
  //!< Packs a character string "to_attach" into "packed_form".
bool detach(byte_array &packed_form, astring &to_detach);
  //!< Unpacks a character string "to_attach" from "packed_form".

} //namespace.

#endif

