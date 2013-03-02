package test.java.semantics;

import java.io.FileNotFoundException;

//hmmm: convert this to simple unit test.
public class behavior_of_finally_when_exception_in_catch
{
	private int calibrador = 17;

	private boolean ranRight = false;

	public behavior_of_finally_when_exception_in_catch()
	{
	}

	public int funkyTown() throws FileNotFoundException
	{
		if (calibrador < 3) {
			// we should never get here. it should always raise an exception.
			System.out.println("where did you put it?");
		} else {
			throw new FileNotFoundException("is it larger than a breadbox?");
		}
		return 25;
	}

	public void runTest() throws Throwable
	{
		try {
			int zooty = funkyTown();
			System.out.println("zooty is " + zooty + " but how did we get here???");
		} catch (Throwable cause) {
			System.out.println("caught exception, now will rethrow.  so far this is fine.");
			throw cause;
		} finally {
			System.out.println("ran finally clause; our assumptions are safe.  we MUST see this line!!");
			ranRight = true;
		}
	}

	public static void main(String s[]) throws Exception
	{
		// we are asserting that the finally clause of an exception handler will still
		// fire when an exception is raised in the catch clause. otherwise, all our
		// assumptions about being able to use finally properly are thrown out the window.
		behavior_of_finally_when_exception_in_catch tony = new behavior_of_finally_when_exception_in_catch();
		try {
			tony.runTest();
		} catch (Throwable cause) {
			// yawn.
		}
		System.out.println("the finally clause needs to have run above; otherwise, we've got bad problems.");
		if (!tony.ranRight) {
			System.out.println("FAILURE in assumptions about the finally clause!!!");
		} else {
			System.out.println("okay, cool.  test succeeded.");
		}
	}
}
