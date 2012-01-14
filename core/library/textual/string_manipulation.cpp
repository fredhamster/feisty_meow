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

#include "parser_bits.h"
#include "string_manipulation.h"

#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/mutex.h>
#include <mathematics/chaos.h>

using namespace basis;
using namespace mathematics;

namespace textual {

//SAFE_STATIC_CONST(astring_object, string_manipulation::splitter_finding_set,
//    ("\t\r\n -,;?!.:"))
const char *splitter_finding_set = "\t\r\n -,;?!.:";
  // any of these characters make a valid place to break a line.

astring string_manipulation::make_random_name(int min, int max)
{
  chaos rando;
  int length = rando.inclusive(min, max);
    // pick a size for the string.
  astring to_return;
  for (int i = 0; i < length; i++) {
    int chah = rando.inclusive(0, 26);
      // use a range one larger than alphabet size.
    char to_add = 'a' + chah;
    if (chah == 26) to_add = '_';
      // patch the extra value to be a separator.
    to_return += to_add;
  }
  return to_return;
}

astring string_manipulation::long_line(char line_item, int repeat)
{ return astring(line_item, repeat); }

astring string_manipulation::indentation(int spaces)
{
  astring s;
  for (int i = 0; i < spaces; i++) s += ' ';
  return s;
}

void string_manipulation::carriage_returns_to_spaces(astring &to_strip)
{
  for (int j = 0; j < to_strip.length(); j++) {
    int original_j = j;  // track where we started looking.
    if (!parser_bits::is_eol(to_strip[j])) continue;
    // we have found at least one CR.  let's see what else there is.
    if ( (to_strip[j] == '\r') && (to_strip[j + 1] == '\n') ) {
      // this is looking like a DOS CR.  let's skip that now.
      j++;
    }
    j++;  // skip the one we know is a CR.
    if (parser_bits::is_eol(to_strip[j])) {
      // we are seeing more than one carriage return in a row.  let's
      // truncate that down to just one.
      j++;
      while (parser_bits::is_eol(to_strip[j]) && (j < to_strip.length()))
        j++;  // skip to next one that might not be CR.
      // now we think we know where there's this huge line of CRs.  we will
      // turn them all into spaces except the first.
      to_strip[original_j] = '\n';
      for (int k = original_j + 1; k < j; k++) to_strip[k] = ' ';
      // put the index back so we'll start looking at the non-CR char.
      j--;
      continue;  // now skip back out to the main loop.
    } else {
      // we see only one carriage return, which we will drop in favor of
      // joining those lines together.  we iterate here since we might have
      // seen a DOS CR taking up two spaces.
      for (int k = original_j; k < j; k++) to_strip[k] = ' ';
    }
  }

}

void string_manipulation::split_lines(const astring &input_in, astring &output,
    int min_column, int max_column)
{
  output = "";
  if (max_column - min_column + 1 < 2) return;  // what's the point?

  astring input = input_in;  // make a copy to work on.
  carriage_returns_to_spaces(input);

  int col = min_column;
  astring indent_add = indentation(min_column);
  output = indent_add;  // start with the extra space.

  bool just_had_break = false;
    // set true if we just handled a line break in the previous loop.
  bool put_accum_before_break = false;  // true if we must postpone CR.
  astring accumulated;
    // holds stuff to print on next go-round.

  // now we parse across the list counting up our line size and making sure
  // we don't go over it.
  for (int j = 0; j < input.length(); j++) {

//char to_print = input[j];
//if (parser_bits::is_eol(to_print)) to_print = '_';
//printf("[%d: val=%d, '%c', col=%d]\n", j, to_print, to_print, col);
//fflush(0);

    // handle the carriage return if it was ordered.
    if (just_had_break) {
      if (put_accum_before_break) {
        output += accumulated;
        // strip off any spaces from the end of the line.
        output.strip_spaces(astring::FROM_END);
        output += parser_bits::platform_eol_to_chars();
        accumulated = "";
        j++;  // skip the CR that we think is there.
      }
      // strip off any spaces from the end of the line.
      output.strip_spaces(astring::FROM_END);
      output += parser_bits::platform_eol_to_chars();
      col = min_column;
      output += indent_add;
      just_had_break = false;
      if (accumulated.length()) {
        output += accumulated;
        col += accumulated.length();
        accumulated = "";
      }
      j--;
      continue;
    }

    put_accum_before_break = false;

    // skip any spaces we've got at the current position.
    while ( (input[j] == ' ') || (input[j] == '\t') ) {
      j++;
      if (j >= input.length()) break;  // break out of subloop if past it.
    }

    if (j >= input.length()) break;  // we're past the end.

    // handle carriage returns when they're at the current position.
    char current_char = input[j];
    if (parser_bits::is_eol(current_char)) {
      just_had_break = true;  // set the state.
      put_accum_before_break = true;
      continue;
    }

//hmmm: the portion below could be called a find word break function.

    bool add_dash = false;  // true if we need to break a word and add hyphen.
    bool break_line = false;  // true if we need to go to the next line.
    bool invisible = false;  // true if invisible characters were seen.
    bool end_sentence = false;  // true if there was a sentence terminator.
    bool punctuate = false;  // true if there was normal punctuation.
    bool keep_on_line = false;  // true if we want add current then break line.
    char prior_break = '\0';  // set for real below.
    char prior_break_plus_1 = '\0';  // ditto.

    // find where our next normal word break is, if possible.
    int next_break = input.find_any(splitter_finding_set, j);
    // if we didn't find a separator, just use the end of the string.
    if (negative(next_break))
      next_break = input.length() - 1;

    // now we know where we're supposed to break, but we don't know if it
    // will all fit.
    prior_break = input[next_break];
      // hang onto the value before we change next_break.
    prior_break_plus_1 = input[next_break + 1];
      // should still be safe since we're stopping before the last zero.
    switch (prior_break) {
      case '\r': case '\n':
        break_line = true;
        just_had_break = true;
        put_accum_before_break = true;
        // intentional fall-through.
      case '\t': case ' ':
        invisible = true;
        next_break--;  // don't include it in what's printed.
        break;
      case '?': case '!': case '.':
        end_sentence = true;
        // if we see multiples of these, we count them as just one.
        while ( (input[next_break + 1] == '?')
            || (input[next_break + 1] == '!')
            || (input[next_break + 1] == '.') ) {
          next_break++;
        }
        // make sure that there's a blank area after the supposed punctuation.
        if (!parser_bits::white_space(input[next_break + 1]))
          end_sentence = false;
        break;
      case ',': case ';': case ':':
        punctuate = true;
        // make sure that there's a blank area after the supposed punctuation.
        if (!parser_bits::white_space(input[next_break + 1]))
          punctuate = false;
        break;
    }

    // we'll need to add some spaces for certain punctuation.
    int punct_adder = 0;
    if (punctuate || invisible) punct_adder = 1;
    if (end_sentence) punct_adder = 2;

    // check that we're still in bounds.
    int chars_added = next_break - j + 1;
    if (col + chars_added + punct_adder > max_column + 1) {
      // we need to break before the next breakable character.
      break_line = true;
      just_had_break = true;
      if (col + chars_added <= max_column + 1) {
        // it will fit without the punctuation spaces, which is fine since
        // it should be the end of the line.
        invisible = false;
        punctuate = false;
        end_sentence = false;
        punct_adder = 0;
        keep_on_line = true;
      } else if (min_column + chars_added > max_column + 1) {
        // this word won't ever fit unless we break it.
        int chars_left = max_column - col + 1;
          // remember to take out room for the dash also.
        if (chars_left < 2) {
          j--;  // stay where we are.
          continue;
        } else {
          next_break = j + chars_left - 2;
          chars_added = next_break - j + 1;
          if (next_break >= input.length())
            next_break = input.length() - 1;
          else if (next_break < j)
            next_break = j;
          add_dash = true;
        }
      }
    }

    astring adding_chunk = input.substring(j, next_break);
      // this is what we've decided the next word chunk to be added will be.
      // we still haven't completely decided where it goes.

    if (break_line) {
      col = min_column;
      if (add_dash || keep_on_line) {
        // include the previous stuff on the same line.
        output += adding_chunk;
        if (add_dash) output += "-";
        j = next_break;
        continue;  // done with this case.
      }

      // don't include the previous stuff; make it go to the next line.
      accumulated = adding_chunk;
      if (punctuate || invisible) {
        accumulated += " ";
      } else if (end_sentence) {
        accumulated += "  ";
      }
      j = next_break;
      continue;
    }

    // add the line normally since it should fit.
    output += adding_chunk;
    col += chars_added + punct_adder;  // add the characters added.
    j = next_break;
    just_had_break = false;  // reset the state.

    // handle when we processed an invisible or punctuation character.
    if (punctuate || invisible) {
      output += " ";
    } else if (end_sentence) {
      output += "  ";
    }
  }
  // make sure we handle any leftovers.
  if (accumulated.length()) {
    output.strip_spaces(astring::FROM_END);
    output += parser_bits::platform_eol_to_chars();
    output += indent_add;
    output += accumulated;
  }
  output.strip_spaces(astring::FROM_END);
  output += parser_bits::platform_eol_to_chars();
}

char string_manipulation::hex_to_char(abyte to_convert)
{
  if (to_convert <= 9) return char('0' + to_convert);
  else if ( (to_convert >= 10) && (to_convert <= 15) )
    return char('A' - 10 + to_convert);
  else return '?';
}

abyte string_manipulation::char_to_hex(char to_convert)
{
  if ( (to_convert >= '0') && (to_convert <= '9') )
    return char(to_convert - '0');
  else if ( (to_convert >= 'a') && (to_convert <= 'f') )
    return char(to_convert - 'a' + 10);
  else if ( (to_convert >= 'A') && (to_convert <= 'F') )
    return char(to_convert - 'A' + 10);
  else return 0;
}

byte_array string_manipulation::string_to_hex(const astring &to_convert)
{
  byte_array to_return(0, NIL);
  for (int i = 0; i < to_convert.length() / 2; i++) {
    int str_index = i * 2;
    abyte first_byte = char_to_hex(to_convert.get(str_index));
    abyte second_byte = char_to_hex(to_convert.get(str_index + 1));
    abyte to_stuff = abyte(first_byte * 16 + second_byte);
    to_return.concatenate(to_stuff);
  }
  return to_return;
}

astring string_manipulation::hex_to_string(const byte_array &to_convert)
{
  astring to_return;
  for (int i = 0; i < to_convert.length() * 2; i += 2) {
    int str_index = i / 2;
    char first_char = hex_to_char(char(to_convert.get(str_index) / 16));
    char second_char = hex_to_char(char(to_convert.get(str_index) % 16));
    to_return += astring(first_char, 1);
    to_return += astring(second_char, 1);
  }
  return to_return;
}

} //namespace.

