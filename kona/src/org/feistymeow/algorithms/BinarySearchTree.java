package org.feistymeow.algorithms;


//hmmm: move to better folder path.

// inspired by http://pages.cs.wisc.edu/~cs367-1/readings/Binary-Search-Trees/ 

/**
 * The type of node held in our binary search tree.
 */
class TreeNode<K, V> 
{
    private K key;
    private V value;
    private TreeNode<K, V> left, right;
 
    public TreeNode(K key, V value, TreeNode<K, V> left, TreeNode<K, V> right) {
        this.key = key;
        this.value = value;
        this.left = left;
        this.right = right;
    }
   
    public K getKey() { return key; }
    public V getValue() { return value; }
    

    public TreeNode<K, V> getLeft() { return left; }
    public TreeNode<K, V> getRight() { return right; }
 
    public void setKey(K newK) { key = newK; }
    public void setLeft(TreeNode<K, V> newL) { left = newL; }
    public void setRight(TreeNode<K, V> newR) { right = newR; }

    public void setValue(V newV) { value = newV; }
}

class DuplicateException extends RuntimeException
{
	private static final long serialVersionUID = 1L;
	DuplicateException() {}
}

/**
 * A binary search tree implementation.
 * 
 * Insert, Delete and Search are done in O(n log(n)) time.
 */
public class BinarySearchTree<K extends Comparable<K>, V> 
{
    private TreeNode<K, V> root; // ptr to the root of the BinarySearchTree
 
    public BinarySearchTree() { root = null; } 
    
    
    // add key and associated value to this BinarySearchTree;
    // error if key is already there
    public void insert(K key, V value) throws DuplicateException {
        root = insert(root, value, key);
    }  

    private TreeNode<K, V> insert(TreeNode<K, V> n, V value, K key) throws DuplicateException {
        if (n == null) {
            return new TreeNode<K, V>(key, value, null, null);
        }
         
        if (n.getKey().equals(key)) {
            throw new DuplicateException();
        }
        
        if (key.compareTo(n.getKey()) < 0) {
            // add key to the left subtree
            n.setLeft( insert(n.getLeft(), value, key) );
            return n;
        }
        
        else {
            // add key to the right subtree
            n.setRight( insert(n.getRight(), value, key) );
            return n;
        }
    }
      
    // remove the node containing key from this BinarySearchTree if it is there;
    // otherwise, do nothing
    public void delete(K key) {
        root = delete(root, key);
    }

    
    private K smallest(TreeNode<K, V> n)
 // precondition: n is not null
 // postcondition: return the smallest value in the subtree rooted at n

 {
     if (n.getLeft() == null) {
         return n.getKey();
     } else {
         return smallest(n.getLeft());
     }
 }

    private TreeNode<K, V> delete(TreeNode<K, V> n, K key) 
    {
        if (n == null) {
            return null;
        }
        
        if (key.equals(n.getKey())) {

        	if (n.getLeft() == null && n.getRight() == null) {
                return null;
            }
            if (n.getLeft() == null) {
                return n.getRight();
            }
            if (n.getRight() == null) {
                return n.getLeft();
            }
           
            // if we get here, then n has 2 children
            K smallVal = smallest(n.getRight());
            n.setKey(smallVal);
            n.setRight( delete(n.getRight(), smallVal) );
            return n; 
        }
        
        else if (key.compareTo(n.getKey()) < 0) {
            n.setLeft( delete(n.getLeft(), key) );
            return n;
        }
        
        else {
            n.setRight( delete(n.getRight(), key) );
            return n;
        }
    }

      
    // if key is in this BinarySearchTree, return its associated value; otherwise, return null
    public boolean lookup(K key) {
        return lookup(root, key);
    }

    private boolean lookup(TreeNode<K, V> n, K key) {
        if (n == null) {
            return false;
        }
        
        if (n.getKey().equals(key)) {
            return true;
        }
        
        if (key.compareTo(n.getKey()) < 0) {
            // key < this node's key; look in left subtree
            return lookup(n.getLeft(), key);
        }
        
        else {
            // key > this node's key; look in right subtree
            return lookup(n.getRight(), key);
        }
    }
     
//    // print the values in this BinarySearchTree in sorted order (to p)
//    public void print(PrintStream p) {
//    	
//    }
}

