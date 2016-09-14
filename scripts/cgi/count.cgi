#!/usr/bin/perl

#
# A simple counter program which maintains counts for
# all files in a single counts file. Some systems may
# not allow "." files to be created, so you should
# rename the count file.
#
#

#
# get name of accessed document
#
$doc = $ENV{'DOCUMENT_URI'};
$host= $ENV{'REMOTE_HOST'};
$path= $ENV{'PATH_INFO'};
$server= $ENV{'SERVER_NAME'};

#
# open counts file for read & write
# get an exclusive lock on it
#
open(COUNTS, "+< /usr/lib/cgi-bin/hit_counts") || die("error: can't open hit_counts.\n");
flock(COUNTS, 2);  # will wait for lock

#
# read counts database into associative array
#
while (<COUNTS>) {
  chop;
  ($count, $file) = split(/:/, $_);
  $counts{$file} = $count;
}

# strip off silly www hostname bits.
if ($server =~ /^www\./) {
  $server =~ s/^www\.//;
}

# translate some domains into other domains to avoid maintaining multiple
# hit count lists for the same sites that have different names.
if ($server =~ /^gruntose\.org/) {
  $server =~ s/^gruntose\.org/gruntose.com/;
}
if ($server =~ /^gruntose\.net/) {
  $server =~ s/^gruntose\.net/gruntose.com/;
}
if ($server =~ /^cromp\.net/) {
  $server =~ s/^cromp\.net/cromp.org/;
}
if ($server =~ /^hoople\.net/) {
  $server =~ s/^hoople\.net/hoople.org/;
}

#
# increment count of hit document, but not if I'm the
# one accessing it.
#

#we could do without that line probably.

#
# increment count for this file
#
$counts{$server.$doc}++;

#
# rewrite count file
# put file marker back at beginning
#
seek(COUNTS, 0, 0);

foreach $file (keys %counts) {
  print COUNTS $counts{$file}, ":", $file, "\n";
}

close(COUNTS);

#
# print count string to STDOUT
#
print "Content-type: text/plain\n\n";

print $counts{$server.$doc};

