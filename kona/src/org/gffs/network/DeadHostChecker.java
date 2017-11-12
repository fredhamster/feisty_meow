package org.gffs.network;

import java.util.HashMap;

import org.apache.commons.lang3.builder.HashCodeBuilder;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

//import edu.virginia.vcgr.genii.client.ClientProperties;

/**
 * Manages a list of hosts that have proven to be down recently. Has support to not immediately fail the host, as this can lead to too quick
 * an assumption that the host is down, but after N tries, the host is out. It will then be tested again periodically so we know when it comes
 * back online.
 */
public class DeadHostChecker
{
	static private Log _logger = LogFactory.getLog(DeadHostChecker.class);
	
	//need better substitute for cli props.
	public static class ClientProperties {
		int timeout = 2 * 60 * 1000;  // 2 minutes timeout by default, in ms.
		
		ClientProperties() {
		}
	}
	static public ClientProperties props;
	

	// this value specifies how many attempts can fail before the host is considered down.
	static private final int HOW_MANY_DOWNS_ALLOWED = 1;

	/*
	 * this is the longest amount of time between checking of dead hosts that we'll ever pause. exponential backoff will occur up until this
	 * delay time, and then stay at this delay time afterwards.
	 */
	static private final int MAXIMUM_ALLOWABLE_CHECKING_DELAY = 60 * 1000 * 5; // current is 5 minutes max for exponential backoff on retries.

	public static class HostKey
	{
		public String hostname;
		public int port;

		HostKey(String hostname, int port)
		{
			this.hostname = hostname;
			this.port = port;
		}

		@Override
		public int hashCode()
		{
			return new HashCodeBuilder(37, 839). // two randomly chosen prime numbers
			// if deriving: appendSuper(super.hashCode()).
				append(hostname).append(port).toHashCode();
		}

		@Override
		public boolean equals(Object o)
		{
			if (!(o instanceof HostKey))
				return false; // wrong object.
			HostKey realo = (HostKey) o;
			return realo.hostname.equals(hostname) && (realo.port == port);
		}

		@Override
		public String toString()
		{
			return hostname + ":" + port;
		}
	}

	static final HashMap<HostKey, RetryInfo> deadHosts = new HashMap<HostKey, RetryInfo>();

	public static class RetryInfo
	{
		public long nextTime;
		public int delay;
		public int downCount = 0;

		public RetryInfo()
		{
			// We just failed, so base a delay on the overall timeout to delay our next attempt.
			delay = initialDelay();
			nextTime = System.currentTimeMillis() + delay;
		}

		int initialDelay()
		{
			return props.timeout / 2;
		}

		boolean isThisHostDead()
		{
			if (downCount < HOW_MANY_DOWNS_ALLOWED) {
				return false;
			}
			if (System.currentTimeMillis() > nextTime) {
				// this host is being allowed a retry.
				nextTime = System.currentTimeMillis() + delay;
				return false;
			}
			return true;
		}

		void recordDown()
		{
			downCount++;
		}
	}

	/**
	 * checks the host in our records and returns true if it is considered alive and false if it is considered dead.
	 */
	public static boolean evaluateHostAlive(String host, int port)
	{
		HostKey key = new HostKey(host, port);

		// Added July 14, 2015 by ASG to deal with dead hosts and not bother trying to talk to them. The timeouts kill us.
		synchronized (deadHosts) {
			if (deadHosts.containsKey(host)) {
				RetryInfo inf = deadHosts.get(key);
				if (inf == null) {
					_logger.warn("logic error: dead hosts list said it had host " + key + " was listed but we got a null record for it.");
					return true;
				}
				return !inf.isThisHostDead();
			} else {
				// up as far as we know; no record exists.
				if (_logger.isTraceEnabled())
					_logger.debug("host " + key + " is fine as far as we know.");
				return true;
			}
		}
	}

	public static void addHostToDeadPool(String host, int port)
	{
		HostKey key = new HostKey(host, port);

		synchronized (deadHosts) {
			RetryInfo inf = deadHosts.get(key);
			if (inf == null) {
				// Not there, set it up and add it.
				inf = new RetryInfo();
				deadHosts.put(key, inf);
			}

			boolean alreadyDead = false;
			if (inf.isThisHostDead()) {
				// this one is already down so expand the timeout.
				if (_logger.isDebugEnabled())
					_logger.warn("host " + key + " is considered dead already; increasing delay.");
				inf.delay *= 2;
				inf.nextTime = System.currentTimeMillis() + inf.delay;

				if (inf.delay > MAXIMUM_ALLOWABLE_CHECKING_DELAY) {
					inf.delay = MAXIMUM_ALLOWABLE_CHECKING_DELAY;
				}
				// flag this so we don't say something again below.
				alreadyDead = true;
			}

			// we definitely saw this host as down at least once, so record that now.
			inf.recordDown();

			if (!inf.isThisHostDead()) {
				// still up, although we needed to record that failure.
				if (_logger.isDebugEnabled())
					_logger.debug("host " + key + " is not dead yet but suffered a connection problem.");
			} else {
				// this is dead now. say something about it if we didn't already.
				if (!alreadyDead && _logger.isDebugEnabled())
					_logger.warn("host " + key + " is newly considered dead due to communication problems.");
			}
		}
	}

	public static void removeHostFromDeadPool(String host, int port)
	{
		HostKey key = new HostKey(host, port);

		// Well, the host was reported alive again, so remove if it is in deadHosts.
		synchronized (deadHosts) {
			if (deadHosts.containsKey(key)) {
				if (_logger.isDebugEnabled()) {
					// if it's not present, we don't say anything.
					_logger.debug("host " + key + " is being removed from dead host pool.");
				}
				// drop it from the list.
				deadHosts.remove(key);
			}
		}
	}

}
