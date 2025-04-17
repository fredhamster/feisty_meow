#! /usr/bin/env python3

"""

Name   : filename helper
Author : Chris Koeritz
Rights : Copyright (C) 1996-$now by Author

Purpose:

  Functions that manipulate filenames in various helpful ways.

License:
This program is free software; you can redistribute it and/or modify it
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License or (at your option)
any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a
version of the License.  Please send any updates to "fred@gruntose.com".

"""

import os
import path

############################################################################

#hmmm: lots of interesting perl interrupt handling stuff.  do we need any of that?  betting not.
##  #hmmm: make this lower-level, a script that is inherited by all perl scripts.
##
##sub yeti_interrupt_handler {
##  die "caught an interrupt; exiting.";
##}
##
### hook in a ctrl-c catcher, since that seems to be universally needed.
##sub install_interrupt_catcher {
##  $SIG{INT} = 'yeti_interrupt_handler';
##  $SIG{QUIT} = 'yeti_interrupt_handler';
###print "mapped int and quit signals";
##  return 0
##}

############################################################################

# takes an array of filenames (each possibly containing spaces and/or
# wildcards) and resolves it to a useful list of actual files.
def glob_list(original_names: list) -> list:
  """
  takes a set of filenames that may be relative (or really arcane) and globs them into a normal list of filenames.
  """

  to_return = []  # the final form of the name list.
  print("temp list is: " + original_names)

  # scan through the list we're given.
  for entry in original_names:
    print("entry is: " + entry)

    chopped_filename = split_filename(entry)
    print("chopped 0=" + chopped_filename[0])
    print("chopped 1=" + chopped_filename[1])

    if chopped_filename[0] == "." or chopped_filename[0] == "..":
      # add the simple directory name into the list.
      to_return.append(chopped_filename[0])
      continue

    if chopped_filename[1] == ".":
      # add a directory that didn't have more pattern attached.
      to_return.append(chopped_filename[0])
      continue

    # get all the contents from the directory (both subdirectories and files).
    files_found = os.listdir(chopped_filename[0])

    # a dictionary of patterns to find in filenames and their safer replacements.
    replacement_patterns = [ 
      r's/\.': r'\\.',  # replace periods with escaped ones.
      r's/\*': r'.*',   # replace asterisks with dot star.
      r's/\+': r'\\+',  # escape plusses.
      r's/\?': r'\\?',  # escape question marks.
      r's/\|': r'\\|',  # escape pipe char.
      r's/\$': r'\\\$', # escape dollar sign.
      r's/\[': r'\\[',  # escape open bracket.
      r's/\]': r'\\]',  # escape close bracket.
      r's/\(': r'\\(',  # escape open quote.
      r's/\)': r'\\)',  # escape close quote.
      r's/\{': r'\\{',  # escape open curly bracket.
      r's/\}': r'\\}'   # escape close curly bracket.
    ]

    for possible_name in files_found:
      match = chopped_filename[1]

      for seek, replacer in replacement_patterns:
        match = re.sub(seek, replacer, match)

      # make sure that we match against the whole string.
      match = "^" . match . "\$"
      print("possibname is '" + possible_name + "':")
      if re.search(match, possible_name):
        # this one matches, so add it to our list.
        to_return.append(chopped_filename[0] + possible_name)
        print("got a match on:" + chopped_filename)

  return to_return

############################################################################

#hmmm: unscanned below here-- monsters !!!

# reports if two file names are the same file.
def same_file(file1: str, file2: str):
uhhhh
  ($dev1, $ino1, $junk1) = stat $file1;
  ($dev2, $ino2, $junk2) = stat $file2;

  return ($dev1 == $dev2) && ($ino1 == $ino2);
}

############################################################################

# splits a filename into a directory and file specification.

