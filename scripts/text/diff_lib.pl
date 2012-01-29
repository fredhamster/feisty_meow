#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : diff_lib                                                          #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    A collection of services used for creating a report of differences       #
#  between two directories.                                                   #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

# note: this script and the scripts that use it require the Text::Diff
# support from CPAN.  this can be installed by using cpan and telling it
#    install Text::Diff

require "ctime.pl";
require "filename_helper.pl";
require "importenv.pl";

use Text::Diff;

sub diff_dirs {
  local(@arguments) = @_;

#  print "args: @arguments\n";

  ############################################################################

  # constants used for header creation.
  $long_line = "--------------\n";
  $break_line = "\n##############\n";
  $separator = "\n";
  $differing_header = "Differences seen for %1:\n";

  ############################################################################

  # state machine constants and initialization.

  $STARTING = 2;  # initial state.
  $SAW_DIFF = 3;  # recently saw a difference.
  $SAW_SAME = 4;  # recently saw no differences.

  $state = $STARTING;

  ############################################################################

  $header_printed = 0;  # did we print the header for this directory yet?

  ############################################################################

  if ($#arguments < 0) {
    # complain if they don't pass a directory name.
    &differ_instructions;
    return;
  }

  # get the comparison directory from the arguments.
  local($compare_directory) = &sanitize_name($arguments[0]);
  if ($compare_directory eq "") {
    # it's a bad directory name, only composed of slashes.
    &differ_instructions;
    return;
  }
  if (-l $compare_directory) {
    # we will not operate on links, due to recursion issues.
##    &differ_instructions;
    print "skipping link: $compare_directory\n";
    return;
  }
  if (! -d $compare_directory) {
    # a missing directory is just not good.
    print "skipping missing directory: $compare_directory\n";
    return;
  }

  # the current directory is usually used as the starting point.
  local($source_directory) = ".";
  if ($arguments[1] ne "") {
    # they specified a current directory.
    $source_directory = $arguments[1];
  }
  $source_directory = &sanitize_name($source_directory);
  if (-l $source_directory) {
    # we will not operate on links, due to recursion issues.
#    &differ_instructions;
    print "skipping link: $source_directory\n";
    return;
  }
  if (! -d $source_directory) {
    # a missing directory is just not good.
    print "skipping missing directory: $source_directory\n";
    return;
  }

#  print "src=$source_directory, dest=$compare_directory\n";

# keep a local copy of the names for print_header.
#hmmm: fix bad design.
  local($temp_src) = $source_directory;
  local($temp_dest) = $compare_directory;

  # iterate over all the files in the source directory.
  opendir CURDIR, $source_directory
      || die("couldn't open $source_directory for reading.\n");
  foreach $filename (readdir CURDIR) {
    if ( ($filename eq ".") || ($filename eq "..") ) { next; }
    $filename = $source_directory."/".$filename;
    # make sure we compare against the base part of the name.
    @name_components = split(/\//, $filename);
    $basename = $name_components[$#name_components];
#    print "doing diff of $filename against $compare_directory/$basename\n";
    &do_diff($filename, $compare_directory."/".$basename);
  }
  closedir CURDIR;

  if ($state == $SAW_SAME) {
    print $long_line;
  }

  # report files that exist in target directory, but not here, or vice-versa.
  &back_compare($compare_directory, $source_directory);
  &back_compare($source_directory, $compare_directory);

  # only print the closure line if we printed some other stuff.
  if ($header_printed) {
    print "\n";
###    print $break_line;
  }

  return;  # scram.
}

############################################################################

sub print_header {
  # this function prints out the header prior to printing out any real
  # data.  if there are no diffs, the header should never get printed.
  print "$break_line\n";
  local($printable_date) = &ctime(time);
  $printable_date =~ s/\n//g;
  print "[$printable_date]\n";
  print "Left (<) is \"$temp_src\".\n";
  print "Right (>) is \"$temp_dest\".\n";

  $header_printed = 1;
}

############################################################################

sub differ_instructions {
  print "
differ:
This program needs a directory name.  The directory name is used to
specify a target directory where there are files which are mostly similar
to the files in this directory.  The files in the current directory are
compared to those in the specified target directory and the differences
are sent to the standard output.  Note that neither directory may be
a symbolic link, as that can lead to crazy recursion.
";
}

############################################################################

sub cpdiff_instructions {
  print "
cpdiff:
This program needs two directory names.  The first is the source and the
second is a target directory where the files are mostly similar to the files
in the source directory.  The files in source are compared to those in the
target directory and if a file differs, then it's copied from the source to
the target.  Also, if the file does not exist in the target directory, it
gets copied.
";
}

############################################################################

sub synchronize_instructions {
  print "
synch_build:
This program needs one directory name to be passed on the command line.
This directory is where the installation's executable files live.  Any files
that are in the installation bin directory will be compared against the files
in the build repository (specified by an environment variable FEISTY_MEOW_DIR).
If files differ, they will be copied from the repository into the installation
directory.
";
}

############################################################################

# replaces the variable marks in the text with the two substitutions.

sub replace_text {
  local($text, $subst_1, $subst_2) = @_;

  local($text_copy) = $text;
  $text_copy =~ s/%1/$subst_1/;
  $text_copy =~ s/%2/$subst_2/;

  return $text_copy;
}

sub change_to_saw_diffs {
  print $separator, $long_line;
}

sub change_to_saw_same {
}

############################################################################

# checks the differences between the two files and creates appropriate output.

sub do_diff {
  local($first, $second) = @_; 

  # turn stupid pc slashes into normal ones.
  $first =~ s/\\/\//g;
  $second =~ s/\\/\//g;

  @first_components = split(/\//, $first);
  $base_filename1 = $first_components[$#first_components];
  @second_components = split(/\//, $second);
  $base_filename2 = $second_components[$#second_components];

  # skip if it's a directory.
  if (-d $first) { return; }

  # skip the file if we don't think it's important.
  if (! &important_filename($base_filename1)) { return; }

  if (! -e $second) {
    # make sure we don't bother if the file's not in the target directory.
    return;
  }

  local($diff_output) = diff $first, $second, { STYLE => "OldStyle" };

  if (!length($diff_output)) {
    # there are no differences.
    &change_to_saw_same;
  } else {
    # there were differences.
    if (!$header_printed) { &print_header; }

    &change_to_saw_diffs;

    print &replace_text($differing_header, $base_filename1);
    print $long_line;

    if (-B $first || -B $second) {
      print "Binary files $first and $second differ.\n";
    } else {
      print $diff_output;
    }
  }
}

############################################################################

# this function copies files as needed by comparing whether a file has
# changed or not.  third parm is a flag added to cp, like -p to preserve
# the timestamps.

sub do_cpdiff {
  local($first, $second, $third) = @_; 

  @first_components = split(/\//, $first);
  $base_filename1 = $first_components[$#first_components];
  @second_components = split(/\//, $second);
  $base_filename2 = $second_components[$#second_components];

  # turn stupid pc slashes into normal ones.
  $second =~ s/\\/\//g;

  # skip if it's a directory.
  if (-d $first) { return; }

  # skip the file if we don't think it's important.
  if (! &important_filename($base_filename1)) { return; }

  if (! -e $second) {
    # the file doesn't exist yet, so copy it.
#    print "copying $first -> $second\n";
    system("cp -f -v $third \"$first\" \"$second\"");
    return;
  }

  local($diff_output) = diff $first, $second, { STYLE => "OldStyle" };
  if (length($diff_output)) {
    # there were differences in the two files, so copy the source.
#    print "copying $first -> $second\n";
    system("cp -f -v $third \"$first\" \"$second\"");
    return;
  }
#print "did nothing; $first & $second are identical\n";
}

############################################################################

# back_compare checks to make sure that all of the files in the target
# directory are represented in the source directory.

sub back_compare {
  local($source, $target) = @_;
  local(@missing_list);

#  print "backcomp: source=$source target=$target\n";

  opendir CURDIR, $target
      || die("couldn't open $target for reading.\n");
  foreach $filename (readdir CURDIR) {
    if ( ($filename eq ".") || ($filename eq "..") ) { next; }

    $filename = $target."/".$filename;
    # chop up the filename.
    @dirs = split(/\//, $filename);
    # get the basename from the last array element.
    $basename = $dirs[$#dirs];

    # skip the file if it seems like junk.
    if (! &important_filename($basename)) { next; }

#    print "backcomp: file a=$source/$basename b=$filename\n";
    if ( (! -e "$source/$basename") && (! -d $filename) ) {
      push(@missing_list, $basename);
    }
  }
  closedir CURDIR;

  if ($#missing_list < 0) { return; }

  if (!$header_printed) { &print_header; }

  print "$separator
==============
These exist in \"$target\",
but not in \"$source\":
==============
";

  foreach $filename (@missing_list) {
    print "$filename  ";
  }
}

############################################################################

# this function copies files from the source directory to the target directory
# if there is a file of the same name whose contents differ, or if there is
# no file of that name in the target directory.

sub copy_diff_dirs {
  local(@arguments) = @_;
#  print "args: @arguments\n";

  if ($#arguments < 1) {
    # complain if they don't pass two directory names.
    &cpdiff_instructions;
    return;
  }

  # the source directory is the first parameter.
  local($source_directory) = &sanitize_name($arguments[0]);
  if ($source_directory eq "") {
    # it's a bad directory name, only composed of slashes.
    &cpdiff_instructions;
    return;
  }
  if (-l $source_directory) {
    # we will not operate on links, due to recursion issues.
    &cpdiff_instructions;
    return;
  }

  # get the comparison directory from the arguments.
  local($compare_directory) = &sanitize_name($arguments[1]);
  if ($compare_directory eq "") {
    # it's a bad directory name here.
    &cpdiff_instructions;
    return;
  }
  if (-l $compare_directory) {
    # we will not operate on links, due to recursion issues.
    &cpdiff_instructions;
    return;
  }

#  print "src=$source_directory, dest=$compare_directory\n";

  # iterate over all the files in the source directory.
  opendir CURDIR, $source_directory
      || die("couldn't open $source_directory for reading.\n");
  foreach $filename (readdir CURDIR) {
    if ( ($filename eq ".") || ($filename eq "..") ) { next; }
    $filename = $source_directory."/".$filename;
    # make sure we compare against the base part of the name.
    @name_components = split(/\//, $filename);
    $basename = $name_components[$#name_components];
#    print "diffing $filename against $compare_directory/$basename\n";
    &do_cpdiff($filename, $compare_directory."/".$basename, "-p");
  }
  closedir CURDIR;
}

############################################################################

# this version of copy_diff_dirs (see above) causes the dates of the copied
# files to be changed to "now".

#hmmm: extract the common bits used in copy_diff_dirs into useful functions.

sub copy_diff_dirs_using_now {
  local(@arguments) = @_;
#  print "args: @arguments\n";

  if ($#arguments < 1) {
    # complain if they don't pass two directory names.
    &cpdiff_instructions;
    return;
  }

  # the source directory is the first parameter.
  local($source_directory) = &sanitize_name($arguments[0]);
  if ($source_directory eq "") {
    # it's a bad directory name, only composed of slashes.
    &cpdiff_instructions;
    return;
  }

  # get the comparison directory from the arguments.
  local($compare_directory) = &sanitize_name($arguments[1]);
  if ($compare_directory eq "") {
    # it's a bad directory name here.
    &cpdiff_instructions;
    return;
  }

#  print "src=$source_directory, dest=$compare_directory\n";

  # iterate over all the files in the source directory.
  opendir CURDIR, $source_directory
      || die("couldn't open $source_directory for reading.\n");
  foreach $filename (readdir CURDIR) {
    if ( ($filename eq ".") || ($filename eq "..") ) { next; }
    $filename = $source_directory."/".$filename;
    # make sure we compare against the base part of the name.
    @name_components = split(/\//, $filename);
    $basename = $name_components[$#name_components];
#    print "diffing $filename against $compare_directory/$basename\n";
    &do_cpdiff($filename, $compare_directory."/".$basename);
  }
  closedir CURDIR;
}

############################################################################

# makes sure all of the exes and dynamic libs are up to date in the
# installed executable directory.

sub synchronize_against_build
{
  local(@arguments) = @_;

#  print "args: @arguments\n";

  if ($#arguments < 0) {
    # complain if they don't pass a directory name.
    &synchronize_instructions;
    return;
  }

  # clean up the directory they passed.
  local($install_directory) = &sanitize_name($arguments[0]);
  if ($install_directory eq "") {
    # it's a bad directory name, only composed of slashes.
    &synchronize_instructions;
    return;
  }

#  print "install=$install_directory\n";
#  print "repos=$FEISTY_MEOW_DIR\n";

  # iterate over all the files in the source directory.
  opendir CURDIR, $install_directory
      || die("couldn't open $install_directory for reading.\n");
  $compare_directory = "$FEISTY_MEOW_DIR/dll";
  foreach $filename (readdir CURDIR) {
    if ( ($filename eq ".") || ($filename eq "..") ) { next; }
    if (! ($filename =~ /\.dll$/)) { next; }
    $filename = $install_directory."/".$filename;
    # make sure we compare against the base part of the name.
    @name_components = split(/\//, $filename);
    $basename = $name_components[$#name_components];
    if (! -e $compare_directory."/".$basename) {
      next;
    }
#    print "diffing $filename against $compare_directory/$basename\n";
    &do_cpdiff($compare_directory."/".$basename, $filename);
  }
  closedir CURDIR;

  # repeat for the exe directory.
  opendir CURDIR, $install_directory
      || die("couldn't open $install_directory for reading.\n");
  $compare_directory = "$FEISTY_MEOW_DIR/exe";
  foreach $filename (readdir CURDIR) {
    if ( ($filename eq ".") || ($filename eq "..") ) { next; }
    if (! ($filename =~ /\.exe$/)) { next; }
    $filename = $install_directory."/".$filename;
    # make sure we compare against the base part of the name.
    @name_components = split(/\//, $filename);
    $basename = $name_components[$#name_components];
    if (! -e $compare_directory."/".$basename) {
      next;
    }
#    print "diffing $filename against $compare_directory/$basename\n";
    &do_cpdiff($compare_directory."/".$basename, $filename);
  }
  closedir CURDIR;
}

############################################################################

1;

