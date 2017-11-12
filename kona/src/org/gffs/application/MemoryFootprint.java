package org.gffs.application;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import net.sourceforge.sizeof.SizeOf;

/**
 * can retrieve the size of java objects to assist tuning of caches and memory usage. this requires the SizeOf jar and -javaagent setting to
 * point at that jar. this is the project page: https://sourceforge.net/projects/sizeof/?source=typ_redirect
 */
public class MemoryFootprint
{
	static private Log _logger = LogFactory.getLog(MemoryFootprint.class);

	// static SizeOf _sizeEstimater = new SizeOf();
	static {
		// don't count statics in the memory size.
		SizeOf.skipStaticField(true);
		// only complain about large objects if they're bigger than the limit below.
		SizeOf.setMinSizeToLog(5 * 1024 * 1024);
	}

	/**
	 * can report the size of the object 'o' if instrumentation has been set up. if instrumentation is absent, all object sizes will be
	 * reported as zero.
	 */
	public static long getFootprint(Object o)
	{
		if (!_logger.isDebugEnabled()) {
			_logger.error("abusive memory footprint called when not in debug mode.  a logging statement is wrong.");
			return 0;
		}
		try {
			return SizeOf.sizeOf(o);
		} catch (Exception e) {
			_logger.debug("error retrieving SizeOf object; is SizeOf.jar in javaagent?");
			return 0;
		}
	}

	/**
	 * reports the size of the object 'o' plus the size of all other objects reachable from it.
	 */
	public static long getDeepFootprint(Object o)
	{
		if (!_logger.isDebugEnabled()) {
			_logger.error("abusive memory footprint called when not in debug mode.  a logging statement is wrong.");
			return 0;
		}

		try {
			return SizeOf.deepSizeOf(o);
		} catch (Exception e) {
			_logger.debug("error retrieving SizeOf object; is SizeOf.jar in javaagent?");
			return 0;
		}
	}
}
