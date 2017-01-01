package org.feistymeow.textual;

import java.util.HashSet;
import java.util.Set;

public class SimpleDictionary extends HashSet<String>
//or alternatively, BinaryTree<String>
//=> what is BST implem for java!  is it balanced?
{
  public SimpleDictionary (Set<String> words) {
    addAll(words);
    computeLongestWord();
  }
  
  public SimpleDictionary (String words[]) {
	for (String word : words) {
		add(word);
	}
    computeLongestWord();
  }

  public int computeLongestWord() {
    previouslyComputedLongestWord = 1;
    
    //hmmm: iterate on set to find longest.
    
//kludge implem placeholder.
previouslyComputedLongestWord = 100;
	return previouslyComputedLongestWord;
  }
  
  public boolean lookup(String toFind) {
    return contains(toFind);
  }

  public int longestWord() {
    return previouslyComputedLongestWord;
  }

  int previouslyComputedLongestWord;
}

