#ifndef STRING_TABLE_CLASS
#define STRING_TABLE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : string_table                                                      *
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

#include "symbol_table.h"

#include <basis/astring.h>
#include <basis/contracts.h>

namespace structures {

//! Provides a symbol_table that holds strings as the content.
/*! This is essentially a table of named strings. */

class string_table
: public symbol_table<basis::astring>,
  public virtual basis::packable,
  public virtual basis::hoople_standard
{
public:
  string_table(int estimated_elements = 100) : symbol_table<basis::astring>(estimated_elements),
        _add_spaces(false) {}
    //!< the "estimated_elements" specifies how many items to prepare to efficiently hold.
  string_table(const string_table &to_copy);
  virtual ~string_table();

  DEFINE_CLASS_NAME("string_table");

  string_table &operator = (const string_table &to_copy);

  bool operator ==(const string_table &to_compare) const;

  virtual bool equal_to(const equalizable &to_compare) const {
    const string_table *cast = dynamic_cast<const string_table *>(&to_compare);
    if (!cast) return false;
    return operator ==(*cast);
  }

  #define STRTAB_COMMENT_PREFIX "#comment#"
    //!< anything beginning with this is considered a comment.
    /*!< a numerical uniquifier should be appended to the string to ensure that
    multiple comments can be handled per table. */

  static bool is_comment(const basis::astring &to_check);

  basis::astring text_form() const;
    //!< prints the contents of the table into the returned string.
    /*!< if names in the table start with the comment prefix (see above), then
    they will not be printed as "X=Y" but instead as just "Y". */

  virtual void text_form(basis::base_string &fill) const { fill = text_form(); }

  // dictates whether the output will have spaces between the assignment
  // character and the key name and value.  default is to not add them.
  bool add_spaces() const { return _add_spaces; }
  void add_spaces(bool add_them) { _add_spaces = add_them; }

  virtual int packed_size() const;
  virtual void pack(basis::byte_array &packed_form) const;
  virtual bool unpack(basis::byte_array &packed_form);

private:
  bool _add_spaces;  // records whether we add spaces around the assignment.
};

} //namespace.

#endif

