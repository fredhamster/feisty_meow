package test.java.semantics;

import java.io.FileNotFoundException;

//import java.util.List;

class finally_behavior_test
{
    int calibrador = 17;

    public finally_behavior_test()
    {
    }

    public int funkyTown() throws FileNotFoundException
    {
        if (calibrador < 3) {
            // we should never get here.  it should always raise an exception.
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
            System.out.println("caught exception, now will rethrow.");
            throw cause;
        } finally {
            System.out.println("still got to finally, our assumptions are safe.");
        }
    }

    public static void main(String s[]) throws Exception
    {
        // we are asserting that the finally clause of an exception handler will still
        // fire when an exception is raised in the catch clause. otherwise, all our
        // assumptions about being able to use finally properly are thrown out the window.
        finally_behavior_test tony = new finally_behavior_test();
        try {
            tony.runTest();
        } catch (Throwable cause) {
            //yawn.
        }
        System.out.println("Hey, did the finally clause say it ran above?");
        System.out.println("If so, great.  If not, we've got problems.");
    }

}
