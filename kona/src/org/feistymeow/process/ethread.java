package org.feistymeow.process;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * A simple java thread that hearkens back to the HOOPLE C++ ethread in features.
 * 
 * @author Chris Koeritz
 * @copyright Copyright (c) 2010-$now By Feisty Meow Concerns Ltd.
 * @license This file is free software; you can modify and redistribute it under the terms of the
 *          Apache License v2.0: http://www.apache.org/licenses/LICENSE-2.0
 */
public abstract class ethread implements Runnable
{
	private static Log c_logger = LogFactory.getLog(ethread.class);

	// the actual java thread object.
	private volatile Thread c_RealThread = null;
	// provides synchronization for the thread.
	private volatile Object c_lock = new Object();

	/**
	 * creates a new ethread without starting it.
	 */
	public ethread()
	{
	}

	/**
	 * Begins execution of the thread.
	 */
	public void start()
	{
		synchronized (c_lock) {
			if (null == this.c_RealThread) {
				this.c_RealThread = new Thread(this);
				c_logger.debug("starting thread " + c_RealThread.getId());
				this.c_RealThread.start();
			}
		}
	}

	/**
	 * Stops execution of the thread, or at least attempts to.
	 */
	public void stop()
	{
		synchronized (c_lock) {
			Thread goAway = c_RealThread;
			c_RealThread = null;
			if (null != goAway) {
				goAway.interrupt();
			}
		}
	}

	/**
	 * this is the main function that derived classes must implement. it does the actual work that
	 * the thread is intended to perform. note that the derived version must not do anything to
	 * cause the thread to be ripped out while performActivity is still being invoked.
	 */
	// hmmm: should this stay void, or use a bool status to indicate if the thread should die?
	abstract void performActivity();

	/**
	 * Returns true if the thread isn't null.
	 */
	public boolean threadRunning()
	{
		synchronized (c_lock) {
			return (null != this.c_RealThread);
		}
	}

	/**
	 * this is the override from Runnable that allows us to call our own performActivity method.
	 * implementors should not override this; they should override performActivity instead.
	 */
	public void run()
	{
		synchronized (c_lock) {
			if (false == threadRunning()) {
				return; // stopped before it ever started. how can this be? we just got invoked.
			}
			performActivity();
		}
	}
}

// hmmm: still in progress...
// hmmm: missing the timed features of ethread.
// hmmm: missing cancel, exempt_stop, sleep_time, should_stop,

