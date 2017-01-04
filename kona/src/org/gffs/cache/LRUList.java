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


class LRUList<KeyType, DataType> extends CacheList<KeyType, DataType>
{
	public LRUList()
	{
		super(RoleBasedCacheNode.ROLE_LRU);
	}

	@Override
	public void insert(RoleBasedCacheNode<KeyType, DataType> node)
	{
		// LRU inserts ALWAYS go at the tail
		if (_tail == null) {
			_head = _tail = node;
			return;
		}

		_tail.setNext(_myRole, node);
		node.setPrevious(_myRole, _tail);
		_tail = node;
	}

	public void noteUse(RoleBasedCacheNode<KeyType, DataType> node)
	{
		remove(node);
		insert(node);
	}
}