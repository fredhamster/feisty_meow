package org.feistymeow.dragdrop;

import java.awt.*;
import java.io.*;
import java.util.*;
import java.util.List;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.log4j.PropertyConfigurator;

/**
 * A demo of the DragonDropHandler being used with a JList. Much love to the internet for lots of
 * examples.
 * 
 * @author Chris Koeritz
 * @copyright Copyright (c) 2012-$now By University of Virginia
 * @license This file is free software; you can modify and redistribute it under the terms of the
 *          Apache License v2.0: http://www.apache.org/licenses/LICENSE-2.0
 */
@SuppressWarnings("serial")
public class dragdrop_list_test extends JFrame implements ListSelectionListener
{
    private DraggableDroppableList list;
    private JTextField fileName;
    static private Log logger = LogFactory.getLog(dragdrop_list_test.DraggableDroppableList.class);

    public dragdrop_list_test(String startPath)
    {
        super("dragdrop_test");

        // Create the list and put it in a scroll pane
        list = new DraggableDroppableList();
        DefaultListModel listModel = (DefaultListModel) list.getModel();
        list.setCellRenderer(new CustomCellRenderer());
        list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        list.setSelectedIndex(0);
        list.addListSelectionListener(this);
        JScrollPane listScrollPane = new JScrollPane(list);

        String dirName = startPath + "/"; // make sure we think of it as a directory.
        String filelist[] = new File(dirName).list();
        for (int i = 0; i < filelist.length; i++) {
            String thisFileSt = dirName + filelist[i];
            File thisFile = new File(thisFileSt);
            // skip directories for now.
            if (thisFile.isDirectory())
                continue;
            // skip dot files.
            if (filelist[i].startsWith("."))
                continue;
            try {
                listModel.addElement(makeNode(thisFile.getName(), thisFile.toURI().toURL()
                        .toString(), thisFile.getAbsolutePath()));
            } catch (java.net.MalformedURLException e) {
            }
        }

        fileName = new JTextField(50);
        list.setSelectedIndex(0);
        int sel_index = list.getSelectedIndex();
        Object obj_at_index = listModel.getElementAt(sel_index);
        String name = obj_at_index.toString();
        fileName.setText(name);

        // Create a panel that uses FlowLayout (the default).
        JPanel buttonPane = new JPanel();
        buttonPane.add(fileName);

        Container contentPane = getContentPane();
        contentPane.add(listScrollPane, BorderLayout.CENTER);
        contentPane.add(buttonPane, BorderLayout.NORTH);
    }

    public void valueChanged(ListSelectionEvent e)
    {
        if (e.getValueIsAdjusting() == false) {
            fileName.setText("");
            if (list.getSelectedIndex() != -1) {
                String name = list.getSelectedValue().toString();
                fileName.setText(name);
            }
        }
    }

    private static Hashtable<String, String> makeNode(String name, String url, String strPath)
    {
        Hashtable<String, String> hashtable = new Hashtable<String, String>();
        hashtable.put("name", name);
        hashtable.put("url", url);
        hashtable.put("path", strPath);
        return hashtable;
    }

    public class DraggableDroppableList extends JList implements IDragonDropDataProvider
    {
        public DraggableDroppableList()
        {
            setModel(new DefaultListModel());
            setTransferHandler(new DragonTransferHandler(this));
            setDragEnabled(true);
        }

        @Override
        public boolean consumeDropList(List<Object> fileSet, Point location)
        {
            logger.debug("into consume dropped files, file set is:");
            for (int i = 0; i < fileSet.size(); i++) {
                logger.debug("   " + ((File) fileSet.get(i)).getPath());
            }
            return true;
        }

        @Override
        public List<Object> provideDragList()
        {
            ArrayList<Object> toReturn = new ArrayList<Object>();
            if (getSelectedIndex() == -1)
                return toReturn;
            Object obj = getSelectedValue();
            if (obj != null) {
                @SuppressWarnings("unchecked")
                Hashtable<String, String> table = (Hashtable<String, String>) obj;
                toReturn.add(new File((String) table.get("path")));
            }
            return toReturn;
        }
    }

    public class CustomCellRenderer implements ListCellRenderer
    {
        DefaultListCellRenderer listCellRenderer = new DefaultListCellRenderer();

        public Component getListCellRendererComponent(JList list, Object value, int index,
                boolean selected, boolean hasFocus)
        {
            listCellRenderer.getListCellRendererComponent(list, value, index, selected, hasFocus);
            listCellRenderer.setText(getValueString(value));
            return listCellRenderer;
        }

        private String getValueString(Object value)
        {
            String returnString = "null";
            if (value != null) {
                if (value instanceof Hashtable<?, ?>) {
                    @SuppressWarnings("unchecked")
                    Hashtable<String, String> h = (Hashtable<String, String>) value;
                    String name = (String) h.get("name");
                    String url = (String) h.get("url");
                    returnString = name + " ==> " + url;
                } else {
                    returnString = "X: " + value.toString();
                }
            }
            return returnString;
        }
    }

    public static void main(String s[])
    {
        PropertyConfigurator.configure("log4j.properties");
        // starting with user's personal area.
        String homedir = System.getenv("HOME");
        if ((homedir == null) || (homedir.length() == 0)) {
            // fall back to the root if no home directory.
            homedir = "/";
        }
        JFrame frame = new dragdrop_list_test(homedir);
        frame.addWindowListener(new WindowAdapter()
        {
            public void windowClosing(WindowEvent e)
            {
                System.exit(0);
            }
        });
        frame.pack();
        frame.setVisible(true);
    }
}
