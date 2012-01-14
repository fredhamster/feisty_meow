#ifndef CROMP_TRANSACTION_CLASS
#define CROMP_TRANSACTION_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : cromp_transaction                                                 *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    The basic structure passed around the network for CROMP requests and     *
*  responses.                                                                 *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/contracts.h>
#include <octopus/infoton.h>
#include <octopus/entity_defs.h>

namespace cromp {

//////////////

class cromp_transaction
{
public:
  virtual ~cromp_transaction();
  DEFINE_CLASS_NAME("cromp_transaction");

  enum outcomes {
    OKAY = basis::common::OKAY,
    GARBAGE = basis::common::GARBAGE,
    PARTIAL = basis::common::PARTIAL,  // header is good but not all data is there yet.

    DEFINE_OUTCOME(WAY_TOO_SMALL, -41, "The package is too small for even "
        "a header"),
    DEFINE_OUTCOME(ILLEGAL_LENGTH, -42, "The package claims a length larger "
        "than we allow")
  };
  static const char *outcome_name(const basis::outcome &to_name);

  static void flatten(basis::byte_array &packed_form, const octopi::infoton &request,
          const octopi::octopus_request_id &id);
    // encapsulate the "request" with the "id" in a wire-friendly format in the
    // "packed_form".  this makes the infoton a bit more seaworthy out on the
    // network using a recognizable header.

  static bool unflatten(basis::byte_array &packed_form, basis::byte_array &still_flat,
          octopi::octopus_request_id &id);
    // re-inflates the infoton from the "packed_form" as far as retrieving
    // the original chunk of bytes in "still_flat".  the "id" is also unpacked.

  static int minimum_flat_size(const octopi::octopus_request_id &id);
  static int minimum_flat_size(const structures::string_array &classifier,
          const octopi::octopus_request_id &id);
    // returns the amount of packing overhead added by this class given the
    // request "id" that will be used.  the second method can include the
    // "classifier" also, in order to add in overhead from infoton::fast_pack.
    // neither of these methods considers the flat size of the associated
    // infoton.

  static bool resynchronize(basis::byte_array &packed_form);
    // chows down on the "packed_form" until we see a header or there's
    // nothing left in the array.  note that if there's a header already
    // present, this will stop immediately.  be sure to zap at least one
    // byte from the front if there was already a header present.

  static basis::outcome peek_header(const basis::byte_array &packed_form, int &length);
    // examines the data in "packed_form" and judges whether we think it's
    // got a valid transaction there yet or not.  the outcome returned is one
    // of the peek_outcomes.  if the outcome is OKAY or PARTIAL, then
    // the operation can be considered successful and the "length" is set to
    // the expected size of the "packed_form".  however, OKAY is the only
    // outcome denoting that the whole package is present.
};

} //namespace.

#endif

