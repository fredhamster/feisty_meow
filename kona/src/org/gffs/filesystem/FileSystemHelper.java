package org.gffs.filesystem;

import java.io.File;
import java.io.FileNotFoundException;
import java.nio.file.Files;
import java.nio.file.Path;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class FileSystemHelper
{
	private static Log _logger = LogFactory.getLog(FileSystemHelper.class);

	/**
	 * returns true if the path contains a symbolic link anywhere within it.
	 */
	public static boolean pathContainsLink(String path) throws FileNotFoundException
	{
		// replace any backslashes with forward slashes.
		path = path.replaceAll("\\+", "/");

		// make sure path is absolute.
		if (!path.startsWith("/")) {
			String msg = "path passed in was not absolute: '" + path + "'";
			_logger.error(msg);
			throw new FileNotFoundException(msg);
		}

		// replace any double slashes with single ones.
		path = path.replaceAll("//", "/");
		String[] components = path.split("/");

		String currentPath = ""; // never expected to be a link.
		for (String component : components) {
			currentPath = currentPath + "/" + component;
			if (isFileSymLink(new File(currentPath))) {
				return true;
			}
		}
		return false;

		/*
		 * future: this could be more useful if it returned the position of the link as a component in path, but then we also need to accept a
		 * starting point for the link searching so they can find all of them.
		 */
	}

	/**
	 * returns true if the path specified is actually a symbolic link.
	 */
	public static boolean isFileSymLink(File path)
	{
		Path nioPath = path.toPath();
		return Files.isSymbolicLink(nioPath);
	}

	/**
	 * makes the string a more friendly filesystem path for java. this includes turning backslashes into forward slashes.
	 */
	public static String sanitizeFilename(String toClean)
	{
		if (toClean == null)
			return toClean; // can't fix nothing.
		// _logger.debug("before='" + toClean + "'");

		String toReturn = toClean.replace('\\', '/');

		// future: any other cleanings we should do on the path?

		// _logger.debug("after='" + toReturn + "'");
		return toReturn;
	}

	static public void main(String[] args)
	{
		String uglyPath = "C:\\Program Files\\GenesisII\\";
		String fixedPath = sanitizeFilename(uglyPath);
		String expectedPath = "C:/Program Files/GenesisII/";
		if (!fixedPath.equals(expectedPath)) {
			System.err.println("FAILURE IN PARSING!  result is not right: '" + fixedPath + "' when it should be '" + expectedPath);
			System.exit(1);
		} else {
			System.err.println("parsing occurred as expected.");
		}
		System.exit(0);
	}

}