sub split_filename {
  local($chewed_name) = &remove_trailing_slashes(@_);
  $chewed_name = &canonicalize($chewed_name);
  $chewed_name = &patch_name_for_pc($chewed_name);
  if ($chewed_name =~ /\//) {
    # there's a slash in there.
    local($directory_part) = $chewed_name;
    $directory_part =~ s/^(.*\/)[^\/]*$/\1/;
    local($file_part) = $chewed_name;
    $file_part =~ s/^.*\/([^\/]*)$/\1/;
    if ($file_part eq "") {
      # if there was no file specification, just add a non-matching spec.
      $file_part = ".";
    }
    return ($directory_part, $file_part);
  } elsif ($chewed_name eq ".") {
    return (".", "");
  } elsif ($chewed_name eq "..") {
    return ("..", "");
  } else {
    # no slash in this name, so we fix that and also make sure we match
    # the whole name.
    return ("./", $chewed_name);
  }
}

############################################################################

# returns the base part of the filename; this omits any directories.

sub basename {
  local(@parts) = &split_filename(@_);
  return $parts[1];
}

# returns the directory part of the filename.

sub dirname {
  local(@parts) = &split_filename(@_);
  return $parts[0];
}

# returns the extension found on the filename, if any.
sub extension {
  local($base) = &basename(@_);
#printf "base is $base";
  local($found) = -1;
  for (local($i) = length($base) - 1; $i >= 0; $i--) {
#printf "char is " . substr($base, $i, 1) . "";
    if (substr($base, $i, 1) eq '.') {
      $found = $i;
#printf "got period found is $found";
      last;
    }
  }
  if ($found >=0) {
    return substr($base, $found, length($base) - $found);
  }
  return "";  # no extension seen.
}

# returns the portion of the filename without the extension.
sub non_extension {
  local($full) = &remove_trailing_slashes(@_);
  $full = &canonicalize($full);
  $full = &patch_name_for_pc($full);
  local($ext) = &extension($full);
  local($to_remove) = length($ext);
  return substr($full, 0, length($full) - $to_remove);
}

############################################################################

# removes all directory slashes (either '/' or '\') from the end of a string.

sub remove_trailing_slashes {
  local($directory_name) = @_;
  # start looking at the end of the string.
  local($inspection_point) = length($directory_name) - 1;
  while ($inspection_point > 0) {
    # examine the last character in the string to see if it's a slash.
    local($final_char) = substr($directory_name, $inspection_point, 1);
    # leave the loop if it's not a slash.
    if ( ($final_char ne "/") && ($final_char ne "\\") ) { last; }
    chop($directory_name);  # remove the slash.
    $inspection_point--;  # check the new last character.
  }

  return $directory_name;
}

############################################################################

# returns the proper directory separator for this platform.  this requires
# an environment variable called "OS" for non-Unix operating systems.  the
# valid values for that are shown below.

sub directory_separator {
  if ( ($OS eq "Windows_NT") || ($OS eq "Windows_95") 
      || ($OS eq "DOS") || ($OS eq "OS2") ) { return "\\"; }
  else { return "/"; }
}

############################################################################

# these mutate the directory slashes in a directory name.

# the one we use most frequently; it uses the unix slash.
sub canonicalize {
  return &canonicalizer(@_, "/");
}

# one that turns names into the style native on the current platform.
sub native_canonicalize {
  return &canonicalizer(@_, &directory_separator());
}

# one that explicitly uses pc style back-slashes.
sub pc_canonicalize {
  return &canonicalizer(@_, "\\");
}

# one that explicitly does unix style forward slashes.
sub unix_canonicalize {
  return &canonicalizer(@_, "/");
}

