/*
*  Name   : astring
*  Author : Chris Koeritz
**
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include "astring.h"
#include "definitions.h"
#include "functions.h"
#include "guards.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __WIN32__
  #undef strcasecmp 
  #undef strncasecmp 
  #define strcasecmp strcmpi
  #define strncasecmp strnicmp
#endif

//#define DEBUG_STRING
  // uncomment for debugging version.

#define no_increment
  // macro just documents a blank parameter in the code.

namespace basis {

const int LONGEST_SPRINTF = 600;  // the longest a simple sprintf can be here.

const char CASE_DIFFERENCE = char('A' - 'a');
  // the measurement of the difference between upper and lower case.

// this factor is used to bias dynamic sprintfs for cases where the length
// is specified, but the actual string is shorter than that length.
const int MAX_FIELD_FUDGE_FACTOR = 64;

const abyte empty_char_star[] = { 0 };
  // used to initialize empty strings.

//////////////

bool astring_comparator(const astring &a, const astring &b) { return a.equal_to(b); }

int calculate_proper_length(int repeat) { return negative(repeat)? 1 : repeat + 1; }

//////////////

astring::astring()
: c_character_manager(1, empty_char_star),
  c_held_string((char * const *)c_character_manager.internal_offset_mem())
{}

astring::astring(const base_string &initial)
: c_character_manager(strlen(initial.observe()) + 1, (abyte *)initial.observe()),
  c_held_string((char * const *)c_character_manager.internal_offset_mem())
{}

astring::astring(char initial, int repeat)
: c_character_manager(calculate_proper_length(repeat))
{
  if (!initial) initial = ' ';  // for nulls, we use spaces.
  int new_size = c_character_manager.length() - 1;
  memset(c_character_manager.access(), initial, new_size);
  c_character_manager.put(new_size, '\0');
  c_held_string = (char * const *)c_character_manager.internal_offset_mem();
}

astring::astring(const astring &s1)
: base_string(),
  c_character_manager(s1.c_character_manager),
  c_held_string((char * const *)c_character_manager.internal_offset_mem())
{
}

astring::astring(const char *initial)
: c_character_manager(calculate_proper_length(initial? int(strlen(initial)) : 0))
{
  c_character_manager.put(0, '\0');
  if (!initial) return;  // bail because there's no string to copy.
  strcpy(access(), initial);
  c_held_string = (char * const *)c_character_manager.internal_offset_mem();
}

astring::astring(special_flag flag, const char *initial, ...)
: c_character_manager(1, empty_char_star),
  c_held_string((char * const *)c_character_manager.internal_offset_mem())
{
  if (!initial) return;
  if ( (flag != UNTERMINATED) && (flag != SPRINTF) ) {
    operator = (astring(astring::SPRINTF, "unknown flag %d", flag));
    return;
  }

  va_list args;
  va_start(args, initial);

  if (flag == UNTERMINATED) {
    // special process for grabbing a string that has no terminating nil.  
    int length = va_arg(args, int);  // get the length of the string out.
    c_character_manager.reset(length, (abyte *)initial);
    c_character_manager += abyte(0);
    va_end(args);
    return;
  }

  // only other flag currently supported is sprintf, so we do that...
  base_sprintf(initial, args);
  va_end(args);
}

astring::~astring() { c_held_string = NIL; }

const astring &astring::empty_string() { return bogonic<astring>(); }

void astring::text_form(base_string &state_fill) const { state_fill.assign(*this); }

int astring::length() const { return c_character_manager.length() - 1; }

byte_array &astring::get_implementation() { return c_character_manager; }

char *astring::access() { return (char *)c_character_manager.access(); }

char astring::get(int index) const { return (char)c_character_manager.get(index); }

const char *astring::observe() const
{ return (const char *)c_character_manager.observe(); }

bool astring::equal_to(const equalizable &s2) const
{
  const astring *s2_cast = cast_or_throw(s2, *this);
  return comparator(*s2_cast) == 0;
}

bool astring::less_than(const orderable &s2) const
{
  const astring *s2_cast = dynamic_cast<const astring *>(&s2);
  if (!s2_cast) throw "error: astring::<: unknown type";
  return comparator(*s2_cast) < 0;
}

int astring::comparator(const astring &s2) const
{ return strcmp(observe(), s2.observe()); }

bool astring::equal_to(const char *that) const
{ return strcmp(observe(), that) == 0; }

bool astring::contains(const astring &to_find) const
{ return (find(to_find, 0) < 0) ? false : true; }

astring &astring::operator += (const astring &s1)
{ insert(length(), s1); return *this; }

void astring::shrink()
{
  astring copy_of_this(observe());
  c_character_manager.swap_contents(copy_of_this.c_character_manager);
}

astring &astring::sprintf(const char *initial, ...)
{
  va_list args;
  va_start(args, initial);
  astring &to_return = base_sprintf(initial, args);
  va_end(args);
  return to_return;
}

astring &astring::base_sprintf(const char *initial, va_list &args)
{
  reset();
  if (!initial) return *this;  // skip null strings.
  if (!initial[0]) return *this;  // skip empty strings.

  // these accumulate parts of the sprintf format within the loop.
  char flag_chars[23], width_chars[23], precision_chars[23], modifier_chars[23];

  // thanks for the inspiration to k&r page 156.
  for (const char *traverser = initial; *traverser; traverser++) {
#ifdef DEBUG_STRING
    printf("index=%d, char=%c\n", traverser - initial, *traverser);
#endif

    if (*traverser != '%') {
      // not a special character, so just drop it in.
      *this += *traverser;
      continue;
    }
    traverser++; // go to the next character.
#ifdef DEBUG_STRING
    printf("index=%d, char=%c\n", traverser - initial, *traverser);
#endif
    if (*traverser == '%') {
      // capture the "%%" style format specifier.
      *this += *traverser;
      continue;
    }
    bool failure = false;
      // becomes set to true if something didn't match in a necessary area.

    seek_flag(traverser, flag_chars, failure);
    if (failure) {
      *this += '%';
      *this += flag_chars;
      continue;
    }
    seek_width(traverser, width_chars);
    seek_precision(traverser, precision_chars);
    seek_modifier(traverser, modifier_chars);
    get_type_character(traverser, args, *this, flag_chars,
        width_chars, precision_chars, modifier_chars);
  }
  return *this;
}

void astring::seek_flag(const char *&traverser, char *flag_chars, bool &failure)
{
  flag_chars[0] = '\0';
  failure = false;
  bool keep_going = true;
  while (!failure && keep_going) {
    switch (*traverser) {
      case '-': case '+': case ' ': case '\011': case '#':
        flag_chars[strlen(flag_chars) + 1] = '\0';
        flag_chars[strlen(flag_chars)] = *traverser++;
        break;
      default:
        // we found a character that doesn't belong in the flags.
        keep_going = false;
        break;
    }
  }
#ifdef DEBUG_STRING
  if (strlen(flag_chars)) printf("[flag=%s]\n", flag_chars);
  else printf("no flags\n");
#endif
}

void astring::seek_width(const char *&traverser, char *width_chars)
{
  width_chars[0] = '\0';
  bool no_more_nums = false;
  bool first_num = true;
  while (!no_more_nums) {
    char wideness[2] = { *traverser, '\0' };
    if (first_num && (wideness[0] == '0')) {
      strcpy(width_chars, wideness);
      traverser++;
    } else if (first_num && (wideness[0] == '*') ) {
      strcpy(width_chars, wideness);
      traverser++;
      no_more_nums = true;
    } else if ( (wideness[0] <= '9') && (wideness[0] >= '0') ) {
      // a failure?
      strcat(width_chars, wideness);
      traverser++;
    } else no_more_nums = true;
    first_num = false;
  }
#ifdef DEBUG_STRING
  if (strlen(width_chars)) printf("[width=%s]\n", width_chars);
  else printf("no widths\n");
#endif
}

void astring::seek_precision(const char *&traverser, char *precision_chars)
{
  precision_chars[0] = '\0';
  if (*traverser != '.') return;
  strcpy(precision_chars, ".");
  traverser++;
  bool no_more_nums = false;
  bool first_num = true;
  while (!no_more_nums) {
    char preciseness[2] = { *traverser, '\0' };
    if (first_num && (preciseness[0] == '0')) {
      strcat(precision_chars, preciseness);
      traverser++;
      no_more_nums = true;
    } else if (first_num && (preciseness[0] == '*') ) {
      strcat(precision_chars, preciseness);
      traverser++;
      no_more_nums = true;
    } else if ( (preciseness[0] <= '9') && (preciseness[0] >= '0') ) {
      strcat(precision_chars, preciseness);
      traverser++;
    } else no_more_nums = true;
    first_num = false;
  }
#ifdef DEBUG_STRING
  if (strlen(precision_chars)) printf("[precis=%s]\n", precision_chars);
  else printf("no precision\n");
#endif
}

void astring::seek_modifier(const char *&traverser, char *modifier_chars)
{
  modifier_chars[0] = '\0';
  switch (*traverser) {
    case 'F': case 'N': case 'h': case 'l': case 'L': {
        modifier_chars[strlen(modifier_chars) + 1] = '\0';
        modifier_chars[strlen(modifier_chars)] = *traverser++;
      break;
    }
  }
#ifdef DEBUG_STRING
  if (strlen(modifier_chars)) printf("[mod=%s]\n", modifier_chars);
  else printf("no modifiers\n");
#endif
}

void astring::get_type_character(const char * &traverser, va_list &args,
    astring &output_string, const char *flag_chars, const char *width_chars,
    const char *precision_chars, const char *modifier_chars)
{
  char formatting[120];
  strcpy(formatting, "%");
  strcat(formatting, flag_chars);
  strcat(formatting, width_chars);
  strcat(formatting, precision_chars);
  strcat(formatting, modifier_chars);
  char tmposh[2] = { *traverser, '\0' };
  strcat(formatting, tmposh);
#ifdef DEBUG_STRING
  printf("format: %s\n", formatting);
#endif

  enum argument_size { bits_8, bits_16, bits_32, bits_64, bits_80 };
  bool ints_are_32_bits;
#ifdef __WIN32__
  ints_are_32_bits = true;
#elif defined(__OS2__)
  ints_are_32_bits = true;
#elif defined(__MSDOS__)
  ints_are_32_bits = false;
#elif defined(__WIN32__)
  ints_are_32_bits = false;
#else
  ints_are_32_bits = true;
#endif
  argument_size next_argument;
  bool use_dynamic_sprintf = false;  // for dynamic printing of strings only.
  // get the type character first and ensure it's valid.
  switch (*traverser) {
    case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':
      next_argument = bits_16;
      if (ints_are_32_bits) next_argument = bits_32;
      break;
    case 'f': case 'e': case 'g': case 'E': case 'G':
      next_argument = bits_64;
      break;
    case 'c':
      next_argument = bits_8;
      break;
    case 's':
      next_argument = bits_32;
      use_dynamic_sprintf = true;
      break;
    case 'n':
      next_argument = bits_32; //?????
      break;
    case 'p':
      next_argument = bits_32; //????
      break;
    default:
      // this is an error; the character is not recognized, so spew out
      // any characters accumulated so far as just themselves.
#ifdef DEBUG_STRING
      printf("failure in type char: %c\n", *traverser);
#endif
      output_string += formatting;
      return;
  }
/* hmmm: not supported yet.
  if (width_chars && (width_chars[0] == '*')) {
  }
  if (precision_chars && (precision_chars[0] == '*')) {
  }
*/
  if (strlen(modifier_chars)) {
    switch (modifier_chars[0]) {
      case 'N':  // near pointer.
        next_argument = bits_16;
        if (ints_are_32_bits) next_argument = bits_32;
        break;
      case 'F':  // far pointer.
        next_argument = bits_32;
        break;
      case 'h':  // short int.
        next_argument = bits_16;
        break;
      case 'l':  // long.
        next_argument = bits_32;
        break;
      case 'L':  // long double;
        next_argument = bits_80;
        break;
      default:
        // a failure has occurred because the modifier character is not
        // one of the recognized values.  everything is just spewed out.
#ifdef DEBUG_STRING
        printf("failure in modifier: %s\n", modifier_chars);
#endif
        output_string += formatting;
        return;
    }
  }
  // action time: the output string is given a tasty value.
  char temp[LONGEST_SPRINTF];
  char *temp2 = NIL;  // for dynamic only.
  switch (next_argument) {
//hmmm: this switch is where support would need to be added for having two
//      arguments (for the '*' case).
    case bits_8: case bits_16:
      if (ints_are_32_bits) ::sprintf(temp, formatting, va_arg(args, long));
      else ::sprintf(temp, formatting, va_arg(args, int));
      break;
    case bits_32:
      if (use_dynamic_sprintf) {
        // currently we only do dynamic sprintf for strings.
        char *to_print = va_arg(args, char *);
        // check if it's valid and if we really need to do it dynamically.
        if (!to_print) {
          // bogus string; put in a complaint.
          use_dynamic_sprintf = false;
          ::sprintf(temp, "{error:parm=NIL}");
        } else if (strlen(to_print) < LONGEST_SPRINTF - 2) {
          // we're within our bounds, plus some safety room, so just do a
          // regular sprintf.
          use_dynamic_sprintf = false;
          ::sprintf(temp, formatting, to_print);
        } else {
          // it's too long, so we definitely need to do it dynamically.
          temp2 = new char[strlen(to_print) + MAX_FIELD_FUDGE_FACTOR];
          ::sprintf(temp2, formatting, to_print);
        }
      } else ::sprintf(temp, formatting, va_arg(args, void *));
      break;
    case bits_64:
      ::sprintf(temp, formatting, va_arg(args, double));
      break;
    case bits_80:
      ::sprintf(temp, formatting, va_arg(args, long double));
      break;
  }
  if (use_dynamic_sprintf) {
    output_string += temp2;
    delete [] temp2;
  } else output_string += temp;
}

