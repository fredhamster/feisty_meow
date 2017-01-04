package org.gffs.cache;

/*
 * Copyright 2006 University of Virginia
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may
 * obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions
 * and limitations under the License.
 */

import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * This cache attempt to efficiently handle cached items that may time out after a certain period of time. It does this by maintaining 3
 * separate data structures. The first is a HashMap which allows for quick access to the cached data based off of the key. The second is a
 * linked list which maintains the LRU property of the items. The third is a list ordered by timeout so that items that have timed out can be
 * identified quickly (i.e., a straight traversal of this list). All data structures share the exact same data node (not equivalent, but
 * identical instances of the node class). This means that once a node is identified through any of the means indicated (key, LRU property,
 * timeout property), the node for all three data structures has been identified and does not need to be looked up in the others.
 * 
 * @author mmm2a
 * 
 * @param <KeyType>
 * @param <DataType>
 */
public class TimedOutLRUCache<KeyType, DataType>
{
	static Log _logger = LogFactory.getLog(TimedOutLRUCache.class);

	private HashMap<KeyType, RoleBasedCacheNode<KeyType, DataType>> _map;
	private LRUList<KeyType, DataType> _lruList;
	private TimeoutList<KeyType, DataType> _timeoutList;

	private int _maxElements;
	private long _defaultTimeoutMS;
	private Thread _activeTimeoutThread = null;
	private boolean _logCacheEjection = false;
	public String _cacheName = null; // the name for this cache.

	public TimedOutLRUCache(int maxElements, long defaultTimeoutMS, String cacheName)
	{
		if (maxElements < 1)
			throw new IllegalArgumentException("\"maxElements\" must be greater than 0.");
		_cacheName = cacheName;
		if (_cacheName == null)
			throw new IllegalArgumentException("must provide a non-null cache name");

		_maxElements = maxElements;
		_defaultTimeoutMS = defaultTimeoutMS;
		_map = new HashMap<KeyType, RoleBasedCacheNode<KeyType, DataType>>(_maxElements);
		_lruList = new LRUList<KeyType, DataType>();
		_timeoutList = new TimeoutList<KeyType, DataType>();
	}

	/**
	 * returns the number of elements held in the cache currently.
	 */
	public int size()
	{
		return _map.size();
	}

	/**
	 * allows this cache to log when items are removed due to overloading or timing out.
	 */
	public void setCacheEjectionLogging(boolean logRemovals)
	{
		_logCacheEjection = logRemovals;
	}

	public void activelyTimeoutElements(boolean activelyTimeout)
	{
		synchronized (_map) {
			if (_activeTimeoutThread == null) {
				if (activelyTimeout)
					startActiveTimeout();
			} else {
				if (!activelyTimeout)
					stopActiveTimeout();
			}
		}
	}

	public String debugPrefix()
	{
		return _cacheName + ": ";
	}

	public void put(KeyType key, DataType data, long timeoutMS)
	{
		RoleBasedCacheNode<KeyType, DataType> newNode =
			new RoleBasedCacheNode<KeyType, DataType>(key, data, new Date(System.currentTimeMillis() + timeoutMS));

		synchronized (_map) {
			RoleBasedCacheNode<KeyType, DataType> oldNode = _map.remove(key);
			if (oldNode != null) {
				_lruList.remove(oldNode);
				_timeoutList.remove(oldNode);
			}

			if (_map.size() >= _maxElements)
				clearStale();

			while (_map.size() >= _maxElements) {
				RoleBasedCacheNode<KeyType, DataType> node = _lruList.removeFirst();
				if (_logCacheEjection && _logger.isDebugEnabled())
					_logger.debug(debugPrefix() + "overloaded cache: removing cached item with key: " + node.getKey());
				_timeoutList.remove(node);
				_map.remove(node.getKey());
			}

			_map.put(key, newNode);
			_lruList.insert(newNode);
			_timeoutList.insert(newNode);

			_map.notify();
		}
	}

	public void put(KeyType key, DataType data)
	{
		put(key, data, _defaultTimeoutMS);
	}

	public int getMaximumElements()
	{
		return _maxElements;
	}

	/**
	 * tickles the object so that it won't expire for another default expiration period. true is returned if the object was there and got
	 * updated.
	 */
	public boolean refresh(KeyType key)
	{
		synchronized (_map) {
			RoleBasedCacheNode<KeyType, DataType> node = _map.get(key);
			if (node == null)
				return false;
			_lruList.remove(node);
			node.setInvalidationDate(_defaultTimeoutMS);
			// move the node to the end of the LRU list, since we just accessed it.
			_lruList.insert(node);
			// also fix its position in the timeout list.
			_timeoutList.remove(node);
			_timeoutList.insert(node);
			return true;
		}
	}

	// hmmm: highly experimental memory analysis code here!
	// private final long CHECK_INTERVAL = 1000 * 60; // one minute interval between deep checks currently.
	private final long CHECK_INTERVAL = 1000 * 10;// hmmm: way too fast interval, being used for debugging.

