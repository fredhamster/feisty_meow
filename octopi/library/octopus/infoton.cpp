/*****************************************************************************\
*                                                                             *
*  Name   : infoton                                                           *
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

#include "infoton.h"

#include <basis/functions.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <structures/string_array.h>
#include <textual/byte_formatter.h>

using namespace basis;
using namespace loggers;
using namespace structures;
using namespace textual;

namespace octopi {

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

//#define DEBUG_INFOTON
  // if uncommented, then extra checks are made.

const abyte FAST_PACK_VERSION = 0x14;
  // this single byte version number should be increased when the network
  // protocol changes.  it only ensures that the fast_pack method will reject
  // a lower version.

infoton::infoton(const string_array &classifier)
: _classifier(new string_array(classifier))
{
//  FUNCDEF("constructor [string_array]");
}

infoton::infoton(const astring &class_1)
: _classifier(new string_array)
{
//  FUNCDEF("constructor [one string]");
  *_classifier += class_1;
}

infoton::infoton(const astring &class_1, const astring &class_2)
: _classifier(new string_array)
{
//  FUNCDEF("constructor [two strings]");
  *_classifier += class_1;
  *_classifier += class_2;
}

infoton::infoton(const astring &class_1, const astring &class_2,
    const astring &class_3)
: _classifier(new string_array)
{
//  FUNCDEF("constructor [three strings]");
  *_classifier += class_1;
  *_classifier += class_2;
  *_classifier += class_3;
}

infoton::infoton(const infoton &to_copy)
: root_object(),
  packable(),
  clonable(),
  _classifier(new string_array(*to_copy._classifier))
{}

infoton::~infoton()
{ WHACK(_classifier); }

infoton &infoton::operator = (const infoton &to_copy)
{ *_classifier = *to_copy._classifier; return *this; }

const string_array &infoton::classifier() const
{ return *_classifier; }

bool infoton::check_classifier(const astring &classname, const astring &caller)
{
  bool to_return = true;
  if (!_classifier->length())
    to_return = false;
  for (int i = 0; i < _classifier->length(); i++) {
    if (!(*_classifier)[i].length())
      to_return = false;
  }
  if (!to_return) {
    program_wide_logger::get().log(classname + "::" + caller
        + ": invalid classifier provided.", ALWAYS_PRINT);
  }
  return to_return;
}

void infoton::set_classifier(const string_array &new_classifier)
{
#ifdef DEBUG_INFOTON
  FUNCDEF("set_classifier [string_array]");
#endif
  *_classifier = new_classifier;
#ifdef DEBUG_INFOTON
  check_classifier(class_name(), func);
#endif
}

void infoton::set_classifier(const astring &class_1)
{
#ifdef DEBUG_INFOTON
  FUNCDEF("set_classifier [1 string]");
#endif
  _classifier->reset();
  *_classifier += class_1;
#ifdef DEBUG_INFOTON
  check_classifier(class_name(), func);
#endif
}

void infoton::set_classifier(const astring &class_1, const astring &class_2)
{
#ifdef DEBUG_INFOTON
  FUNCDEF("set_classifier [2 strings]");
#endif
  _classifier->reset();
  *_classifier += class_1;
  *_classifier += class_2;
#ifdef DEBUG_INFOTON
  check_classifier(class_name(), func);
#endif
}

void infoton::set_classifier(const astring &class_1, const astring &class_2,
    const astring &class_3)
{
#ifdef DEBUG_INFOTON
  FUNCDEF("set_classifier [3 strings]");
#endif
  _classifier->reset();
  *_classifier += class_1;
  *_classifier += class_2;
  *_classifier += class_3;
#ifdef DEBUG_INFOTON
  check_classifier(class_name(), func);
#endif
}

int infoton::fast_pack_overhead(const string_array &classifier)
{
  return classifier.packed_size()  // for classifier.
      + sizeof(int)  // for the package size.
      + 1;  // for the version byte.
}

void infoton::fast_pack(byte_array &packed_form, const infoton &to_pack)
{
//  FUNCDEF("fast_pack");
  structures::attach(packed_form, FAST_PACK_VERSION);
    // add the tasty version byte as the very first item.
  structures::pack_array(packed_form, to_pack.classifier());
  // must first put the packed infoton into a byte array, then use the
  // byte array's packing support.
  int len_prior = packed_form.length();
  structures::attach(packed_form, int(0));
    // save space for length.
//hmmm: this could use obscure_pack for more reliability.
  to_pack.pack(packed_form);
  int added_len = packed_form.length() - sizeof(int) - len_prior;

  // shift in the length in the place where we made space.
  basis::un_int temp = basis::un_int(added_len);
  for (basis::un_int i = 0; i < sizeof(int); i++) {
    packed_form[len_prior + i] = abyte(temp % 0x100);
    temp >>= 8;
  }
}

bool infoton::test_fast_unpack(const byte_array &packed_form,
    int &packed_length)
{
//  FUNCDEF("test_fast_unpack");
  packed_length = 0;
  if (!packed_form.length()) return false;

  // make sure we have the right version number, first.
  if (packed_form[0] != FAST_PACK_VERSION)
    return false;

  un_int strings_held = 0;
  byte_array len_bytes = packed_form.subarray(1, 2 * sizeof(int));
  if (!structures::obscure_detach(len_bytes, strings_held) || !strings_held) {
    return false;
  }

  // check through all of the strings.
  const void *zero_posn = packed_form.observe() + sizeof(int) * 2 + 1;
  for (int i = 0; i < (int)strings_held; i++) {
    // locate the zero termination if possible.
    int index = int((abyte *)zero_posn - packed_form.observe());
    zero_posn = memchr(packed_form.observe() + index, '\0',
        packed_form.length() - index);
    // make sure we could find the zero termination.
    if (!zero_posn) {
      // nope, never saw a zero.  good thing we checked.
      return false;
    }
  }

  // base our expected position for the data length on the position of the
  // last string we found.
  int datalen_start = int((abyte *)zero_posn - packed_form.observe()) + 1;
  byte_array just_len = packed_form.subarray(datalen_start,
      datalen_start + sizeof(int) - 1);
  if (!structures::detach(just_len, packed_length)) return false;
  packed_length += datalen_start + sizeof(int);
    // include the classifier length and integer package length.
  return true;
}

bool infoton::fast_unpack(byte_array &packed_form, string_array &classifier,
    byte_array &info)
{
  FUNCDEF("fast_unpack");
  classifier.reset();
  info.reset();
  abyte version_checking = 0;
  if (!structures::detach(packed_form, version_checking)) return false;
  if (version_checking != FAST_PACK_VERSION) return false;
  if (!structures::unpack_array(packed_form, classifier)) return false;
  int len = 0;
  if (!structures::detach(packed_form, len)) return false;
  if (len > packed_form.length()) {
    // not enough data.
    continuable_error(static_class_name(), func, "failed to have enough data!");
    return false;
  }
  info = packed_form.subarray(0, len - 1);
  packed_form.zap(0, len - 1);
  return true;
}

} //namespace.