//hmmm: de-redundify this function, which is identical to the constructor.
void astring::reset(special_flag flag, const char *initial, ...)
{
  reset();  // clear the string out.
  if (!initial) return;
  if ( (flag != UNTERMINATED) && (flag != SPRINTF) ) {
    operator = (astring(astring::SPRINTF, "unknown flag %d", flag));
    return;
  }

  va_list args;
  va_start(args, initial);

  if (flag == UNTERMINATED) {
    // special process for grabbing a string that has no terminating nil.  
    int length = va_arg(args, int);  // get the length of the string out.
    c_character_manager.reset(length, (abyte *)initial);
    c_character_manager += abyte(0);
    va_end(args);
    return;
  }

  // only other flag currently supported is sprintf, so we do that...
  base_sprintf(initial, args);
  va_end(args);
}

void astring::pad(int len, char padding)
{
  if (length() >= len) return;
  byte_array pad(len - length());
  memset(pad.access(), padding, pad.length());
  operator += (astring(UNTERMINATED, (char *)pad.observe(), pad.length()));
}

void astring::trim(int len)
{
  if (length() <= len) return;
  zap(len, end());
}

astring &astring::operator = (const astring &s1)
{
  if (this != &s1)
    c_character_manager = s1.c_character_manager;
  return *this;
}

