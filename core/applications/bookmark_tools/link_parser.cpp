/*****************************************************************************\
*                                                                             *
*  Name   : link_parser                                                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Processes html files and finds the links.  A database in the HOOPLE      *
*  link format is created from the links found.                               *
*                                                                             *
*******************************************************************************
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

// Notes:
//
// the standard link structure in html is similar to this:
//     <a href="blahblah">Link Name and Launching Point</a>
//
// the standard we adopt for section titles is that it must be a heading
// marker.  that formatting looks like this, for example:
//     <h3 assorted_stuff>The Section Title:</h3>

#include "bookmark_tree.h"

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <filesystem/byte_filer.h>
#include <filesystem/filename.h>
#include <loggers/critical_events.h>
#include <loggers/file_logger.h>
#include <structures/stack.h>
#include <structures/static_memory_gremlin.h>
#include <textual/parser_bits.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;

#undef BASE_LOG
#define BASE_LOG(s) program_wide_logger::get().log(s, ALWAYS_PRINT)
#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

//#define DEBUG_LINK_PARSER
  // uncomment for noisier run to seek problems.

////////////////////////////////////////////////////////////////////////////

const int MAX_FILE_SIZE = 4 * MEGABYTE;
  // this is the largest html file size we will process.

////////////////////////////////////////////////////////////////////////////

// a macro that increments the position in the string and restarts the loop.
#define INCREM_N_GO { curr_index++; continue; }

// puts the current character on the intermediate string.
#define ADD_INTERMEDIATE \
  intermediate_text += full_contents[curr_index]

// returns a character in lower-case, if 'a' is in upper case.
char normalize_char(char a)
{
  if ( (a >= 'A') && (a <= 'Z') ) return a + 'a' - 'A';
  return a;
}

// returns true if the two characters are the same, ignoring upper/lower case.
bool caseless_equals(char a, char b) { return normalize_char(a) == normalize_char(b); }

// a macro that skips all characters until the specified one is seen.
#define JUMP_TO_CHAR(to_find, save_them) { \
  while ( (curr_index < full_contents.length()) \
      && !caseless_equals(to_find, full_contents[curr_index]) ) { \
    if (save_them) ADD_INTERMEDIATE; \
    curr_index++; \
  } \
}

// increments the state, the current character and restarts the loop.
#define NEXT_STATE_INCREM { \
  state = parsing_states(state+1);  /* move forward in states. */ \
  curr_index++; \
  continue; \
}

// cleans out the disallowed characters in the string provided.
#define CLEAN_UP_NAUGHTY(s) { \
  while (s.replace("\n", " ")) {} \
  while (s.replace("\r", "")) {} \
  s.strip_spaces(); \
}

//was before the strip_spaces code above.
/*
  int indy = s.find("--"); \
  while (non_negative(indy)) { \
    s[indy] = ' ';  / * replace the first dash with a space. * / \
    for (int i = indy + 1; i < s.length(); i++) { \
      if (s[i] != '-') break; \
      s.zap(i, i); \
      i--; \
    } \
    indy = s.find("--"); \
  } \
  while (s.replace("  ", " ")) {} \
*/

// cleans up underscores in areas that are supposed to be english.
#define MAKE_MORE_ENGLISH(s) \
  s.replace_all('_', ' ')

void strain_out_html_codes(astring &to_edit)
{
  for (int i = 0; i < to_edit.length(); i++) {
    if (to_edit[i] != '<') continue;
    // found a left bracket.
    int indy = to_edit.find('>', i);
    if (negative(indy)) return;  // bail out, unexpected unmatched bracket.
    to_edit.zap(i, indy);
    i--;  // skip back to reconsider current place.
  }
}

// writes out the currently accumulated link info.
#define WRITE_LINK { \
  /* clean naughty characters out of the names. */ \
  CLEAN_UP_NAUGHTY(url_string); \
  CLEAN_UP_NAUGHTY(name_string); \
  if (url_string.ends(name_string)) { \
    /* handle the name being boring. replace with the intermediate text. */ \
    MAKE_MORE_ENGLISH(intermediate_text); \
    strain_out_html_codes(intermediate_text); \
    CLEAN_UP_NAUGHTY(intermediate_text); \
    if (intermediate_text.length()) \
      name_string = intermediate_text; \
  } \
  /* output a link in the HOOPLE format. */ \
  astring to_write = "\"L\",\""; \
  to_write += translate_web_chars(name_string); \
  to_write += "\",\""; \
  to_write += abbreviate_category(last_heading); \
  to_write += "\",\""; \
  to_write += translate_web_chars(url_string); \
  to_write += "\"\n"; \
  output_file.write(to_write); \
  _link_count++; \
}

