#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : safedel                                                           #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    This program moves all of the files specified on the command line        #
#  into the temporary storage directory, rather than just deleting them.      #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

require Cwd;
require "ctime.pl";
require "filename_helper.pl";
require "inc_num.pl";
require "importenv.pl";
require "zap_the_dir.pl";

#hmmm: need a usage statement.

if ($#ARGV < 0) {
  die "Too few arguments to command.";
}

$DEV_NULL = "&> /dev/null";
if ($OS eq "UNIX") {
  $FIND_ENDING = "';'";
  $zip = "zip -y ";
} elsif ( ($OS eq "DOS") || ($OS eq "Windows_95")
    || ($OS eq "Windows_98") || ($OS eq "Windows_NT") ) {
  $FIND_ENDING = "';'";
  $zip = "zip ";
} else {
  die "The Operating System variable (OS) is not set.\n";
}

# The zip program has slightly different parameters depending on the
# version that will be run.  The DOS version needs to see a -P to remember
# the directory names.
$use_path = '';
$wildcard = "";  # used for reasonable zips, like os/2 or unix.

# set the filename used for numbering.
local($NUMBER_FILE) = "$TMP/aa_safedel.num";

# Retrieve the current deleted file number.
$number = &get_number($NUMBER_FILE);

# Skip to the next one to ensure we're the only ones that ever have this one.
&next_number($NUMBER_FILE);

# Chomp on all the files specified.
&safedel(@ARGV);

exit 0;

# The safedel procedure does most of the work.

sub safedel {
  # get the list of files and directories to whack.
  local(@to_delete) = &glob_list(@_);
#  print "final list of whackees: @to_delete\n";

  # we store the deleted files in a directory under the temporary directory.
  $temp_subdir = $TMP . "/zz_del_keep";
  if (! -d $temp_subdir) {
    mkdir "$temp_subdir", 0777;
      # create the subdirectory under temp if missing.
    if (! -d $temp_subdir) {
      die "the directory $temp_subdir could not be created!\n";
    }
  }

  # reset the list of objects actually whacked.
  local(@deleted) = ();
#  print "deleted list is @deleted\n";

  # iterate over the files that we have been told to nuke.
  foreach $file (@to_delete) {
    # go through each object that should be deleted...
    $file = &remove_trailing_slashes($file);
    if (substr($file, length($file) - 1, 1) eq ":") {
      die "removing the root directory of a drive is not permitted!";
    }
    if ( ($file =~ /^.*\/\.$/) || ($file =~ /^.*\/\.\.$/) ) {
      print "ignoring attempt to remove current or parent directory.\n";
      next;
    }
    $tempfile = $temp_subdir . "/temp" . "$number";
#   print "tempfile is $tempfile; file is $file.\n";
    if (-d $file) {
      # ensure there aren't any read only files.
      system("chmod -R u+rw \"$file\"");
      # store the directory in the trash storage.
      system("$zip -rm$use_path $tempfile \"$file$wildcard\" $DEV_NULL");
        # zip up the files into the safekeeper directory.
      # recursively unlink in case zip doesn't remove the empty dir.
      if (-d $file) {
        # remove the directory itself if possible, since zip did not.
        &recursively_zap_dirs($file);
      }
      push(@deleted, "$file");
    } elsif (-f $file) {
      # store the file in the trash storage.
      system("chmod u+rw \"$file\"");
      system("$zip -m$use_path $tempfile \"$file\" $DEV_NULL");
      push(@deleted, "$file");
    } else {
      print "$0 cannot find \"$file\" to delete it.\n";
    }
  }
  if (@deleted) {
    print "Trashed [@deleted].\n";
    open(REPORT, ">>$TMP/zz_safedel.rpt");

    local($printable_date) = &ctime(time);
    $printable_date =~ s/\n//g;
    print REPORT $printable_date . " -- safedel: \"temp" . $number . ".zip\" <= [@deleted]\n";
    close(REPORT);
  } else {
    print "No files were deleted.\n";
  }
}

