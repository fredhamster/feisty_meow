package org.feistymeow.dragdrop;

import java.awt.*;
import java.io.*;
import java.util.*;
import java.util.List;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.DefaultTreeSelectionModel;
import javax.swing.tree.MutableTreeNode;
import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.TreePath;
import javax.swing.tree.TreeSelectionModel;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.log4j.PropertyConfigurator;

/**
 * A demo of the DragonDropHandler being used with a JTree.
 * 
 * @author Chris Koeritz
 * @copyright Copyright (c) 2012-$now By University of Virginia
 * @license This file is free software; you can modify and redistribute it under the terms of the
 *          Apache License v2.0: http://www.apache.org/licenses/LICENSE-2.0
 */
@SuppressWarnings("serial")
public class dragdrop_tree_test extends JFrame implements TreeSelectionListener
{
    private DraggableDroppableTree larch;
    private JTextField fileName;
    static private Log logger = LogFactory.getLog(dragdrop_tree_test.DraggableDroppableTree.class);

    public dragdrop_tree_test(String startPath)
    {
        super("dragdrop_test");

        // create the tree, configure it to show our hashtable nodes, and put it in
        // a scroll pane.
        larch = new DraggableDroppableTree(startPath);
        DefaultTreeModel treeModel = (DefaultTreeModel) larch.getModel();
        larch.setCellRenderer(new CustomCellRenderer());
        TreeSelectionModel selmod = new DefaultTreeSelectionModel();
        selmod.setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
        larch.setSelectionModel(selmod);
        larch.addTreeSelectionListener(this);
        JScrollPane listScrollPane = new JScrollPane(larch);
        // get the files that live in the specified directory.
        String dirName = startPath + "/"; // make sure we think of it as a
                                          // directory.
        String filelist[] = new File(dirName).list();
        MutableTreeNode root_node = (MutableTreeNode) treeModel.getRoot();
        if (root_node == null) {
            logger.error("something is not right about tree.  has null root.");
            System.exit(1);
        }
        // load up the tree with the files in the directory they passed.
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
                // need to trap exceptions from the URI/URL functions.
                DefaultMutableTreeNode newNode = new DefaultMutableTreeNode(makeNode(
                        thisFile.getName(), thisFile.toURI().toURL().toString(),
                        thisFile.getAbsolutePath()));
                treeModel.insertNodeInto(newNode, root_node, root_node.getChildCount());
            } catch (java.net.MalformedURLException e) {
                logger.warn("caught an exception while trying to process path: "
                        + thisFile.getAbsolutePath());
            }
        }

        // set our status bar to have the current path info.
        fileName = new JTextField(50);
        // select the root.
        larch.setSelectionPath(larch.getPathForRow(0));

        // pop out all the nodes.
        larch.expandAll();

        // Create a panel that uses FlowLayout (the default).
        JPanel buttonPane = new JPanel();
        buttonPane.add(fileName);

        Container contentPane = getContentPane();
        contentPane.add(listScrollPane, BorderLayout.CENTER);
        contentPane.add(buttonPane, BorderLayout.NORTH);
    }

    @SuppressWarnings("unchecked")
    // given a mutable tree node, this will fetch out the embedded hash table.
    Hashtable<String, String> NodeToTable(Object node)
    {
        if (!(node instanceof DefaultMutableTreeNode))
            return null;
        Object content = ((DefaultMutableTreeNode) node).getUserObject();
        if (content != null) {
            if (content instanceof Hashtable<?, ?>) {
                try {
                    return (Hashtable<String, String>) content;
                } catch (Throwable cause) {
                    logger.error("failed to cast our tree node to a hashtable.");
                }
            }
        }
        return null;
    }

    public void valueChanged(TreeSelectionEvent e)
    {
        fileName.setText("");
        TreePath sel_path = larch.getSelectionPath();
        if (sel_path != null) {
            Hashtable<String, String> table = NodeToTable(sel_path.getLastPathComponent());
            if (table != null) {
                String name = (String) table.get("name");
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

    public class DraggableDroppableTree extends JTree implements IDragonDropDataProvider
    {
        public DraggableDroppableTree(String startPath)
        {
            String url = "";
            try {
                url = new File(startPath).toURI().toURL().toString();
            } catch (Throwable cause) {
                logger.warn("failed to calculate URL for " + startPath);
            }
            setModel(new DefaultTreeModel(new DefaultMutableTreeNode(
                    makeNode("top", url, startPath))));
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
            TreePath tsp = getSelectionPath();
            if (tsp == null)
                return toReturn;
            logger.debug("got the path...");
            Hashtable<String, String> table = NodeToTable(tsp.getLastPathComponent());
            if (table != null) {
                toReturn.add(new File(table.get("path")));
            }
            return toReturn;
        }

        public void expandAll()
        {
            int row = 0;
            while (row < getRowCount()) {
                expandRow(row);
                row++;
            }
        }

    }

    public class CustomCellRenderer implements TreeCellRenderer
    {
        DefaultTreeCellRenderer defRend = new DefaultTreeCellRenderer();

        private String getValueString(Object value)
        {
            String returnString = "empty";
            Hashtable<String, String> table = NodeToTable(value);
            if (table != null) {
                returnString = table.get("name") + " -> " + table.get("url");
            } else {
                returnString = "??: " + value.toString();
            }
            return returnString;
        }

        @Override
        public Component getTreeCellRendererComponent(JTree tree, Object value, boolean selected,
                boolean expanded, boolean leaf, int row, boolean hasFocus)
        {
            defRend.getTreeCellRendererComponent(tree, value, selected, expanded, leaf, row,
                    hasFocus);
            defRend.setText(getValueString(value));
            return defRend;
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
        JFrame frame = new dragdrop_tree_test(homedir);
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
