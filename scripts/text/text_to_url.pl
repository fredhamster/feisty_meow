#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : text_to_url                                                       #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 2005-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Turns a text file into a web page, where the URLs in the text file       #
#  appear as links in the web page.                                           #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

#require "filename_helper.pl";
#require "importenv.pl";
#require "inc_num.pl";

&generate_web_page(@ARGV);

exit 0;

sub generate_web_page {
  local($text_file, $web_page) = @_;
  if ($text_file eq "") {
    print "The first parameter must be a text file to use as input.\n";
    return;
  }
  if (! -e $text_file) {
    print "The text file that you specified does not exist.\n";
    return;
  }
  if ($web_page eq "") {
    print "The second parameter must be a web page to create.\n";
    return;
  }
  if (-e $web_page) {
    print "The web page you specified is already present--not overwriting.\n";
    return;
  }

  open(INPUT_FILE, "<$text_file")
      || die("Could not open the text file $text_file for reading.\n");
  open(OUTPUT_FILE, ">$web_page")
      || die("Could not open the web page $web_page for writing.\n");

  # dump the web heading stuff out.
  print OUTPUT_FILE "
<!DOCTYPE doctype PUBLIC \"-//w3c//dtd html 4.0 transitional//en\">
<html>
<head>
  <meta http-equiv=\"Content-Type\"
 content=\"text/html; charset=iso-8859-1\">
  <meta name=\"GENERATOR\"
 content=\"Fredzilla/14 [en] (linux; U) [Muttscape]\">
  <meta name=\"Author\" content=\"Fred T. Hamster\">
  <title>Links scavenged from $text_file</title>
</head>
<body link=\"#ffff99\" vlink=\"#ffcc33\" alink=\"#ffcc66\"
 style=\"background-color: rgb(0, 102, 0); color: rgb(204, 255, 255);\">
<h1>unsorted</h1>
<br>
";

  while (<INPUT_FILE>) {
    local($current) = $_;
    chomp $current;  # take CR off of end.

    # take spaces off the end of the line.
    while (substr($current, -1, 1) eq " ") { chop $current; }
    # take spaces off the front of the line.
    while (substr($current, 0, 1) eq " ") { $current = substr($current, 1); }

    # this block repairs partial URLs, if there is not protocol present.
    if ($current =~ /[^h][^t][^t][^p][^:][^\/\\][^\/\\]www\./) {
      # repair a missing http:// in front.
      $just_text = $current;
      $just_text =~ s/(.*)www\.[^ ]*(.*)/\1 \2/;
#print "just text is $just_text\n";
      $current =~ s/.*(www\.[^ ]*).*/http:\/\/\1/;
#print "curr is $current\n";
      print OUTPUT_FILE "$just_text\n<br>\n";
    } elsif ($current =~ /[^f][^t][^p][^:][^\/\\][^\/\\]ftp\./) {
      # repair a missing ftp:// in front.
      $just_text = $current;
      $just_text =~ s/(.*)ftp\.[^ ]*(.*)/\1 \2/;
#print "just text is $just_text\n";
      $current =~ s/.*(ftp\.[^ ]*).*/ftp:\/\/\1/;
#print "curr is $current\n";
      print OUTPUT_FILE "$just_text\n<br>\n";
###      print OUTPUT_FILE "<a href=\"ftp://$current\">$current</a><br>\n";
    }

    # look for matches to our supported URL types.
    if ($current =~ /http:/) {
      # treat a web URL simply by plugging it into a link definition.
      $just_url = $current;
      $just_url =~ s/.*(http:[^ ]*).*/\1/;
#print "just url is $just_url\n";
      $just_text = $current;
      $just_text =~ s/(.*)http:[^ ]*(.*)/\1 \2/;
#print "just text is $just_text\n";
      print OUTPUT_FILE "$just_text\n";
      print OUTPUT_FILE "<br><a href=\"$just_url\">$just_url</a><br>\n";
    } elsif ($current =~ /https:/) {
      # treat a secure web URL simply by plugging it into a link definition.
      $just_url = $current;
      $just_url =~ s/.*(https:[^ ]*).*/\1/;
#print "just url is $just_url\n";
      $just_text = $current;
      $just_text =~ s/(.*)https:[^ ]*(.*)/\1 \2/;
#print "just text is $just_text\n";
      print OUTPUT_FILE "$just_text\n";
      print OUTPUT_FILE "<br><a href=\"$just_url\">$just_url</a><br>\n";
    } elsif ($current =~ /ftp:/) {
      # treat an ftp URL simply by plugging it into a link definition.
      $just_url = $current;
      $just_url =~ s/.*(ftp:[^ ]*).*/\1/;
#print "just url is $just_url\n";
      $just_text = $current;
      $just_text =~ s/(.*)ftp:[^ ]*(.*)/\1 \2/;
#print "just text is $just_text\n";
      print OUTPUT_FILE "$just_text\n";
      print OUTPUT_FILE "<br><a href=\"$just_url\">$just_url</a><br>\n";
#      print OUTPUT_FILE "<a href=\"$current\">$current</a><br>\n";
    } else {
      # just print a regular line of text.
      print OUTPUT_FILE "$current<br>\n";
    }
  }

  print OUTPUT_FILE "</body>\n</html>\n";

  close INPUT_FILE;
  close OUTPUT_FILE;
}

1;