astring &astring::operator = (const char *s1)
{
  reset();
  *this += s1;
  return *this;
}

void astring::zap(int position1, int position2)
{
  bounds_return(position1, 0, end(), );
  bounds_return(position2, 0, end(), );
  c_character_manager.zap(position1, position2);
}

void astring::to_lower()
{
  for (int i = 0; i < length(); i++)
    if ( (get(i) >= 'A') && (get(i) <= 'Z') )
      c_character_manager.put(i, char(get(i) - CASE_DIFFERENCE));
}

void astring::to_upper()
{
  for (int i = 0; i < length(); i++)
    if ( (get(i) >= 'a') && (get(i) <= 'z') )
      c_character_manager.put(i, char(get(i) + CASE_DIFFERENCE));
}

astring astring::lower() const
{
  astring to_return(*this);
  to_return.to_lower();
  return to_return;
}

astring astring::upper() const
{
  astring to_return(*this);
  to_return.to_upper();
  return to_return;
}

void astring::copy(char *array_to_stuff, int how_many) const
{
  if (!array_to_stuff) return;
  array_to_stuff[0] = '\0';
  if ( (how_many <= 0) || (length() <= 0) ) return;
  strncpy(array_to_stuff, observe(), minimum(how_many, int(length())));
  array_to_stuff[minimum(how_many, int(length()))] = '\0';
}

