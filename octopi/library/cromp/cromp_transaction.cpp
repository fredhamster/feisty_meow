/*****************************************************************************\
*                                                                             *
*  Name   : cromp_transaction                                                 *
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

#include "cromp_transaction.h"

#include <basis/environment.h>
#include <basis/mutex.h>
#include <loggers/critical_events.h>
#include <loggers/file_logger.h>
#include <loggers/program_wide_logger.h>
#include <octopus/entity_defs.h>
#include <octopus/infoton.h>
#include <sockets/tcpip_stack.h>
#include <structures/static_memory_gremlin.h>
#include <textual/parser_bits.h>

#include <stdio.h>

using namespace basis;
using namespace loggers;
using namespace octopi;
using namespace sockets;
using namespace structures;
using namespace textual;

namespace cromp {

#define DEBUG_CROMP_TRANSACTION
  // uncomment for noisy version.

const int MAXIMUM_TRANSACTION = 100 * MEGABYTE;
  // the largest transaction we allow in cromp.  if more information needs
  // to be passed, then do it in chunks.

#undef LOG
#ifdef DEBUG_CROMP_TRANSACTION
  // since the transaction stuff is so low-level, we risk a feedback loop if
  // we log stuff when the program wide logger is itself a communication
  // object.
  #define LOG(s) CLASS_EMERGENCY_LOG(file_logger(environment::TMP() + "/cromp_transaction.log"), s)
#else
  #define LOG(s) 
#endif

SAFE_STATIC(mutex, __cromp_transaction_lock, )

cromp_transaction::~cromp_transaction()
{}

const char *cromp_transaction::outcome_name(const outcome &to_name)
{
  switch (to_name.value()) {
    case WAY_TOO_SMALL: return "WAY_TOO_SMALL";
    case ILLEGAL_LENGTH: return "ILLEGAL_LENGTH";
    default: return communication_commons::outcome_name(to_name);
  }
}

byte_array &cromp_name_array()
{
  static byte_array _hidden_cromp_array;
  static bool _initted = false;
  if (!_initted) {
    auto_synchronizer l(__cromp_transaction_lock());
    // check again in case someone scooped us.
    if (!_initted) {
      // add the special name field.
      attach(_hidden_cromp_array, abyte('c'));
      attach(_hidden_cromp_array, abyte('r'));
      attach(_hidden_cromp_array, abyte('o'));
      attach(_hidden_cromp_array, abyte('m'));
      attach(_hidden_cromp_array, abyte('p'));
      attach(_hidden_cromp_array, abyte('!'));
      // add the space for the length.
      for (int i = 0; i < 8; i++)
        attach(_hidden_cromp_array, abyte('?'));
      _initted = true;
    }
  }
  return _hidden_cromp_array;
}

int cromp_transaction::minimum_flat_size(const octopus_request_id &id)
{
  return cromp_name_array().length()  // cromp identifier in header.
      + id.packed_size();  // size of the request id.
}

int cromp_transaction::minimum_flat_size(const string_array &classifier,
    const octopus_request_id &id)
{
  return minimum_flat_size(id)
      + infoton::fast_pack_overhead(classifier);
          // size required for infoton::fast_pack.
}

void cromp_transaction::flatten(byte_array &packed_form,
    const infoton &request, const octopus_request_id &id)
{
#ifdef DEBUG_CROMP_TRANSACTION
  FUNCDEF("pack");
#endif
  int posn = packed_form.length();
    // save where we started adding.

  packed_form += cromp_name_array();
    // add the cromp prefix and space for the length.

  // add the identifier.
  id.pack(packed_form);

  // add the real data.
  infoton::fast_pack(packed_form, request);
#ifdef DEBUG_CROMP_TRANSACTION
  // make a copy of the packed infoton to compare.
  byte_array temp_holding;
  infoton::fast_pack(temp_holding, request);
#endif

//hmmm: check if too big!

  // backpatch the length now.
  a_sprintf len_string("%08x", packed_form.length() - posn);
#ifdef DEBUG_CROMP_TRANSACTION
  LOG(a_sprintf("len string is %s", len_string.s()));
#endif
  for (int j = 6; j < 14; j++)
    packed_form[posn + j] = abyte(len_string[j - 6]);

#ifdef DEBUG_CROMP_TRANSACTION
  byte_array copy = packed_form.subarray(posn, packed_form.last());
  byte_array tempo;
  octopus_request_id urfid;
  if (!cromp_transaction::unflatten(copy, tempo, urfid))
    continuable_error(static_class_name(), func,
        "failed to unpack what we just packed.");
  else if (urfid != id)
    continuable_error(static_class_name(), func, "wrong id after unpack.");
  else if (tempo != temp_holding)
    continuable_error(static_class_name(), func, "wrong data after unpack.");
#endif

}

bool cromp_transaction::unflatten(byte_array &packed_form,
    byte_array &still_flat, octopus_request_id &req_id)
{
#ifdef DEBUG_CROMP_TRANSACTION
  FUNCDEF("unflatten");
#endif
  still_flat.reset();
  int len = 0;
  // not ready yet.
  if (peek_header(packed_form, len) != OKAY) {
#ifdef DEBUG_CROMP_TRANSACTION
    LOG("failed to peek the header!");
#endif
    return false;
  }
  packed_form.zap(0, 14 - 1);
  if (!req_id.unpack(packed_form)) return false;
  int array_len = len - 14 - req_id.packed_size();

#ifdef DEBUG_CROMP_TRANSACTION
  if (array_len > packed_form.length())
    continuable_error(static_class_name(), func,
        "data needed is insufficient!  peek was wrong.");
#endif

  still_flat = packed_form.subarray(0, array_len - 1);
  packed_form.zap(0, array_len - 1);
  return true;
}

#define WHACK_AND_GO { packed_form.zap(0, 0); continue; }

#define CHECK_LENGTH \
  if (packed_form.length() < necessary_length) { \
    /* to this point, we are happy with the contents. */ \
    return true; \
  } \
  necessary_length++; /* require the next higher length. */

