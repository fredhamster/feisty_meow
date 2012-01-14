#ifndef AVERAGER_CLASS
#define AVERAGER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : averager                                                          *
*  Author : Chris Koeritz                                                     *
*                                                                             * 
*******************************************************************************
* Copyright (c) 1997-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/array.h>
#include <basis/functions.h>

namespace mathematics {

//! Maintains a list of numbers and provides the average for them.
/*!
  The list entries can be weighted if desired.  If the list grows too large,
  the first chunk of the entries is added as a weighted average and the list's
  size is reduced appropriately.
*/

template <class contents>
class averager
{
public:
  averager(int entries = 100, bool compacting = true);
    //!< creates an averager whose list length is restricted to "entries".
    /*!< if "entries" that is zero, then the maximum length is used.  if
    "compacting" is true, then the entries which need to be removed during
    a compaction will be added to the list as a weighted average.  if it's
    false, then the unfortunate entries are just eliminated.  the first style
    leads to a more steady state version of the average while the other is
    more recent history.  note that the lowest reasonable value for "entries"
    is eight due to the compaction algorithm; lower values will work, but the
    averager will allow the list to grow to eight anyway. */

  void add(contents value, int count = 1);
    //!< adds a new "value" to the averager, with an optional "count".

  contents average() const { return average(0, length() - 1); }
    //!< reports the overall average of the whole list.

  int samples() const;
    //!< returns the total number of samples recorded in the average.

  int length() const { return _averages.length(); }
    //!< returns the current length of the averages list.

  contents average(int start, int end) const;
    //!< reports the average over the range from "start" to "end" inclusive.

  struct weighted_entry { contents value; int count; };
    //!< structure holding a weighted chunk of the average.
    /*!< an entry in the list of averages has a "value" and a "count" that
    measures the weight with which that "value" is considered. */

  weighted_entry get(int index) const { return _averages.get(index); }
    //!< accesses the entry stored at the "index" specified.

  void compact();
    //!< chops off the oldest portion of the averager.
    /*!< if this is a compacting style averager, then the older data is
    coalesced and added as a weighted entry. */

  void check_for_compaction();
    //!< checks whether the averager needs to be compacted yet or not.
    /*!< the decision is made according to the maximum allowable size in
    "entries" passed to the constructor.  if "entries" is zero, the maximum
    allowable size is used instead. */

private:
  bool _do_compaction;  //!< do truncated values coalesce to a weighted entry?
  basis::array<weighted_entry> _averages;  //!< current list.
  int _entries;  //!< maximum length of list.
};

//////////////

//! keeps an average on a stream of integers.
class int_averager : public averager<int>
{
public:
  int_averager(int entries = 100, bool compacting = true)
          : averager<int>(entries, compacting) {}
};

//////////////

// implementations below...

const int AVERAGER_SIZE_LIMIT = 180000;  // the most items we'll try to handle.

template <class contents>
averager<contents>::averager(int entries, bool compacting)
: _do_compaction(compacting), _averages(), _entries(entries)
{
  int unit_size = sizeof(weighted_entry);
  if (basis::negative(_entries) || !_entries)
    _entries = int(AVERAGER_SIZE_LIMIT / unit_size);
}

template <class contents>
void averager<contents>::compact()
{
  if (length() < 8) return;  // sorry, you're too short for this wild ride.
  int end_whacking = _averages.length() / 4;
  if (_do_compaction) {
    contents whacked_average = average(0, end_whacking);
    _averages.zap(1, end_whacking);
    _averages[0].value = whacked_average;
    _averages[0].count = end_whacking + 1;
  } else _averages.zap(0, end_whacking);
}

template <class contents>
void averager<contents>::check_for_compaction()
{
  // make sure that we don't go over our allotted space.
  int unit_size = sizeof(weighted_entry);
  int limit = basis::minimum(AVERAGER_SIZE_LIMIT, _entries * unit_size);
  if (int(_averages.length() + 2) * unit_size >= limit) compact();
}

template <class contents>
void averager<contents>::add(contents value, int count)
{
  weighted_entry to_add;
  to_add.value = value;
  to_add.count = count;
  check_for_compaction();
  _averages += to_add;
}

template <class contents>
contents averager<contents>::average(int start, int end) const
{
  bounds_return(start, 0, length() - 1, 0);
  bounds_return(end, start, length() - 1, 0);
  bounds_return(end - start + 1, 1, length(), 0);

  contents accum = 0;
  contents count = 0;
  for (int i = start; i <= end; i++) {
    accum += get(i).value * get(i).count;
    count += get(i).count;
  }
  if (!count) count = 1;
  return accum / count;
}

template <class contents>
int averager<contents>::samples() const
{
  int to_return = 0;
  for (int i = 0; i < length(); i++) to_return += get(i).count;
  return to_return;
}

} //namespace.

#endif

