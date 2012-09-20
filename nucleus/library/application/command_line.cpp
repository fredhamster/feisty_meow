/*****************************************************************************\
*                                                                             *
*  Name   : command_line                                                      *
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

#include "command_line.h"

#include <basis/functions.h>
#include <basis/astring.h>
#include <basis/mutex.h>
#include <configuration/application_configuration.h>
#include <filesystem/directory.h>
#include <filesystem/filename.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_array.h>
#include <textual/parser_bits.h>
#include <loggers/program_wide_logger.h>

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

using namespace basis;
using namespace configuration;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;

namespace application {

DEFINE_ARGC_AND_ARGV;

command_parameter::command_parameter(parameter_types type)
: _type(type), _text(new astring) {}

command_parameter::command_parameter(parameter_types type, const astring &text)
: _type(type), _text(new astring(text)) {}

command_parameter::command_parameter(const command_parameter &to_copy)
: _type(VALUE), _text(new astring)
{ *this = to_copy; }

command_parameter::~command_parameter() { WHACK(_text); }

const astring &command_parameter::text() const { return *_text; }

void command_parameter::text(const astring &new_text) { *_text = new_text; }

command_parameter &command_parameter::operator =
    (const command_parameter &to_copy)
{
  if (this == &to_copy) return *this;
  _type = to_copy._type;
  *_text = *to_copy._text;
  return *this;
}

//////////////

// option_prefixes: the list of valid prefixes for options on a command line.
// these are the characters that precede command line arguments.  For Unix,
// the default is a dash (-), while for DOS most programs use forward-slash
// (/).  Adding more characters is trivial; just add a character to the list
// before the sentinel of '\0'.
#if defined(_MSC_VER) || defined(__MINGW32__)
  static char option_prefixes[] = { '-', '/', '\0' };
#elif defined(__UNIX__)
  static char option_prefixes[] = { '-', '\0' };
#else
  #error "I don't know what kind of operating system this is."
#endif

bool it_is_a_prefix_char(char to_test)
{
  for (int i = 0; option_prefixes[i]; i++)
    if (to_test == option_prefixes[i]) return true;
  return false;
}

//////////////

class internal_cmd_line_array_of_parms : public array<command_parameter> {};

//////////////

SAFE_STATIC_CONST(command_parameter, command_line::cmdline_blank_parm, )
  // our default return for erroneous indices.

command_line::command_line(int argc, char *argv[])
: _implementation(new internal_cmd_line_array_of_parms),
  _program_name(new filename(directory::absolute_path(argv[0])))
{
  argv++;  // skip command name in argv.

  // loop over the rest of the fields and examine them.
  string_array string_list;  // accumulated below.
  while (--argc > 0) {
    astring to_store = argv[0];  // retrieve the current string.
    string_list += to_store;  // put the string in our list.
    argv++;  // next string.
  }
  parse_string_array(string_list);
}

command_line::command_line(const astring &full_line)
: _implementation(new internal_cmd_line_array_of_parms),
  _program_name(new filename)
{
  astring accumulator;
  string_array string_list;
  bool in_quote = false;
//hmmm: this is not quote right yet.
//      use the separate command line method, but get it to run iteratively
//      so we can keep pulling them apart?  maybe it already does!
//      separate is better because it handles escaped quotes.
  for (int i = 0; i < full_line.length(); i++) {
    char to_examine = full_line.get(i);
    if (to_examine == '"') {
      // it's a quote character, so maybe we can start eating spaces.
      if (!in_quote) {
        in_quote = true;
        continue;  // eat the quote character but change modes.
      }
      // nope, we're closing a quote.  we assume that the quotes are
      // around the whole argument.  that's the best win32 can do at least.
      in_quote = false;
      to_examine = ' ';  // trick parser into logging the accumulated string.
      // intentional fall-through to space case.
    }

    if (parser_bits::white_space(to_examine)) {
      // if this is a white space, then we start a new string.
      if (!in_quote && accumulator.t()) {
        // only grab the accumulator if there are some contents.
        string_list += accumulator;
        accumulator = "";
      } else if (in_quote) {
        // we're stuffing the spaces into the string since we're quoted.
        accumulator += to_examine;
      }
    } else {
      // not white space, so save it in the accumulator.
      accumulator += to_examine;
    }
  }
  if (accumulator.t()) string_list += accumulator;
    // that partial string wasn't snarfed during the loop.
  // grab the program name off the list so the parsing occurs as expected.
  *_program_name = directory::absolute_path(string_list[0]);
  string_list.zap(0, 0);
  parse_string_array(string_list);
}

command_line::~command_line()
{
  WHACK(_program_name);
  WHACK(_implementation);
}

int command_line::entries() const { return _implementation->length(); }

filename command_line::program_name() const { return *_program_name; }

const command_parameter &command_line::get(int field) const
{
  bounds_return(field, 0, entries() - 1, cmdline_blank_parm());
  return _implementation->get(field);
}

void command_line::separate_command_line(const astring &cmd_line,
    astring &app, astring &parms)
{
  char to_find = ' ';  // the command separator.
  if (cmd_line[0] == '\"') to_find = '\"';
    // if the first character is a quote, then we are seeing a quoted phrase
    // and need to look for its completing quote.  otherwise, we'll just look
    // for the next space.

  int seek_posn = 1;  // skip the first character.  we have accounted for it.
  // skim down the string, looking for the ending of the first phrase.
  while (seek_posn < cmd_line.length()) {
    // look for our parameter separator.  this will signify the end of the
    // first phrase / chunk.  if we don't find it, then it should just mean
    // there was only one item on the command line.
    int indy = cmd_line.find(to_find, seek_posn);
    if (negative(indy)) {
      // yep, there wasn't a matching separator, so we think this is just
      // one chunk--the app name.
      app = cmd_line;
      break;
    } else {
      // now that we know where our separator is, we need to find the right
      // two parts (app and parms) based on the separator character in use.
      if (to_find == '\"') {
        // we are looking for a quote character to complete the app name.
        if (cmd_line[indy - 1] == '\\') {
          // we have a backslash escaping this quote!  keep seeking.
          seek_posn = indy + 1;
          continue;
        }
        app = cmd_line.substring(0, indy);
        parms = cmd_line.substring(indy + 2, cmd_line.end());
          // skip the quote and the obligatory space character after it.
        break;
      } else {
        // simple space handling here; no escapes to worry about.
        app = cmd_line.substring(0, indy - 1);
        parms = cmd_line.substring(indy + 1, cmd_line.end());
        break;
      }
    }
  }
}

bool command_line::zap(int field)
{
  bounds_return(field, 0, entries() - 1, false);
  _implementation->zap(field, field);
  return true;
}

// makes a complaint about a failure and sets the hidden commands to have a
// bogus entry so they aren't queried again.
#define COMPLAIN_CMDS(s) \
  listo_cmds += "unknown"; \
  COMPLAIN(s)

string_array command_line::get_command_line()
{
  FUNCDEF("get_command_line");
  string_array listo_cmds;
  // the temporary string below can be given a flat formatting of the commands
  // and it will be popped out into a list of arguments.
  astring temporary;
#ifdef __UNIX__
  if (!_global_argc || !_global_argv) {
    // our global parameters have not been set, so we must calculate them.
    temporary = application_configuration::get_cmdline_from_proc();
  } else {
    // we have easy access to command line arguments supposedly, so use them.
    for (int i = 0; i < _global_argc; i++) {
      // add a string entry for each argument.
      listo_cmds += _global_argv[i];
    }
    // we don't need a long string to be parsed; the list is ready.
    return listo_cmds;
  }
#elif defined(__WIN32__)
  // we have easy access to the original list of commands.
  for (int i = 0; i < _global_argc; i++) {
    // add a string entry for each argument.
    listo_cmds += _global_argv[i];
  }
  return listo_cmds;
#else
  COMPLAIN_CMDS("this OS doesn't support getting the command line.");
  return listo_cmds;
#endif

  // now that we have our best guess at a flat representation of the command
  // line arguments, we'll chop it up.

//hmmm: this algorithm doesn't support spaces in filenames currently.
//hmmm: for windows, we can parse the quotes that should be around cmd name.
//hmmm: but for unix, the ps command doesn't support spaces either.  how to
//      get around that to support programs with spaces in the name?
  int posn = 0;
  int last_posn = -1;
  while (posn < temporary.length()) {
    posn = temporary.find(' ', posn);
    if (non_negative(posn)) {
      // found another space to turn into a portion of the command line.
      listo_cmds += temporary.substring(last_posn + 1, posn - 1);
        // grab the piece of string between the point just beyond where we
        // last saw a space and the position just before the space.
      last_posn = posn;  // save the last space position.
      posn++;  // push the pointer past the space.
    } else {
      // no more spaces in the string.  grab what we can from the last bit
      // of the string that we see.
      if (last_posn < temporary.length() - 1) {
        // there's something worthwhile grabbing after the last place we
        // saw a space.
        listo_cmds += temporary.substring(last_posn + 1,
            temporary.length() - 1);
      }
      break;  // we're done finding spaces.
    }
  }

  return listo_cmds;
}

astring command_line::text_form() const
{
  astring to_return;
  const astring EOL = parser_bits::platform_eol_to_chars();
  for (int i = 0; i < entries(); i++) {
    const command_parameter &curr = get(i);
    to_return += a_sprintf("%d: ", i + 1);
    switch (curr.type()) {
      case command_parameter::CHAR_FLAG:
        to_return += astring("<char flag> ") + curr.text() + EOL;
        break;
      case command_parameter::STRING_FLAG:
        to_return += astring("<string flag> ") + curr.text() + EOL;
        break;
      case command_parameter::VALUE:  // pass through to default.
      default:
        to_return += astring("<value> ") + curr.text() + EOL;
        break;
    }
  }
  return to_return;
}

bool command_line::find(char option_character, int &index,
    bool case_sense) const
{
  astring opt(option_character, 1);  // convert to a string once here.
  if (!case_sense) opt.to_lower();  // no case-sensitivity.
  for (int i = index; i < entries(); i++) {
//hmmm: optimize this too.
    if (get(i).type() == command_parameter::CHAR_FLAG) {
      bool success = (!case_sense && get(i).text().iequals(opt))
          || (case_sense && (get(i).text() == opt));
      if (success) {
        // the type is appropriate and the value is correct as well...
        index = i;
        return true;
      }
    }
  }
  return false;
}

bool command_line::find(const astring &option_string, int &index,
    bool case_sense) const
{
  FUNCDEF("find");
if (option_string.length() && (option_string[0] == '-') )
LOG(astring("found option string with dash!  string is: ") + option_string);

  for (int i = index; i < entries(); i++) {
    if (get(i).type() == command_parameter::STRING_FLAG) {
      bool success = (!case_sense && get(i).text().iequals(option_string))
          || (case_sense && (get(i).text() == option_string));
      if (success) {
        // the type is appropriate and the value is correct as well...
        index = i;
        return true;
      }
    }
  }
  return false;
}

bool command_line::get_value(char option_character, astring &value,
    bool case_sense) const
{
  value = "";
  int posn = 0;  // where we find the flag.
  if (!find(option_character, posn, case_sense)) return false;

  // get the value after the flag, if there is such.
  posn++;  // this is where we think our flag's value lives.
  if (posn >= entries()) return false;

  // there's still an entry after where we found our flag; grab it.
  command_parameter cp = get(posn);
  if (cp.type() != command_parameter::VALUE) return false;

  // finally; we've found an appropriate text value.
  value = cp.text();
  return true;
}

bool command_line::get_value(const astring &option_string, astring &value,
    bool case_sense) const
{
  FUNCDEF("get_value");
if (option_string.length() && (option_string[0] == '-') )
LOG(astring("found option string with dash!  string is: ") + option_string);

  value = "";
  int posn = 0;  // where we find the flag.
  if (!find(option_string, posn, case_sense)) return false;

  // get the value after the flag, if there is such.
  posn++;  // this is where we think our flag's value lives.
  if (posn >= entries()) return false;

  // there's still an entry after where we found our flag; grab it.
  command_parameter cp = get(posn);
  if (cp.type() != command_parameter::VALUE) return false;

  // finally; we've found an appropriate text value.
  value = cp.text();
  return true;
}

void command_line::parse_string_array(const string_array &to_parse)
{
  bool still_looking_for_flags = true;  // goes to false when only values left.
  // loop over the fields and examine them.
  for (int i = 0; i < to_parse.length(); i++) {
    // retrieve a character from the current string.
    int index = 0;
    char c = to_parse[i].get(index++);
    // we check whether it's a prefix character, and if so, what kind.
    if (still_looking_for_flags && it_is_a_prefix_char(c)) {
      // at least one prefix is there, so treat this as a flag.
      bool gnu_type_of_flag = false;
      if (it_is_a_prefix_char(to_parse[i].get(index))) {
        // there's a special GNU double flag beginner.
        index++;  // skip that extra one.
        if ( (index >= to_parse[i].length())
            || parser_bits::white_space(to_parse[i].get(index))) {
          // special case of '--' (or '//' i suppose) with white space or
          // nothing else afterwards; indicates that the rest of the items
          // should just be values, not flags.
          still_looking_for_flags = false;
          continue;  // we ate that item.
        }
        gnu_type_of_flag = true;
      }
      // everything after the prefixes is considered part of the flag; they're
      // either individual flag characters (on a single prefix) or they're the
      // full name for the flag (gnu style).
      c = 1;  // reset to a true bool value.
      astring gnu_accumulator;  // if processing a gnu flag, it arrives here.
      while (c) {
        if (!gnu_type_of_flag) {
          // add as many flag parameters as possible.
          c = to_parse[i].get(index++);
            // c will be zero once we hit the end of the string.
          if (c) {
            command_parameter to_add(command_parameter::CHAR_FLAG, astring(c, 1));
            *_implementation += to_add;
          }
        } else {
          // the gnu flag name is added to here.
          c = to_parse[i].get(index++);  // zero at end of string.
          if (c)
            gnu_accumulator += c;  // one more character.
        }
      }
      if (gnu_accumulator.t()) {
        // we've accumulated a gnu flag, so store it.
        command_parameter to_add(command_parameter::STRING_FLAG,
            gnu_accumulator);
        *_implementation += to_add;
      }
    } else {
      // add a value type of command_parameter.
      astring found = to_parse[i];
      command_parameter to_add(command_parameter::VALUE, found);
      *_implementation += to_add;
    }
  }
}

astring command_line::gather(int &index) const
{
  astring to_return;
  for (int i = index; i < entries(); i++) {
    if (get(i).type() == command_parameter::CHAR_FLAG) {
      index = i;
      return to_return;
    } else to_return += get(i).text();
  }
  index = entries() - 1;
  return to_return;
}

} //namespace.

