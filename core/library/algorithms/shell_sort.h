#ifndef SHELL_SORT_CLASS
#define SHELL_SORT_CLASS

//////////////
// Name   : shell_sort
// Author : Chris Koeritz
//////////////
// Copyright (c) 1991-$now By Author.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation:
//     http://www.gnu.org/licenses/gpl.html
// or under the terms of the GNU Library license:
//     http://www.gnu.org/licenses/lgpl.html
// at your preference.  Those licenses describe your legal rights to this
// software, and no other rights or warranties apply.
// Please send updates for this code to: fred@gruntose.com -- Thanks, fred.
//////////////

namespace algorithms {

//! Orders an array in O(n log n) time.
/*!
  This algorithm is based on Kernighan and Ritchie's "The C Programming Language".
*/

template <class type>
void shell_sort(type v[], int n, bool reverse = false)
  //!< this function sorts a C array of the "type" with "n" elements.
  /*!< the "type" of object must support comparison operators.  if "reverse"
  is true, then the list is sorted highest to lowest. */
{
  type temp;
  int gap, i, j;
  // the gap sizes decrease quadratically(?).  they partition the array of
  // items that need to be sorted into first two groups, then four, then
  // eight, ....
  // within each gap's worth of the array, the next loop takes effect...
  for (gap = n / 2; gap > 0; gap /= 2) {
    // the i indexed loop is the base for where the comparisons are made in
    // the j indexed loop.  it makes sure that each item past the edge of
    // the gap sized partition gets considered.
    for (i = gap; i < n; i++) {
      // the j indexed loop looks at the values in our current gap and ensures
      // that they are in sorted order.
      if (!reverse) {
        // normal ordering.
        for (j = i - gap; j >= 0 && v[j] > v[j + gap]; j = j - gap) {
          // swap the elements that are disordered.
          temp = v[j]; v[j] = v[j + gap]; v[j + gap] = temp;
        }
      } else {
        // reversed ordering.
        for (j = i - gap; j >= 0 && v[j] < v[j + gap]; j = j - gap) {
          // swap the elements that are disordered.
          temp = v[j]; v[j] = v[j + gap]; v[j + gap] = temp;
        }
      }
    }
  }
}

} // namespace.

#endif // outer guard.

