#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : differ (with recursion support)                                   #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Compares files in a directory hierarchy to files in another hierarchy.   #
#  Any files with the same name will be compared and files that exist in one  #
#  but not the other are reported.                                            #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

require "diff_lib.pl";
require "filename_helper.pl";

&install_interrupt_catcher;

# check that we received at least one parameter.
if ($#ARGV < 0) {
  &print_instructions;
  exit 1;
}

# get the two directories from the command line.
local($destination) = $ARGV[0];
local($source) = ".";
if ($#ARGV > 0) {
  # get the location they provided.
  $source = $ARGV[1];
}

# make the names a little more manipulable.
$source = &sanitize_name($source);
# print "source is now: $source\n";
$destination = &sanitize_name($destination);
# print "dest is now: $destination\n";

# call the routine that does all the work.
&recurse_dirs($source, $destination);

exit 0;

############################################################################

sub print_instructions
{
  print "
differ:\n
This program requires at least one directory name as a parameter.  This is
the location to use as the target directory.  The second parameter, if
provided, is the location to use as the source directory.  The recursive
differ script will traverse the source hierarchy and attempt to compare
the contents of the directories in that hierarchy against the destination
hierarchy.  This will fail where the hierarchies have different directory
structures.  But where the directories exist in both hierarchies, their
contents are compared against each other and documented in the output.
";
}

############################################################################

sub recurse_dirs
{
  local($src, $dest) = @_;

  if ($src =~ /\/CVS$/) { return; }  # skip comparing repositories.

  if (! -d $src) {
    print "$src is not a directory.\n";
    return;
  } elsif (-l $src) {
    return;
  }
  
#  print "recurse_dirs: source is $src and destination is $dest.\n";

  opendir(SRC_DIR, $src);
  local(@source_list) = readdir(SRC_DIR);
  closedir(SRC_DIR);
  opendir(DEST_DIR, $dest);
  local(@dest_list) = readdir(DEST_DIR);
  closedir(DEST_DIR);

##  if (&same_file($src, $dest)) {
##    # these appear to be the same directory.  we shouldn't recurse in.
##print "found same dirs!  $src and $dest\n";
##  }

  # now actually call the differ.  this is a prefix traveral of the tree.
  if (-l $src) {
    return;
  } elsif (-d $dest) {
    # remember that the destination is actually the first parameter for
    # diff_dirs, although the source is first for differ...
#    print "diffing src=$src against dest=$dest\n";
    &diff_dirs($dest, $src);
  } else {
    print "$break_line\n";
    print "Source exists at \"$src\", but target does not.\n";
  }

  # iterate through the directory.
  local($name);
  foreach $name (@source_list) {
    local($compare_name) = $dest . "/" . $name;
#    print "name is $name and to compare is $compare_name.\n";
    local($new_name) = $src . "/" . $name;
    if ( (-d $new_name) && ($name ne ".") && ($name ne "..")
        && ($name ne ".svn") ) {
#      print "recursing on: source $name and destination $compare_name.\n";
      &recurse_dirs($new_name, $compare_name);
    }
  }
}

