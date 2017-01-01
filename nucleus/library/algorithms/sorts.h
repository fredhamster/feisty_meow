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

#include <stdio.h>
//temp!

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

	//! dumps the contents of the list out, assuming that the type can be turned into an int.
	template<class type>
	basis::astring dump_list(type v[], int size)
	{
		basis::astring ret;
		for (int i = 0; i < size; i++) {
			ret += basis::a_sprintf("%d ", (int)v[i]);
		}
		return ret;
	}

	//! shell sort algorithm.
	/*!
	 * Sorts a C array of the "type" with "n" elements.
	 * Operates on the original array.
	 * Performs in O(n log(n)) time.
	 * Algorithm is based on Kernighan and Ritchie's "The C Programming Language".
	 */
	template<class type>
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
						temp = v[j];
						v[j] = v[j + gap];
						v[j + gap] = temp;
					}
				} else {
					// reversed ordering.
					for (j = i - gap; j >= 0 && v[j] < v[j + gap]; j = j - gap) {
						// swap the elements that are disordered.
						temp = v[j];
						v[j] = v[j + gap];
						v[j + gap] = temp;
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
	template<class type>
	basis::array<type> merge(basis::array<type> &first, basis::array<type> &second,
	  bool reverse)
	{
		basis::array<type> to_return;
		// operate until we've consumed both of the arrays.
		while ((first.length() > 0) || (second.length() > 0)) {
			if (first.length() <= 0) {
				// nothing left in first, so use the second.
				to_return += second[0];
				second.zap(0, 0);
			} else if (second.length() <= 0) {
				to_return += first[0];
				first.zap(0, 0);
			} else if ( (!reverse && (first[0] <= second[0]))
					|| (reverse && (first[0] >= second[0]))) {
				// the first list has a better value to add next.
				to_return += first[0];
				first.zap(0, 0);
			} else {
				// the second list has a better value to add next.
				to_return += second[0];
				second.zap(0, 0);
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
	template<class type>
	basis::array<type> merge_sort(const basis::array<type> &v, bool reverse = false)
	{
		if (v.length() <= 1) {
			return basis::array<type>(v);
		}
		int midway = v.length() / 2;
		basis::array<type> firstPart = merge_sort(v.subarray(0, midway - 1), reverse);
		basis::array<type> secondPart = merge_sort(v.subarray(midway, v.length() - 1), reverse);
		return merge(firstPart, secondPart, reverse);
	}

//////////////

	/*!
	 * a heap is a structure that can quickly return the highest (or lowest) value,
	 * depending on how the priority of the item is defined.
	 * a "normal" heap keeps the highest element available first; a reverse sorted heap
	 * keeps the lowest element available first.
	 * restructuring the heap is fast, and is O(n log(n)).
	 * the implicit structure is a binary tree
	 * represented in a flat array, where the children of a node at position n are
	 * in positions n * 2 + 1 and n * 2 + 2 (zero based).
	 */
//hmmm: move this class out to basis?.
	template<class type>
	class heap
	{
	public:
		heap(type to_sort[], int n, bool reverse)
		{
			_total = n;
			_reverse = reverse;
			_heapspace = to_sort;
			heapify();
		}

		//! swaps the values in the heap stored at positions a and b.
		void swap_values(int a, int b)
		{
			type temp = _heapspace[a];
			_heapspace[a] = _heapspace[b];
			_heapspace[b] = temp;
		}

		//! get the index of the parent of the node at i.
		/*! this will not return the parent index of the root, since there is no parent. */
		int parent_index(int i)
		{
			return i / 2;  // rely on integer division to shave off remainder.
		}

		//! returns the left child of node at position i.
		int left_child(int i)
		{
			return 2 * i + 1;
		}

		//! returns the right child of node at position i.
		int right_child(int i)
		{
			return 2 * i + 2;
		}

		//! re-sorts the heapspace to maintain the heap ordering.
		void heapify()
		{
			int start = parent_index(_total - 1);
			// iterate from the back of the array towards the front, so depth-first.
			while (start >= 0) {
				// sift down the node at the index 'start' such that all nodes below it are heapified.
				sift_down(start, _total - 1);
				start--;  // move the start upwards towards the root.
			}
		}

		void sift_down(int start, int end)
		{
			int root = start;

			// while the current root still has a kid...
			while (left_child(root) <= end) {
				int child = left_child(root);
				// figure out which child to swap with.
				int swap = root;
				// check if the root should be swapped with this kid.
				if ((!_reverse && (_heapspace[swap] > _heapspace[child]))
				    || (_reverse && (_heapspace[swap] < _heapspace[child])))
				{
					swap = child;
				}
				// check if the other child should be swapped with the root or left kid.
				if ((child + 1 <= end)
				    && ((!_reverse && (_heapspace[swap] > _heapspace[child + 1]))
				        || (_reverse && (_heapspace[swap] < _heapspace[child + 1]))))
				{
					swap = child + 1;
				}
				if (swap == root) {
					// the root has the largest (or smallest) element, so we're done.
					return;
				} else {
					swap_values(root, swap);
					root = swap;
					// repeat to continue sifting down the child now.
				}
			}
		}

		//! re-sorts the heapspace to maintain the heap ordering.  this uses sift_up.
		void alt_heapify()
		{
			int end = 1;  // start at first child.

			while (end < _total) {
				// sift down the node at the index 'start' such that all nodes below it are heapified.
				sift_up(0, end++);
			}
		}

		//! start is how far up in the heap to sort.  end is the node to sift.
		void sift_up(int start, int end)
		{
			int child = end;
			// loop until we hit the starting node, where we're done.
			while (child > start) {
				int parent = parent_index(child);
				if ((!_reverse && (_heapspace[parent] < _heapspace[child]))
				    || (_reverse && (_heapspace[parent] > _heapspace[child])))
				{
					swap_values(parent, child);
					child = parent;
					// continue sifting at the parent now.
				} else {
					// done sorting.
					return;
				}
			}
		}

	private:
		bool _reverse = false;  // is the sorting in reverse?
		int _total = 0;
		int *_heapspace = NULL_POINTER;
	};

	/*!
	 * heap sort
	 *
	 * operates in O(n log(n)).
	 * sorts the original array.
	 */
	template<class type>
	void heap_sort(type v[], int n, bool reverse = false)
	{
		// reverse the sense of "reverse", since our algorithm expects a normal heap (with largest on top).
		heap<type> hap(v, n, !reverse);

		//temp
//		printf("hey after heaping: %s\n", dump_list(v, n).s());

		int end = n - 1;
		while (end > 0) {

//printf("moving value %d\n", (int)v[0]);
			// a[0] is the root and largest value for a normal heap. The swap moves it to the real end of the list and takes it out of consideration.
			hap.swap_values(end, 0);
			// reduce the heap size by 1.
			end--;
			// that swap ruined the heap property, so re-heapify.
			hap.sift_down(0, end);
		}
	}

//////////////

	template<class type>
	void partition(type v[], int start, int end)
	{

	}

//! the recursive version of quick sort that does the work for our convenience method.
	template<class type>
	void inner_quick_sort(type v[], int n, int start, int end, bool reverse = false)
	{
		if (start >= end) {
			// nothing to see here.
		} else {
			// figure out where to pivot, and sort both halves around the pivot index.
			int pivot = partition(v, start, end);
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
	template<class type>
	void quick_sort(type v[], int n, bool reverse = false)
	{
		inner_quick_sort(v, n, 0, n - 1, reverse);
	}

}  // namespace.

#endif // outer guard.

