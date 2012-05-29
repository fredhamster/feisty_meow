package org.feistymeow.dragdrop;

import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.util.List;

import javax.swing.JComponent;
import javax.swing.TransferHandler;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * A transfer handler that can be extended and used to inter-operate with DragonDropManager. This
 * object is not strictly necessary to use, but it can help if one has not already implemented one's
 * own transfer handler.
 * 
 * @author Chris Koeritz
 * @copyright Copyright (c) 2012-$now By University of Virginia
 * @license This file is free software; you can modify and redistribute it under the terms of the
 *          Apache License v2.0: http://www.apache.org/licenses/LICENSE-2.0
 */
@SuppressWarnings("serial")
public class DragonTransferHandler extends TransferHandler
{
    static private Log logger = LogFactory.getLog(DragonTransferHandler.class);
    IDragonDropDataProvider c_provider;

    public DragonTransferHandler(IDragonDropDataProvider provider)
    {
        c_provider = provider;
    }

    @Override
    public boolean canImport(TransferSupport support)
    {
        if (support == null) return false;
        if (!support.isDrop())
            return false; // we don't support cut&paste here.
        logger.debug("canImport: base just saying okay.");
        return true;
    }

    @Override
    protected Transferable createTransferable(JComponent c)
    {
        logger.debug("createTransferable: at base, returning ListTransferable.");
        return new ListTransferable(c_provider.provideDragList());
    }

    @Override
    protected void exportDone(JComponent source, Transferable data, int action)
    {
        logger.debug("exportDone: base got event for component " + source.toString());
    }

    @Override
    public int getSourceActions(JComponent c)
    {
        return COPY;
    }

    @Override
    public boolean importData(TransferSupport support)
    {
        if (support == null) return false;
        logger.debug("importData: at base...");

        if (support.isDataFlavorSupported(DataFlavor.javaFileListFlavor)) {
            logger.debug("importing data with java files flavor");
            List<Object> files = ListTransferable.extractData(support.getTransferable());
            if ((files != null) && (files.size() != 0)) {
                c_provider.consumeDropList(files, support.getDropLocation().getDropPoint());
                return true;
            }
        } else if (support.isDataFlavorSupported(ListTransferable.getURIListFlavor1())
                || support.isDataFlavorSupported(ListTransferable.getURIListFlavor2())) {
            logger.debug("importing data with uri list flavor");
            List<Object> files = ListTransferable.extractData(support.getTransferable());
            if ((files != null) && (files.size() != 0)) {
                c_provider.consumeDropList(files, support.getDropLocation().getDropPoint());
                return true;
            }
        }
        logger.warn("passing importData request to superclass, which will probably fail.");
        return super.importData(support);
    }
}