bool astring::iequals(const astring &that) const
{ return strcasecmp(observe(), that.observe()) == 0; }

bool astring::iequals(const char *that) const
{ return strcasecmp(observe(), that) == 0; }

int astring::ifind(char to_find, int position, bool reverse) const
{ return char_find(to_find, position, reverse, false); }

int astring::find(char to_find, int position, bool reverse) const
{ return char_find(to_find, position, reverse, true); }

int astring::find_any(const char *to_find, int position, bool reverse) const
{ return char_find_any(to_find, position, reverse, true); }

int astring::ifind_any(const char *to_find, int position, bool reverse) const
{ return char_find_any(to_find, position, reverse, false); }

int astring::find_non_match(const char *to_find, int position,
    bool reverse) const
{ return char_find_any(to_find, position, reverse, false, true); }

char simple_lower(char input)
{
  if ( (input <= 'Z') && (input >= 'A') ) return input - CASE_DIFFERENCE;
  return input;
}

int astring::char_find(char to_find, int position, bool reverse,
    bool case_sense) const
{
  if (position < 0) return common::OUT_OF_RANGE;
  if (position > end()) return common::OUT_OF_RANGE;
  if (reverse) {
    for (int i = position; i >= 0; i--) {
      if (case_sense && (get(i) == to_find)) return i;
      else if (simple_lower(get(i)) == simple_lower(to_find)) return i;
    }
  } else {
    if (case_sense) {
      const char *const pos = strchr(observe() + position, to_find);
      if (pos) return int(pos - observe());
    } else {
      for (int i = position; i < length(); i++)
        if (simple_lower(get(i)) == simple_lower(to_find)) return i;
    }
  }
  return common::NOT_FOUND;
}

