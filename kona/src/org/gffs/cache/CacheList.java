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

abstract class CacheList<KeyType, DataType>
{
	protected int _myRole;

	protected RoleBasedCacheNode<KeyType, DataType> _head;
	protected RoleBasedCacheNode<KeyType, DataType> _tail;

	protected CacheList(int role)
	{
		_myRole = role;

		_head = _tail = null;
	}

	public abstract void insert(RoleBasedCacheNode<KeyType, DataType> node);

	public RoleBasedCacheNode<KeyType, DataType> removeFirst()
	{
		if (_head == null)
			return null;

		RoleBasedCacheNode<KeyType, DataType> ret = _head;

		_head = _head.getNext(_myRole);
		if (_head != null)
			_head.setPrevious(_myRole, null);
		else
			_tail = null;

		ret.clearLinks(_myRole);
		return ret;
	}

	public RoleBasedCacheNode<KeyType, DataType> peekFirst()
	{
		if (_head == null)
			return null;

		return _head;
	}

	public void remove(RoleBasedCacheNode<KeyType, DataType> node)
	{
		if (node.getPrevious(_myRole) == null) // At the head of the list
			_head = node.getNext(_myRole);
		else
			node.getPrevious(_myRole).setNext(_myRole, node.getNext(_myRole));

		if (node.getNext(_myRole) == null) // At the tail of the list
			_tail = node.getPrevious(_myRole);
		else
			node.getNext(_myRole).setPrevious(_myRole, node.getPrevious(_myRole));

		node.clearLinks(_myRole);
	}

	public void clear()
	{
		_head = null;
		_tail = null;
	}
}