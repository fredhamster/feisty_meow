package org.feistymeow.textual;

import java.util.HashSet;
import java.util.Set;

public class WordBreakFinder
{
  public WordBreakFinder(SimpleDictionary lookups) {
    _lookups = lookups;
  }

  public Set<String> findStringsWithGoodSpacing(String orig) {
    HashSet<String> toReturn = new HashSet<String>();
    int startingChunk = Math.min(orig.length(), _lookups.longestWord());
    for (int i = startingChunk; i >= 1; i--) {
      String first = orig.substring(0, i);
      if (!_lookups.lookup(first)) {
        // fail fast.  this length chunk of the word doesn't match.
        continue;
      }
      String second = orig.substring(i, orig.length());
      // first part of string is listed in dictionary.  we can trivially add it if the other part is empty.
      if (second.length() == 0) {
    	  toReturn.add(first);
    	  continue;
      }
      Set<String> matches = findStringsWithGoodSpacing(second);
      if (matches != null) {
        for (String matched : matches) {
           toReturn.add(first + " " + matched);
        }
      }
    }
    if (toReturn.isEmpty()) return null;
    return toReturn;
  }

  SimpleDictionary _lookups;  

  public static void main(String argv[]) {
    String smallWordList[] = {
       "hops", "barley", "malt", "beer", "turnip"
    };
    SimpleDictionary lookups = new SimpleDictionary(smallWordList);
    WordBreakFinder finder = new WordBreakFinder(lookups);
    
    String toBreak = "maltbeerbeerturniphops";
    Set<String> matches =  finder.findStringsWithGoodSpacing(toBreak);
    if (matches != null) {
      System.out.println("matches found:");
      for (String match : matches) {
         System.out.println(match);
      }
    } else {
      System.out.println("ERROR: failed to find matches!");
    }

    toBreak = "maltbeerbeturniphops";
    matches =  finder.findStringsWithGoodSpacing(toBreak);
    if (matches != null) {
      System.out.println("ERROR: matches found:");
      for (String match : matches) {
         System.out.println(match);
      }
    } else {
      System.out.println("found no matches, which is correct.");
    }
  }

}



