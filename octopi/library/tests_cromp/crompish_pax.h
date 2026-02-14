#ifndef CROMPISH_PAX_GROUP
#define CROMPISH_PAX_GROUP

/*****************************************************************************\
*                                                                             *
*  Name   : crompish packets for tester                                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Some simple transactions that can be used for the CROMP tester.          *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/chaos.h>
#include <basis/string_array.h>
#include <cromp/cromp_transaction.h>
#include <geometric/screen_rectangle.h>
#include <octopus/infoton.h>
#include <octopus/tentacle_helper.h>

using namespace geometric;

class bubble : public infoton
{
public:
  bubble(int data_segment_size = 0, const screen_rectangle &boundaries
      = screen_rectangle(), int color = 0)
    // constructs a bubble within the "boundaries" that has "color" and a data
    // segment size specified by "data_segment_size".  the color definitions
    // reside elsewhere.
  : infoton(bubble_classing()), _data()
  { reset(data_segment_size, boundaries, color); }

  const string_array &bubble_classing() {
    static istring bubbs[2] = { "bubble", "rubble" };
    static string_array barray(2, bubbs);
    return barray;
  }

  void reset(int data_segment_size, const screen_rectangle &boundaries,
      int color) {
    _color = color;
    _bounds = boundaries;
    _data.reset(data_segment_size);
  }

  int data_length() const { return _data.length(); }

  clonable *clone() const { return cloner<bubble>(*this); }

  byte_array &data() { return _data; }

  int non_data_overhead() const { return packed_size() - _data.length(); }

  virtual void pack(byte_array &packed_form) const {
    basis::attach(packed_form, _color);
    _bounds.pack(packed_form);
    basis::attach(packed_form, _data);
  }

  int packed_size() const {
    return _data.length() + 2 * sizeof(int)       // packed byte array.
        + sizeof(int)                         // packed color.
        + 4 * sizeof(int);                    // packed screen rectangle.
  }

  virtual bool unpack(byte_array &packed_form) {
    if (!basis::detach(packed_form, _color)) return false;
    if (!_bounds.unpack(packed_form)) return false;
    if (!basis::detach(packed_form, _data)) return false;
    return true;
  }

private:
  screen_rectangle _bounds;
  int _color;
  byte_array _data;
};

////////////////////////////////////////////////////////////////////////////

class bubbles_tentacle : public tentacle_helper<bubble>
{
public:
  bubbles_tentacle(bool backgrounded)
      : tentacle_helper<bubble>(bubble().classifier(), backgrounded)
  {}
};

#endif