# this more general routine gets a directory separator passed in.  it then
# replaces all the separators with that one.
sub canonicalizer {
  local($directory_name) = $_[0];
  local($dirsep) = $_[1];

#print "old dir name is \"$directory_name\"";
  
  if ($OS =~ /win/i) {
#somewhat abbreviated check; only catches windoze systems, not dos or os2.
    # IS_MSYS is calculated by feisty meow scripts startup; it will be
    # non-empty if this is the msys tool kit.
    if (length($IS_MSYS) > 0) {
      # msys utilities version (http://www.mingw.org)
      $directory_name =~ s/^(.):[\\\/](.*)$/\/\1\/\2/;
    } else {
      # cygwin utilities version (http://www.cygwin.com)
      $directory_name =~ s/^(.):[\\\/](.*)$/\/cygdrive\/\1\/\2/;
    }
#print "new dir name is \"$directory_name\"";
  }

  # turn all the non-default separators into the default.
  for (local($j) = 0; $j < length($directory_name); $j++) {
    if ( (substr($directory_name, $j, 1) eq "\\") 
        || (substr($directory_name, $j, 1) eq "/") ) {
      substr($directory_name, $j, 1) = $dirsep;
    }
  }
  # remove all occurrences of double separators except for the first
  # double set, which could be a UNC filename.
  local($saw_sep) = 0;
  for (local($i) = 1; $i < length($directory_name); $i++) {
    # iterate through the string looking for redundant separators.
    if (substr($directory_name, $i, 1) eq $dirsep) {
      # we found a separator character.
      if ($saw_sep) {
        # we had just seen a separator, so this is two in a row.
        local($head, $tail) = (substr($directory_name, 0, $i - 1),
            substr($directory_name, $i, length($directory_name) - 1));
        $directory_name = $head . $tail;
          # put the name back together without this redundant character.
        $i--;  # skip back one and try again.
      } else {
        # we have now seen a separator.
        $saw_sep = 1;
      }
    } else {
      # this character was not a separator.
      $saw_sep = 0;
    }
  }
  if ($directory_name =~ /^.:$/) {
    # fix a dos style directory that's just X:, since we don't want the
    # current directory to be used on that device.  that's too random.
    # instead, we assume they meant the root of the drive.
    $directory_name = $directory_name . "/";
  }
  return $directory_name;
}

############################################################################

# fixes a PC directory name if it is only a drive letter plus colon.

sub patch_name_for_pc {
  local($name) = @_;
#print "name=$name";
  if (length($name) != 2) { return $name; }
  local($colon) = substr($name, 1, 1);
#print "colon=$colon";
  # check whether the string needs patching.
  if ($colon eq ":") {
    # name is currently in feeble form of "X:"; fix it.
    $name = $name . '/';
  }
#print "returning=$name";
  return $name;
}

############################################################################

# tells whether a filename is important or not.  the unimportant category
# can usually be safely ignored or deleted.

sub important_filename {
  local($name) = &basename($_[0]);
  
  # these are endings that we consider unimportant.  where a caret is used
  # at the front, we will match only the whole string.  double slashes are
  # used before periods to ensure we match a real period character.
  local(@junk_files) = ("~", "^\\.#.*", "^\\._.*", "\\.aps", "\\.bak",
      "^binaries", "^bin.ant", "^bin.eclipse",
      "\\.clw", "^cpdiff_tmp\\.txt", "^\\.ds_store", "^diffs\\.txt",
      "^diff_tmp\\.txt", "\\.dsp", "\\.dsw", "\\.gid", "gmon\\.out", "\\.isr",
      "^isconfig\\.ini", "\\.log", "^manifest.txt", "^obj",
      "\\.obj", "\\.output", "\\.plg", "^RCa.*", "^Release", "\\.res",
      "\\.sbr", ".*scc", "^Setup\\.dbg", "^Setup\\.inx",
      "^Setup\\.map", "^Setup\\.obs", "^Selenium_.*Login.html",
      "\\.stackdump", "^string1033\\.txt", "\\.suo", "\\.swp",
      "^thumbs.db", "[a-zA-Z0-9]\\.tmp", "^trans\\.tbl", "\\.user", "_version\\.h",
      "_version\\.rc", "^waste", "\\.ws4", "\\.wsm");

  foreach $temp (@junk_files) {
    $temp = $temp . '$';
    if ($name =~ /${temp}/i) { return 0; }
      # we hit a match on it being unimportant.
  }

  return 1;  # anything else is considered important.
}

