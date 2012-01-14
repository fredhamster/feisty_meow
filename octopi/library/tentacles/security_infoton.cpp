/*****************************************************************************\
*                                                                             *
*  Name   : security_infoton                                                  *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "security_infoton.h"

#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/mutex.h>
#include <structures/string_array.h>
#include <structures/static_memory_gremlin.h>
#include <octopus/tentacle.h>

using namespace basis;
using namespace structures;
//using namespace textual;

namespace octopi {

security_infoton::security_infoton()
: infoton(security_classifier()),
  _mode(LI_LOGIN),
  _success(tentacle::NOT_FOUND),
  _verification(new byte_array)
{}

security_infoton::security_infoton(login_modes mode, const outcome &success,
    const byte_array &verification)
: infoton(security_classifier()),
  _mode(mode),
  _success(success),
  _verification(new byte_array(verification))
{}

security_infoton::security_infoton(const security_infoton &to_copy)
: root_object(),
  infoton(to_copy),
  _mode(to_copy._mode),
  _success(to_copy._success),
  _verification(new byte_array(*to_copy._verification))
{
}

security_infoton::~security_infoton()
{ WHACK(_verification); }

clonable *security_infoton::clone() const
{ return cloner<security_infoton>(*this); }

security_infoton &security_infoton::operator =(const security_infoton &to_copy)
{
  if (this == &to_copy) return *this;
  set_classifier(to_copy.classifier());
  _mode = to_copy._mode;
  _success = to_copy._success;
  *_verification = *to_copy._verification;
  return *this;
}

const byte_array &security_infoton::verification() const
{ return *_verification; }

byte_array &security_infoton::verification() { return *_verification; }

const astring login_classifier[] = { "#octsec" };

SAFE_STATIC_CONST(string_array, security_infoton::security_classifier,
    (1, login_classifier))

int security_infoton::packed_size() const
{
  return sizeof(_mode)
      + sizeof(int)  // packed outcome.
      + _verification->length() + sizeof(int);
}

void security_infoton::pack(byte_array &packed_form) const
{
  structures::attach(packed_form, int(_mode));
  attach(packed_form, _success.value());
  structures::attach(packed_form, *_verification);
}

bool security_infoton::unpack(byte_array &packed_form)
{
  int int_hold;
  if (!structures::detach(packed_form, int_hold)) return false;
  _mode = login_modes(int_hold);
  int value;
  if (!detach(packed_form, value)) return false;
  _success = outcome(value);
  if (!structures::detach(packed_form, *_verification)) return false;
  return true;
}

} //namespace.

