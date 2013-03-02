package test.java.semantics;

import java.util.List;

class instance_of
{
    public instance_of() {}
    
    public Object cogitate() {
        return null;
    }

    public static void main(String s[]) throws Exception
    {
        // we are just asserting that it is safe to do instanceof on an object that is null.
        // let's test that theory.
        instance_of tony = new instance_of();
        Object fred = tony.cogitate();  // will produce null.
        if (fred instanceof List<?>) {
            throw new Exception("that should not have happened!");
        } else {
            System.out.println("told us null is not an instance of List, which is correct.");
        }
        
    }
}