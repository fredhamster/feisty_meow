package org.feistymeow.process;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * A simple java thread that hearkens back to the HOOPLE C++ ethread in features.
 * 
 * @author Chris Koeritz
 * @copyright Copyright (c) 2010-$now By Feisty Meow Concerns Ltd.
 * @license This file is free software; you can modify and redistribute it under the terms of the Apache License v2.0:
 *          http://www.apache.org/licenses/LICENSE-2.0
 */
public abstract class ethread implements Runnable
{
	private static Log c_logger = LogFactory.getLog(ethread.class);

	// the actual java thread object.
	private volatile Thread c_RealThread = null;
	// provides synchronization for the thread.
	private volatile Object c_lock = new Object();
	// this is the timing period, for a timed thread. if zero, then this is a single shot thread.
	private long c_period = 0;
	// records whether the thread should shut down or not.
	private boolean c_stopThread = false;
	// snooze between checks on the stop timer.
	final long SNOOZE_PERIOD = 20;

	/**
	 * creates a new single-shot ethread without starting it. this type of thread will run just once.
	 */
	public ethread()
	{
	}

	/**
	 * creates a new periodic ethread without starting it. this type of thread runs every "period" milliseconds until stopped or until the
	 * performActivity method returns false.
	 */
	public ethread(long period)
	{
		c_period = period;
	}

	/**
	 * this is the main function that derived classes must implement. it does the actual work that the thread is intended to perform. note
	 * that the derived version must not do anything to cause the thread to be ripped out while performActivity is still being invoked. the
	 * return value should be true if the thread can continue executing. this is meaningless for single shot threads executed via runOnce, but
	 * matters for the periodic threads started with runPeriodic.
	 */
	abstract public boolean performActivity();

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
		cancel();
		while (true) {
			if (threadAlive()) {
				try {
					Thread.sleep(40);
				} catch (InterruptedException e) {
					// ignoring this since we'll keep snoozing as needed.
				}
			} else {
				break;
			}
		}
	}

	/**
	 * Signals the thread to stop executing, but does not wait for it.
	 */
	void cancel()
	{
		synchronized (c_lock) {
			c_stopThread = true;
			Thread goAway = c_RealThread;
			c_RealThread = null;
			if (null != goAway) {
				goAway.interrupt();
			}
		}
	}

	/**
	 * Returns true if the thread object is still alive. this does not necessarily mean it is currently active.
	 */
	public boolean threadAlive()
	{
		synchronized (c_lock) {
			return this.c_RealThread != null;
		}
	}

	/**
	 * returns true if the thread has been told to stop running.
	 */
	public boolean shouldStop()
	{
		synchronized (c_lock) {
			return c_stopThread;
		}
	}

	/**
	 * this is the override from Runnable that allows us to call our own performActivity method. implementors should not override this; they
	 * should override performActivity instead.
	 */
	@Override
	public void run()
	{
		if (!threadAlive()) {
			return; // stopped before it ever started. how can this be? we just got invoked.
		}
		try {
			while (true) {
				boolean keepGoing = performActivity();
				if (!keepGoing) {
					c_logger.debug("thread returned false, signifying it wants to exit.  now dropping it.");
					break;
				}
				if (c_period == 0) {
					// not a periodic thread, so we're done now.
					break;
				}
				long nextRun = System.currentTimeMillis() + c_period;
				while (System.currentTimeMillis() < nextRun) {
					if (shouldStop()) {
						break;
					}
					try {
						Thread.sleep(SNOOZE_PERIOD);
					} catch (InterruptedException e) {
						// well, we'll hit it again soon.
					}
				}
			}
		} catch (Throwable t) {
			c_logger.info("exception thrown from performActivity: " + t.getLocalizedMessage(), t);
		}
		// reset the thread held since we're leaving right now.
		c_stopThread = true;
		c_RealThread = null;
	}
}
