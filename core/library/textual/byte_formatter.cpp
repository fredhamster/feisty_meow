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

#include "byte_formatter.h"
#include "parser_bits.h"
#include "string_manipulation.h"

#include <basis/functions.h>
#include <structures/bit_vector.h>
#include <structures/string_array.h>

//#define DEBUG_BYTE_FORMAT
  // uncomment for noisier version.

#undef LOG
#ifdef DEBUG_BYTE_FORMAT
  #define LOG(s) printf("%s\n", astring(s).s())
#else
  #define LOG(s) {}
#endif

#define LINE_SIZE 80

using namespace basis;
using namespace structures;

namespace textual {

void byte_formatter::print_char(abyte to_print, astring &out, char replace)
{
  int temp = to_print % 128;
  if (!parser_bits::is_printable_ascii(to_print)) out += replace;
  else out += char(temp);
}

void byte_formatter::print_chars(const abyte *to_print, int len, astring &out, char replace)
{
  for (int i = 0; i < len; i++)
    print_char(to_print[i], out, replace);
}

void byte_formatter::make_eight(basis::un_int num, astring &out)
{
  basis::un_int thresh = 0x10000000;
  while (thresh >= 0x10) {
    if (num < thresh)
      out += '0';
    thresh >>= 4;  // zap a nibble.
  }
}

astring byte_formatter::text_dump(const abyte *location, basis::un_int length, basis::un_int label,
    const char *eol)
{
  astring to_return;
  text_dump(to_return, location, length, label, eol);
  return to_return;
}

void byte_formatter::text_dump(astring &output, const byte_array &to_dump, basis::un_int label,
    const char *eol)
{
  text_dump(output, to_dump.observe(), to_dump.length(), label, eol);
}

astring byte_formatter::text_dump(const byte_array &to_dump, basis::un_int label, const char *eol)
{
  astring output;
  text_dump(output, to_dump.observe(), to_dump.length(), label, eol);
  return output;
}

// this is the real version of text_dump.  all the others use it.
void byte_formatter::text_dump(astring &to_return, const abyte *location, basis::un_int length,
    basis::un_int label, const char *eol)
{
  to_return = "";
  int entry_size = 4;
  int preamble = 14;

  basis::un_int entries_per_line = (LINE_SIZE - preamble) / entry_size;

  for (basis::un_int i = 0; i < length; i += entries_per_line) {
    make_eight(i + label, to_return);
    to_return += astring(astring::SPRINTF, "%x", i + label) + astring(" | ");
    for (basis::un_int j = 0; j < entries_per_line; j++) {
      if (i + j >= length) {
        // if at the end of the loop, just print spaces.
        to_return += "   ";
      } else {
        int ord_of_current_char = *(location + i + j) & 0xFF;
        if (ord_of_current_char < 0x10) to_return += '0';
        to_return += astring(astring::SPRINTF, "%x", int(ord_of_current_char));
        to_return += ' ';
      }
    }

    to_return += "| ";
    for (basis::un_int k = i; k < i + entries_per_line; k++) {
      if (k >= length) to_return += ' ';
        // if past the end of the block, just add spaces.
      else print_char(*(location + k), to_return);
    }
    to_return += astring(" |") + eol;
  }
}

void byte_formatter::parse_dump(const astring &dumped_form, byte_array &bytes_found)
{
  bytes_found.reset();
  string_array lines_found;
  // iterate over the string and break it up into lines.
  for (int i = 0; i < dumped_form.length(); i++) {
    int indy = dumped_form.find('\n', i);
//hmmm: not platform invariant.  what about '\r' if we see it?

    if (negative(indy)) {
      // no more lines found.
      if (i < dumped_form.length() - 1) {
        // grab the last bit as a line.
        lines_found += dumped_form.substring(i, dumped_form.length() - 1);
      }
      break;
    }
    // found a normal line ending, so drop everything from the current
    // position up to the ending into the list of strings.
    lines_found += dumped_form.substring(i, indy - 1);
    i = indy + 1;  // jump to next potential line.
  }
  // now process the lines that we've found.
  for (int j = 0; j < lines_found.length(); j++) {
    // first step is to find the pipe character that brackets the actual
    // data.  we ignore the "address" located before the pipe.
    astring &s = lines_found[j];
    int bar_one = s.find('|', 0);
    if (negative(bar_one)) continue;  // skip this one; it's malformed.
    // now we look for the second pipe that comes before the text form of
    // the data.  we don't care about the text or anything after.
    int bar_two = s.find('|', bar_one + 1);
    if (negative(bar_two)) continue;  // skip for same reason.
    astring s2 = s.substring(bar_one + 1, bar_two - 1);
    byte_array this_part;
    string_to_bytes(s2, this_part);
    bytes_found += this_part;
  }
}

//////////////

void byte_formatter::bytes_to_string(const abyte *to_convert, int length, astring &as_string,
    bool space_delimited)
{
  if (!to_convert || !length) return;  // nothing to do.
  if (negative(length)) return;  // bunk.
  as_string = "";  // reset the output parameter.

  // the pattern is used for printing the bytes and considering the delimiter.
  astring pattern("%02x");
  if (space_delimited) pattern += " ";

  // now zip through the array and dump it into the string.
  for (int i = 0; i < length; i++)
    as_string += astring(astring::SPRINTF, pattern.s(), to_convert[i]);
}

// returns true if the character is within the valid ranges of hexadecimal
// nibbles (as text).
bool byte_formatter::in_hex_range(char to_check)
//hmmm: move this to parser bits.
{
  return ( (to_check <= '9') && (to_check >= '0') )
      || ( (to_check <= 'f') && (to_check >= 'a') )
      || ( (to_check <= 'F') && (to_check >= 'A') );
}

void byte_formatter::string_to_bytes(const char *to_convert, byte_array &as_array)
{
  as_array.reset();  // clear the array.
  const int len = int(strlen(to_convert));

  // the parser uses a simple state machine for processing the string.
  enum states { FINDING_HEX, IGNORING_JUNK };
  states state = IGNORING_JUNK;

  int digits = 0;  // the number of digits we've currently found.
  int accumulator = 0;  // the current hex duo.

  // loop through the string.
  for (int i = 0; i < len; i++) {
    switch (state) {
      case IGNORING_JUNK: {
        if (in_hex_range(to_convert[i])) {
          i--;  // skip back to where we were before now.
          state = FINDING_HEX;
          continue;  // jump to the other state.
        }
        // otherwise, we could care less what the character is.
        break;
      }
      case FINDING_HEX: {
        if (digits >= 2) {
          // we have finished a hex byte.
          as_array += abyte(accumulator);
          accumulator = 0;
          digits = 0;
          i--;  // skip back for the byte we haven't eaten yet.
          state = IGNORING_JUNK;  // jump to other state for a new item.
          continue;
        }
        // we really think this is a digit here and we're not through with
        // accumulating them.
        accumulator <<= 4;
        digits++;
        accumulator += string_manipulation::char_to_hex(to_convert[i]);

        // now we sneakily check the next character.
        if (!in_hex_range(to_convert[i+1])) {
          // we now know we should not be in this state for long.
          if (digits) {
            // there's still some undigested stuff.
            digits = 2;  // fake a finished byte.
            continue;  // keep going, but eat the character we were at.
          }
          // well, there's nothing lost if we just jump to that state.
          state = IGNORING_JUNK;
          continue;
        }
        break;
      }
    }
  }
  if (digits) {
    // snag the last unfinished bit.
    as_array += abyte(accumulator);
  }
}

void byte_formatter::bytes_to_string(const byte_array &to_convert, astring &as_string,
    bool space_delimited)
{
  bytes_to_string(to_convert.observe(), to_convert.length(), as_string,
      space_delimited);
}

void byte_formatter::string_to_bytes(const astring &to_convert, byte_array &as_array)
{ string_to_bytes(to_convert.s(), as_array); }

void byte_formatter::bytes_to_shifted_string(const byte_array &to_convert, astring &as_string)
{
#ifdef DEBUG_BYTE_FORMAT
  FUNCDEF("bytes_to_shifted_string");
#endif
  bit_vector splitter(8 * to_convert.length(), to_convert.observe());
  int i;  // track our current position.
  for (i = 0; i < splitter.bits(); i += 7) {
    abyte curr = 1;  // start with a bit set already.
    for (int j = i; j < i + 7; j++) {
      curr <<= 1;  // move to the left.
      if (j < splitter.bits())
        curr |= abyte(splitter.on(j));  // or in the current position.
    }
    as_string += char(curr);
  }
#ifdef DEBUG_BYTE_FORMAT
  LOG(a_sprintf("%d bytes comes out as %d char string.",
      to_convert.length(), as_string.length()).s());
#endif
}

void byte_formatter::shifted_string_to_bytes(const astring &to_convert, byte_array &as_array)
{
#ifdef DEBUG_BYTE_FORMAT
  FUNCDEF("shifted_string_to_bytes");
#endif
  bit_vector accumulator;

  for (int i = 0; i < to_convert.length(); i++) {
    abyte current = abyte(to_convert[i]) & 0x7F;
      // get the current bits but remove the faux sign bit.
    accumulator.resize(accumulator.bits() + 7);
    // now shift off the individual pieces.
    for (int j = 0; j < 7; j++) {
      // get current bit's value.
      current <<= 1;  // shift it up.
      abyte set_here = current & 0x80;  // test the highest order bit.
      // now flip that bit on or off based on what we saw.
      accumulator.set_bit(i * 7 + j, bool(set_here));
    }
  }

  int remainder = accumulator.bits() % 8;
  accumulator.resize(accumulator.bits() - remainder);
    // chop off any extraneous bits that are due to our shifting.

#ifdef DEBUG_BYTE_FORMAT
  // there should be no remainder.  and the number of bits should be a multiple
  // of eight now.
  if (accumulator.bits() % 8)
    deadly_error("byte_formatter", func, "number of bits is erroneous.");
#endif

  const byte_array &accumref = accumulator;
  for (int q = 0; q < accumulator.bits() / 8; q++)
    as_array += accumref[q];

#ifdef DEBUG_BYTE_FORMAT
  LOG(a_sprintf("%d chars comes out as %d bytes.",
      to_convert.length(), as_array.length()).s());
#endif
}

} // namespace

