#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : list_to_html                                                      #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Munges multiple files together into one output file with html formatting #
#  that makes it appropriate for a numbered list.                             #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

require "filename_helper.pl";

# this is our default output filename for the html list.
local($output_file) = "text_listing.html";

# make sure they gave us some files to process.
if ($#ARGV < 0) {
  &instructions;
  exit 0;
}

# clear and initialize our output file before doing anything else.
open(O_FILE, ">" . $output_file);

# add in the boilerplate for an html document and our initial nesting level.
print O_FILE "\
<!DOCTYPE html PUBLIC \"-//w3c//dtd html 4.0 transitional//en\">\
<html>\
<head>\
<title>generated html from list_to_html script</title>\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\
<meta name=\"author\" content=\"Fred T. Hamster\">\
</head>\
<body>\
<ol>\n";

close O_FILE;

# now iterate over the list of files and dump them to our output file.
local($filename);
foreach $filename (&glob_list(@ARGV)) {
  $chewed_filename = &basename($filename);
  $chewed_filename =~ s/_/ /g;
  $chewed_filename =~ s/\.[a-zA-Z][a-zA-Z][a-zA-Z]$//;
  $chewed_filename =~ s/\.[a-zA-Z][a-zA-Z][a-zA-Z][a-zA-Z]$//;
#print "new filename is $chewed_filename\n";
  &do_dump($filename, $chewed_filename, $output_file);
}

open(O_FILE, ">>" . $output_file);

# return from first nesting and close out the document.
print O_FILE "\n</ol>\
</body>\
</html>\n";

close O_FILE;

exit 0;

############################################################################

# prints help out on using this amazing program.

sub instructions {
  print "

list_to_html: this program needs a set of filenames to print out in html
list format.  the resulting information will be placed in a file called
\"$output_file\" in the current directory.

";
}

############################################################################

# do_dump: prints the contents of the first parameter's file out by
# concatenating it onto the second parameter's file.  since it's an append,
# if you mess up, the file will at least not be overwritten.

sub do_dump {

  ($to_dump, $chewed_name, $output_file) = @_;

#  print "dumpfile=$to_dump and chewed=$chewed_name and outfile=$output_file\n";

  if (&basename($to_dump) eq &basename($output_file)) {
    # we don't ever dump to the same file as we're outputting to.  that would
    # be quite silly and would make the file pretty useless, as well as
    # rendering the original file corrupted possibly.
    return;
  }

  $header = "
<li>%1</li>
<ol>
";

  $footer = "</ol>
";

  $header_copy = $header;
  $header_copy =~ s/%1/$chewed_name/;

##print $header_copy;

  open(TO_DUMP, "<$to_dump");
  open(OUT, ">>$output_file");

  print OUT $header_copy;

  while (<TO_DUMP>) {
    local($line) = $_;
#local($i);
#for ($i = 0; $i < length($line); $i++) {
#$curr_char = substr($line, $i, 1);
#print "[$curr_char]";
#}
#print "\n";
    local($curr_char) = 0;
#print "start len=" . length($line) . "\n";
    do {
#print "chopping\n";
      chop $line;  # remove end of line char(s).
      $curr_char = substr($line, length($line) - 1, 1);
        # get new last char in string.
    } while ( ($curr_char eq "\r") || ($curr_char eq "\n") );
    local($do_print) = 1;
    if (length($line) == 0) {
      #continue;  # skip the first blank lines.
      $do_print = 0;  #no continue statement??
    } else {
      $just_started = 0;
        # a non-blank line has been seen.  now we reset our flag so we stop
        # checking for blank lines.
    }
    if ($do_print) { print OUT "<li>" . $line . "</li>\n"; }
  }

  print OUT $footer;
  close OUT;
}

