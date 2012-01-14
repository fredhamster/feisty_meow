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

#include "list_parsing.h"
#include "parser_bits.h"

#include <basis/astring.h>
#include <structures/set.h>
#include <structures/string_table.h>

#include <ctype.h>
#include <stdio.h>

using namespace basis;
using namespace structures;

namespace textual {

#undef LOG
#define LOG(to_print) printf("%s::%s: %s\n", static_class_name(), func, astring(to_print).s())

list_parsing::~list_parsing() {}  // needed since we use the class_name macro.

// by Gary Hardley.
bool list_parsing::get_ids_from_string(const astring &to_parse, int_set &identifiers)
{
  identifiers.clear();  // clear existing ids, if any.
  int_array found;
  bool ret = get_ids_from_string(to_parse, found);
  if (!ret) return false;
  for (int i = 0; i < found.length(); i++) identifiers.add(found[i]);
  return true;
}

// by Gary Hardley.
bool list_parsing::get_ids_from_string(const astring &to_parse,
    int_array &identifiers)
{
  identifiers.reset();  // clear existing ids, if any.
  if (!to_parse) return false;
    // if an empty string is passed, return an empty set.

  int last_id = -1;
  int tmp_id;
  bool done = false;
  char last_separator = ' ';

  int index = 0;
  while (!done && (index < to_parse.length())) {
    tmp_id = 0;
    bool got_digit = false;
    while ( (to_parse[index] != ',') && (to_parse[index] != '-')
        && (to_parse[index] != ' ') && (index < to_parse.length()) ) {
      if (!isdigit(to_parse[index])) return false;
      tmp_id *= 10;
      tmp_id += int(to_parse[index++]) - 0x30;
      got_digit = true;
    }

    if (got_digit) {
      if (tmp_id > MAXINT32) return false;

      if (last_id == -1) {
        last_id = tmp_id;
        identifiers += last_id;
      } else {
        // if the last separator was a dash, this is a range
        if (last_separator == '-') {
          if (tmp_id >= last_id) {
            for (int i = last_id + 1; i <= tmp_id; i++) 
                identifiers += i;
          }
          else {
            for (int i = tmp_id; i < last_id; i++) 
                identifiers += i;
          }
          last_id = 0;
          last_separator = ' ';
        } else {
          last_id = tmp_id;
          identifiers += last_id;
        }
      }
    } else {
      // did not read an address, to_parse[index] must be a non-digit.
      if ( (to_parse[index] != ' ') && (to_parse[index] != '-')
          && (to_parse[index] != ',') ) return false;
      last_separator = to_parse[index++];
    }
  }
  return true;
}

//by chris koeritz.
astring list_parsing::put_ids_in_string(const int_set &ids, char separator)
{
  astring to_return;
  for (int i = 0; i < ids.length(); i++) {
    to_return += a_sprintf("%d", ids[i]);
    if (i < ids.length() - 1) {
      to_return += separator;
      to_return += " ";
    }
  }
  return to_return;
}

//by chris koeritz.
astring list_parsing::put_ids_in_string(const int_array &ids, char separator)
{
  astring to_return;
  for (int i = 0; i < ids.length(); i++) {
    to_return += a_sprintf("%d", ids[i]);
    if (i < ids.length() - 1) {
      to_return += separator;
      to_return += " ";
    }
  }
  return to_return;
}

// ensures that quotes inside the string "to_emit" are escaped.
astring list_parsing::emit_quoted_chunk(const astring &to_emit)
{
  astring to_return('\0', 256);  // avoid reallocations with large pre-alloc.
  to_return = "";  // reset to get blank string but keep pre-alloc.
  for (int i = 0; i < to_emit.length(); i++) {
    char next_char = to_emit[i];
    if ( (next_char == '"') || (next_char == '\\') )
      to_return += "\\";  // add the escape before quote or backslash.
    to_return += astring(next_char, 1);
  }  
  return to_return;
}

void list_parsing::create_csv_line(const string_table &to_csv, astring &target)
{
  target = astring::empty_string();
  for (int i = 0; i < to_csv.symbols(); i++) {
    target += astring("\"") + emit_quoted_chunk(to_csv.name(i))
        + "=" + emit_quoted_chunk(to_csv[i]) + "\"";
    if (i < to_csv.symbols() - 1) target += ",";
  }
}

void list_parsing::create_csv_line(const string_array &to_csv, astring &target)
{
  target = astring::empty_string();
  for (int i = 0; i < to_csv.length(); i++) {
    target += astring("\"") + emit_quoted_chunk(to_csv[i]) + "\"";
    if (i < to_csv.length() - 1) target += ",";
  }
}

// we do handle escaped quotes for csv parsing, so check for backslash.
// and since we handle escaped quotes, we also have to handle escaping the
// backslash (otherwise a quoted item with a backslash as the last character
// cannot be handled appropriately, because it will be interpreted as an
// escaped quote instead).  no other escapes are implemented right now.
#define handle_escapes \
  if (to_parse[i] == '\\') { \
    if ( (to_parse[i + 1] == '"') || (to_parse[i + 1] == '\\') ) { \
      i++; \
      accumulator += to_parse[i]; \
      continue; /* skip normal handling in sequel. */ \
    } \
  }

const int ARRAY_PREFILL_AMOUNT = 7;
  // a random default for pre-filling.

#define ADD_LINE_TO_FIELDS(new_line) { \
  storage_slot++;  /* move to next place to store item. */ \
  /* make sure we have enough space for the next slot and then some. */ \
/*LOG(a_sprintf("fields curr=%d stowslot=%d", fields.length(), storage_slot));*/ \
  if (fields.length() < storage_slot + 2) \
    fields.insert(fields.length(), ARRAY_PREFILL_AMOUNT); \
/*LOG(a_sprintf("now fields=%d stowslot=%d", fields.length(), storage_slot));*/ \
  fields[storage_slot] = new_line; \
}

