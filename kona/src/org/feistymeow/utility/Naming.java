package org.feistymeow.utility;

/*
 * Some helper methods that retrieve the function and class names where code is executing.
 * This can be helpful for doing very specific logging, and will work even if the log4j
 * settings have turned off the function names and such (given that you really want to
 * see those in a log entry).  
 * 
 * @author Chris Koeritz
 */
public class Naming {

	// returns the name of the function invoking this method.
	public static String thisFunctionName() {
		Throwable ex = new Throwable();
		StackTraceElement[] trace = ex.getStackTrace();
		// StackTrace trace = new StackTrace(0, true);
		String meth = trace[0].getMethodName(); // this should be *this*
		// function.
		for (int i = 1; i < trace.length; i++) {
			if ((trace[i].getMethodName() != meth)
					&& (!trace[i].getMethodName().contains("formatted_names"))) {
				// we've gone back far enough.
				return trace[i].getMethodName();
			}
		}
		return "unknown";
	}

	// provides a method to get the current class name and function name without
	// needing to embed a lot of code into individual functions.
	public static String thisClassName() {
		Throwable ex = new Throwable();
		StackTraceElement[] trace = ex.getStackTrace();
		// StackTrace trace = new StackTrace(0, true);

		String meth = trace[0].getMethodName(); // this should be *this*
		// function.
		for (int i = 1; i < trace.length; i++) {
			String currClass = trace[i].getClassName();
			if ((trace[i].getMethodName() != meth)
					&& (!trace[i].getMethodName().contains("formatted_names"))) {
				// we've gone back far enough.
				String simpleClassName = extractSimpleClassName(currClass);
				return simpleClassName;
			}
		}
		return "unknown";
	}

	public String thisPackageName() {
		Throwable ex = new Throwable();
		StackTraceElement[] trace = ex.getStackTrace();
		// StackTrace trace = new StackTrace(0, true);

		String meth = trace[0].getMethodName(); // this should be *this*
		// function.
		for (int i = 1; i < trace.length; i++) {
			String currClass = trace[i].getClassName();
			if ((trace[i].getMethodName() != meth)
					&& (!trace[i].getMethodName().contains("formatted_names"))) {
				// we've gone back far enough.
				String packageName = extractPackageName(currClass);
				return packageName;
			}
		}
		return "unknown";
	}

	public static String extractPackageName(String fullClassName) {
		if ((null == fullClassName) || ("".equals(fullClassName)))
			return "";

		// The package name is everything preceding the last dot.
		// Is there a dot in the name?
		int lastDot = fullClassName.lastIndexOf('.');

		// Note that by fiat, I declare that any class name that has been
		// passed in which starts with a dot doesn't have a package name.
		if (0 >= lastDot)
			return "";

		// Otherwise, extract the package name.
		return fullClassName.substring(0, lastDot);
	}

	public static String extractSimpleClassName(String fullClassName) {
		if ((null == fullClassName) || ("".equals(fullClassName)))
			return "";

		// The simple class name is everything after the last dot.
		// If there's no dot then the whole thing is the class name.
		int lastDot = fullClassName.lastIndexOf('.');
		if (0 > lastDot)
			return fullClassName;

		// Otherwise, extract the class name.
		return fullClassName.substring(++lastDot);
	}

	// returns a nicely formatted string containing the class and function name.
	// the string also contains a colon and space on the end so other text
	// can be concatenated right up against it while still being readable.
	public static String formatted_names() {
		String class_name = thisClassName();
		String function_name = thisFunctionName();
		return class_name + "." + function_name + ": ";
	}
};
