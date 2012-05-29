package org.feistymeow.filesystem;

import java.net.URLDecoder;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/*
 * Provides access to the relevant paths for a Java application.
 * This class cannot provide static members due to runtime constraints.
 * 
 * @author Chris Koeritz
 */
public class PathHelper {
    private static Log c_logger = LogFactory.getLog(PathHelper.class);
    
	// locates the home directory where *this* application is installed.
	// this can be used as a root path for finding configuration files
	// needed by the application.
	public String findHome() {
		String path = ".";
		try {
			path = URLDecoder.decode(getClass().getProtectionDomain().getCodeSource().getLocation().getPath(), "x-www-form-urlencoded");
			// we remove the leading slash that is sometimes present, but only if the path looks.
			// like a dos path.
			if ( (path.length() >=3) && (path.charAt(0) == '/') && (path.charAt(2) == ':') )
				path = path.substring(1);
			// we chop the last component off, because we want an actual path.
			int lastSlash = path.lastIndexOf('/');
			path = path.substring(0, lastSlash);
		} catch (Exception ex) {
			c_logger.error("caught exception during path calculation: " + ex.toString());
			// unknown what we should say here, so we return the default.
		}
		return path;
	}	    	
};

