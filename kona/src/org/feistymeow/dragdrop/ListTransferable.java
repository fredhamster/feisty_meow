package org.feistymeow.dragdrop;

import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.Reader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import java.util.Vector;
import java.util.StringTokenizer;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Implements a transferable object that understands URI lists as well as java file lists. This is
 * useful for implementing file drag and drop that will work across different platforms (such as
 * Gnome on Linux).
 * 
 * @author Chris Koeritz
 * @copyright Copyright (c) 2012-$now By University of Virginia
 * @license This file is free software; you can modify and redistribute it under the terms of the
 *          Apache License v2.0: http://www.apache.org/licenses/LICENSE-2.0
 */
@SuppressWarnings("serial")
public class ListTransferable extends Vector<Object> implements Transferable
{
    static private Log logger = LogFactory.getLog(ListTransferable.class);

    public ListTransferable()
    {
    }

    public ListTransferable(Object initial)
    {
        if (initial != null) add(initial);
    }

    public ListTransferable(List<Object> initial)
    {
        if (initial != null) addAll(initial);
    }

    /**
     * create a new flavor. this one understands URI lists, such as: file:///home/fred/arf.txt\r\n
     * file:///etc/inputrc\r\n http://gruntose.com\r\n ...
     */
    private static DataFlavor URIListFlavor;
    static {
        try {
            URIListFlavor = new DataFlavor("text/uri-list;class=java.lang.String");
        } catch (ClassNotFoundException e) {
            logger.error("should never happen", e);
        }
    }
    private static DataFlavor AltURIListFlavor;
    static {
        try {
            AltURIListFlavor = new DataFlavor("text/uri-list;representationclass=java.lang.String");
        } catch (ClassNotFoundException e) {
            logger.error("should never happen", e);
        }
    }

    /**
     * accessors for our special featured flavors of URI lists.
     */
    public static DataFlavor getURIListFlavor1()
    {
        return URIListFlavor;
    }

    public static DataFlavor getURIListFlavor2()
    {
        return AltURIListFlavor;
    }

    /**
     * register the types of transfers that we understand. this is really only the normal java file
     * list and our new URI list.
     */
    protected ArrayList<DataFlavor> FLAVORS = new ArrayList<DataFlavor>(Arrays.asList(
            DataFlavor.javaFileListFlavor, URIListFlavor, AltURIListFlavor));

    /**
     * a function that must be overridden by derived classes if they are not initially seeding the
     * vector of objects that we hold. the caller of this function expects it will populate the
     * vector held here with usable objects.
     */
    public boolean loadDataJustInTime(DataFlavor flavor)
    {
        logger.warn("base loadDataJustInTime.  derived class should have implemented this.");
        return false;
    }

    /**
     * using the set of files that we've been handed, we can do transfers using our two supported
     * flavors.
     */
    public Object getTransferData(DataFlavor flavor) throws UnsupportedFlavorException,
            java.io.IOException
    {
        if (flavor == null) return null;
        if (size() == 0) {
            logger.debug("size was zero, so loading data just in time");
            boolean worked = loadDataJustInTime(flavor);
            if (!worked || (size() == 0)) {
                logger.warn("failed to retrieve data just in time for getTransferData.");
                return null;
            }
        }
        // help from workaround at http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=4899516
        logger.debug("responding to flavor: " + flavor.toString());
        if (flavor.equals(DataFlavor.javaFileListFlavor)) {
            logger.debug("java file list flavor...");
            List<Object> data = new java.util.ArrayList<Object>();
            data.addAll(this);
            return data;
        } else if (flavor.equals(URIListFlavor) || flavor.equals(AltURIListFlavor)) {
            logger.debug("uri list flavor...");
            StringBuilder data = new StringBuilder();
            Iterator<Object> iter = iterator();
            while (iter.hasNext()) {
                Object x = iter.next();
                if (x instanceof File) {
                    File elem = (File) x;
                    data.append(elem.toURI() + "\r\n");
                } else if (x instanceof String) {
                    data.append((String) x + "\r\n");
                } else {
                    logger.debug("did not know how to handle type in transfer: " + x.toString());
                }
            }
            logger.debug("returning URI string: " + data.toString());
            return data.toString();
        } else {
            logger.debug("getTransferData: didn't know how to handle the requested flavor.");
            throw new UnsupportedFlavorException(flavor);
        }
    }

    /**
     * returns the list of all transfer flavors we understand.
     */
    public DataFlavor[] getTransferDataFlavors()
    {
        return (DataFlavor[]) FLAVORS.toArray(new DataFlavor[FLAVORS.size()]);
    }

    /**
     * reports if a particular flavor is handled here.
     */
    public boolean isDataFlavorSupported(DataFlavor flavor)
    {
        if (flavor == null) return false;
        for (int i = 0; i < FLAVORS.size(); i++) {
            if (flavor.equals((DataFlavor) FLAVORS.get(i))) {
                return true;
            }
        }
        logger.debug("failed to find flavor: " + flavor.toString());
        return false;
    }

