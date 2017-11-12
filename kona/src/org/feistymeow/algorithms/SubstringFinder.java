package org.feistymeow.algorithms;

/**
 * used to find strings in other strings.
 *
 * part of google interview prep.
 */

public class SubstringFinder
{

	/**
	 * returns the position of 'x' in 'y', or returns a negative number.
	 */
	public int findXinY(String x, String y)
	{
		if ((x == null) || (y == null))
			return -1; // nulls are invalid to compare.
		if (y.length() < x.length())
			return -1; // x cannot be found in shorter string.
		for (int yIter = x.length() - 1; yIter < y.length(); yIter++) {
			boolean good = true;
			/*
			 * xIter has to start at valid value in x, and that needs to be at end of x. board actually had uncorrected error here, where
			 * xIter started at yIter, which is really wrong.
			 */
			// simplified xIter which was inducing all sorts of OBOBs. even though i found the end in yIter,
			// i go forwards on xIter.
			for (int xIter = 0; xIter < x.length(); xIter++) {
				int yComparePosition = (yIter - x.length() + 1) + xIter;
				if (x.charAt(xIter) != y.charAt(yComparePosition)) {
					// System.out.println("no match -- y[" + yIter + "]=" + y.charAt(yPosn) + " x[" + xIter + "]=" + x.charAt(xIter));
					good = false;
					break;
				}
			}
			if (good) {
				int toReturn = yIter - x.length() + 1;
				// System.out.println("found at position " +toReturn );
				return toReturn;
			}
		}

		return -1;
	}

	public static void main(String[] argv)
	{
		SubstringFinder sf = new SubstringFinder();

		String x1 = "petunia";
		String y1 = "sometimes my flowers are roses and sometimes they are petunias and sometimes they are turnips.";
		if (sf.findXinY(x1, y1) != 54) {
			System.out.println("FAILURE: did not find at right index for test 1");
		} else {
			System.out.println("OKAY: found substring at right index for test 1");
		}

		String x2 = "qn";
		String y2 = "xaqno";
		if (sf.findXinY(x2, y2) != 2) {
			System.out.println("FAILURE: did not find at right index for test 2");
		} else {
			System.out.println("OKAY: found substring at right index for test 2");
		}

		String x3 = "qn";
		String y3 = "xaqon";
		if (sf.findXinY(x3, y3) >= 0) {
			System.out.println("FAILURE: found non-existent string for test 3");
		} else {
			System.out.println("OKAY: did not find substring for test 3");
		}

	}
}