//hmmm: parameterize what is meant by a quote.  maybe comma too.
//by chris koeritz.
bool list_parsing::parse_csv_line(const astring &to_parse, string_array &fields)
{
  FUNCDEF("parse_csv_line");
  // the current field we're chowing.  we puff it out to start with to
  // avoid paying for expanding its memory later.
  astring accumulator(' ', 256);
  accumulator = astring::empty_string();

  // the state machine goes through these states until the entire string
  // is consumed.
  enum states { seeking_quote, eating_string, seeking_comma };
  states state = seeking_quote;

  bool no_second_quote = false;  // true if we started without a quote.
  bool just_saw_comma = false;  // true if seeking comma was the last state.

  int storage_slot = -1;

  for (int i = 0; i < to_parse.length(); i++) {
    switch (state) {
      case seeking_quote:
        if (parser_bits::white_space(to_parse[i])) continue;
        if (to_parse[i] == ',') {
          // a missing quoted string counts as an empty string.
          ADD_LINE_TO_FIELDS(astring::empty_string());
          just_saw_comma = true;
          continue;
        }
        just_saw_comma = false;  // cancel that state.
        if (to_parse[i] != '"') {
          // short circuit the need for a quote.
          accumulator += to_parse[i];
          no_second_quote = true;
        }
        state = eating_string;
        break;
      case eating_string:
        just_saw_comma = false;  // no longer true.
        if (no_second_quote && (to_parse[i] != ',') ) {
          handle_escapes;
          accumulator += to_parse[i];
        } else if (!no_second_quote && (to_parse[i] != '"') ) {
          handle_escapes;
          accumulator += to_parse[i];
        } else {
          // we found the closing quote (or comma).  add the string.
          if (no_second_quote) {
            state = seeking_quote;
            just_saw_comma = true;
          } else state = seeking_comma;
          ADD_LINE_TO_FIELDS(accumulator)
          accumulator = astring::empty_string();
          no_second_quote = false;
        }
        break;
      case seeking_comma:
        if (parser_bits::white_space(to_parse[i])) continue;
        if (to_parse[i] == ',') {
          // we got what we wanted.
          state = seeking_quote;
          just_saw_comma = true;
          continue;
        }
        // well, there was no comma.  that's an error.
        return false;
        break;
      default: {
        LOG("erroneous state reached during csv parsing");
        break;
      }
    }
  }
  if ( (state == eating_string) && (accumulator.length()) )
    ADD_LINE_TO_FIELDS(accumulator)
  else if (just_saw_comma)
    ADD_LINE_TO_FIELDS(astring::empty_string())
  if (fields.length() > storage_slot + 1)
    fields.zap(storage_slot + 1, fields.last());
  return true;
}

} //namespace.


