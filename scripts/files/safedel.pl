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
require "filename_helper.pl";
require "inc_num.pl";
require "zap_the_dir.pl";

use Env qw(TMP OS DEBUG_FEISTY_MEOW);

#hmmm: need a usage statement.

if ($#ARGV < 0) {
  die "Too few arguments to command.";
}

$DEV_NULL = "> /dev/null 2> /dev/null";
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
#hmmm: make this into a debug option. 
#print "list of whackees: @to_delete\n";

  # we store the deleted files in a directory under the temporary directory.
  if (! -d $TMP) { 
    mkdir "$TMP", 0700;
    if (! -d $TMP) {
      die "the TMP directory $TMP could not be created!\n";
    }
  }
  $temp_subdir = $TMP . "/zz_safedel_keep";
  if (! -d $temp_subdir) {
    mkdir "$temp_subdir", 0700;
      # create the subdirectory under temp if missing.
    if (! -d $temp_subdir) {
      die "the directory $temp_subdir could not be created!\n";
    }
  }

  # reset the list of objects actually whacked.
  local(@deleted) = ();

  # iterate over the files that we have been told to nuke.
  foreach $file (@to_delete) {
    # go through each object that should be deleted...
    $file = &remove_trailing_slashes($file);
    if (substr($file, length($file) - 1, 1) eq ":") {
      die "removing the root directory of a drive is not permitted!";
    }

#print "file to whack: '$file'\n";

    if ( ($file =~ /^.*\/\.$/) || ($file =~ /^.*\/\.\.$/) ) {
      print "ignoring attempt to remove current or parent directory.\n";
      next;
    }

#hmmm: extract this shared bit of code as new method (also in shared snarfer)
    $date_tool = "date";
    local($datestamp) = `$date_tool +%Y-%m-%d-%H%M`;
    while ($datestamp =~ /[\r\n]$/) { chop $datestamp; }
    $archive_file = $temp_subdir . "/del-$number-" . $datestamp;
#print "archive_file is $archive_file; file is $file.\n";

    if (-d $file) {
      # ensure there aren't any read only files.
      system("chmod -R u+rw '$file'");
      # store the directory in the trash storage.
      system("$zip -rm $archive_file '$file' $DEV_NULL");
        # zip up the files into the safekeeper directory.
      # recursively unlink in case zip doesn't remove the empty dir.
      if (-d $file) {
        # remove the directory itself if possible, since zip did not.
        &recursively_zap_dirs($file);
      }
      push(@deleted, "\"$file\"");
    } elsif (-f $file) {
#print "about to chmod file\n";
      # make the file writable by our user if possible (which resets any
      # prior permissions as long as we're the owner).
      system("chmod u+rw '$file'");
      # store the file in the trash storage.
#print "about to run: system [$zip -m $archive_file '$file' $DEV_NULL]";
      system("$zip -m $archive_file '$file' $DEV_NULL");
      push(@deleted, "\"$file\"");
    } else {
      print "$0 cannot find \"$file\" to delete it.\n";
    }
  }
  if (@deleted) {
    if ($DEBUG_FEISTY_MEOW != "") {
      print "Trashed [@deleted].\n";
    }
    open(REPORT, ">>$TMP/zz_safedel_report.txt");

    local($printable_date) = scalar(localtime());
#&ctime(time);
    $printable_date =~ s/\n//g;
    local($just_archived_filename) = `basename "$archive_file"`;
    while ($just_archived_filename =~ /[\r\n]$/) { chop $just_archived_filename; }
    print REPORT "\n";
    print REPORT $printable_date . " -- created \"" . $just_archived_filename . ".zip\"\n";
    print REPORT $printable_date . " -- from [@deleted]\n";
    close(REPORT);
  } else {
#hmmm: oh good, and we should always bug people about nothing having been done?
#      this is especially tiresome when our own scripts cause safedel to be invoked,
#      since then they are automatically noisy and blathery.
#hmmm: make this into a debug option. 
#    print "No files were deleted.\n";
  }
}

