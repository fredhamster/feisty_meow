package org.feistymeow.algorithms;

import java.util.HashSet;

/**
 * example algorithms from google interview videos. these focus on finding a number as a sum within a list of numbers.
 */
class SumFinder
{
	/*
	 * spec notes:
	 * 
	 * the numbers are ints, both positive and negative.
	 * 
	 * don't worry about overflow for math ops involving the sum or list members.
	 * 
	 * the input list is not necessarily sorted.
	 * 
	 * the result is just a boolean of whether the requested sum was found or not. it would be easy enough to return a pair though, with the
	 * two numbers that added to the sum.
	 * 
	 * this solution assumes that the list fits in memory.
	 */
	boolean findSumInList(int sum, int list[])
	{
		HashSet<Integer> wanting = new HashSet<Integer>();
		for (int curr : list) {
			if (wanting.contains(curr)) {
				// we found a match for the complement we had stored earlier, so return true.
				return true;
			}
			wanting.add(sum - curr);
		}
		return false;
	}

	/*
	 * implement more general case also that can use any number of numbers in the set to sum?
	 *
	 * e.g. if the number itself equals the sum, fine and dandy. or if three numbers sum to it, that's also a match. this should return a set
	 * of all matches, where a match is a collection of ints that add to the sum.
	 */
	// returning from this other method...
	// return = new ArrayList<Integer>(Arrays.asList(curr, sum - curr));

	// test app for above methods...

	public static void main(String argv[])
	{
		SumFinder finder = new SumFinder();

		int list_1[] = { 7, 5, 8, 2, 9, 4, 1, 2 };
		int sum_1 = 8;

		if (!finder.findSumInList(sum_1, list_1)) {
			System.out.println("FAILURE: ON TEST CASE 1");
		} else {
			System.out.println("OKAY: ON TEST CASE 1");
		}

		//////////////

		int list_2[] = { 1, 9, 3, 2, 4, 4, 1 };
		int sum_2 = 8;

		if (!finder.findSumInList(sum_2, list_2)) {
			System.out.println("FAILURE: ON TEST CASE 2");
		} else {
			System.out.println("OKAY: ON TEST CASE 2");
		}

		//////////////

		int list_3[] = { 1, 9, 3, 2 };
		int sum_3 = 8;

		if (finder.findSumInList(sum_3, list_3)) {
			System.out.println("FAILURE: ON TEST CASE 3");
		} else {
			System.out.println("OKAY: ON TEST CASE 3");
		}
	}
}
