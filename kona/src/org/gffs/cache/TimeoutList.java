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

class TimeoutList<KeyType, DataType> extends CacheList<KeyType, DataType>
{
	public TimeoutList()
	{
		super(RoleBasedCacheNode.ROLE_TIMEOUT);
	}

	@Override
	public void insert(RoleBasedCacheNode<KeyType, DataType> node)
	{
		// We'll start at the end because most likely we're adding to the end
		if (_tail == null) {
			// The list is empty
			_head = _tail = node;
			return;
		}

		RoleBasedCacheNode<KeyType, DataType> tmp;
		for (tmp = _tail; tmp != null; tmp = tmp.getPrevious(_myRole)) {
			if (tmp.getInvalidationDate().compareTo(node.getInvalidationDate()) <= 0) {
				// current node invalidates before me, so I should go after him
				node.setPrevious(_myRole, tmp);
				node.setNext(_myRole, tmp.getNext(_myRole));
				tmp.setNext(_myRole, node);
				if (node.getNext(_myRole) == null) {
					// Adding to the tail
					_tail = node;
				} else {
					node.getNext(_myRole).setPrevious(_myRole, node);
				}

				return;
			}
		}

		// We add to the head of the list
		node.setNext(_myRole, _head);
		_head.setPrevious(_myRole, node);
		_head = node;
	}
}