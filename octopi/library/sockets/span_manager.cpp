/*****************************************************************************\
*                                                                             *
*  Name   : span_manager                                                      *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1990-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "span_manager.h"

#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <structures/bit_vector.h>
#include <structures/set.h>

using namespace basis;
using namespace structures;
//using namespace basis;

namespace sockets {

span_manager::span_manager(int packs)
: _implementation(new bit_vector(packs))
{}

span_manager::span_manager(const span_manager &to_copy)
: _implementation(new bit_vector(*to_copy._implementation))
{}

span_manager::~span_manager() { WHACK(_implementation); }

span_manager &span_manager::operator =(const span_manager &to_copy)
{
  if (this == &to_copy) return *this;
  *_implementation = *to_copy._implementation;
  return *this;
}

int span_manager::missing_sequence() const
{ return _implementation->find_first(0); }

void span_manager::reset(int packs) { _implementation->resize(packs); }

const bit_vector &span_manager::vector() const { return *_implementation; }

bit_vector &span_manager::vector() { return *_implementation; }

int span_manager::received_sequence() const
{
  int recd_to = _implementation->find_first(0);
  if (negative(recd_to)) return _implementation->bits() - 1;
  return recd_to - 1;
}

void span_manager::make_received_list(int_array &to_make, int max_spans) const
{
  to_make.reset(0);
  int zeros_start_at = _implementation->find_first(0);
  if (negative(zeros_start_at)) {
    // all bits are set in the vector, so the whole message is received.
    to_make.concatenate(0);
    to_make.concatenate(short(_implementation->bits() - 1));
    return;
  }
  zeros_start_at--;
  // the sequence of ones ends right before the first zero
  if (zeros_start_at >= 0) {
    to_make.concatenate(0);
    to_make.concatenate(short(zeros_start_at));
  }

  int ones_to_here;  // keeps track of the position of the ones.

  for (int outer_loop = zeros_start_at + 2;
      outer_loop < _implementation->bits(); ) {
    // the first zero is zeros_start_at + 1, so we start at the first
    // unknown bit.
    if (_implementation->on(outer_loop)) {
      // the bit is a one, so we are in a one-gathering mode.
      ones_to_here = outer_loop;
      int inner_loop = outer_loop + 1;
      while (inner_loop < _implementation->bits()) {
        if (_implementation->on(inner_loop)) ones_to_here=inner_loop;
        else break;  // a zero is found at this position, so leave loop.
        inner_loop++;
      }
      // the stretch of ones is entered in the array.
      to_make.concatenate(short(outer_loop));
      to_make.concatenate(short(ones_to_here));
      if ( (max_spans >= 0) && (to_make.length() >= 2 * max_spans) )
        return;
      outer_loop = ones_to_here + 1;
    } else {
      // the bit is a zero, so we are gathering zeros.
      int inner_loop = outer_loop + 1;
      ones_to_here = _implementation->bits();
      while (inner_loop < _implementation->bits()) {
        if (!_implementation->on(inner_loop)) inner_loop++;
        else {
          // ones_to_here is set to the first position of a one, actually.
          ones_to_here = inner_loop;
          break;
        }
      }
      // the loop variable is set to the first unknown position again.
      outer_loop = ones_to_here;
    }
  }
}

bool span_manager::update(const int_array &new_spans)
{
  for (int i = 0; i < new_spans.length(); i += 2) {
    if ( (new_spans.get(i) >= _implementation->bits())
        || (new_spans.get(i+1) >= _implementation->bits()) )
      return false;
    for (int j = new_spans.get(i); j <= new_spans.get(i+1); j++)
      _implementation->light(j);
  }
  return true;
}

void span_manager::make_missing_list(int_array &to_make, int max_spans) const
{
  to_make.reset(0);
  int ones_start_at = _implementation->find_first(1);
  if (negative(ones_start_at)) {
    // all bits are zero in the vector; no packets have been received.
    to_make.concatenate(0);
    to_make.concatenate(short(_implementation->bits() - 1));
    return;
  }
  ones_start_at--;
  // the sequence of zeros ends right before the first one
  if (ones_start_at >= 0) {
    to_make.concatenate(0);
    to_make.concatenate(short(ones_start_at));
  }

  int zeros_to_here;
  for (int outer_loop = ones_start_at + 2;
      outer_loop < _implementation->bits(); ) {
    int inner_loop;
    // the first one is ones_start_at+1, so we start at the first unknown bit
    if (!_implementation->on(outer_loop)) {
      // the bit is a zero, so we are in a zero-gathering mode.
      zeros_to_here = outer_loop;
      int inner_loop = outer_loop + 1;
      while (inner_loop < _implementation->bits()) {
        if (!_implementation->on(inner_loop)) zeros_to_here=inner_loop;
        else break;
          // a one is found at this position, so leave loop.
        inner_loop++;
      }
      // the stretch of zeros is entered in the array.
      to_make.concatenate(short(outer_loop));
      to_make.concatenate(short(zeros_to_here));
      if ( (max_spans >= 0) && (to_make.length() >= 2 * max_spans) ) return;
      outer_loop = zeros_to_here + 1;
    } else {
      // the bit is a one, so we are gathering ones.
      inner_loop = outer_loop + 1;
      zeros_to_here = _implementation->bits();
      while (inner_loop < _implementation->bits()) {
        if (_implementation->on(inner_loop))
          inner_loop++;
        else {
          // zeros_to_here is set to the first position of a zero, actually.
          zeros_to_here = inner_loop;
          break;
        }
      }
      // the loop variable is set to the first unknown position again.
      outer_loop = zeros_to_here;
    }
  }
}

astring span_manager::funky_print(const int_array &to_spew, int rec_seq) const
{
  astring to_return(astring::SPRINTF, "through %d, [", rec_seq);
  for (int i = 0; i < to_spew.length(); i += 2) {
    to_return += astring(astring::SPRINTF, " %d-%d", to_spew.get(i),
        to_spew.get(i+1));
  }
  to_return += astring(" ] ");
  return to_return;
}

astring span_manager::print_received_list() const
{
  int_array hold_info;
  make_received_list(hold_info);
  astring to_return("received ");
  to_return += funky_print(hold_info, received_sequence());
  return to_return;
}

astring span_manager::print_missing_list() const
{
  int_array hold_info;
  make_missing_list(hold_info);
  astring to_return("missing ");
  to_return += funky_print(hold_info, received_sequence());
  return to_return;
}

} //namespace.

