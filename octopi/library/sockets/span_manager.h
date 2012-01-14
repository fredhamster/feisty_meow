#ifndef SPAN_MANAGER_CLASS
#define SPAN_MANAGER_CLASS

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

#include <structures/bit_vector.h>

namespace sockets {

//! Manages lists of numbers representing the completion of some activity.
/*!
  A number is either present or absent from the list.  The numbers are
  organized according to spans, where a span is simply a range of numbers,
  such as 33-238.  the span manager allows numbers to be added or removed
  from the list.  it also can create the full list of either the present or
  absent numbers.
*/

class span_manager
{
public:
  span_manager(int number_of_items);
    // keeps track of "number_of_items" worth of whatever the unit is.

  span_manager(const span_manager &to_copy);

  ~span_manager();

  span_manager &operator =(const span_manager &to_copy);

  void reset(int number_of_items);
    //!< sets up the span manager with a new configuration.

  bool update(const basis::int_array &new_spans);
    //!< updates the span information.
    /*!< the spans listed in the hold are added to the span manager.  if the
    spans are successfully added, then true is returned. */

  int received_sequence() const;
    //!< returns the highest chunk number at which all chunks are ready.
    /*!< or a negative number is returned if chunk 0 has still not been
    received yet. */

  int missing_sequence() const;
    //!< returns the number of the chunk where the first item is missing.
    /*!< if none of them are missing, then a negative number is returned. */

  void make_received_list(basis::int_array &spans, int max_spans = -1) const;
    //!< Creates a list for the received spans that are ready.
    /*!< The numbers in the list refer to ranges of spans fully received.  For
    example, if 0, 3, 4, 5, 8, 9, 12, 13, 14, and 28 have been received, the
    span list is [0-0], [3-5], [8-9], [12-14], and [28-28].  The "max_spans"
    is the longest the list is allowed to be, unless it is negative, which
    means include all of them. */

  void make_missing_list(basis::int_array &spans, int max_spans = -1) const;
    //!< creates a list representing the spans that are not ready yet.

  const structures::bit_vector &vector() const;
    //!< observes the held bit_vector that represents the spans.
  structures::bit_vector &vector();
    //!< provides access to the held bit_vector that represents the spans.

  basis::astring print_received_list() const;
    //!< prints out the span list for received blocks into a string.

  basis::astring print_missing_list() const;
    //!< prints out the span list for missing blocks into a string.

private:
  structures::bit_vector *_implementation;  //!< our underlying structure holding spans.

  basis::astring funky_print(const basis::int_array &to_spew, int rec_seq) const;
    //!< prints the span holder to a string and returns it.
};

} //namespace.

#endif