// writes out the current section in the HOOPLE format.
// currently the parent category is set to Root.
#define WRITE_SECTION { \
  CLEAN_UP_NAUGHTY(last_heading);  /* clean the name. */ \
  /* output a category definition. */ \
  astring to_write = "\"C\",\""; \
  to_write += last_heading; \
  to_write += "\",\""; \
  to_write += abbreviate_category(last_parents.top()); \
  to_write += "\"\n"; \
  output_file.write(to_write); \
  _category_count++; \
}

// clears our accumulator strings.
#define RESET_STRINGS { \
  url_string = astring::empty_string(); \
  name_string = astring::empty_string(); \
  intermediate_text = astring::empty_string(); \
}

////////////////////////////////////////////////////////////////////////////

class link_parser : public application_shell
{
public:
  link_parser();
  DEFINE_CLASS_NAME("link_parser");
  virtual int execute();
  int print_instructions(const filename &program_name);

private:
  int _link_count;  // number of links.
  int _category_count;  // number of categories.

  astring url_string;  // the URL we've parsed.
  astring name_string;  // the name that we've parsed for the URL.
  astring last_heading;  // the last name that was set for a section.
  stack<astring> last_parents;  // the history of the parent names.
  astring intermediate_text;  // strings we saw before a link.

  astring heading_num;
    // this string form of a number tracks what kind of heading was started.

  astring abbreviate_category(const astring &simplify);
    // returns the inner category nickname if the category has one.

  astring translate_web_chars(const astring &vervoom);
    // translates a few web chars that are safe for csv back into their non-encoded form.
};

////////////////////////////////////////////////////////////////////////////

link_parser::link_parser()
: application_shell(),
  _link_count(0),
  _category_count(0),
  last_heading("Root"),
  last_parents()
{
  last_parents.push(last_heading);  // make sure we have at least one level.
}

int link_parser::print_instructions(const filename &program_name)
{
  a_sprintf to_show("%s:\n\
This program needs two filenames as command line parameters.  The -i flag\n\
is used to specify the input filename and the -o flag specifies the output\n\
file to be created.  The input file is expected to be an html file\n\
containing links to assorted web sites.  The links are gathered, along with\n\
descriptive text that happens to be near them, to create a link database in\n\
the HOOPLE link format and write it to the output file.  HOOPLE link format\n\
is basically a CSV file that defines the columns 1-4 for describing either\n\
link categories (which support hierarchies) or actual links (i.e., URLs of\n\
interest).  The links are written to a CSV file in the standard HOOPLE link\n\
The HOOPLE link format is documented here:\n\
    http://hoople.org/guides/link_database/format_manifesto.txt\n\
", program_name.basename().raw().s(), program_name.basename().raw().s());
  program_wide_logger::get().log(to_show, ALWAYS_PRINT);
  return 12;
}

astring link_parser::abbreviate_category(const astring &simplify)
{
  astring to_return;
  astring name_portion;
  bookmark_tree::break_name(simplify, name_portion, to_return);
  if (!to_return) return name_portion;
  return to_return;
}

astring link_parser::translate_web_chars(const astring &vervoom)
{
  astring to_return = vervoom;
  to_return.replace_all("&amp;", "&");
  to_return.replace_all("%7E", "~");
  return to_return;
}