############################################################################

sub sanitize_name {
  return &patch_name_for_pc
      (&remove_trailing_slashes
          (&canonicalize(@_)));
}

############################################################################

sub get_drive_letter {
  local($path) = @_;
  if (substr($path, 0, 1) =~ /[a-zA-Z]/) {
    if (substr($path, 1, 1) eq ":") { return substr($path, 0, 1); }
  }
  return "";
}

############################################################################

sub remove_drive_letter {
  local($path) = @_;
  if (substr($path, 0, 1) =~ /[a-zA-Z]/) {
    if (substr($path, 1, 1) eq ":") { return substr($path, 2); }
  }
  return $path;
}

############################################################################

# these return their argument with the case flipped to lower or upper case.

sub lower {
  local($name) = @_;
  $name =~ tr/A-Z/a-z/;
  return $name;
}

sub upper {
  local($name) = @_;
  $name =~ tr/a-z/A-Z/;
  return $name;
}

############################################################################

# recursively deletes a directory that is passed as the single parameter.
# from http://developer.novell.com/wiki/index.php/Recursive_Directory_Remove
sub recursive_delete {
  my $dir;
  foreach $dir (@_) {
    if ( -f "$dir" ) {
print "this is not a dir: $dir  => should whack it here?";
return;
    }

    local *DIR;
    # if we can't open the dir, just skip to the next one.
    opendir DIR, $dir or next;
    while ($_ = readdir DIR) {
      next if /^\.{1,2}$/;
      my $path = "$dir/$_";
      unlink $path if -f $path;
      recursive_delete($path) if -d $path;
    }
    closedir DIR;
    rmdir $dir or print "error - $!";
  }
}

############################################################################

# finds any directories under the arguments, which can be a list of directories.
sub find_directories {
  my @dirs_found = ();
  my $dir;
  foreach $dir (@_) {
    local *DIR;
    # if we can't open the dir, just skip to the next one.
    opendir DIR, $dir or next;
    while ($_ = readdir DIR) {
      # skip if it's current or parent dir.
      next if /^\.{1,2}$/;
      my $path = "$dir/$_";
      # skip if this entry is not itself a directory.
      next if ! -d $path;
      push @dirs_found, $path;
    }
    closedir DIR;
  }
  return @dirs_found;
}

############################################################################

# given a list of paths, this returns an array of all the filenames found therein.
sub find_files {
  my @files_found = ();
  my $dir;
  foreach $dir (@_) {
    if (-f $dir) {
      # that's actually just a file, so add it.
      push @files_found, $dir;
      next;
    }
    local *DIR;
    # if we can't open the dir, just skip to the next one.
    opendir DIR, $dir or next;
    while ($_ = readdir DIR) {
      # skip if it's current or parent dir.
      next if /^\.{1,2}$/;
      my $path = "$dir/$_";
      # skip if this entry is not a file.
      next if ! -f $path;
      push @files_found, $path;
    }
    closedir DIR;
  }
  return @files_found;
}

############################################################################

# finds all directories starting at a particular directory and returns them
# in an array.  does not include the starting directory.
sub recursive_find_directories {
  # first find all the directories within the parameters.
  my @toplevel = find_directories(@_);

  my @to_return;
  push(@to_return, @toplevel);

  # return the composition of the list we found here plus any directories under those.
  # we only recurse if there's something to chew on in our directory list.
  # otherwise, we've hit the bottom of that tree.
  if (scalar @toplevel > 0) {
    my @subs_found = recursive_find_directories(@toplevel);
    push(@to_return, @subs_found);
  }
  return @to_return;
}

############################################################################

1;

