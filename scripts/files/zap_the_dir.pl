#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : zap_the_dir                                                       #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Processes arguments that are expected to be directories.  The directory  #
#  has all the known junk files strained out of it and then it is removed     #
#  if it is empty.                                                            #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

require "filename_helper.pl";

############################################################################

sub remove_whackables {
  local($from_dir) = @_;

  # open the directory and grab all the files out.
  opendir CHECK, $from_dir;
  local(@files) = readdir(CHECK);
  closedir CHECK;

  # iterate over the potentially whackable files.
  local($fname) = "";
  foreach $fname (@files) {
#print "filename is $fname\n";
    # check if this one matches one of our patterns.
    if (! &important_filename($fname)) {
      # it's a junk file; whack it.
      $fname = $from_dir . '/' . $fname;
#print "whacking: $fname.\n";
      unlink $fname;
      if (-f "$fname") {
        print "cleaning file: $fname\n";
        &recursive_delete($fname);
      } else {
        print "skipping item rather than deleting: $fname\n";
      }
    }
  }
}

############################################################################

sub zap_the_dir {
  local(@zap_list) = @_;
  local($to_zap) = "";
  foreach $to_zap (@zap_list) {
    chomp $to_zap;

#print "to_zap is $to_zap\n";
    if (! -d $to_zap) {
      print "$to_zap is not a directory.\n";
      next;
    }

    if ($to_zap =~ /^.*\.svn.*$/) {
#print "hey found a .svn dir!  skipping.\n";
      next;
    }

    &remove_whackables($to_zap);
    opendir WHACK, $to_zap;
    @files_left = readdir(WHACK);
    closedir WHACK;
  
    local($name) = "";
    local($seen_anything) = "";
    foreach $name (@files_left) {
      # check if directory has nothing but the two directory entries in it.
      if ( ($name ne ".") && ($name ne "..") ) {
        if ($to_zap =~ /^.*\.svn.*$/) {
          print "not empty: \"$to_zap/$name\"\n";
        }
        $seen_anything = "true";
        break; 
      }
    }

    if (length($seen_anything)) {
      print "not empty: \"$to_zap\"\n";
    } else {
      # this should now work, if the directory is empty.
      if (!rmdir $to_zap) {
        print "still in use: \"$to_zap\"\n";
        next;
      }
    }
  }
}

############################################################################

sub recursively_zap_dirs {
  local(@zap_dirs) = @_;
  local($zap_dir) = "";
  foreach $zap_dir (@zap_dirs) {
#hmmm: can we use a perl utility to do the directory recursion?
    local(@dirnames) = `find \"$zap_dir\" -depth -mindepth 1 -type d`;
#print "dirnames are:\n@dirnames\n";
    &zap_the_dir(@dirnames);
    &zap_the_dir($zap_dir);
  }
}

############################################################################

