package org.gffs.application;

import java.io.File;
import java.io.IOException;
import java.net.URISyntaxException;
import java.net.URL;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.gffs.filesystem.FileSystemHelper;

/**
 * Some utility functions for getting information about the running application.
 * 
 * @author Chris Koeritz
 */
public class ProgramTools
{
	public static Log _logger = LogFactory.getLog(ProgramTools.class);
	
	/**
	 * produces a list of the stack for a certain number of its elements, called stack frames. this will ignore the fact that this function is
	 * invoked, and start counting stack frames from the immediate caller's perspective (including it).
	 */
	public static String showLastFewOnStack(int howManyFrames)
	{
		StackTraceElement[] elements = Thread.currentThread().getStackTrace();
		StringBuilder toReturn = new StringBuilder();
		/*
		 * don't start at the very first frame; we want to skip backwards to the direct caller of this function.
		 */
		int startFrame = 3;
		int endFrame = Math.min(howManyFrames + 3, elements.length - 1);
		for (int i = startFrame; i < endFrame; i++) {
			if (toReturn.length() != 0) {
				toReturn.append("\n<= ");
			}
			toReturn.append(getStackFrame(i));
		}
		return toReturn.toString();
	}

	/**
	 * returns the Nth frame backwards starting from this function. 0 is this method, 1 is the invoker, 2 is the invoker's invoker, etc.
	 */
	public static String getStackFrame(int which)
	{
		StackTraceElement[] elements = Thread.currentThread().getStackTrace();
		/* a little self-protection to avoid accessing missing parts of the array. */
		if (which >= elements.length)
			which = elements.length - 1;
		return elements[which].toString();
	}

	/**
	 * returns the location where the code is running, as best as can be determined. finds the running location based on our jar files, or if
	 * that's not available, on the assumption of app path being within an appropriate installation (even if not at the top). this method
	 * cannot use standard genesis properties to look up the path, because this function needs to operate before anything else is loaded (for
	 * OSGi usage).
	 */
	static public String getInstallationDirectory()
	{
		String appPath = null;
		// see if we can intuit our location from living in a jar.
		URL url = ProgramTools.class.getProtectionDomain().getCodeSource().getLocation();
		try {
			// get the app path but switch back slashes to forward ones.
			appPath = new File(url.toURI().getSchemeSpecificPart()).toString().replace('\\', '/');
		} catch (URISyntaxException e) {
			String msg = "failed to convert code source url to app path: " + url;
			_logger.error(msg);
			throw new RuntimeException(msg);
		}
		if (_logger.isTraceEnabled())
			_logger.trace("got source path as: " + appPath);
		if (appPath.endsWith(".jar")) {
			// we need to chop off the jar file part of the name.
			int lastSlash = appPath.lastIndexOf("/");
			// if (lastSlash < 0)
			// lastSlash = appPath.lastIndexOf("\\");
			if (lastSlash < 0) {
				String msg = "could not find a slash character in the path: " + appPath;
				_logger.error(msg);
				throw new RuntimeException(msg);
			}
			appPath = appPath.substring(0, lastSlash);
			if (_logger.isTraceEnabled())
				_logger.trace("truncated path since inside jar: " + appPath);
		}
		appPath = appPath.concat("/..");

		if (_logger.isTraceEnabled())
			_logger.trace("jar-intuited startup bundle path: " + appPath);

		File startupDir = new File(appPath);
		if (!startupDir.exists() || !startupDir.isDirectory()) {
			throw new RuntimeException(
				"the location where we believe the installation is running from does not actually exist as a directory.");
		}

		//hmmm: below may not be very general since it does osgi?  but it will work if people use a bundles dir.
		
		/*
		 * make sure we can find our own bundles directory, which is a crucial thing for osgi. if we can't find it, then we really don't know
		 * where home is.
		 */
		File testingBundlesDir = new File(startupDir, "bundles");
		File testingExtDir = new File(startupDir, "ext");
		String lastStartupDirState = "not-equal"; // a string we should never see as a full path.

		while (!testingBundlesDir.exists() || !testingExtDir.exists()) {
			if (_logger.isDebugEnabled()) {
				if (_logger.isTraceEnabled())
					_logger.debug("failed to find bundles directory at '" + startupDir.getAbsolutePath() + "', popping up a level.");
			}

			if (lastStartupDirState.equals(FileSystemHelper.sanitizeFilename(startupDir.getAbsolutePath()))) {
				throw new RuntimeException(
					"caught the startup directory not changing, which means we have hit the root and failed to find our bundles and ext directories.");
			}
			// reset for next time.
			lastStartupDirState = FileSystemHelper.sanitizeFilename(startupDir.getAbsolutePath());

			// pop up a level, since we didn't find our bundles directory.
			startupDir = new File(startupDir, "..");
			testingBundlesDir = new File(startupDir, "bundles");
			testingExtDir = new File(startupDir, "ext");

			if (startupDir.getParent() == null) {
				throw new RuntimeException("failed to find the bundles and ext directories after hitting top of file system paths.");
			}
		}

		// we successfully found the bundles directory, even if we may have had to jump a few hoops.
		if (_logger.isTraceEnabled()) {
			_logger.debug("successfully found bundles directory under path: " + appPath);
		}

		// now resolve the path to an absolute location without relative components.
		try {
			appPath = FileSystemHelper.sanitizeFilename(startupDir.getCanonicalPath());
		} catch (IOException e) {
			_logger.error("could not open osgi directory: " + appPath);
		}
		if (_logger.isTraceEnabled())
			_logger.debug("startup path after resolution with File: " + appPath);
		return appPath;
	}
}
