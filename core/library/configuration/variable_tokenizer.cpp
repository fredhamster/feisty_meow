// Name   : variable_tokenizer
// Author : Chris Koeritz
/*
* Copyright (c) 1997-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include "variable_tokenizer.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <structures/stack.h>
#include <structures/string_table.h>
#include <structures/symbol_table.h>
#include <textual/parser_bits.h>

//#define DEBUG_VARIABLE_TOKENIZER
  // uncomment for noisier run.

const char *SPECIAL_VALUE = " ";
  // special value stored for entries with assignment operators but no
  // value contents.

#undef LOG
#ifdef DEBUG_VARIABLE_TOKENIZER
  #include <stdio.h>
  #define LOG(to_print) printf("%s\n", astring(to_print).s());
#else
  #define LOG(to_print)
#endif

using namespace basis;
using namespace structures;
using namespace textual;

namespace configuration {

variable_tokenizer::variable_tokenizer(int max_bits)
: _implementation(new string_table(max_bits)),
  _assignments(new astring("=")),
  _separators(new astring(",")),
  _quotes(new astring),
  _nesting(false),
  _comments(new astring),
  _comment_number(1),
  _add_spaces(false)
{}

variable_tokenizer::variable_tokenizer(const astring &separator, const astring &assignment,
    int max_bits)
: _implementation(new string_table(max_bits)),
  _assignments(new astring(assignment)),
  _separators(new astring(separator)),
  _quotes(new astring),
  _nesting(false),
  _comments(new astring),
  _comment_number(1),
  _add_spaces(false)
{}

variable_tokenizer::variable_tokenizer(const astring &separator, const astring &assignment,
    const astring &quotes, bool nesting, int max_bits)
: _implementation(new string_table(max_bits)),
  _assignments(new astring(assignment)),
  _separators(new astring(separator)),
  _quotes(new astring(quotes)),
  _nesting(nesting),
  _comments(new astring),
  _comment_number(1),
  _add_spaces(false)
{}

variable_tokenizer::variable_tokenizer(const variable_tokenizer &to_copy)
: _implementation(new string_table),
  _assignments(new astring),
  _separators(new astring),
  _quotes(new astring),
  _nesting(false),
  _comments(new astring),
  _comment_number(1),
  _add_spaces(false)
{ *this = to_copy; }

variable_tokenizer::~variable_tokenizer()
{
  WHACK(_separators);
  WHACK(_assignments);
  WHACK(_implementation);
  WHACK(_quotes);
  WHACK(_comments);
}

int variable_tokenizer::symbols() const { return _implementation->symbols(); }

void variable_tokenizer::set_comment_chars(const astring &comments)
{ *_comments = comments; }

const astring &variable_tokenizer::assignments() const { return *_assignments; }

const astring &variable_tokenizer::separators() const { return *_separators; }

const astring &variable_tokenizer::quotes() const { return *_quotes; }

bool variable_tokenizer::exists(const astring &name) const
{ return !!_implementation->find(name); }

void variable_tokenizer::reset() { _implementation->reset(); }

const string_table &variable_tokenizer::table() const { return *_implementation; }

string_table &variable_tokenizer::table() { return *_implementation; }

variable_tokenizer &variable_tokenizer::operator =(const variable_tokenizer &to_copy)
{
  if (this == &to_copy) return *this;
  *_implementation = *to_copy._implementation;
  *_separators = *to_copy._separators;
  *_assignments = *to_copy._assignments;
  *_quotes = *to_copy._quotes;
  _nesting = to_copy._nesting;
  _add_spaces = to_copy._add_spaces;
  return *this;
}

astring variable_tokenizer::find(const astring &name) const
{
  astring *found = _implementation->find(name);
  if (!found) return "";

  // check that the contents are not just our significator of emptiness.
  if (found->equal_to(SPECIAL_VALUE)) return "";
  return *found;
}

bool variable_tokenizer::okay_for_variable_name(char to_check) const
{
  if (!to_check || separator(to_check) || assignment(to_check)) return false;
  return true;
}

bool variable_tokenizer::separator(char to_check) const
{
  // special case allows a CR separator to be either flavor.
  if (parser_bits::is_eol(to_check)
      && (astring::matches(*_separators, '\n')
           || astring::matches(*_separators, '\r')) ) return true;
  return astring::matches(*_separators, to_check);
}

bool variable_tokenizer::assignment(char to_check) const
{ return astring::matches(*_assignments, to_check); }

bool variable_tokenizer::quote_mark(char to_check) const
{ return astring::matches(*_quotes, to_check); }

bool variable_tokenizer::comment_char(char to_check) const
{ return astring::matches(*_comments, to_check); }

#define COOL to_tokenize.length()
  // true if the string should continue to be parsed.

// sets "current" to the first character in the string.
#define CHOP { \
  current = to_tokenize[0]; \
  to_tokenize.zap(0, 0); \
}

bool variable_tokenizer::parse(const astring &to_tokenize_in)
{
  FUNCDEF("parse");
  astring to_tokenize(to_tokenize_in);  // de-const.
//hmmm: do we need a copy?  try scooting based on a current pos.

  astring name, value;  // accumulated during the loop.
  char current;  // the most recent character from to_tokenize.
  bool just_ate_blank_line = false;
    // records when we handle a blank line as a comment.

  // loop over the string.
  while (COOL) {
    name.reset();
    value.reset();

    // pre-processing to remove extra eols and white space in front.
    if (is_eol_a_separator() && parser_bits::is_eol(to_tokenize[0])) {
      CHOP;
      // chop any white space but don't eat any non-white space coming up.
      while (COOL && parser_bits::white_space(current)) {
        CHOP;
        if (!parser_bits::white_space(current)) {
          // oops; we ate something we shouldn't have, since it will be
          // chopped when we get in the main loop.
          to_tokenize.insert(0, astring(current, 1));
        }
      }
    }

    // chop the first character off for analysis.
    CHOP;

    // ignore any white space until we hit a variable or other good stuff.
    if (parser_bits::white_space_no_cr(current))
      continue;

    // ignore eol unless they are in separator list.
    bool handle_as_comment = false;
    if (parser_bits::is_eol(current) && !is_eol_a_separator()) {
      continue;
    } else if (just_ate_blank_line && parser_bits::is_eol(current)) {
      just_ate_blank_line = false;
      continue;
    } else if (parser_bits::is_eol(current) && is_eol_a_separator()) {
//LOG("found eol and it's a separator here");
      handle_as_comment = true;
    }

    if (comment_char(current) || handle_as_comment) {
      // set our flag since we are going to eat the end of line in any case.
      just_ate_blank_line = true;
      // seek all text until next separator.
      while (COOL && !separator(current)) {
        value += current;
        CHOP;
      }
      // add the item with our ongoing comment number.
      a_sprintf name("%s%d", STRTAB_COMMENT_PREFIX, _comment_number);
      _implementation->add(name, value);
      _comment_number++;  // go to next comment number to keep unique.
LOG(astring("got comment: ") + name + " -> " + value);
      continue;  // got our chunk, keep going.
    }

    just_ate_blank_line = false;  // reset our flag.

    // skip characters we can't use for a variable name.
    if (!okay_for_variable_name(current)) continue;

    // we've found the start of a variable.
    while (COOL && okay_for_variable_name(current)) {
      // accumulate the variable name.
      name += current;
      CHOP;  // get the next character.
    }
    if (!COOL) {
      // we're at the end of the line, so deal with this situation.
      if (!separator(current) && !parser_bits::white_space(current) )
        name += current;  // get the character from the end of the line.
LOG(astring("last add: ") + name + " -> " + value);
      _implementation->add(name, value);  // store what we built.
      continue;  // skip the rest; we're at the END of the line man.
    }

    // skip spaces after variable name.
    while (COOL && parser_bits::white_space_no_cr(current)) CHOP;

    bool found_assignment = false;  // assume there isn't one.
    if (assignment(current)) {
      // we found the assignment operator and are starting on the value.
      CHOP;  // skip the assignment operator.
      found_assignment = true;
    }

    // skip spaces after the assignment statement.
    while (COOL && parser_bits::white_space_no_cr(current)) CHOP;

    // track the quoting that we have to deal with in parsing a value.
    stack<char> q_stack(!int(_nesting));
      // create an unbounded stack for nesting.

    while (COOL) {
      // check if the current character is a quote.
      bool ignore_separator = false;
      if (quote_mark(current)) {
        if (!q_stack.size()) {
          // nothing on the stack yet, so start accumulating.
          ignore_separator = true;
          q_stack.push(current);
        } else if (current == q_stack.top()) {
          // we got the end of this quoting.
          q_stack.pop();
          // check if we're done with any quotes.  if not, we still need to
          // ignore the separators.
          if (q_stack.size())
            ignore_separator = true;
        } else {
          // if we are using a bounded stack, it means we only support one
          // level of quoting at a time.  thus, this quote character simply
          // falls in as a regular character.  but if we're unbound, then
          // we can nest arbitrary levels of quotes.
          if (q_stack.kind() == stack<char>::UNBOUNDED)
            q_stack.push(current);
          // we have something on the stack already so we're still ignoring
          // separators.  we just don't care about this type of quote.
          ignore_separator = true;
        }
      } else if (q_stack.size()) {
        // it's not a quote but we're still trying to chow the matching
        // quote character.
        ignore_separator = true;
      }

      // look for the separator.
      if (!ignore_separator && separator(current)) {
        break;
      }

      // accumulate the value.
      value += current;
      CHOP;  // get the next character.
    }
    // get the last character if it's relevant.
    if (!separator(current) && !parser_bits::white_space(current) ) {
      value += current;
    }

    if (found_assignment && !value) {
      // use our special case for empty values, since there was an assignment
      // operator but no value afterwards.
      value = SPECIAL_VALUE;
    }

    // store the accumulated variable name and value, but only if the name
    // is non-empty.  otherwise, it's not much of a definition.
    if (name.t()) {
      // strip spaces at the end of the name.
      while (parser_bits::white_space_no_cr(name[name.end()]))
        name.zap(name.end(), name.end());
      // strip spaces at the end of the value unless it's the special case.
      if (!value.equal_to(SPECIAL_VALUE)) {
        while (parser_bits::white_space(value[value.end()]))
          value.zap(value.end(), value.end());
      }
LOG(astring("normal add: ") + name + " -> " + value);
      _implementation->add(name, value);  // store what we built.
      just_ate_blank_line = true;  // flag that we don't want next EOL.
      // reset, just in case.
      name.reset();
      value.reset();
    }
  }
  // currently we just kind of bully through whatever string is provided and do not
  // flag any error conditions.  but people do like to know if it worked or not.  they can
  // make their own conclusions if there are not enough variables defined for their needs.
  return true;
}

bool variable_tokenizer::is_eol_a_separator() const
{
  for (int i = 0; i < _separators->length(); i++) {
    char sep = _separators->get(i);
    // correct the separator for platform when it's the end of the line.
    if (parser_bits::is_eol(sep)) return true;
  }
  return false;
}

void variable_tokenizer::text_form(astring &accumulator) const
{
  accumulator.reset();
  bool added_sep = false;
  for (int i = 0; i < _implementation->symbols(); i++) {
    added_sep = false;
    if (!string_table::is_comment(_implementation->name(i))) {
      // a normal assignment is here.
      accumulator += _implementation->name(i);
      if (_implementation->operator [](i).t()) {
        if (_add_spaces) accumulator += " ";
        accumulator += _assignments->get(0);
        if (_add_spaces) accumulator += " ";
        accumulator += _implementation->operator [](i);
      }
    } else {
      // this one is a comment.  just spit out the value.
      if (_implementation->operator [](i).t())
        accumulator += _implementation->operator [](i);
    }
    // correct the separator for platform when it's the end of the line.
    if (is_eol_a_separator()) {
      accumulator += parser_bits::platform_eol_to_chars();
    } else {
      added_sep = true;  // record that we put a separator in there.
      accumulator += _separators->get(0);
      accumulator += ' ';
    }
  }
  // strip the final separator and space back off, if we added them.
  if (added_sep)
    accumulator.zap(accumulator.end() - 1, accumulator.end());
}

astring variable_tokenizer::text_form() const
{
  astring accumulator;
  text_form(accumulator);
  return accumulator;
}

} //namespace.

