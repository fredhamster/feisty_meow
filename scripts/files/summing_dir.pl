#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : summing_dir                                                       #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 2000-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Provides a somewhat stylized directory lister.                           #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

require "filename_helper.pl";

use Env qw($TMP $color_add $TERM);

local($chewed_line) = "";
local(@arg_list);
local($print_list) = "";

# if there were no parameters, we make the default listing for the current
# directory.
if ($#ARGV < 0) {
  $print_list = "Current Directory";
  @arg_list = ( "." );
} else {
  local(@munged_list) = &patch_name_for_pc(&remove_trailing_slashes(@ARGV));
  $print_list = "@munged_list";
  @arg_list = &glob_list(@ARGV);
}

foreach $dir (@arg_list) {
  if ($dir eq "-al") { next; }  # skip ls directives.
  if ($dir eq "-l") { next; }  # skip ls directives.
  $chewed_line = $chewed_line . " \"$dir\"";
}

if ("$chewed_line" eq "") {
  print "No files matched that path specification.\n";
  exit 0;
}

# show the header, now that we know there's something to print.
print "[" . $print_list . "]\n\n";

local($temp_file)=`mktemp "$TMP/zz_frdsumdir.XXXXXX"`;
chop($temp_file);

# drop the main payload, the list of directory info, but also save that
# info to a file for analysis.
system("ls -HhlF $color_add $chewed_line");
system("ls -HhlF $color_add $chewed_line > $temp_file");
  # the color_add variable, if defined, will have flags for setting the
  # directory listing color scheme.

local($lengths) = 0;

# open the file and process the lines to get file lengths.
open(DIRLIST, "<$temp_file");
# we only want to match ls -al style output lines, and only want to keep the size.
$pattern="^[^ ]+ +[^ ]+ +[^ ]+ +[^ ]+ +([0-9.]+[KMG]?).*\$";
foreach $file_line (<DIRLIST>) {
  if ($file_line =~ /$pattern/) {
    (local $munged = $file_line) =~ s/$pattern/\1/;
    if ($munged =~ /K$/) {
      chop $munged;
      $munged *= 1024.0;
      #print "K munged is now $munged\n";
    }
    if ($munged =~ /M$/) {
      chop $munged;
      $munged *= 1024.0 * 1024.0;
      #print "M munged is now $munged\n";
    }
    if ($munged =~ /G$/) {
      chop $munged;
      $munged *= 1024.0 * 1024.0 * 1024.0;
      #print "G munged is now $munged\n";
    }
    $lengths += $munged;
  }
}
close(DIRLIST);
unlink($temp_file);  # clean up.

local($total)=int($lengths);
local($kbytes)=int($total / 102.4) / 10;
local($mbytes)=int($kbytes / 102.4) / 10;
local($gbytes)=int($mbytes / 102.4) / 10;

print "\n";
# print a fancy listing showing bytes at least, but only showing mb and gb if appropriate.
print "These files occupy $total bytes ($kbytes KB";
if ($mbytes ne 0) {
 print " / $mbytes MB";
}
if ($gbytes ne 0) {
 print " / $gbytes GB";
}
print ").\n";

print "Overall Drive Usage (megs):\n";

system("df -m $chewed_line >$temp_file");

# now eat the file again, but this time to get drive space info.
open(DIRLIST, "<$temp_file");
local($space_info) = "";
local($did_title) = 0;  # true if we have printed the title by now.
foreach $file_line (<DIRLIST>) {
  ($space_info = $file_line) =~ s/[^ ]* *([^ ]*) *([^ ]*) *([^ ]*) *([^ ]*) *.*$/\1	\2	\3	\4/;
  if (!$did_title) {
    # if the title hasn't been printed yet, we take some of the info out of
    # the line and use it for the right sense of the last column.
    print "Total	Used	Free	";
    $space_info =~ s/[^	]*	*[^	]*	*[^	]*	*([^	]*)/\1/;
    print "$space_info";
    $did_title = 1;
  } else {
  }
}
close(DIRLIST);
unlink($temp_file);  # clean up.

print "$space_info\n";

exit 0;

