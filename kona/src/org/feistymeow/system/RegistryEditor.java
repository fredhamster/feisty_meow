
package org.feistymeow.system;

import java.io.*;

/*
 * this class supports reading and setting values in the Windows registry.
 * 
 * some example code came from http://www.rgagnon.com/javadetails/java-0480.html
 * @author Chris Koeritz
 */
public class RegistryEditor {

	// the values below can be used for the expected type in the registry calls.
	public static final String STRING_TYPE = "REG_SZ";
	public static final String DOUBLEWORD_TYPE = "REG_DWORD";  

	// looks up the key provided in the registry and returns true if the key seems to exist.
	// this is just a check for presence; it could have no subkeys or values under it, but it at
	// least seems to be there.
	public static boolean checkKey(String keyName) {
		try {
			// make a command to run in windows that will query the value.
			StringBuilder command = new StringBuilder(QUERY_COMMAND);
			command.append("\"");
			command.append(keyName);
			command.append("\"");
			// start the command running and trap its output.
			Process process = Runtime.getRuntime().exec(command.toString());
			StreamReader reader = new StreamReader(process.getInputStream());
			reader.start();
			process.waitFor();  // let the system call finish up.
			reader.join();  // let the stream reader finish also.
			return process.exitValue() == 0;
		} catch (Exception e) {
			return false;
		}
	}

	// retrieves the key provided from the registry and selects out the value requested.
	// it is necessary to know the expected type, such as REG_SZ or REG_DWORD.
	// the value is always returned as a string and needs to be cast to different types as appropriate.
	public static String getValue(String keyName, String valueName, String typeExpected) {
		try {
			// make a command to run in windows that will query the value.
			StringBuilder command = new StringBuilder(QUERY_COMMAND);
			command.append("\"");
			command.append(keyName);
			command.append("\"");
			command.append(VALUE_FLAG); 
			command.append(valueName);
			// start the command running and trap its output.
			Process process = Runtime.getRuntime().exec(command.toString());
			StreamReader reader = new StreamReader(process.getInputStream());
			reader.start();
			process.waitFor();  // let the system call finish up.
			reader.join();  // let the stream reader finish also.
			// now grab the results from running the command and extract the answer.
			String result = reader.getResult();
			int p = result.indexOf(typeExpected);
			if (p == -1) return null;
			// return the result with the stuff before the type chopped off.
			return result.substring(p + typeExpected.length()).trim();
		} catch (Exception e) {
			return null;
		}
	}

	// makes a change to the specified "keyName" value called "valueName".  the old value will be replaced
	// with the "newValue" provided.  the key and the value do not have to exist prior to the call, but if
	// they already did exist, they'll be updated.
	public static boolean setValue(String keyName, String valueName, String typeExpected, String newValue) {
		try {
			// make a command to run in windows that will set the value.
			StringBuilder command = new StringBuilder(SET_COMMAND);
			command.append("\"");
			command.append(keyName);
			command.append("\"");
			command.append(VALUE_FLAG); 
			command.append(valueName);
			command.append(TYPE_FLAG);
			command.append(typeExpected);
			command.append(FORCE_FLAG);
			command.append(DATA_FLAG);
			command.append(newValue);
			//System.out.println("command to run: " + command);
			// start the command running and trap its output.
			Process process = Runtime.getRuntime().exec(command.toString());
			StreamReader reader = new StreamReader(process.getInputStream());
			reader.start();
			process.waitFor();  // let the system call finish up.
			reader.join();  // let the stream reader finish also.
			return (process.exitValue() == 0);  // zero exit is a success.
		} catch (Exception e) {
			return false;
		}
	}

	// removes the "valueName" value from the key "keyName".  true is returned on success.
	public static boolean deleteValue(String keyName, String valueName) {
		try {
			// make a command to run in windows that will set the value.
			StringBuilder command = new StringBuilder(DELETE_COMMAND);
			command.append("\"");
			command.append(keyName);
			command.append("\"");
			command.append(VALUE_FLAG);
			command.append(valueName);
			command.append(FORCE_FLAG);
			//System.out.println("command to run: " + command);
			// start the command running and trap its output.
			Process process = Runtime.getRuntime().exec(command.toString());
			StreamReader reader = new StreamReader(process.getInputStream());
			reader.start();
			process.waitFor();  // let the system call finish up.
			reader.join();  // let the stream reader finish also.
			return (process.exitValue() == 0);  // zero exit is a success.
		} catch (Exception e) {
			return false;
		}
	}	

	// removes the entire key "keyName" from the registry.  true is returned on success.
	public static boolean deleteKey(String keyName) {
		try {
			// make a command to run in windows that will set the value.
			StringBuilder command = new StringBuilder(DELETE_COMMAND);
			command.append("\"");
			command.append(keyName);
			command.append("\"");
			command.append(FORCE_FLAG);
			//System.out.println("command to run: " + command);
			// start the command running and trap its output.
			Process process = Runtime.getRuntime().exec(command.toString());
			StreamReader reader = new StreamReader(process.getInputStream());
			reader.start();
			process.waitFor();  // let the system call finish up.
			reader.join();  // let the stream reader finish also.
			return (process.exitValue() == 0);  // zero exit is a success.
		} catch (Exception e) {
			return false;
		}
	}

	// constants used in the registry code for talking to windows' reg application.
	private static final String QUERY_COMMAND = "reg query ";
	private static final String VALUE_FLAG = " /v ";

	private static final String SET_COMMAND = "reg add ";
	private static final String TYPE_FLAG = " /t ";
	private static final String DATA_FLAG = " /d ";
	private static final String FORCE_FLAG = " /f ";

	private static final String DELETE_COMMAND = "reg delete ";

	// wrapper class for stream reading came from web example mentioned above. 
	static class StreamReader extends Thread {
		private InputStream is;
		private StringWriter sw;

		StreamReader(InputStream is) {
			this.is = is;
			sw = new StringWriter();
		}

		public void run() {
			int c;
			try {
				while ((c = is.read()) != -1) sw.write(c);
			} catch (IOException e) { /*nothing*/ }
		}

		String getResult() { return sw.toString(); }
	}
}
