#ifndef XML_PARSER_CLASS
#define XML_PARSER_CLASS

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

#include <basis/contracts.h>

// forward.
#include <basis/astring.h>
#include <structures/string_table.h>

namespace textual {

//! Parses XML input and invokes a callback for the different syntactic pieces.

// hmmm, could this be the first class ever named this?  perhaps it should be
// in a textual namespace.  -->after current sprint.

class xml_parser
{
public:
  xml_parser(const basis::astring &to_parse);
  virtual ~xml_parser();

  DEFINE_CLASS_NAME("xml_parser");

  //! the possible ways that operations here can complete.
  enum outcomes {
    OKAY = basis::common::OKAY
//uhhh...
  };

  static const char *outcome_name(const basis::outcome &to_name);
    //!< reports the string version of "to_name".

  void reset(const basis::astring &to_parse);
    //!< throws out any accumulated information and uses "to_parse" instead.

  basis::outcome parse();
    //!< starts the parsing process on the current string.
    /*!< this will cause callbacks to be invoked for each of the xml syntactic
    elements. */

  virtual basis::outcome header_callback(basis::astring &header_name,
          structures::string_table &attributes);
    //!< invoked when a well-formed xml header is seen in the input stream.
    /*!< the following applies to all of the callbacks: the derived method must
    return an outcome, which will be used by the parser.  if the outcome is
    OKAY, then parsing will continue.  any other outcome will cause parsing
    to stop and will become the return value of the parse() method. */

  virtual basis::outcome tag_open_callback(basis::astring &tag_name,
          structures::string_table &attributes);
    //!< an xml tag has been opened in the input stream.

  virtual basis::outcome tag_close_callback(basis::astring &tag_name);
    //!< an xml tag was closed in the input stream.

  virtual basis::outcome content_callback(basis::astring &content);
    //!< invoked when plain text content is found inside an opened tag.

private:
  basis::astring *_xml_stream;  // the stringful of xml information.
  
};

} //namespace.

#endif