bool imatches_any(char to_check, const astring &list)
{
  for (int i = 0; i < list.length(); i++)
    if (simple_lower(to_check) == simple_lower(list[i])) return true;
  return false;
}

bool matches_any(char to_check, const astring &list)
{
  for (int i = 0; i < list.length(); i++)
    if (to_check == list[i]) return true;
  return false;
}

bool matches_none(char to_check, const astring &list)
{
  bool saw_match = false;
  for (int i = 0; i < list.length(); i++)
    if (to_check == list[i]) {
      saw_match = true;
      break;
    }
  return !saw_match;
}

int astring::char_find_any(const astring &to_find, int position, bool reverse,
    bool case_sense, bool invert_find) const
{
  if (position < 0) return common::OUT_OF_RANGE;
  if (position > end()) return common::OUT_OF_RANGE;
  if (reverse) {
    for (int i = position; i >= 0; i--) {
      if (!invert_find) {
        if (case_sense && matches_any(get(i), to_find)) return i;
        else if (imatches_any(get(i), to_find)) return i;
      } else {
//printf("rev posn=%d char=%c", i, get(i));
        // case-sensitivity is not used for inverted finds.
        if (matches_none(get(i), to_find)) return i;
      }
    }
  } else {
    for (int i = position; i < length(); i++) {
      if (!invert_find) {
        if (case_sense && matches_any(get(i), to_find)) return i;
        else if (imatches_any(get(i), to_find)) return i;
      } else {
        // case-sensitivity is not used for inverted finds.
//printf("fwd posn=%d char=%c", i, get(i));
        if (matches_none(get(i), to_find)) return i;
      }
    }
  }
  return common::NOT_FOUND;
}

int astring::find(const astring &to_find, int posn, bool reverse) const
{ return str_find(to_find, posn, reverse, true); }

int astring::ifind(const astring &to_find, int posn, bool reverse) const
{ return str_find(to_find, posn, reverse, false); }

