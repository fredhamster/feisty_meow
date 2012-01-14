/*****************************************************************************\
*                                                                             *
*  Name   : encryption_wrapper                                                *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2004-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "encryption_wrapper.h"

#include <basis/byte_array.h>
#include <basis/mutex.h>
#include <loggers/program_wide_logger.h>
#include <structures/static_memory_gremlin.h>

using namespace basis;
using namespace loggers;
using namespace structures;

namespace octopi {

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

encryption_wrapper::encryption_wrapper(const byte_array &wrapped)
: infoton(encryption_classifier()),
  _wrapped(wrapped)
{}

encryption_wrapper::encryption_wrapper(const encryption_wrapper &to_copy)
: root_object(),
  infoton(to_copy),
  _wrapped(to_copy._wrapped)
{}

encryption_wrapper::~encryption_wrapper() {}

clonable *encryption_wrapper::clone() const
{ return cloner<encryption_wrapper>(*this); }

encryption_wrapper &encryption_wrapper::operator =
    (const encryption_wrapper &to_copy)
{
  if (this == &to_copy) return *this;
  _wrapped = to_copy._wrapped;
  return *this;
}

const char *wrap_encryption_classifier[] = { "#octrap" };

SAFE_STATIC_CONST(string_array, encryption_wrapper::encryption_classifier,
    (1, wrap_encryption_classifier))

int encryption_wrapper::packed_size() const
{
  return _wrapped.length() + sizeof(int);  // wrapped array size.
}

void encryption_wrapper::pack(byte_array &packed_form) const
{
  structures::attach(packed_form, _wrapped);
}

bool encryption_wrapper::unpack(byte_array &packed_form)
{
  if (!structures::detach(packed_form, _wrapped)) return false;
  return true;
}

//////////////

unwrapping_tentacle::unwrapping_tentacle()
: tentacle_helper<encryption_wrapper>
    (encryption_wrapper::encryption_classifier(), false)
{}

unwrapping_tentacle::~unwrapping_tentacle()
{}

outcome unwrapping_tentacle::reconstitute(const string_array &classifier,
    byte_array &packed_form, infoton * &reformed)
{
  if (classifier != encryption_wrapper::encryption_classifier())
    return NO_HANDLER;

  return reconstituter(classifier, packed_form, reformed,
      (encryption_wrapper *)NIL);
}

outcome unwrapping_tentacle::consume(infoton &formal(to_chow),
    const octopus_request_id &formal(item_id), byte_array &transformed)
{
  FUNCDEF("consume");
  transformed.reset();
  LOG("should never enter this method.");
  return common::NOT_IMPLEMENTED;
}

} //namespace.

