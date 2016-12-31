#ifndef ASSORTED_SORTS_GROUP
#define ASSORTED_SORTS_GROUP

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

/*
 * general considerations:
 *
 * + Generic objects to be sorted must support comparison operators.
 *
 * + If the "reverse" flag is true, the arrays will be sorted in reverse order.
 *   Reverse order here means "descending", such that array element i is always greater than or equal to array element i+1.
 *   Normal order is "ascending", such that element i is always less than or equal to array element i+1.
 *
 */

//! shell sort algorithm.
/*!
 * Sorts a C array of the "type" with "n" elements.
 * Operates on the original array.
 * Performs in O(n log(n)) time.
 * Algorithm is based on Kernighan and Ritchie's "The C Programming Language".
*/
template <class type>
void shell_sort(type v[], int n, bool reverse = false)
{
  type temp;
  int gap, i, j;
  /* the gap sizes decrease quadratically(?).  they partition the array of
   * items that need to be sorted into first two groups, then four, then
   * eight, etc.  the inner loop iterates across each gap's worth of the array.
	 */
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

//////////////

/*!
 * sorted array merge
 *
 * merges two sorted arrays into a single sorted array.
 */
template <class type>
basis::array<type> merge(const basis::array<type> &first, basis::array<type> &second, bool reverse)
{
	int first_iter = 0;
	int second_iter = 0;
	//hmmm: careful below; remember differences in heap allocated objects versus new-ed ones.
	//this might be really inefficient to return on stack..?
	basis::array<type> to_return;
	// operate until we've consumed both of the arrays.
	while ((first_iter < first.length()) && (second_iter < second.length())) {
		if ( (!reverse && (first[first_iter] <= second[second_iter]))
				|| (reverse && (first[first_iter] >= second[second_iter])) ) {
			// next item from first array goes into the merged array next.
			to_return += first[first_iter++];
		} else {
			// next item from second array goes into the merged array next.
			to_return += second[second_iter++];
		}
	}
	return to_return;
}

/*!
 * merge sort
 *
 * operates in O(n log(n)) time.
 * returns a new array with sorted data.
 */
template <class type>
basis::array<type> merge_sort(const basis::array<type> &v, bool reverse = false)
{
	if (v.length() <= 1) {
		return new basis::array<type>(v);
	}
	int midway = v.length() / 2;
	basis::array<type> firstPart = merge_sort(v.subarray(0, midway - 1));
	basis::array<type> secondPart = merge_sort(v.subarray(midway, v.length() - 1));
	return merge(firstPart, secondPart, reverse);
}

//////////////

/*
 * a heap is a structure that can quickly return the highest (or lowest) value,
 * depending on how the priority of the item is defined.  restructuring is
 * also fast, when new data are added.  the implicit structure is a binary tree
 * represented in a flat array, where the children of a node at position n are
 * in positions n * 2 + 1 and n * 2 + 2 (zero based).
 */
//hmmm: move this class out to basis?.
template <class type>
class heap
{
public:
	heap(int max_elements, bool reverse) {
		_max_elements = max_elements;
		_reverse = reverse;
		_heapspace = new basis::array<type> (_max_elements);
	}

	virtual ~heap() {
		WHACK(_heapspace);
	}

	//! swaps the values in the heap stored at positions a and b.
	void swap(int a, int b)
	{
		type temp = _heapspace[a];
		_heapspace[a] = _heapspace[b];
		_heapspace[b] = temp;
	}

	//! re-sorts the heapspace to maintain the heap ordering.
	void heapify()
	{

	}

  void add(type to_add) {
  	//
  }


private:
	int _max_elements;
	bool _reverse;
	basis::array<type> *_heapspace = NULL_POINTER;
};

/*!
 * heap sort
 *
 * operates in O(n log(n)).
 * sorts the original array.
 */
template <class type>
void heap_sort(type v[], int n, bool reverse = false)
{
	// use heap.  do sorty.
}

//////////////

template <class type>
void partition(type v[], int start, int end)
{

}

//! the recursive version of quick sort that does the work for our convenience method.
template <class type>
void inner_quick_sort(type v[], int n, int start, int end, bool reverse = false)
{
	if (start >= end) {
		// nothing to see here.
	} else {
		// figure out where to pivot, and sort both halves around the pivot index.
		int pivot = partition(v,	start, end);
		quicksort(v, start, pivot - 1);
		quicksort(v, pivot + 1, end);
	}
}

/*!
 * quick sort
 *
 * operates in O(n log(n)) time on average, worst case O(n^2).
 * sorts the original array.
 */
template <class type>
void quick_sort(type v[], int n, bool reverse = false)
{
	inner_quick_sort(v, n, 0, n-1, reverse);
}

} // namespace.

#endif // outer guard.