    /**
     * a helper method that can process transfer data from either a java file list or a URI list.
     */
    @SuppressWarnings("unchecked")
    static public List<Object> extractData(Transferable tran) {
        if (tran == null) return null;
        if (tran.isDataFlavorSupported(DataFlavor.javaFileListFlavor)) {
            logger.debug("extractData seeing java files flavor.");
            try {
                return (List<Object>) tran.getTransferData(DataFlavor.javaFileListFlavor);
            } catch (Throwable cause) {
                logger.error("extractData caught exception for java file list.", cause);
                return null;
            }
        } else if (tran.isDataFlavorSupported(ListTransferable.getURIListFlavor1())
                || tran.isDataFlavorSupported(ListTransferable.getURIListFlavor2())) {
            logger.debug("extractData seeing uri list flavor.");
            try {
                return textURIListToFileList((String) tran.getTransferData(getURIListFlavor1()));
            } catch (Throwable cause) {
                logger.error("extractData caught exception for URI list.", cause);
                return null;
            }
        }
        logger.error("extractData: Transferable did not support known data flavor.");
        return null;
    }

    /**
     * translates the string in "data" into a list of Files.
     * 
     * @param data
     *            a string formatted with possibly multiple URIs separated by CRLF.
     * @return a list of the files as java File objects. many thanks to
     *         http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=4899516
     */
    public static List<Object> textURIListToFileList(String data)
    {
        if (data == null) return null;
        List<Object> list = new ArrayList<Object>(0);
        for (StringTokenizer st = new StringTokenizer(data, "\r\n"); st.hasMoreTokens();) {
            String s = st.nextToken();
            if (s.startsWith("#")) {
                // the line is a comment (as per the RFC 2483)
                continue;
            }
            try {
                java.net.URI uri = new java.net.URI(s);
                java.io.File file = new java.io.File(uri);
                list.add(file);
            } catch (java.net.URISyntaxException e) {
                // this is a malformed URI.
                logger.error("Found a malformed URI of: " + data);
            } catch (IllegalArgumentException e) {
                // the URI is not a valid 'file:' URI
                logger.error("Found invalid 'file:' URI of: " + data);
            }
        }
        return list;
    }

    /**
     * This function will retrieve the file list from a standard file list flavor.
     */
    @SuppressWarnings("unchecked")
    public static List<Object> processStandardFileList(Transferable tran)
    {
        if (tran == null) return null;
        logger.debug("trying java file list flavor.");
        try {
            return (List<Object>) tran.getTransferData(DataFlavor.javaFileListFlavor);
        } catch (Throwable cause) {
            logger.debug("failed to retrieve transfer data for standard java file list flavor.",
                    cause);
        }
        return new ArrayList<Object>();
    }

    /**
     * checks if the transferable is appropriate to try to use as a java Reader.
     */
    public static boolean checkReaderFlavor(Transferable tran)
    {
        if (tran == null) return false;
        DataFlavor[] flavors = tran.getTransferDataFlavors();
        for (int i = 0; i < flavors.length; i++) {
            if (flavors[i].isRepresentationClassReader())
                return true;
        }
        return false;
    }

    /**
     * Use a Reader to handle an incoming transferable.
     */
    public static List<Object> processReaderFlavor(Transferable tran)
    {
        if (tran == null) return null;
        logger.debug("trying URI list flavor.");
        DataFlavor[] flavors = tran.getTransferDataFlavors();
        for (int i = 0; i < flavors.length; i++) {
            if (flavors[i].isRepresentationClassReader()) {
                // it looks like we can work with this flavor just fine.
                logger.debug("found a reader flavor.");
                try {
                    Reader reader = flavors[i].getReaderForText(tran);
                    BufferedReader br = new BufferedReader(reader);
                    return createFileArray(br);
                } catch (Throwable cause) {
                    logger.debug("failed to scan reader for file list.");
                }
            }
        }
        return new ArrayList<Object>();
    }

    private static String ZERO_CHAR_STRING = "" + (char) 0;

    public static List<Object> createFileArray(BufferedReader bReader) {
        if (bReader == null) return null;
        try {
            List<Object> list = new ArrayList<Object>();
            String line = null;
            while ((line = bReader.readLine()) != null) {
                try {
                    // kde seems to append a 0 char to the end of the reader
                    if (ZERO_CHAR_STRING.equals(line))
                        continue;
                    File file = new java.io.File(new java.net.URI(line));
                    list.add(file);
                } catch (Exception ex) {
                    logger.error("Error with " + line + ": " + ex.getMessage());
                }
            }

            return list;
        } catch (IOException ex) {
            logger.error("IOException while working on file list");
        }
        return new ArrayList<Object>();
    }

}
