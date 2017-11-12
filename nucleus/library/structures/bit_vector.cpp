/*****************************************************************************\
*                                                                             *
*  Name   : bit_vector                                                        *
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

#include "bit_vector.h"

#include <basis/byte_array.h>
#include <basis/definitions.h>
#include <basis/functions.h>
#include <basis/guards.h>

//#define DEBUG_BIT_VECTOR
  // uncomment this to get debugging noise.

#undef LOG
#ifdef DEBUG_BIT_VECTOR
  #define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)
#else
  #define LOG(s) {}
#endif

using namespace basis;

namespace structures {

bit_vector::bit_vector()
: _implementation(new byte_array(0, NULL_POINTER)), _number_of_bits(0)
{}

bit_vector::bit_vector(int number_of_bits, const abyte *initial)
: _implementation(new byte_array(0, NULL_POINTER)), _number_of_bits(0)
{
  reset(number_of_bits);
  if (!initial) return;
  _implementation->reset(number_of_packets(number_of_bits,
      int(BITS_PER_BYTE)), initial);
}

bit_vector::bit_vector(const bit_vector &to_copy)
: _implementation(new byte_array(*to_copy._implementation)),
  _number_of_bits(to_copy._number_of_bits)
{}

bit_vector::~bit_vector() { WHACK(_implementation); }

bit_vector &bit_vector::operator = (const bit_vector &to_copy)
{
  if (this == &to_copy) return *this;
  *_implementation = *to_copy._implementation;
  _number_of_bits = to_copy._number_of_bits;
  return *this;
}

bit_vector::operator const byte_array & () const { return *_implementation; }

int bit_vector::bits() const { return _number_of_bits; }

bool bit_vector::whole() const { return negative(find_first(0)); }

bool bit_vector::empty() const { return negative(find_first(1)); }

bool bit_vector::operator == (const bit_vector &that) const
{ return compare(that, 0, _number_of_bits); }

bool bit_vector::on(int position) const
{ return get_bit(into_two_dim(position)); }

bool bit_vector::off(int position) const { return !on(position); }

void bit_vector::resize(int number_of_bits)
{
//printf("resize invoked, old size %d, new size %d...\n", bits(), number_of_bits);
  if (negative(number_of_bits)) return;
  if (bits() == number_of_bits) return;
  int old_size = bits();
  _number_of_bits = number_of_bits;
  int number_of_bytes = number_of_packets(number_of_bits, int(BITS_PER_BYTE));
  _implementation->resize(number_of_bytes);
  // clear new space if the vector got larger.
  if (old_size < number_of_bits) {
    // lazy reset doesn't compute byte boundary, just does 8 bits.
    for (int i = old_size; i < old_size + 8; i++) {
      clear(i);
//printf("clearing bit %d.\n", i);
    }
    // clear the bytes remaining.
    int old_bytes = number_of_packets(old_size + 8, int(BITS_PER_BYTE));
    for (int j = old_bytes; j < number_of_bytes; j++) {
//printf("clearing bit %d through %d.\n", j * 8, j * 8 + 7);
      _implementation->use(j) = 0;
    }
  }
}

void bit_vector::reset(int number_of_bits)
{
  resize(number_of_bits);
  memset(_implementation->access(), 0, _implementation->length());
}

bit_vector::two_dim_location bit_vector::into_two_dim(int position) const
{
  two_dim_location to_return;
  to_return.c_byte = position / BITS_PER_BYTE;
  to_return.c_offset = position % BITS_PER_BYTE;
  return to_return;
}

bool bit_vector::get_bit(const two_dim_location &pos_in2) const
{
  bounds_return(pos_in2.c_byte * BITS_PER_BYTE + pos_in2.c_offset, 0,
    _number_of_bits - 1, false);
  abyte test_mask = abyte(1 << pos_in2.c_offset);
  return TEST(abyte(_implementation->get(pos_in2.c_byte)), test_mask);
}

void bit_vector::set_bit(int position, bool value)
{
  bounds_return(position, 0, bits() - 1, );
  set_bit(into_two_dim(position), value);
}

void bit_vector::set_bit(const two_dim_location &pos_in2, bool set_it)
{
  abyte test_mask = abyte(1 << pos_in2.c_offset);
  if (set_it) SET(_implementation->use(pos_in2.c_byte), test_mask);
  else CLEAR((abyte &)_implementation->get(pos_in2.c_byte), test_mask);
}

bool bit_vector::operator [](int position) const
{
  bounds_return(position, 0, _number_of_bits - 1, false);
  return get_bit(into_two_dim(position));
}

void bit_vector::light(int position)
{
  bounds_return(position, 0, _number_of_bits - 1, );
  set_bit(into_two_dim(position), true);
}

void bit_vector::clear(int position)
{
  bounds_return(position, 0, _number_of_bits - 1, );
  set_bit(into_two_dim(position), false);
}

int bit_vector::find_first(bool to_find) const
{
  const abyte whole_set = abyte(0xFF);
  // search through the whole bytes first.
  for (int full_byte = 0; full_byte < _implementation->length(); full_byte++) {
    if ( (to_find && _implementation->get(full_byte))
         || (!to_find && (_implementation->get(full_byte) != whole_set)) ) {
      // the first appropriate byte is searched for the first appropriate bit.
      for (int i = full_byte * BITS_PER_BYTE; i < minimum
          (int(_number_of_bits), (full_byte+1)*BITS_PER_BYTE); i++) {
        if (on(i) == to_find) return i;
      }
      return common::NOT_FOUND;
    }
  }
  return common::NOT_FOUND;
}

bool bit_vector::compare(const bit_vector &that, int start, int stop) const
{
  for (int i = start; i <= stop; i++) {
    if (on(i) != that.on(i)) return false;
  }
  return true;
}

astring bit_vector::text_form() const
{
  astring to_return;
  int bits_on_line = 0;
  const int estimated_elements_on_line = 64;
  for (int i = 0; i < _number_of_bits; i++) {
    // below prints out the bit number heading.
    if (bits_on_line == 0) {
      if (i != 0) to_return += "\n";
      if (i < 10000) to_return += " ";
      if (i < 1000) to_return += " ";
      if (i < 100) to_return += " ";
      if (i < 10) to_return += " ";
      to_return += a_sprintf("%d", i);
      to_return += " | ";
    }
    if (on(i)) to_return += "1";
    else to_return += "0";
    bits_on_line++;
    if (bits_on_line >= estimated_elements_on_line) bits_on_line = 0;
    else if ( !(bits_on_line % BITS_PER_BYTE) ) to_return += " ";
  }
  to_return += "\n";
  return to_return;
}

bit_vector bit_vector::subvector(int start, int end) const
{
  bounds_return(start, 0, bits() - 1, bit_vector());
  bounds_return(end, 0, bits() - 1, bit_vector());
  int size = end - start + 1;
  bit_vector to_return(size);
  for (int i = start; i <= end; i++) to_return.set_bit(i - start, on(i));
  return to_return;
}

bool bit_vector::overwrite(int start, const bit_vector &to_write)
{
  bounds_return(start, 0, bits() - 1, false);
  int end = start + to_write.bits() - 1;
  bounds_return(end, 0, bits() - 1, false);
  for (int i = start; i <= end; i++) set_bit(i, to_write[i - start]);
  return true;
}

enum endian { LEFT_ENDIAN, RIGHT_ENDIAN };
endian host_byte_order = LEFT_ENDIAN;  // bytes within words.
endian host_bit_order = LEFT_ENDIAN;  // bits within bytes.

// probably the treatment for right endian in either case
// of bytes or bits is wrong.

bool bit_vector::set(int start, int size, basis::un_int source)
{
  bounds_return(start, 0, bits() - 1, false);
  int end = start + size - 1;
  bounds_return(end, 0, bits() - 1, false);
  bounds_return(size, 1, 32, false);
  bit_vector from_int(32, (abyte *)&source);

// is this algorithm even remotely near the truth?
  if (host_bit_order == RIGHT_ENDIAN)
    from_int._implementation->resize(size, byte_array::NEW_AT_BEGINNING);
  else from_int.resize(size);  // left endian machine.
  overwrite(start, from_int);
  return true;
}

basis::un_int bit_vector::get(int start, int size) const
{
  int end = start + size - 1;
  bit_vector segment = subvector(start, end);
  // padding to bytes.
  int new_length = segment.bits();
  if (new_length % 8) {
    new_length = ( (new_length+8) / 8) * 8;
    LOG(a_sprintf("new size is %d.", new_length));
  }
  segment.resize(new_length);

  if (host_bit_order == RIGHT_ENDIAN) {
    bit_vector new_segment(segment.bits());
    for (int i = 0; i < segment.bits(); i += 8)
      for (int j = i; j < i + BITS_PER_BYTE; j++)
        if (j < segment.bits())
          new_segment.set_bit(i + (7 - (j - i)), segment[j]);
    segment = new_segment;  // copy the new form.
  }

  LOG("new seg after bit copy:");
  LOG(segment);

  basis::un_int to_return = 0;

  int bytes_used = number_of_packets(segment.bits(), int(BITS_PER_BYTE));
  // 4 = # of bytes in a int.
  for (int i = minimum(4, bytes_used) - 1; i >= 0; i--) {
#ifdef DEBUG_BIT_VECTOR
    bit_vector tmp(8, &segment._implementation->get(i));
    LOG(a_sprintf("%d: src bits %s", i, tmp.text_form().s()));
#endif

#ifdef DEBUG_BIT_VECTOR
    bit_vector tmp4(32, (abyte *)&to_return);
    LOG(a_sprintf("%d: pre shift dest %s", i, tmp4.text_form().s()));
#endif
    if (host_byte_order == LEFT_ENDIAN) to_return <<= 8;
    else to_return >>= 8;
#ifdef DEBUG_BIT_VECTOR
    bit_vector tmp5(32, (abyte *)&to_return);
    LOG(a_sprintf("%d: post shift dest %s", i, tmp5.text_form().s()));
#endif

    basis::un_int mask = segment._implementation->get(i);
    if (host_byte_order == RIGHT_ENDIAN) mask <<= 23;
#ifdef DEBUG_BIT_VECTOR
    bit_vector tmp3(32, (abyte *)&to_return);
    LOG(a_sprintf("%d: pre dest bits %s", i, tmp3.text_form().s()));
#endif
    SET(to_return, mask);
#ifdef DEBUG_BIT_VECTOR
    bit_vector tmp2(32, (abyte *)&to_return);
    LOG(a_sprintf("%d: post dest bits %s", i, tmp2.text_form().s()));
#endif
  }

#ifdef DEBUG_BIT_VECTOR
  bit_vector tmp(32, (abyte *)&to_return);
  LOG(a_sprintf("final bits %s", tmp.text_form().s()));
#endif
  return to_return;
}

} //namespace.
