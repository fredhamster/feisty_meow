/*****************************************************************************\
*                                                                             *
*  Name   : ini_parser                                                        *
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

#include "ini_parser.h"
#include "table_configurator.h"
#include "variable_tokenizer.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <structures/amorph.h>
#include <structures/string_array.h>
#include <structures/string_table.h>
#include <textual/parser_bits.h>

//#define DEBUG_INI_PARSER
  // uncomment for noisy version.

#undef LOG
#ifdef DEBUG_INI_PARSER
  #define LOG(to_print) printf("%s\n", astring(to_print).s())
#else
  #define LOG(a) {}
#endif

//////////////

using namespace basis;
using namespace structures;
using namespace textual;
//using namespace ;

namespace configuration {

//algorithm:
//  gather section until next section definition or end of file.
//  parse the section with variable_tokenizer.
//  eat that out of the string.
//  repeat.

ini_parser::ini_parser(const astring &to_parse, treatment_of_defaults behavior)
: table_configurator(behavior),
  _well_formed(false),
  _preface(new astring)
{
  reset(to_parse);
}

ini_parser::~ini_parser()
{
  WHACK(_preface);
}

void ini_parser::chow_through_eol(astring &to_chow)
{
  while (to_chow.length()) {
    if (parser_bits::is_eol(to_chow[0])) {
      // zap all carriage return type chars now that we found one.
      while (to_chow.length() && parser_bits::is_eol(to_chow[0])) {
        *_preface += to_chow[0];
        to_chow.zap(0, 0);
      }
      return;  // mission accomplished.
    }
    *_preface += to_chow[0];
    to_chow.zap(0, 0);
  }
}

/*
//this is a super expensive operation...
// it would be better to have the parser be a bit more intelligent.
void strip_blank_lines(astring &to_strip)
{
  bool last_was_ret = false;
  for (int i = 0; i < to_strip.length(); i++) {
    if (parser_bits::is_eol(to_strip[i])) {
      if (last_was_ret) {
        // two in a row; now that's bogus.
        to_strip.zap(i, i);
        i--;  // skip back.
        continue;
      }
      last_was_ret = true;
      to_strip[i] = '\n';  // make sure we know which type to look for.
    } else {
      if (last_was_ret && parser_bits::white_space(to_strip[i])) {
        // well, the last was a return but this is white space.  that's also
        // quite bogus.
        to_strip.zap(i, i);
        i--;  // skip back.
        continue;
      }
      last_was_ret = false;
    }
  }
}
*/

void ini_parser::reset(const astring &to_parse)
{
  _well_formed = false;
  table_configurator::reset();  // clean out existing contents.
  _preface->reset();  // set the preface string back to nothing.
  add(to_parse);
}

void ini_parser::add(const astring &to_parse)
{
  astring parsing = to_parse;
//  strip_blank_lines(parsing);
  _preface->reset();  // set the preface string back to nothing.
  while (parsing.length()) {
    astring section_name;
    bool found_sect = parse_section(parsing, section_name);
    if (!found_sect) {
      // the line is not a section name.  toss it.
      chow_through_eol(parsing);
      continue;  // try to find another section name.
    }
    // we got a section.  yee hah.
    int next_sect = 0;
    for (next_sect = 0; next_sect < parsing.length(); next_sect++) {
//      LOG(astring("[") + astring(parsing[next_sect], 1) + "]");
      if (parser_bits::is_eol(parsing[next_sect])) {
        // we found the requisite return; let's see if a section beginning
        // is just after it.  we know nothing else should be, since we stripped
        // out the blank lines and blanks after CRs.
        if (parsing[next_sect + 1] == '[') {
          // aha, found the bracket that should be a section start.
          break;  // done seeking next section beginning.
        }
      }
    }
    // skip back one if we hit the end of the string.
    if (next_sect >= parsing.length()) next_sect--;
    // now grab what should be all values within a section.
    LOG(a_sprintf("bounds are %d to %d, string len is %d.", 0, next_sect,
        parsing.length()));
    astring sect_parsing = parsing.substring(0, next_sect);
    LOG(astring("going to parse: >>") + sect_parsing + "<<");
    parsing.zap(0, next_sect);
    variable_tokenizer section_reader("\n", "=");
    section_reader.set_comment_chars(";#");
    section_reader.parse(sect_parsing);
    LOG(astring("read: ") + section_reader.text_form());
    merge_section(section_name, section_reader.table());
  }
  _well_formed = true;
}

void ini_parser::merge_section(const astring &section_name,
    const string_table &to_merge)
{
  if (!section_exists(section_name)) {
    // didn't exist yet, so just plunk it in.
    put_section(section_name, to_merge);
    return;
  }
  
  // since the section exists, we just write the individual entries from the
  // new section.  they'll stamp out any old values.
  for (int i = 0; i < to_merge.symbols(); i++)
    put(section_name, to_merge.name(i), to_merge[i]);
}

bool ini_parser::parse_section(astring &to_parse, astring &section_name)
{
  section_name = "";  // reset the section.

  // we have a simple state machine here...
  enum states {
    SEEKING_OPENING_BRACKET,  // looking for the first bracket.
    EATING_SECTION_NAME       // got a bracket, now getting section name.
  };
  states state = SEEKING_OPENING_BRACKET;

  // zip through the string trying to find a valid section name.
  for (int i = 0; i < to_parse.length(); i++) {
    char curr = to_parse[i];
    LOG(astring("<") + astring(curr, 1) + ">");
    switch (state) {
      case SEEKING_OPENING_BRACKET:
        // we're looking for the first bracket now...
        if (parser_bits::white_space(curr)) continue;  // ignore white space.
        if (curr != '[') return false;  // argh, bad characters before bracket.
        state = EATING_SECTION_NAME;  // found the bracket.
        break;
      case EATING_SECTION_NAME:
        // we're adding to the section name now...
        if (curr == ']') {
          // that's the end of the section name.
          to_parse.zap(0, i);  // remove what we saw.
//should we take out to end of line also?
//eventually up to eol could be kept as a comment?
          return true;
        }
        section_name += curr;  // add a character to the name.
        break;
      default:
        //LOG("got to unknown case in section parser!");
        return false;
    }
  }
  // if we got to here, the section was badly formed...  the whole string was
  // parsed through but no conclusion was reached.
  return false;
}

bool ini_parser::restate(astring &new_ini, bool add_spaces)
{
  new_ini = *_preface;  // give it the initial text back again.
  string_array sects;
  sections(sects);
  for (int i = 0; i < sects.length(); i++) {
    new_ini += astring("[") + sects[i] + "]" + parser_bits::platform_eol_to_chars();
    string_table tab;
    if (!get_section(sects[i], tab)) continue;  // serious error.
    tab.add_spaces(add_spaces);
    new_ini += tab.text_form();
  }
  return true;
}

} //namespace.


