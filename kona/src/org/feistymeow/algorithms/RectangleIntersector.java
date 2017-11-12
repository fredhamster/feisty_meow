package org.feistymeow.algorithms;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Vector;

import org.feistymeow.algorithms.RectangleIntersector.SortedElement;
import org.feistymeow.algorithms.RectangleIntersector.SortedElementComparator;

/**
 * reports if any two rectangles in a list intersect. uses screen coordinates.
 */
public class RectangleIntersector
{
	public RectangleIntersector()
	{
		
	}
	
	public static class Point
	{
		Point(double x, double y)
		{
			this.x = x;
			this.y = y;
		}

		double x, y;
	}

	public static class Rectangle
	{
		Rectangle(Point ul, Point lr)
		{
			this.ul = ul;
			this.lr = lr;
		}

		Point ul, lr;
	}

	public static boolean doesPointOverlap(Point p, Rectangle r)
	{
		return p.x <= r.lr.x && p.x >= r.ul.x && p.y <= r.lr.y && p.y >= r.ul.y;
	}

	public static boolean doRectanglesOverlap(Rectangle r1, Rectangle r2)
	{
		return doesPointOverlap(r1.ul, r2) || doesPointOverlap(r1.lr, r2) || doesPointOverlap(r2.ul, r1) || doesPointOverlap(r2.lr, r1);
	}

	/**
	 * find any overlapping pair of rectangles in the list provided.
	 */
	public static Vector<Rectangle> findOverlapBruteForce(Vector<Rectangle> list)
	{
		// terrible brute force algorithm below.
		for (int i = 0; i < list.size(); i++) {
			for (int j = i; j < list.size(); j++) {
				if (doRectanglesOverlap(list.get(i), list.get(j))) {
					ArrayList<Rectangle> toReturn = new ArrayList<Rectangle>();
					toReturn.add(list.get(i));
					toReturn.add(list.get(j));
				}
			}
		}
		return null;
	}

	public static class SortedElement
	{
		double value; // the key.
		boolean lowerEdge; // is this the left side?
		Rectangle source; // where did this value come from.

		public SortedElement(double value, boolean lowerEdge, Rectangle source)
		{
			this.value = value;
			this.lowerEdge = lowerEdge;
			this.source = source;
		}
	}

	public static class SortedElementComparator implements Comparator<SortedElement>
	{
		@Override
		public int compare(SortedElement k1, SortedElement k2)
		{
			SortedElementKey key1 = new SortedElementKey(k1.value);
			return key1.compareTo(new SortedElementKey(k2.value));
		}
		
		Double value;
	}

	public static class SortedElementKey implements Comparable<SortedElementKey>
	{
		public SortedElementKey(double key)
		{
			this.key = key;
		}
		
		@Override
		public int compareTo(SortedElementKey k2)
		{ 
			return (key == k2.key) ? 0 : (key < k2.key) ? -1 : 1;
		}
		
		double key;
	}
	
	/**
	 * find any overlapping pair of rectangles in the list provided.
	 * 
	 * this is classed as a good answer... if it works.
	 */
	public static Vector<Rectangle> findOverlap(Vector<Rectangle> list)
	{
		// first phase, produce a sorted list of the x coordinates of the rectangles.
		// this completes in O(n log(n)) time.
		ArrayList<SortedElement> xEdges = new ArrayList<SortedElement>();
		for (Rectangle r : list) {
			xEdges.add(new SortedElement(r.lr.x, true, r));
			xEdges.add(new SortedElement(r.ul.x, false, r));
		}
		// we're assuming this is an efficient sort; i've heard rumors that it's a heapsort.
		// if so we're good. if not, we'd write our own heapsort (see feisty meow nucleus sorts.h).
		xEdges.sort(new SortedElementComparator());

		// second phase, crawl across the sorted list and build up a binary search tree
		// with the y coordinates from the rectangles. we will use this to check for
		// intersections.
		BinarySearchTree<SortedElementKey, SortedElement> bst = new BinarySearchTree<SortedElementKey, SortedElement>();

		for (SortedElement scanner : xEdges) {
			Rectangle source = scanner.source;
			if (scanner.lowerEdge) {
				// for x, this is the left edge.
				// search for compatible top and bottom.
				
//that means what?  a value that is less than or equal the top and gte the bottom?
//we can traverse the tree manually, looking for any  in the range we want, but that means
				//digging in and changing the bst to allow us to single step downwards, or some such.
				
				//hmmm: POSTPONE.  i want to do more things on the board, and this problem's gone WAY overtime.
//hmmm: need to fix this implementation; it is bustard.				
				
				
				// we want to add the two y components of the rectangle to our tree.				
				bst.insert(new SortedElementKey(source.lr.y), new SortedElement(source.lr.y, true, source));
				bst.insert(new SortedElementKey(source.ul.y), new SortedElement(source.ul.y, false, source));
			} else {
				// for x, this is the right edge.
				// we will remove from the bst the two values for our top and bottom y.
				bst.delete(new SortedElementKey(source.lr.y));
				bst.delete(new SortedElementKey(source.ul.y));
				
				
			}
			
//what is missing?			
			
			
		}
		
		
		return null;
	}

	public static void main(String[] argv)
	{
		Rectangle r1 = new Rectangle(new Point(0, 0), new Point(1, 1));
		Rectangle r2 = new Rectangle(new Point(3, 2), new Point(4, 3));
		Rectangle r3 = new Rectangle(new Point(2, 3), new Point(3, 4));
		Rectangle r4 = new Rectangle(new Point(5, 6), new Point(7, 9));

		Vector<Rectangle> list1 = new Vector<Rectangle>(Arrays.asList(r1, r2, r3));
		Vector<Rectangle> list2 = new Vector<Rectangle>(Arrays.asList(r1, r2));
		Vector<Rectangle> list3 = new Vector<Rectangle>(Arrays.asList(r1, r3, r4));

		RectangleIntersector secto = new RectangleIntersector();

		Vector<Rectangle> answer1 = secto.findOverlap(list1);
		Vector<Rectangle> answer2 = secto.findOverlap(list2);
		Vector<Rectangle> answer3 = secto.findOverlap(list3);

		if (answer1 == null) {
			System.out.println("FAILURE: test 1 did not find intersection in list");
		} else {
			System.out.println("OKAY: test 1 found intersections " + answer1.get(0) + " " + answer1.get(1));
		}
		if (answer2 != null) {
			System.out.println("FAILURE: test 2 found an intersection in list that has none");
		} else {
			System.out.println("OKAY: test 2 no intersections found");
		}
		if (answer3 != null) {
			System.out.println("FAILURE: test 3 found an intersection in list that has none");
		} else {
			System.out.println("OKAY: test 3 no intersections found");
		}
	}

}