int astring::str_find(const astring &to_find, int posn, bool reverse,
    bool case_sense) const
{
  bounds_return(posn, 0, end(), common::OUT_OF_RANGE);
  if (!to_find.length()) return common::BAD_INPUT;

  // skip some steps by finding the first place that the first character of
  // the string resides in our string.
  if (case_sense)
    posn = find(to_find[0], posn, reverse);
  else posn = ifind(to_find[0], posn, reverse);
  if (posn < 0) return common::NOT_FOUND;
  
//hmmm: there is a better way to do this loop in terms of the number of
//      comparisons performed.  knuth morris pratt algorithm?
  if (case_sense) {
//hmmm: this could use strncmp too?
    if (reverse) {
      if (posn > length() - to_find.length())
        posn = length() - to_find.length();
      for (int i = posn; i >= 0; i--)
        if (!memcmp((void *)&observe()[i], (void *)to_find.observe(),
              to_find.length()))
          return i;
    } else {
      const int find_len = to_find.length();
      const int str_len = length();
      const char first_char = to_find[0];
      bounds_return(posn, 0, str_len - find_len, common::OUT_OF_RANGE);
      for (int i = posn - 1;
          ( ( (i = find(first_char, i + 1)) >= 0)
            && (str_len - i >= find_len) ); no_increment) {
        if (!memcmp((void *)&observe()[i], (void *)to_find.observe(),
              to_find.length()))
          return i;
      }
    }
  } else {
    // not case-sensitive.
    if (reverse) {
      if (posn > length() - to_find.length())
        posn = length() - to_find.length();
      for (int i = posn; i >= 0; i--)
        if (!strncasecmp(&observe()[i], to_find.observe(), to_find.length()))
          return i;
    } else {
      bounds_return(posn, 0, length() - to_find.length(), common::OUT_OF_RANGE);
      for (int i = posn; i < length() - to_find.length() + 1; i++)
        if (!strncasecmp(&observe()[i], to_find.observe(), to_find.length()))
          return i;
    }
  }
  return common::NOT_FOUND;
}

astring astring::operator + (const astring &s1) const
{
  astring to_return(*this);
  to_return += s1;
  return to_return;
}

char &astring::operator [] (int position)
{
  if (position < 0) position = 0;
  if (position > end()) position = 0;
  abyte &found = c_character_manager.use(position);
  char &to_return = *((char *)(&found));
  return to_return;
}

const char &astring::operator [] (int position) const
{
  if (position < 0) position = 0;
  if (position > end()) position = 0;
  const abyte &found = c_character_manager.get(position);
  const char &to_return = *((const char *)(&found));
  return to_return;
}

int astring::convert(int default_value) const
{
  if (!length()) return default_value;
  int to_return;
  int fields = sscanf(observe(), "%d", &to_return);
  if (fields < 1) return default_value;
  return to_return;
}

long astring::convert(long default_value) const
{
  if (!length()) return default_value;
  long to_return;
  int fields = sscanf(observe(), "%ld", &to_return);
  if (fields < 1) return default_value;
  return to_return;
}

float astring::convert(float default_value) const
{
  if (!length()) return default_value;
  float to_return;
  int fields = sscanf(observe(), "%f", &to_return);
  if (fields < 1) return default_value;
  return to_return;
}

double astring::convert(double default_value) const
{
  if (!length()) return default_value;
  double to_return;
  int fields = sscanf(observe(), "%lf", &to_return);
  if (fields < 1) return default_value;
  return to_return;
}

astring &astring::operator += (const char *s1)
{
  if (!s1) return *this;
  int len = length();
  c_character_manager.insert(len, int(strlen(s1)));
  memmove((char *)&c_character_manager[len], s1, int(strlen(s1)));
  return *this;
}

astring &astring::operator += (char s1)
{
  int len = length();
  c_character_manager.insert(len, 1);
  c_character_manager.put(len, s1);
  return *this;
}

bool astring::compare(const astring &to_compare, int start_first,
  int start_second, int count, bool case_sensitive) const
{
  bounds_return(start_first, 0, end(), false);
  bounds_return(start_second, 0, to_compare.end(), false);
  bounds_return(start_first + count, start_first, length(), false);
  bounds_return(start_second + count, start_second, to_compare.length(), false);

  if (!case_sensitive) {
    return !strncasecmp(&observe()[start_first],
        &to_compare.observe()[start_second], count);
  } else {
    return !memcmp((void *)&observe()[start_first],
        (void *)&to_compare.observe()[start_second], count);
  }
}