bool cromp_transaction::resynchronize(byte_array &packed_form)
{
#ifdef DEBUG_CROMP_TRANSACTION
  FUNCDEF("resynchronize");
#endif
  while (true) {
    if (!packed_form.length()) {
//#ifdef DEBUG_CROMP_TRANSACTION
      LOG("roasted entire contents...");
//#endif
      return false;
    }
    if (packed_form[0] != 'c') WHACK_AND_GO;
    int necessary_length = 2;
    CHECK_LENGTH;
    if (packed_form[1] != 'r') WHACK_AND_GO;
    CHECK_LENGTH;
    if (packed_form[2] != 'o') WHACK_AND_GO;
    CHECK_LENGTH;
    if (packed_form[3] != 'm') WHACK_AND_GO;
    CHECK_LENGTH;
    if (packed_form[4] != 'p') WHACK_AND_GO;
    CHECK_LENGTH;
    if (packed_form[5] != '!') WHACK_AND_GO;
    for (int k = 6; k < 14; k++) {
      CHECK_LENGTH;
      if (!parser_bits::is_hexadecimal(packed_form[k]))
        WHACK_AND_GO;
    }
#ifdef DEBUG_CROMP_TRANSACTION
    LOG("found header again...");
#endif
    return true;  // looks like we resynched.
  }
}

outcome cromp_transaction::peek_header(const byte_array &packed_form,
    int &length)
{
#ifdef DEBUG_CROMP_TRANSACTION
  FUNCDEF("peek_header");
#endif
  length = 0;
#ifdef DEBUG_CROMP_TRANSACTION
  LOG("checking for header");
#endif
  if (packed_form.length() < 14) return WAY_TOO_SMALL;
  if ( (packed_form[0] != 'c') || (packed_form[1] != 'r')
      || (packed_form[2] != 'o') || (packed_form[3] != 'm')
      || (packed_form[4] != 'p') || (packed_form[5] != '!') )
    return GARBAGE;
#ifdef DEBUG_CROMP_TRANSACTION
  LOG("obvious header bits look fine");
#endif

  astring len_string;
  for (int k = 6; k < 14; k++) {
    if (!parser_bits::is_hexadecimal(packed_form[k])) {
#ifdef DEBUG_CROMP_TRANSACTION
      LOG("found corruption in hex bytes");
#endif
      return GARBAGE;
    }
    len_string += char(packed_form[k]);
  }
#ifdef DEBUG_CROMP_TRANSACTION
  LOG("length was unpacked okay");
#endif
  basis::un_int temp_len = (basis::un_int)length;
  int items = sscanf(len_string.s(), "%08x", &temp_len);
  length = temp_len;
  if (!items) {
#ifdef DEBUG_CROMP_TRANSACTION
    LOG(astring("couldn't parse the len_string of: ") + len_string);
#endif
    return GARBAGE;
  }

#ifdef DEBUG_CROMP_TRANSACTION
  LOG(a_sprintf("length string is %s, len calc is %d and bytes "
      "given are %d", len_string.s(), length, packed_form.length()));
#endif
  if (length > MAXIMUM_TRANSACTION) return ILLEGAL_LENGTH;
  if (length > packed_form.length()) return PARTIAL;
  return OKAY;
}

} //namespace.

