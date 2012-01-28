#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : generate_aliases                                                  #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    This script generates YETI alias files.  Alias files contain a list of   #
#  definitions for command aliases that are written in the specified shell    #
#  dialect (such as bash or perl) and which are additionally tailored for the #
#  operating system to be used.                                               #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

require "importenv.pl";

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
  $aliasname =~ s/^.*\/([^\/]*)/\1/;
#print "alias became $aliasname\n";
  local($source_dir) = shift(@_);
#print "bash alias is $aliasname, dir is $source_dir\n";
  print she "alias $aliasname=\"bash $source_dir/$full_alias.sh\"\n";
}

# makes an alias for a perl script given the alias name.
sub make_perl_alias {
  local($aliasname) = shift(@_);
  local($full_alias) = $aliasname;
  $aliasname =~ s/^.*\/([^\/]*)/\1/;
#print "alias became $aliasname\n";
  local($source_dir) = shift(@_);
#print "perl alias is $aliasname, dir is $source_dir\n";
  print she "alias $aliasname=\"perl $source_dir/$full_alias.pl\"\n";
}

# given a directory, this returns an array of all the filenames found therein.
sub load_file_names {
  local($path) = shift(@_);
  opendir(that_dir, $path);
  local(@those_files) = sort(readdir(that_dir));
  return @those_files;
}

##############

# make sure we know where to store the files we're creating.
if ( ! length("$FEISTY_MEOW_GENERATED") ) {
  print "\
The FEISTY_MEOW_GENERATED variable is not defined.  This must point to the location where\n\
the generated scripts are stored.  Perhaps you still need to run\n\
bootstrap_shells.sh and set up some environment variables.  Please see\n\
http://yeticode.org for more details.\n";
  exit 1;
#really need to use better exit codes.
}

$FEISTY_MEOW_GENERATED =~ s/\\/\//g;
$FEISTY_MEOW_SCRIPTS =~ s/\\/\//g;
$FEISTY_MEOW_DIR =~ s/\\/\//g;

# create our generated shells directory if it's not already there.
if (! -d $FEISTY_MEOW_GENERATED) {
  mkdir $FEISTY_MEOW_GENERATED;
}

##############

# set the executable bit for yeti binaries for just this current user.
if (-d $BINDIR) {
  system("chmod -R u+x \"$BINDIR\"/*");
}

##############

system("bash \"$FEISTY_MEOW_SCRIPTS\"/core/unter_alia.sh");
  # generate the first set of alias files; these are the root files used
  # by the shell.  each of them will be written to in turn invoke the
  # p_alias files which are made from the set of scripts in FEISTY_MEOW_SCRIPTS
  # (see below).

# trash the old versions.
unlink("$FEISTY_MEOW_GENERATED/p_alias.sh");

printf "writing $FEISTY_MEOW_GENERATED/p_alias.sh...\n";

# open the alias files to be created.
open(she, ">> $FEISTY_MEOW_GENERATED/p_alias.sh");

#print "os is $OS\n";

# find the list of files in the scripts directory.
#opendir(scripts, "$FEISTY_MEOW_SCRIPTS");
#@shell_files = sort(readdir(scripts));
#print "yeti scripts: @shell_files\n";

@shell_files = &load_file_names("$FEISTY_MEOW_SCRIPTS");

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
    # just skip this item; it's a special directory.
  } elsif (-d "$FEISTY_MEOW_SCRIPTS/$file") {
    # if we see a subdirectory in the scripts folder, we add all the
    # scripts in it as aliases.  we recurse only one level.
    opendir(subdir, "$FEISTY_MEOW_SCRIPTS/$file");
    @subdir_files = sort(readdir(subdir));
    foreach $subfile (@subdir_files) {
      push(@shell_files, "$file/$subfile");
    }
  } else {
    # if it's a regular file, we'll try to make an alias for it.  the function
    # will only fire if the ending is appropriate for the script languages we use.
    &make_alias($file, "$FEISTY_MEOW_SCRIPTS");
  }
}

# open the source repository's script directory to find scripts in there.
local($build_shell_path) = "$BUILD_TOP/scripts/generator";
@build_shells = &load_file_names("$build_shell_path");
#opendir(build_shells_dir, $build_shell_path);
#@build_shell_files = sort(readdir(build_shells_dir));
#if (scalar(@build_shell_files) > 0) {
#  print "build shell folders: @build_shell_files\n";
#}
foreach $file (@build_shells) {
  &make_alias($file, "$build_shell_path");
}

close(she);