/*
int astring::icompare(const char *to_compare, int length_in) const
{
  if (!length_in) return 0;  // nothing is equal to nothing.
  int real_len = length_in;
  // if they're passing a negative length, we use the full length.
  if (negative(length_in))
    real_len = length();
  // if we have no length, make the obvious returns now.
  int to_compare_len = int(strlen(to_compare));
  if (!real_len) return to_compare_len? -1 : 0;
  // if the second string is empty, it's always less than the non-empty.
  if (!to_compare_len) return 1;
  int to_return = strncasecmp(observe(), to_compare, real_len);
  if (negative(length_in) && !to_return && (to_compare_len > length()) ) {
    // catch special case for default length when the two are equal except
    // that the second string is longer--this means the first is less than
    // second, not equal.
    return -1;
  } else
    return to_return;
}
*/

/*
bool astring::oy_icompare(const astring &to_compare, int start_first,
    int start_second, int count) const
{
  bounds_return(start_first, 0, end(), false);
  bounds_return(start_second, 0, to_compare.end(), false);
  bounds_return(start_first + count, start_first, length(), false);
  bounds_return(start_second + count, start_second, to_compare.length(), false);
  const char *actual_first = this->observe() + start_first;
  const char *actual_second = to_compare.observe() + start_second;
  return !strncasecmp(actual_first, actual_second, count);
}
*/

bool astring::substring(astring &target, int start, int bender) const
{
  target.reset();
  if (bender < start) return false;
  const int last = end();  // final position that's valid in the string.
  bounds_return(start, 0, last, false);
  bounds_return(bender, 0, last, false);
  target.reset(UNTERMINATED, observe() + start, bender - start + 1);
  return true;
}

astring astring::substring(int start, int end) const
{
  astring to_return;
  substring(to_return, start, end);
  return to_return;
}

astring astring::middle(int start, int count)
{ return substring(start, start + count - 1); }

astring astring::left(int count)
{ return substring(0, count - 1); }

astring astring::right(int count)
{ return substring(end() - count + 1, end()); }

void astring::insert(int position, const astring &to_insert)
{
  bounds_return(position, 0, length(), );
  if (this == &to_insert) {
    astring copy_of_me(to_insert);
    insert(position, copy_of_me);  // not recursive because no longer == me.
  } else {
    c_character_manager.insert(position, to_insert.length());
    c_character_manager.overwrite(position, to_insert.c_character_manager,
        to_insert.length());
  }
}

bool astring::replace(const astring &tag, const astring &replacement)
{
  int where = find(tag);
  if (negative(where)) return false;
  zap(where, where + tag.end());
  insert(where, replacement);
  return true;
}

bool astring::replace_all(const astring &to_replace, const astring &new_string)
{
  bool did_any = false;
  for (int i = 0; i < length(); i++) {
    int indy = find(to_replace, i);
    if (negative(indy)) break;  // get out since there are no more matches.
    i = indy;  // update our position to where we found the string.
    zap(i, i + to_replace.length() - 1);  // remove old string.
    insert(i, new_string);  // plug the new string into the old position.
    i += new_string.length() - 1;  // jump past what we replaced.
    did_any = true;
  }
  return did_any;
}

bool astring::replace_all(char to_replace, char new_char)
{
  bool did_any = false;
  for (int i = 0; i < length(); i++) {
    if (get(i) == to_replace) {
      put(i, new_char);
      did_any = true;
    }
  }
  return did_any;
}

bool astring::matches(const astring &match_list, char to_match)
{
  for (int i = 0; i < match_list.length(); i++)
    if (to_match == match_list.get(i)) return true;
  return false;
}

void astring::strip(const astring &strip_list, how_to_strip way)
{
  if (way & FROM_FRONT)
    while (length() && matches(strip_list, get(0)))
      zap(0, 0);

  if (way & FROM_END)
    while (length() && matches(strip_list, get(end())))
      zap(end(), end());
}

int astring::packed_size() const { return length() + 1; }

void astring::pack(byte_array &target) const
{ attach(target, (char *)c_character_manager.observe()); }

