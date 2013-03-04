/*****************************************************************************\
*                                                                             *
*  Name   : xml_parser                                                        *
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

#include "xml_parser.h"

#include <basis/astring.h>
#include <structures/string_table.h>

using namespace basis;
using namespace structures;

namespace textual {

xml_parser::xml_parser(const astring &to_parse)
{
	_xml_stream = new astring;
  if (!to_parse) {}
}

xml_parser::~xml_parser()
{
}

const char *xml_parser::outcome_name(const outcome &to_name)
{
  return common::outcome_name(to_name);
}

void xml_parser::reset(const astring &to_parse)
{
  if (!to_parse) {}
}

outcome xml_parser::header_callback(astring &header_name,
    string_table &attributes)
{
  if (!header_name || !attributes.symbols()) {}
  return common::OKAY; 
}


outcome xml_parser::tag_open_callback(astring &tag_name,
    string_table &attributes)
{
  if (!tag_name || !attributes.symbols()) {}
  return OKAY;
}

outcome xml_parser::tag_close_callback(astring &tag_name)
{
  if (!tag_name) {}
  return OKAY;
}

outcome xml_parser::content_callback(astring &content)
{
  if (!content) {}
  return OKAY;
}

outcome xml_parser::parse()
{

//phases: we are initially always seeking a bracket bounded tag of some sort.

// the first few constructs must be headers, especially the xml header.

// is it true that headers are the only valid thing to see before real tags
//   start, or can there be raw content embedded in there?
// yes, it seems to be true in mozilla.  you can't have bare content in
// between the headers and the real tags.

// actually, if we allow the file to not start with the xml header and
//   version, then that's a bug.

// need function to accumulate the tag based on structure.  do headers
//   have to have a ? as part of the inner and outer structure?

// test against mozilla to ensure we are seeing the same things; get
//   together some tasty sample files.

// count lines and chars so we can report where it tanked.

// back to phases, (not a precise grammar below)
//   white_space ::= [ ' ' '\n' '\r' '\t' ] *
//   ws ::= white_space
//   text_phrase ::= not_white_space_nor_reserved not_reserved*
//   name ::= text_phrase
//   value ::= not_reserved *
//   lt_char ::= '<'
//   quote ::= '"'

//   xml_file ::= header+ ws tagged_unit+ ws
//   header ::= '<' '?' name ws attrib_list ws '?' '>' ws
//   tagged_unit ::= open_tag content* close_tag ws
//   content ::= [ tagged_unit | text_phrase ] + ws
//   open_tag ::= '<' name ws attrib_list ws '>' ws
//   attrib_list ::= ( attribute ws ) * ws
//   attribute ::= name ws '=' ws quoted_string ws
//   quoted_string ::= '"' not_lt_char_nor_quote '"' ws
//   close_tag :: '<' '/' name ws '>' ws 

//write a recursive descent parser on this grammar and spit out the 
//   productions as callbacks, at least for the major ones already listed.

return common::NOT_IMPLEMENTED;
}

/* callbacks to invoke.
outcome header_callback(astring &header_name, string_table &attributes)
outcome tag_open_callback(astring &tag_name, string_table &attributes)
outcome tag_close_callback(astring &tag_name)
outcome content_callback(astring &content)
*/

} //namespace.