	private Date _nextDeepSizeCheck = new Date((new Date().getTime()) + CHECK_INTERVAL);

	public DataType get(KeyType key)
	{
		Date now = new Date();

		if (now.after(_nextDeepSizeCheck)) {

			/*
			 * hmmm: size check code below, not right yet.
			 * 
			 * would be nice to break that output into k, m, g, etc.
			 * 
			 * trying the deep footprint on 'this' is giving unrealistic very small sizes.
			 * 
			 * also the deep size check is dying with a stack overflow during the large rns directory test, so we cannot use it yet.
			 */
			// if (_logger.isDebugEnabled()) {
			// long sizeUsed = MemoryFootprint.getDeepFootprint(_map) + MemoryFootprint.getDeepFootprint(_lruList)
			// + MemoryFootprint.getDeepFootprint(_timeoutList);
			// _logger.debug(SizeOf.humanReadable(sizeUsed) + " consumed by "+ _cacheName);
			// }

			_nextDeepSizeCheck = new Date((new Date().getTime()) + CHECK_INTERVAL);
		}

		synchronized (_map) {
			RoleBasedCacheNode<KeyType, DataType> node = _map.get(key);
			if (node == null)
				return null;
			_lruList.remove(node);
			if (node.getInvalidationDate().before(now)) {
				// this entry has become stale.
				if (_logCacheEjection && _logger.isDebugEnabled())
					_logger.debug(debugPrefix() + "timed-out entry in get: removing cached item with key: " + node.getKey());
				_map.remove(key);
				_timeoutList.remove(node);
				return null;
			}
			// move the node to the end of the LRU list, since we just accessed it.
			_lruList.insert(node);
			return node.getData();
		}
	}

	public List<DataType> getAll()
	{
		ArrayList<DataType> toReturn = new ArrayList<DataType>();
		synchronized (_map) {
			for (KeyType key : _map.keySet()) {
				toReturn.add(_map.get(key).getData());
			}
		}
		return toReturn;
	}

	public List<DataType> getAllReferenced(List<KeyType> references)
	{
		ArrayList<DataType> toReturn = new ArrayList<DataType>();
		synchronized (_map) {
			for (KeyType key : references) {
				//// for (KeyType key : _map.keySet()) {
				if (_map.containsKey(key)) {
					toReturn.add(_map.get(key).getData());
				} else {
					_logger.error(debugPrefix() + "failed to locate referenced object in cache: " + key);
				}
			}
		}
		return toReturn;
	}

	public void clearStale()
	{
		Date now = new Date();

		synchronized (_map) {
			while (true) {
				RoleBasedCacheNode<KeyType, DataType> node = _timeoutList.peekFirst();
				if (node == null)
					break;

				if (node.getInvalidationDate().compareTo(now) <= 0) {
					if (_logCacheEjection && _logger.isDebugEnabled())
						_logger.debug(debugPrefix() + "removing timed-out node: " + node.getKey());
					_map.remove(node.getKey());
					_timeoutList.removeFirst();
					_lruList.remove(node);
				} else {
					break;
				}
			}
		}
	}

	public void remove(KeyType key)
	{
		synchronized (_map) {
			RoleBasedCacheNode<KeyType, DataType> node = _map.remove(key);
			if (node != null) {
				_lruList.remove(node);
				_timeoutList.remove(node);
			}
		}
	}

	public Set<KeyType> keySet()
	{
		synchronized (_map) {
			return new HashSet<KeyType>(_map.keySet());
		}
	}

	public void clear()
	{
		synchronized (_map) {
			_map.clear();
			_lruList.clear();
			_timeoutList.clear();
		}
	}

	public void startActiveTimeout()
	{
		_activeTimeoutThread = new Thread(new ActiveTimeoutWorker(), "Active Cache Timeout Thread");
		_activeTimeoutThread.setDaemon(true);
		_activeTimeoutThread.start();
	}

	public void stopActiveTimeout()
	{
		Thread tmp = _activeTimeoutThread;
		_activeTimeoutThread = null;
		synchronized (_map) {
			_map.notify();
		}

		try {
			tmp.join();
		} catch (InterruptedException cause) {
		}
	}

	private class ActiveTimeoutWorker implements Runnable
	{
		public void run()
		{
			synchronized (_map) {
				while (_activeTimeoutThread != null) {
					try {
						clearStale();
						RoleBasedCacheNode<KeyType, DataType> firstNode = _timeoutList.peekFirst();
						if (firstNode == null) {
							_map.wait();
						} else {
							Date nextStale = firstNode.getInvalidationDate();
							long timeout = nextStale.getTime() - System.currentTimeMillis();
							if (timeout > 0) {
								_map.wait(timeout);
							}
						}
					} catch (InterruptedException ie) {
					}
				}
			}
		}
	}
}