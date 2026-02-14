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

#include <cromp/cromp_transaction.h>
#include <geometric/screen_rectangle.h>
#include <mathematics/chaos.h>
#include <octopus/infoton.h>
#include <octopus/tentacle_helper.h>
#include <structures/string_array.h>

using namespace geometric;

class bubble : public octopi::infoton
{
public:
  bubble(int data_segment_size = 0, const screen_rectangle &boundaries
      = screen_rectangle(), int color = 0)
    // constructs a bubble within the "boundaries" that has "color" and a data
    // segment size specified by "data_segment_size".  the color definitions
    // reside elsewhere.
  : infoton(bubble_classing()), _data()
  { reset(data_segment_size, boundaries, color); }

  const structures::string_array &bubble_classing() {
    static basis::astring bubbs[2] = { "bubble", "rubble" };
    static structures::string_array barray(2, bubbs);
    return barray;
  }

  void reset(int data_segment_size, const screen_rectangle &boundaries,
      int color) {
    _color = color;
    _bounds = boundaries;
    _data.reset(data_segment_size);
  }

  int data_length() const { return _data.length(); }

  clonable *clone() const { return octopi::cloner<bubble>(*this); }

  basis::byte_array &data() { return _data; }

  int non_data_overhead() const { return packed_size() - _data.length(); }

  virtual void text_form(basis::base_string &to_show) const {
    to_show.assign(basis::a_sprintf("classing=%s, seg size=%d, color=%d, bounds=%s", 
        bubble_classing().text_form().s(), _data.length(), _bounds.text_form().s(), _color));
  }

  virtual void pack(basis::byte_array &packed_form) const {
    structures::attach(packed_form, _color);
    _bounds.pack(packed_form);
    structures::attach(packed_form, _data);
  }

  int packed_size() const {
    return _data.length() + 2 * sizeof(int)       // packed byte array.
        + sizeof(int)                         // packed color.
        + 4 * sizeof(int);                    // packed screen rectangle.
  }

  virtual bool unpack(basis::byte_array &packed_form) {
    if (!structures::detach(packed_form, _color)) return false;
    if (!_bounds.unpack(packed_form)) return false;
    if (!structures::detach(packed_form, _data)) return false;
    return true;
  }

private:
  screen_rectangle _bounds;
  int _color;
  basis::byte_array _data;
};

//////////////

class bubbles_tentacle : public octopi::tentacle_helper<bubble>
{
public:
  bubbles_tentacle(bool backgrounded)
      : octopi::tentacle_helper<bubble>(bubble().classifier(), backgrounded)
  {}
};

#endif

