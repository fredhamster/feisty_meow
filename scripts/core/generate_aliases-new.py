#! /usr/bin/env python3

"""

Name   : generate_aliases
Author : Chris Koeritz
Rights : Copyright (C) 1996-$now by Author

Purpose:

  This script generates feisty meow script alias files.  Alias files
contain a list of definitions for command aliases that are written in the
specified shell dialect (such as bash or perl) and which are additionally
tailored for the operating system to be used.

author: chris koeritz

####
This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License or (at your option)
any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a
version of the License.  Please send any updates to "fred@gruntose.com".
"""

import os
import re
import sys

import filename_helper

# load some variables from the environment, if we can.
HOME = os.environ['HOME']
FEISTY_MEOW_BINARIES = os.environ['FEISTY_MEOW_BINARIES']
BUILD_TOP = os.environ['BUILD_TOP']
FEISTY_MEOW_APEX = os.environ['FEISTY_MEOW_APEX']
FEISTY_MEOW_LOADING_DOCK = os.environ['FEISTY_MEOW_LOADING_DOCK']
FEISTY_MEOW_SCRIPTS = os.environ['FEISTY_MEOW_SCRIPTS']
DEBUG_FEISTY_MEOW = os.environ['DEBUG_FEISTY_MEOW']

print("home is $HOME")



print("bailing now...")
exit(1)



#hmmm: unscanned after here...  there be monsters.



# given a possible aliasable filename, this will decide whether to create a perl
# or bash alias for it.  it needs the filename of the possible alias and the
# directory where that file resides.
def make_alias(file: str, dir: str) -> None:

  found = re.search("^.*\.[pP][yY]", file)
  if found:
    aliasname = re.sub("\.[pP][Yy]", "", file)
    print("aliasname is " + aliasname)
    make_python_alias(aliasname, "$dir");

  found = re.search("^.*\.[sS][hH]", file)
  if found:
blah


  } elsif ($file =~ /\.[sS][hH]$/) { 
    local($aliasname) = $file; $aliasname =~ s/\.[Ss][Hh]$//;
    &make_bash_alias($aliasname, "$dir");
  } elsif ($file =~ /\.[pP][lL]$/) { 
    local($aliasname) = $file; $aliasname =~ s/\.[Pp][lL]$//;
    &make_perl_alias($aliasname, "$dir");
  }
}

# makes an alias for a bash script given the alias name.
sub make_bash_alias {
  local($aliasname) = shift(@_);
  local($full_alias) = $aliasname;
#print "full alias is $full_alias\n";
  $aliasname =~ s/^.*\/([^\/]*)/\1/;
#print "alias became $aliasname\n";
  print she "define_yeti_alias $aliasname=\"bash $full_alias.sh\"\n";
}

# makes an alias for a python script given the alias name.
#hmmm: don't love that we're hardcoding python3 in here, but apparently some systems don't have a 'python' command despite having python installed.
sub make_python_alias {
  local($aliasname) = shift(@_);
  local($full_alias) = $aliasname;
  $aliasname =~ s/^.*\/([^\/]*)/\1/;
#print "alias became $aliasname\n";
  print she "define_yeti_alias $aliasname=\"python3 $full_alias.py\"\n";
}

# makes an alias for a perl script given the alias name.
sub make_perl_alias {
  local($aliasname) = shift(@_);
  local($full_alias) = $aliasname;
  $aliasname =~ s/^.*\/([^\/]*)/\1/;
#print "alias became $aliasname\n";
  print she "define_yeti_alias $aliasname=\"perl $full_alias.pl\"\n";
}

##############

# The "common.alias" file is used in the generated aliases file as a base
# set of generally useful aliases.  We also add aliases for any script files
# (perl, bash, python, etc) that we find in the feisty meow script hierarchy.
# Any *.alias files found in the $FEISTY_MEOW_LOADING_DOCK/custom folder are
# loaded also.
sub rebuild_script_aliases {

  if (length($DEBUG_FEISTY_MEOW)) {
    print "rebuilding generated aliases file...\n";
  }

  # create our generated shells directory if it's not already.
  if ( ! -d $FEISTY_MEOW_LOADING_DOCK ) {
    mkdir $FEISTY_MEOW_LOADING_DOCK;
    if (length($DEBUG_FEISTY_MEOW)) {
      print "made FEISTY_MEOW_LOADING_DOCK at '$FEISTY_MEOW_LOADING_DOCK'\n";
    }
  }

  # test if we can use color in ls...
#  $test_color=` ls --help 2>&1 | grep -i color `;

  # this is an array of files from which to draw alias definitions.
  @ALIAS_DEFINITION_FILES = ("$FEISTY_MEOW_SCRIPTS/core/common.alias");

  # if custom aliases files exist, add them to the list.
#hmmm: would be nice to have this name in a symbol somewhere instead of having "custom" or "customize" everywhere.
  foreach $i (&glob_list("$FEISTY_MEOW_LOADING_DOCK/custom/*.alias")) {
    if (-f $i) { push(@ALIAS_DEFINITION_FILES, $i); }
  }
  if (length($DEBUG_FEISTY_MEOW)) {
    print "using these alias files:\n";
    foreach $i (@ALIAS_DEFINITION_FILES) {
      local $base_of_dir = &basename(&dirname($i));
      local $basename = &basename($i);
      print "  $base_of_dir/$basename\n";
    }
  }

  # write the aliases for sh and bash scripts.
  local $GENERATED_ALIAS_FILE = "$FEISTY_MEOW_LOADING_DOCK/fmc_core_and_custom_aliases.sh";
  if (length($DEBUG_FEISTY_MEOW)) {
    print "writing generated aliases in $GENERATED_ALIAS_FILE...\n";
  }

#hmmm: perhaps a good place for a function to create the header,
#      given the appropriate comment code.

  open GENOUT, ">$GENERATED_ALIAS_FILE" or die "cannot open $GENERATED_ALIAS_FILE";

  print GENOUT "##\n";
  print GENOUT "## generated file: $GENERATED_ALIAS_FILE\n";
  print GENOUT "## please do not edit.\n";
  print GENOUT "##\n";

#  if (length($test_color)) {
#    print GENOUT "export color_add='--color=auto'\n";
#  } else {
#    print GENOUT "export color_add=\n";
#  }

  # plow in the full set of aliases into the file.
  foreach $i (@ALIAS_DEFINITION_FILES) {
    open CURR_ALIASER, "<$i" or die "cannot open current alias file $i";
    foreach $line (<CURR_ALIASER>) {
      print GENOUT "$line";
    }
  }

  close GENOUT;

  if (length($DEBUG_FEISTY_MEOW)) {
    print("done rebuilding generated aliases file.\n");
  }
}