int link_parser::execute()
{
  FUNCDEF("main");
  command_line cmds(_global_argc, _global_argv);  // process the command line parameters.
  astring input_filename;  // we'll store our bookmarks file's name here.
  astring output_filename;  // where the processed marks go.
  if (!cmds.get_value('i', input_filename, false))
    return print_instructions(cmds.program_name());
  if (!cmds.get_value('o', output_filename, false))
    return print_instructions(cmds.program_name());

  BASE_LOG(astring("input file: ") + input_filename);
  BASE_LOG(astring("output file: ") + output_filename);

  astring full_contents;
  byte_filer input_file(input_filename, "r");
  if (!input_file.good())
    non_continuable_error(class_name(), func, "the input file could not be opened");
  input_file.read(full_contents, MAX_FILE_SIZE);
  input_file.close();

  filename outname(output_filename);
  if (outname.exists()) {
    non_continuable_error(class_name(), func, astring("the output file ")
        + output_filename + " already exists.  It would be over-written if "
        "we continued.");
  }

  byte_filer output_file(output_filename, "w");
  if (!output_file.good())
    non_continuable_error(class_name(), func, "the output file could not be opened");

  enum parsing_states {
    // the states below are order dependent; do not change the ordering!
    SEEKING_LINK_START,  // looking for the beginning of an html link.
    SEEKING_HREF,  // finding the href portion of the link.
    GETTING_URL,  // chowing on the URL portion of the link.
    SEEKING_NAME,  // finding the closing bracket of the <a ...>.
    GETTING_NAME,  // chowing down on characters in the link's name.
    SEEKING_CLOSURE,  // looking for the </a> to end the link.
    // there is a discontinuity after SEEKING_CLOSURE, but then the following
    // states are also order dependent.
    SAW_TITLE_START,  // the beginning of a section heading was seen.
    GETTING_TITLE,  // grabbing characters in the title.
    // new text processing states.
    SAW_NESTING_INCREASE,  // a new nesting level has been achieved.
    SAW_NESTING_DECREASE,  // we exited from a former nesting level.
  };

  int curr_index = 0;
  parsing_states state = SEEKING_LINK_START;
  while (curr_index < full_contents.length()) {
    switch (state) {
      case SEEKING_LINK_START:
        // if we don't see a less-than, then it's not the start of html code,
        // so we'll ignore it for now.
        if (full_contents[curr_index] != '<') {
          ADD_INTERMEDIATE;
          INCREM_N_GO;
        }
        // found a left angle bracket, so now we need to decided where to go next for parsing
        // the html coming up.
        curr_index++;
        // see if this is a heading.  if so, we can snag the heading name.
        if (caseless_equals('h', full_contents[curr_index])) {
#ifdef DEBUG_LINK_PARSER
          LOG("into the '<h' case");
#endif
          // check that we're seeing a heading definition here.
          char next = full_contents[curr_index + 1];
          if ( (next >= '0') && (next <= '9') ) {
            // we found our proper character for starting a heading.  we need
            // to jump into that state now.  we'll leave the cursor at the
            // beginning of the number.
            state = SAW_TITLE_START;
            INCREM_N_GO;
          }
        }
        // check if they're telling us a new indentation level of the type we care about.
        if (caseless_equals('d', full_contents[curr_index])) {
#ifdef DEBUG_LINK_PARSER
          LOG("into the '<d' case");
#endif
          // see if they gave us a <dl> tag.
          char next = full_contents[curr_index + 1];
          if (caseless_equals(next, 'l')) {
#ifdef DEBUG_LINK_PARSER
            LOG("into the '<dl' case");
#endif
            state = SAW_NESTING_INCREASE;
            INCREM_N_GO;
          }
        }
        // see if we can find a close for a nesting level.
        if (caseless_equals('/', full_contents[curr_index])) {
#ifdef DEBUG_LINK_PARSER
          LOG("into the '</' case");
#endif
          // see if they gave us a <dl> tag.
          if ( caseless_equals(full_contents[curr_index + 1], 'd')
              && caseless_equals(full_contents[curr_index + 2], 'l') ) {
#ifdef DEBUG_LINK_PARSER
              LOG("into the '</dl' case");
#endif
            state = SAW_NESTING_DECREASE;
            INCREM_N_GO;
          }
        }
        // see if it's not a link, and abandon ship if it's not, since that's the last option
        // for html code that we parse.
        if (!caseless_equals('a', full_contents[curr_index])) {
#ifdef DEBUG_LINK_PARSER
          LOG("into the not an '<a' case");
#endif
          intermediate_text += '<';
          JUMP_TO_CHAR('>', true);
          continue; 
        }
#ifdef DEBUG_LINK_PARSER
        LOG("into the final case, the '<a' case");
#endif
        // found an a, but make sure that's the only character in the word.
        curr_index++;
        if (!parser_bits::white_space(full_contents[curr_index])) {
          intermediate_text += "<a";
          JUMP_TO_CHAR('>', true);
          continue; 
        }
        // this looks like an address so find the start of the href.
        NEXT_STATE_INCREM;
        break;
      case SAW_NESTING_INCREASE:
        last_parents.push(last_heading);
#ifdef DEBUG_LINK_PARSER
        LOG(a_sprintf("nesting inwards, new depth %d", last_parents.size()));
#endif
        JUMP_TO_CHAR('>', false);
        state = SEEKING_LINK_START;
        break;
      case SAW_NESTING_DECREASE:
        last_parents.pop();
#ifdef DEBUG_LINK_PARSER
        LOG(a_sprintf("nesting outwards, new depth %d", last_parents.size()));
#endif
        JUMP_TO_CHAR('>', false);
        state = SEEKING_LINK_START;
        break;
      case SEEKING_HREF:
        JUMP_TO_CHAR('h', false);  // find the next 'h' for "href".
        curr_index++;
        if (!caseless_equals('r', full_contents[curr_index])) continue;
        curr_index++;
        if (!caseless_equals('e', full_contents[curr_index])) continue;
        curr_index++;
        if (!caseless_equals('f', full_contents[curr_index])) continue;
        curr_index++;
        if (full_contents[curr_index] != '=') continue;
        curr_index++;
        if (full_contents[curr_index] != '"') continue;
        // whew, got through the word href and the assignment.  the rest
        // should all be part of the link.
        NEXT_STATE_INCREM;
        break;
      case GETTING_URL:
        // as long as we don't see the closure of the quoted string for the
        // href, then we can keep accumulating characters from it.
        if (full_contents[curr_index] == '"') NEXT_STATE_INCREM;
        url_string += full_contents[curr_index];
        INCREM_N_GO;  // keep chewing on it in this same state.
        break;
      case SEEKING_NAME:
        JUMP_TO_CHAR('>', false);  // find closing bracket.
        NEXT_STATE_INCREM;  // now start grabbing the name characters.
        break;
      case GETTING_NAME:
        // we have to stop grabbing name characters when we spy a new code
        // being started.
        if (full_contents[curr_index] == '<') {
          // if we see a closing command, then we assume it's the one we want.
          if (full_contents[curr_index + 1] == '/')
            NEXT_STATE_INCREM;
          // if we see html inside the name, we just throw it out.
          JUMP_TO_CHAR('>', false);
          curr_index++;
          continue;
        }
        name_string += full_contents[curr_index];
        INCREM_N_GO;  // keep chewing on it in this same state.
        break;
      case SEEKING_CLOSURE:
        JUMP_TO_CHAR('>', false);  // find the closure of the html code.
        // write the link out now.
        WRITE_LINK;
        // clean out our accumulated strings.
        RESET_STRINGS;
        state = SEEKING_LINK_START;
        INCREM_N_GO;
        break;
      case SAW_TITLE_START:
        heading_num = full_contents.substring(curr_index, curr_index);
        JUMP_TO_CHAR('>', false);
        NEXT_STATE_INCREM;  // start eating the name.
        break;
      case GETTING_TITLE: {
        int indy = full_contents.find('<', curr_index);
        if (negative(indy)) {
          state = SEEKING_LINK_START;  // too weird, go back to start.
          continue;
        }
        // push the last title if it differs from the top of the stack.
        last_heading = full_contents.substring(curr_index, indy - 1);
        WRITE_SECTION;
        JUMP_TO_CHAR('<', false);  // now find the start of the header closure.
        JUMP_TO_CHAR('>', false);  // now find the end of the header closure.
        RESET_STRINGS;
        state = SEEKING_LINK_START;  // successfully found section name.
        break;
      }
      default:
        non_continuable_error(class_name(), func, "entered erroneous state!");
    }
  }

  if (url_string.t()) WRITE_LINK;

  output_file.close();

  BASE_LOG(a_sprintf("wrote %d links in %d categories.", _link_count,
      _category_count));

  return 0;
}

////////////////////////////////////////////////////////////////////////////

HOOPLE_MAIN(link_parser, )

