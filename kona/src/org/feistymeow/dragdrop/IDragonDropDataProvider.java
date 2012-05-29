package org.feistymeow.dragdrop;

import java.util.List;
import java.awt.Point;

/**
 * An interface for any object that can interact with a DragonTransferSupport to receive files (or
 * other things) that are dropped on it and that can provide files (or other things) for dragging to
 * another location. Note that the details of finding a selected node or determining what objects
 * are relevant there is entirely up to the component. You probably do not need this interface if
 * you have implemented your own TransferHandler.
 * 
 * @author Chris Koeritz
 * @copyright Copyright (c) 2012-$now By University of Virginia
 * @license This file is free software; you can modify and redistribute it under the terms of the
 *          Apache License v2.0: http://www.apache.org/licenses/LICENSE-2.0
 */
public interface IDragonDropDataProvider
{
    /**
     * A function that is invoked by the handler when some files or other objects are being dragged
     * out of the parent. The handler expects the real provider to come up with a useful set of
     * things to deliver at the drag target.
     */
    public List<Object> provideDragList();

    /**
     * A function that is invoked by the d&d manager when a passel of objects have been dropped on
     * the parent object.
     */
    public boolean consumeDropList(List<Object> dropSet, Point cursor);
}