bool astring::unpack(byte_array &source)
{ return detach(source, *this); }

///int astring::icompare(const astring &to_compare, int length_in) const
///{ return icompare(to_compare.observe(), length_in); }

/*
int astring::slow_strncasecmp(const char *first, const char *second, int length)
{
  int len1 = int(strlen(first));
  int len2 = int(strlen(second));
  if (!length) return 0;  // no characters are equal to none.
  if (!len1 && !len2) return 0;  // equal as empty.
  if (!len1 && len2) return -1;  // first < second.
  if (len1 && !len2) return 1;  // first > second.
  if (positive(length)) {
    len1 = minimum(length, len1);
    len2 = minimum(length, len2);
  }
  for (int i = 0; i < len1; i++) {
    if (i > len2 - 1) return 1;  // first > second, had more length.
    if (simple_lower(first[i]) < simple_lower(second[i]))
      return -1;  // first < second.
    if (simple_lower(first[i]) > simple_lower(second[i]))
      return 1;  // first > second.
  }
  // at this point we know second is equal to first up to the length of
  // first.
  if (len2 > len1) return -1;  // second was longer and therefore greater.
  return 0;  // equal.
}
*/

//////////////

a_sprintf::a_sprintf() : astring() {}

a_sprintf::a_sprintf(const astring &s) : astring(s) {}

a_sprintf::a_sprintf(const char *initial, ...)
: astring()
{
  if (!initial) return;
  va_list args;
  va_start(args, initial);
  base_sprintf(initial, args);
  va_end(args);
}

//////////////

void attach(byte_array &packed_form, const char *to_attach)
{
  const int len = int(strlen(to_attach));
  const int old_pos = packed_form.last();
  packed_form.insert(old_pos + 1, len + 1);
  memmove((char *)packed_form.observe() + old_pos + 1, to_attach, len + 1);
}

bool detach(byte_array &packed_form, astring &to_detach)
{
  if (!packed_form.length()) return false;
  // locate the zero termination if possible.
  const void *zero_posn = memchr(packed_form.observe(), '\0',
      packed_form.length()); 
  // make sure we could find the zero termination.
  if (!zero_posn) {
    // nope, never saw a zero.  good thing we checked.
    to_detach.reset();
    return false;
  }
  // set the string up using a standard constructor since we found the zero
  // position; we know the string constructor will be happy.
  to_detach = (char *)packed_form.observe();
  // compute the length of the string we found based on the position of the
  // zero character.
  int find_len = int((abyte *)zero_posn - packed_form.observe());
  // whack the portion of the array that we consumed.
  packed_form.zap(0, find_len);
  return true;
}

//////////////

// contract fulfillment area.

base_string &astring::concatenate_string(const base_string &s)
{
  const astring *cast = dynamic_cast<const astring *>(&s);
  if (cast) *this += *cast;
  else *this += astring(s.observe());
  return *this;
}

base_string &astring::concatenate_char(char c)
{
  *this += c;
  return *this;
}

base_string &astring::assign(const base_string &s)
{
  const astring *cast = dynamic_cast<const astring *>(&s);
  if (cast) *this = *cast;
  else *this = astring(s.observe());
  return *this;
}

base_string &astring::upgrade(const char *s)
{
  *this = s;
  return *this;
}

bool astring::sub_string(base_string &target, int start, int end) const
{
  astring *cast = dynamic_cast<astring *>(&target);
  if (!cast) throw "error: astring::sub_string: unknown type";
  return substring(*cast, start, end);
}

bool astring::sub_compare(const base_string &to_compare, int start_first,
    int start_second, int count, bool case_sensitive) const
{
  const astring *cast = dynamic_cast<const astring *>(&to_compare);
  if (cast) return compare(*cast, start_first, start_second, count, case_sensitive);
  else return compare(astring(to_compare.observe()), start_first, start_second,
      count, case_sensitive);
}

void astring::insert(int position, const base_string &to_insert)
{
  const astring *cast = dynamic_cast<const astring *>(&to_insert);
  if (cast) this->insert(position, *cast);
  else this->insert(position, astring(to_insert.observe()));
}

} //namespace.

