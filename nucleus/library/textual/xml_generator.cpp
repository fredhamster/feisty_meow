/*****************************************************************************\
*                                                                             *
*  Name   : xml_generator                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2007-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "parser_bits.h"
#include "string_manipulation.h"
#include "xml_generator.h"

#include <basis/astring.h>
#include <structures/stack.h>
#include <structures/string_table.h>

using namespace basis;
using namespace structures;

namespace textual {

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s);

//////////////

class tag_info
{
public:
  astring _tag_name;
  string_table _attribs;

  tag_info() {}

  tag_info(const astring &tag_name, const string_table &attribs)
      : _tag_name(tag_name), _attribs(attribs) {}
};

//////////////

class tag_stack : public stack<tag_info>
{
public:
  tag_stack() : stack<tag_info>(0) {}
};

//////////////

xml_generator::xml_generator(int mods)
: _tags(new tag_stack),
  _accumulator(new astring),
  _human_read(mods & HUMAN_READABLE),
  _clean_chars(mods & CLEAN_ILLEGAL_CHARS),
  _indentation(2)
{
}

xml_generator::~xml_generator()
{
  WHACK(_tags);
  WHACK(_accumulator);
}

const char *xml_generator::outcome_name(const outcome &to_name)
{
  switch (to_name.value()) {
    case ERRONEOUS_TAG: return "ERRONEOUS_TAG";
    default: return common::outcome_name(to_name);
  }
}

void xml_generator::set_indentation(int to_indent)
{
  if (to_indent <= 0) to_indent = 1;
  _indentation = to_indent;
}

void xml_generator::reset()
{
  _accumulator->reset();
// we need a reset on stack.
  while (_tags->pop() == common::OKAY) {}
}

astring xml_generator::generate()
{
  astring to_return;
  generate(to_return);
  return to_return;
}

void xml_generator::generate(astring &generated)
{
  close_all_tags();
  generated = "<?xml version=\"1.0\"?>";  // first string is the version.
  if (_human_read) generated += parser_bits::platform_eol_to_chars();
  generated += *_accumulator;
}

outcome xml_generator::open_tag(const astring &tag_name)
{
  string_table junk;
  return open_tag(tag_name, junk);
}

outcome xml_generator::add_header(const astring &tag_name,
    const string_table &attributes)
{
  tag_info new_item(tag_name, attributes);
  print_open_tag(new_item, HEADER_TAG);
  return OKAY;
}

outcome xml_generator::open_tag(const astring &tag_name, const string_table &attributes)
{
  tag_info new_item(tag_name, attributes);
  print_open_tag(new_item);
  _tags->push(new_item);
  return OKAY;
}

outcome xml_generator::close_tag(const astring &tag_name)
{
  if (_tags->elements() < 1) return NOT_FOUND;
  // check to see that it's the right one to close.
  if (_tags->top()._tag_name != tag_name) return ERRONEOUS_TAG;
  print_close_tag(tag_name);
  _tags->pop();
  return OKAY;
}

void xml_generator::close_all_tags()
{
  while (_tags->elements()) {
    close_tag(_tags->top()._tag_name);
  }
}

outcome xml_generator::add_content(const astring &content)
{
  if (_human_read) {
    astring indentata = string_manipulation::indentation(_indentation);
    int num_indents = _tags->elements();
    for (int i = 0; i < num_indents; i++) 
     *_accumulator += indentata;
  }
  if (_clean_chars) 
    *_accumulator += clean_reserved(content);
  else
    *_accumulator += content;
  if (_human_read) *_accumulator += parser_bits::platform_eol_to_chars();
  return OKAY;
}

void xml_generator::print_open_tag(const tag_info &to_print, int type)
{
  bool is_header = false;
  if (type == HEADER_TAG) is_header = true;

  if (_human_read) {
//hmmm: starting to look like a nice macro for this stuff, param is num levs.
    astring indentata = string_manipulation::indentation(_indentation);
    int num_indents = _tags->elements();
    for (int i = 0; i < num_indents; i++) 
     *_accumulator += indentata;
  }

  if (is_header)
    *_accumulator += "<?";
  else
    *_accumulator += "<";
  if (_clean_chars)
    *_accumulator += clean_reserved(to_print._tag_name);
  else
    *_accumulator += to_print._tag_name;
  for (int i = 0; i < to_print._attribs.symbols(); i++) {
    astring name, content;
    to_print._attribs.retrieve(i, name, content);
    if (_clean_chars) {
      // flush out badness if we were told to.
      clean_reserved_mod(name, true);  // clean spaces.
      clean_reserved_mod(content);
    }
    *_accumulator += " ";
    *_accumulator += name;
    *_accumulator += "=\"";
    *_accumulator += content;
    *_accumulator += "\"";
  }
  if (is_header)
    *_accumulator += "?>";
  else
    *_accumulator += ">";
  if (_human_read) *_accumulator += parser_bits::platform_eol_to_chars();
}

void xml_generator::print_close_tag(const astring &tag_name)
{
  if (_human_read) {
    astring indentata = string_manipulation::indentation(_indentation);
    int num_indents = _tags->elements() - 1;
    for (int i = 0; i < num_indents; i++) 
     *_accumulator += indentata;
  }
  *_accumulator += "</";
  if (_clean_chars)
    *_accumulator += clean_reserved(tag_name);
  else
    *_accumulator += tag_name;
  *_accumulator += ">";
  if (_human_read) *_accumulator += parser_bits::platform_eol_to_chars();
}

#define PLUGIN_REPLACEMENT(posn, repl_string) { \
  to_modify.zap(posn, posn); \
  to_modify.insert(posn, repl_string); \
  posn += int(strlen(repl_string)) - 1; \
}

void xml_generator::clean_reserved_mod(astring &to_modify,
    bool replace_spaces)
{
//could this set live somewhere?
  const char *quot = "&quot;";
  const char *amp = "&amp;";
  const char *lt = "&lt;";
  const char *gt = "&gt;";
  const char *apos = "&apos;";
  const char *space = "_";
    // was going to use %20 but that still won't parse in an attribute name.

  for (int i = 0; i < to_modify.length(); i++) {
    switch (to_modify[i]) {
      case '"': PLUGIN_REPLACEMENT(i, quot); break;
      case '&': PLUGIN_REPLACEMENT(i, amp); break;
      case '<': PLUGIN_REPLACEMENT(i, lt); break;
      case '>': PLUGIN_REPLACEMENT(i, gt); break;
      case '\'': PLUGIN_REPLACEMENT(i, apos); break;
      case ' ': if (replace_spaces) PLUGIN_REPLACEMENT(i, space); break;
      default: continue;
    }
  }
}

astring xml_generator::clean_reserved(const astring &to_modify,
    bool replace_spaces)
{
  astring to_return = to_modify;
  clean_reserved_mod(to_return, replace_spaces);
  return to_return;
}

} //namespace.


