/*****************************************************************************\
*                                                                             *
*  Name   : unhandled_request                                                 *
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

#include "unhandled_request.h"

using namespace basis;
using namespace structures;

namespace octopi {

unhandled_request::unhandled_request(const octopus_request_id &original_id,
    const string_array &original_classifier, const outcome &reason)
: infoton(the_classifier()),
  _original_id(original_id),
  _original_classifier(original_classifier),
  _reason(reason)
{}

clonable *unhandled_request::clone() const 
{ return new unhandled_request(_original_id, _original_classifier, _reason); }

int unhandled_request::packed_size() const
{
  return _original_id.packed_size() + _original_classifier.packed_size()
      + _reason.packed_size();
}

void unhandled_request::text_form(basis::base_string &fill) const
{
  fill.assign(astring("classifier=") + the_classifier().text_form()
      + " original_id=" + _original_id.text_form()
      + a_sprintf(" reason=%d", _reason.value()));
}

const char *initter[] = { "__Unhandled__", NULL_POINTER };

string_array unhandled_request::the_classifier()
{ return string_array(1, initter); }

void unhandled_request::pack(byte_array &packed_form) const 
{
  _original_id.pack(packed_form);
  _original_classifier.pack(packed_form);
  attach(packed_form, _reason.value());
}

bool unhandled_request::unpack(byte_array &packed_form) 
{
  if (!_original_id.unpack(packed_form)) return false;
  if (!_original_classifier.unpack(packed_form)) return false;
  int val;
  if (!detach(packed_form, val)) return false;
  _reason = outcome(val);
  return true;
}

} //namespace.

