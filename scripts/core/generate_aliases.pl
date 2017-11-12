#!/usr/bin/perl

##############
#
#  Name   : generate_aliases
#  Author : Chris Koeritz
#  Rights : Copyright (C) 1996-$now by Author
#
#  Purpose:
#
#    This script generates feisty meow script alias files.  Alias files
#  contain a list of definitions for command aliases that are written in the
#  specified shell dialect (such as bash or perl) and which are additionally
#  tailored for the operating system to be used.
#
##############
#  This program is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by the Free
#  Software Foundation; either version 2 of the License or (at your option)
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a
#  version of the License.  Please send any updates to "fred@gruntose.com".
##############

require "filename_helper.pl";

use Env qw(FEISTY_MEOW_BINARIES BUILD_TOP FEISTY_MEOW_APEX FEISTY_MEOW_LOADING_DOCK FEISTY_MEOW_SCRIPTS DEBUG_FEISTY_MEOW );

# given a possible aliasable filename, this will decide whether to create a perl
# or bash alias for it.  it needs the filename of the possible alias and the
# directory where that file resides.
sub make_alias {
  local($file, $dir) = @_;
  if ($file =~ /\.[pP][lL]$/) { 
    local($aliasname) = $file; $aliasname =~ s/\.[Pp][lL]$//;
    &make_perl_alias($aliasname, "$dir");
  } elsif ($file =~ /\.[sS][hH]$/) { 
    local($aliasname) = $file; $aliasname =~ s/\.[Ss][Hh]$//;
    &make_bash_alias($aliasname, "$dir");
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
  foreach $i (&glob_list("$FEISTY_MEOW_LOADING_DOCK/custom/*.alias")) {
    if (-f $i) { push(@ALIAS_DEFINITION_FILES, $i); }
  }
  if (length($DEBUG_FEISTY_MEOW)) {
    print "using these alias files:\n";
print "HEY IS THIS PROBLEM CHILD?\n";
    foreach $i (@ALIAS_DEFINITION_FILES) {
      local $base_of_dir = &basename(&dirname($i));
      local $basename = &basename($i);
      print "  $base_of_dir/$basename\n";
    }
print "WAS PROBLEM CHILD ABOVE HERE?\n";
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

$FEISTY_MEOW_LOADING_DOCK =~ s/\\/\//g;
$FEISTY_MEOW_SCRIPTS =~ s/\\/\//g;
$FEISTY_MEOW_APEX =~ s/\\/\//g;

# create our generated shells directory if it's not already there.
if (! -d $FEISTY_MEOW_LOADING_DOCK) {
  mkdir $FEISTY_MEOW_LOADING_DOCK;
}

##############

# set the executable bit for binaries for just this current user.
if (-d $FEISTY_MEOW_BINARIES) {
  system("chmod -R u+x \"$FEISTY_MEOW_BINARIES\"/*");
}

# generate the first set of alias files that are defined in the core
# and custom scripts directories.
&rebuild_script_aliases;

# trash the old versions.
unlink("$FEISTY_MEOW_LOADING_DOCK/fmc_aliases_for_scripts.sh");

if (length($DEBUG_FEISTY_MEOW)) {
  printf "writing $FEISTY_MEOW_LOADING_DOCK/fmc_aliases_for_scripts.sh...\n";
}

# open the alias files to be created.
open(she, ">> $FEISTY_MEOW_LOADING_DOCK/fmc_aliases_for_scripts.sh");

# find the list of files in the scripts directory.
@shell_files = (find_files(recursive_find_directories("$FEISTY_MEOW_SCRIPTS")),
    find_files(recursive_find_directories("$FEISTY_MEOW_LOADING_DOCK/custom/scripts")));

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
      || $file =~ /\/customize\/[a-zA-Z0-9_]+\/[a-zA-Z0-9_.]+$/
#hmmm: would be nice to have this name in a symbol somewhere instead of having "customize" everywhere.
      ) {
    # just skip this item; it's a special directory or a file we don't want to include.
#    print "skipping name: $file\n";
  } else {
     &make_alias($file, "");
  }
}

close(she);

1;
