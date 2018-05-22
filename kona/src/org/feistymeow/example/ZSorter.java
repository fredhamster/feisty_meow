package org.feistymeow.example;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public class ZSorter<T extends Comparable<? super T>> {

	/**
	 * returns a new list which is the sorted version of the input
	 * list "toSort".
	 */
	public List<T> sort(List<T> toSort) {
		List<T> toReturn = new ArrayList<T>(toSort);
		Collections.copy(toReturn, toSort);
		Collections.sort(toReturn);
		return toReturn;
	}

	/**
	 * simple console printer for list of T.
	 */
	public void print(List<T> toPrint) {
		boolean first = true;
		for (T curr: toPrint) {
			if (!first) System.out.print(", ");
			first = false;
			System.out.print(curr);
		}
		System.out.println();
	}
	
	public static void main(String[] args) {
		ZSorter<Integer> sorter = new ZSorter<Integer>();
		
		List<Integer> theList
			= new ArrayList<Integer>(Arrays.asList(101, 19, 86, 72, 56, 35, 47));
		System.out.println("prior to sorting:");
		sorter.print(theList);

		// test our sort method.
		List<Integer> newList = sorter.sort(theList);
		System.out.println("after sorting:");
		sorter.print(newList);
	}
}
