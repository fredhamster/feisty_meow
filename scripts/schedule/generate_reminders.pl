#!/usr/bin/perl

##############
#  Name   : generate_reminders
#  Author : Chris Koeritz
##############
# Copyright (c) 1989-$now By Author.  This script is free software; you can
# redistribute it and/or modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation:
#     http://www.gnu.org/licenses/gpl.html
# or under the terms of the GNU Library license:
#     http://www.gnu.org/licenses/lgpl.html
# at your preference.  Those licenses describe your legal rights to this
# software, and no other rights or warranties apply.
# Please send updates for this code to: fred@gruntose.com -- Thanks, fred.
##############

# Credits: thanks to 'calendar version 3' from kernighan & pike for good approaches.

# Note: the calendar format is like so in calendar.dat:
#
# Apr 24 Saint Shanty's Day
# Jul 8 Normality Day
#
# Month names must currently be three letters and must be capitalized.
# The day must be a number, followed by a space.  Anything after that
# space is considered the description of the event.

require "importenv.pl";

##############

# all the date alerts go into this temp file.
local($TEMPO_FILE) = `mktemp "$TMP/zz_reminder.XXXXXX"`;
chop($TEMPO_FILE);

local($USERNAME) = "$REPLYTO";
if (! $USERNAME) { $USERNAME="fred" }

#print "TEMPO is $TEMPO_FILE ; USER is $USERNAME ; \n";

local($CAL_FILE);
if (! $CAL_FILE) {
#  print "the CAL_FILE variable is not set.\n";
#  print "defaulting it to a value that probably does not suit you.\n";
  $CAL_FILE = "$QUARTZDIR/database/calendar.dat";
}

#print "calfile is $CAL_FILE\n";

# we open this really early on, to check if it exists.  we use it way down below.
open(DATES, "<$CAL_FILE") || die("failed to open $CAL_FILE");

##############

# the big block here computes the list of days near today, so we can check the
# entries in the file.

local($MONTH_DAYS) = "Jan 31 Feb 28 Mar 31 Apr 30 May 31 Jun 30 Jul 31 Aug 31 Sep 30 Oct 31 Nov 30 Dec 31 Jan 31";
  # we repeat january so we can wrap around into the future year.

local(@moon_data) = split(' ', $MONTH_DAYS);

#print "got moondays of @moon_data\n";

local $i, %days_in_month, %next_month;

for ($i = 0; $i < 24; $i += 2) {
  $days_in_month{@moon_data[$i]} = $moon_data[$i + 1];
#print "new day=$moon_data[$i + 1]\n";
  $next_month{@moon_data[$i]} = $moon_data[$i + 2];
#print "new mon=$moon_data[$i + 2]\n";
}

#local(@days) = %days_in_month;
#local(@next_month) = %next_month;
#print "got days wik @days\n";
#print "got next mon wik @next_month\n";

local(@date_as_array) = split(' ', `date`);

##############

# now we construct the nearby dates that we care about...

local @checking_dates;

# push the pair of month and date into our list of dates to be checked.
push(@checking_dates, ($date_as_array[1], $date_as_array[2]));

local @pair, @new_pair;

@pair = ($checking_dates[0], $checking_dates[1]);
#print "got a pair @pair\n";

# add one day at a time to make sure we report if we're close enough to an event that happens
# tomorrow or a couple days from now, etc.
for ($i = 1; $i <= 7; $i++) {
  $pair[1] = $pair[1] + 1;  # increment the day entry by one.
  @new_pair = &fix_date_for_rollover(@pair);  # fix for rollovers.
  push(@checking_dates, ($new_pair[0], $new_pair[1]));  # add to the checking list.
}

# add a few days to jump into the future to check every other day for an impending event.
for ($i = 1; $i <= 4; $i++) {
  $pair[1] = $pair[1] + 2;  # increment the day entry by two days.
  @new_pair = &fix_date_for_rollover(@pair);  # fix for rollovers.
  push(@checking_dates, ($new_pair[0], $new_pair[1]));  # add to the checking list.
}

# now look a couple weeks forward also (for a total of about 3 weeks total coverage).
for ($i = 0; $i < 2; $i++) {
  $pair[1] = $pair[1] + 7;  # increment the day entry by a week.
  @new_pair = &fix_date_for_rollover(@pair);  # fix for rollovers.
  push(@checking_dates, ($new_pair[0], $new_pair[1]));  # add to the checking list.
}

##############

# finally we can test the dates in the file and see if they're getting close enough to report on.

#print "got nearby dates to check: @checking_dates\n";

while (<DATES>) {
  local($line) = $_;
  chop($line);
  if (length($line) <= 0) { next; }
#print "line to check: '$line'\n";
  local(@test_date) = split(' ', $line);
  &test_date_for_proximity($test_date[0], $test_date[1], $line);
}

# send mail here if there's anything to say.
if (! -z $TEMPO_FILE) {
  # there are some alerts in there.
#print "will run: system(\"mail -s \"FredMinder: \$(head -1 $TEMPO_FILE)\" $USERNAME <$TEMPO_FILE\");\n";
  system("mail -s \"FredMinder: \$(head -1 $TEMPO_FILE)\" $USERNAME <$TEMPO_FILE");
}

unlink $TEMPO_FILE;

exit 0;

##############

# prints the contents of the first parameter's file out to stdout.
sub fix_date_for_rollover {
  local($month) = @_[0];
  local($day) = @_[1];
#print "fix mon=$month day=$day\n";
  if ($day > $days_in_month{$month}) {
    $day = $day % $days_in_month{$month};
    $month = $next_month{$month};
  }
#print "now mon=$month day=$day\n";
  return ($month, $day);
}

sub test_date_for_proximity {
  local($month) = @_[0];
  local($day) = @_[1];
  local($line) = @_[2];
#print "test mon=$month day=$day\n";

  for ($i = 0; $i < @checking_dates; $i += 2) {
#    print "i is $i\n";
    local($checkmo) = lc $checking_dates[$i];
    local($checkda) = $checking_dates[$i + 1];
#print "checking mon=$checkmo day=$checkda\n";
    if ( (lc $month eq $checkmo) && ($day eq $checkda) ) {
      # found a day that's close enough to match; send an alert about it.
#print "matching found!  mon=$month day=$day\n";
      open(OUTY, ">>$TEMPO_FILE") || die("failed to open $TEMPO_FILE");
      print OUTY $line . "\n";
      close(OUTY);
    }
  }
}