##############

# make sure we know where to store the files we're creating.
if ( ! length("$FEISTY_MEOW_LOADING_DOCK") ) {
  print "\
The FEISTY_MEOW_LOADING_DOCK variable is not defined.  This must point to\
the location where the generated scripts are stored.  You may still need to\
configure the feisty meow script system with something like:\
  bash /opt/feistymeow.org/feisty_meow/scripts/core/reconfigure_feisty_meow.sh\
Please see http://feistymeow.org for more details.\n";
  exit 1;
#really need to use better exit codes.
}

##############

$FEISTY_MEOW_LOADING_DOCK =~ s/\\/\//g;
$FEISTY_MEOW_SCRIPTS =~ s/\\/\//g;
$FEISTY_MEOW_APEX =~ s/\\/\//g;

##############

# create our generated shells directory if it's not already there.
if (! -d $FEISTY_MEOW_LOADING_DOCK) {
  mkdir $FEISTY_MEOW_LOADING_DOCK;
}

##############

# set the executable bit for binaries for just this current user.
if (-d $FEISTY_MEOW_BINARIES) {
  system("find \"$FEISTY_MEOW_BINARIES\" -type f -exec chmod u+x \"{}\" ';'");
}

##############

# generate the first set of alias files that are defined in the core
# and custom scripts directories.
&rebuild_script_aliases;

##############

# trash the old versions.
unlink("$FEISTY_MEOW_LOADING_DOCK/fmc_aliases_for_scripts.sh");

if (length($DEBUG_FEISTY_MEOW)) {
  printf "writing $FEISTY_MEOW_LOADING_DOCK/fmc_aliases_for_scripts.sh...\n";
}

##############

# open the alias files to be created.
open(she, ">> $FEISTY_MEOW_LOADING_DOCK/fmc_aliases_for_scripts.sh");

# find the list of files in the scripts directory.
@shell_files = (find_files(recursive_find_directories("$FEISTY_MEOW_SCRIPTS")),
    find_files("$FEISTY_MEOW_LOADING_DOCK/custom/scripts"),
    find_files(recursive_find_directories("$FEISTY_MEOW_LOADING_DOCK/custom/scripts")));

# strip out the customization files, since they are added in on demand only.
#print "before filtering list: @shell_files\n";
@shell_files = grep ! /\/customize\//, @shell_files;
#print "after filtering list: @shell_files\n";

#printf "found all these files in main script dirs:\n";
#printf "  @shell_files\n";

# construct aliases for items in the scripts directory.
foreach $file (@shell_files) {
  # test to see what type of item we got.
  if ($file =~ '^\.$'
      || $file =~ '^\.\.$'
      || $file =~ '^.svn$'
      || $file =~ '^.git$'
      || $file =~ /\/\.$/
      || $file =~ /\/\.\.$/
      || $file =~ /\/\.svn$/
      || $file =~ /\/\.git$/
      ) {
    # just skip this item; it's a special directory or a file we don't want to include.
    print "skipping name: $file\n";
  } else {
     &make_alias($file, "");
  }
}

close(she);

##############

# prepare a finalizer chunk that is the last thing to load.

open(she, ">> $FEISTY_MEOW_LOADING_DOCK/fmc_ending_sentinel.sh");

# write in our sentinel alias that says alias loading was handled.
print she "define_yeti_alias CORE_ALIASES_LOADED=true\n";

close(she);

##############

1;







####

def main() -> None:
    """ the main driver of activities for this app. """

#hmmm: unchecked below, just copied.
    # make sure they gave us a filename.
    args = len(sys.argv)
    if args < 2:
        print("\
This script needs a filename to operate on.  The file is expected to contain\n\
one line of certificate data, which this script will reformat into a standard\n\
PEM file format.  The PEM file will be output on the console.")
        exit(1)
 
    filename = sys.argv[1]

    # make sure the filename is valid.
    if not os.path.isfile(filename):
        print("The filename provided does not seem to be a readable file:", filename)
        exit(1)

    file = open(filename, "r")

    cert_line = file.readline()
    cert_line = cert_line.strip('\r\n')

    #ugh, no extra noise needed.
    #print()
    #print("below is the properly formatted output sourced from:", filename)
    #print()

####

if __name__ == "__main__":
    main()


