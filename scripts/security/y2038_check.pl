#!/usr/bin/perl

use POSIX;
$ENV{'TZ'} = "GMT";

for ($clock = 2147483641; $clock < 2147483651; $clock++)
{
  print ctime($clock);
}
