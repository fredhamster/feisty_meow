#ifndef EOL_AWARE_CLASS
#define EOL_AWARE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : eol_aware
*  Author : Chris Koeritz
*                                                                             *
*******************************************************************************
* Copyright (c) 1996-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/definitions.h>
#include <textual/parser_bits.h>

namespace loggers {

//! Provides an abstract base for logging mechanisms.
/*!
  This assists greatly in generating diagnostic entries polymorphically and in
  enabling different logging mechanisms to be plugged in easily.  Note that all
  of the functions are defined virtually which enables overriding pretty much
  all of the base functionality.  Use this wisely, if ever.
*/ 

class eol_aware : public virtual basis::root_object
{
public:
  virtual textual::parser_bits::line_ending eol() { return c_ending; }
    //!< observes how line endings are to be printed.

  virtual void eol(textual::parser_bits::line_ending to_set) { c_ending = to_set; }
    //!< modifies how line endings are to be printed.

  virtual basis::astring get_ending() { return textual::parser_bits::eol_to_chars(c_ending); }
    //!< returns a string for the current ending.

  virtual void get_ending(basis::astring &to_end) { to_end = textual::parser_bits::eol_to_chars(c_ending); }
    //!< appends a string for the current ending to "to_end".

private:
  textual::parser_bits::line_ending c_ending;  //!< the current printing style.
};

} //namespace.

#endif

