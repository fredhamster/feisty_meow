#ifndef BASE_ADDRESS_CLASS
#define BASE_ADDRESS_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : base_address                                                      *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Provides a way to describe an endpoint for communication.                *
*                                                                             *
*******************************************************************************
* Copyright (c) 1995-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/contracts.h>

namespace sockets {

// forward.
class machine_uid;

class base_address
: public virtual basis::packable,
  public virtual basis::nameable
{
public:
  // the packable and nameable responsibilities are forwarded to the derived classes.

  virtual bool same_host(const base_address &to_compare) const = 0;
  virtual bool same_port(const base_address &to_compare) const = 0;
    // returns true if the host or port are identical.

  virtual bool shareable(const base_address &to_compare) const = 0;
    // returns true if the two transports can be shared.

  virtual basis::astring text_form() const = 0;
    // returns a readable string representing the address.

  // these flip the address into a string and back.  this is different from
  // text_form() because the reversal must be possible.
  virtual basis::astring tokenize() const = 0;
  virtual bool detokenize(const basis::astring &info) = 0;

  virtual machine_uid convert() const = 0;
    // returns the address in the uniquifying format.

  virtual base_address *create_copy() const = 0;
    // creates a new address that's a copy of this one.  note that this
    // allocates memory that must be deleted by the caller.
};

//////////////

// these macros assist in tokenizing and detokenizing addresses.

//const char *TOKEN_SEPARATOR();
//const char *TOKEN_ASSIGN();
//const char *TOKEN_SEPARATOR() { return ","; }
//const char *TOKEN_ASSIGN() { return "="; }

// begins operation of a variable_tokenizer for loading.  LOADER_EXIT must be called
// after finishing with the variable_tokenizer.
#define LOADER_ENTRY \
  variable_tokenizer addr_parser; \
  addr_parser.parse(info)

#define LOADER_EXIT 
  // currently no implementation.

// locates a variable in the variable_tokenizer.
#define FIND(name, value) astring value = addr_parser.find(name)

// locates a variable like FIND, but returns if it couldn't find it.
#define GRAB(name, value) FIND(name, value); if (!value) return false

// begins operation of a variable_tokenizer for storing.  remember to call STORER_EXIT
// when finished.
#define STORER_ENTRY \
  variable_tokenizer addr_parser

#define STORER_EXIT 
  // currently no implementation.

// adds a new entry into the variable_tokenizer.
#define ADD(name, value) addr_parser.table().add(name, value)

// returns the accumulated tokens in the storing variable_tokenizer.
#define DUMP_EXIT astring to_return = addr_parser.text_form(); \
  STORER_EXIT; \
  return to_return

} //namespace.

#endif

