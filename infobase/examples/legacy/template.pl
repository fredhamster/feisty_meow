#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : template                                                          #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Attempts to pre-instantiate C++ templates to work-around C++ compilers   #
#  that don't support templates (a rare breed, these days).                   #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

# this was a majestic abortive attempt to create a template instantiator for
# compilers that do not possess templates, but which do support some subset
# of C++.  This was necessary at the time, due to our firmware compiler's
# limitations.  This processor never totally worked, although it did produce
# some interesting compilable code.  Might be useful as a demo or maybe just
# as a warning to avoid brain-damaged C++ compilers.

# to do:
#   maintain statistics about placement in file for error resolution.

# limitations so far:
#
# the word "template" must be the first word on the line.
#
# the type to instantiate must be one word (like charstar, not char *).
#
# templates must follow the form templateName<templateType> without
#   any spaces between the angle brackets.

# flag to enable debugging print outs.
$DEBUG_TEMPLATIZER = 1;
#$DEBUG_TEMPLATIZER = 0;

# array to store instance types to convert to
@instance_type_buffer = ();
# flag for checking read from file option
$f_option = 0;
$d_option = 0;

if ($#ARGV < 1) {
  die("
    The template instantiater supports an optional directory path as the first
    parameter (preceded by a -d with no spaces in between the d and the
    directory name) in which to store the generated templates, and then 
    requires the instantiation type as the next argument (or a file
    specification preceded by -f), and a list of files as the last
    arguments.
    The files will be scanned for templates and those templates will be
    instantiated in the type(s) specified.
    Examples:
       perl template.pl char istring.h torpedo.h
       perl template.pl -f instance.txt matrix.h
       perl template.pl -d. -f instance.txt function.h data_src.h
       perl template.pl -dfirm_src\library\basis -f instance.txt amorph.h\n");
}

# Check directory option
if (grep(/^\s*-d/, @ARGV)) {
   $d_option = 1;
   $d_dir = @ARGV[0];
#   print $d_dir, "\n";
   shift;
}


# Check to see if user used a file to specify instantiation types
if (grep(/^\s*-f/, @ARGV)) {
   $f_option = 1;
   shift;
   $types_file = @ARGV[0];

# Open instantiation type file to read from
   open(TYPES_FILE, "<$types_file")
     || die("couldn't open file $types_file for reading");

# Read in all the different types to instantiate
# Create instance_type list
   @tp = <TYPES_FILE>;
   while (@tp) {
      local($line) = @tp;
      chop $line;
      push(@instance_type_buffer, $line);
      shift @tp;
   }
   shift @ARGV;
   &instantiate_templates(@ARGV);
   exit;
}

&instantiate_templates(@ARGV);
exit;

#
# driver of the instantiation process.
#
sub instantiate_templates {
  if (!$f_option) {
  # grab the user's desired instance type.
  $instance_type = @_[0];
  push(@instance_type_buffer, $instance_type);
  print "Instantiation type is \"$instance_type\".\n";
  # jump over the instance type to look at the filenames.
  shift;
  }

  local($i) = 0;
  foreach $filename (@_) {
    open(INPUT_FILE, "<$filename")
      || die("couldn't open file $filename for reading");
    # create an output name for the instance.
    $out_filename = &make_output_name($filename);
    if ($DEBUG_TEMPLATIZER) {
#      print "out file is ", $out_filename, "\n";      
    }
    local($index) = $i + 1;
    print "Instantiating file[$index] as $out_filename.\n";
    # now try opening our output file.
    open(OUTPUT_FILE, ">$out_filename")
      || die("couldn't open file $filename for writing");
    # grab the current file into an array.

    @file_array = <INPUT_FILE>;
    @start_template = @file_array;
    @stop_template = @file_array;
    # process the file's contents as a manipulable array.
    while (@file_array) {
      local($line) = shift @file_array;
      if (grep(/^\s*template/, $line)) {
        @start_template = @file_array;

        # iterate through all the instance types for each template
        foreach $instance_type (@instance_type_buffer) {
           @file_array = @start_template;
           &snag_place_holder($line);
           &snag_object_name;
           &absorb_until_matched_block;
           &replace_place_holder;
           &dump_the_buffer;
           print OUTPUT_FILE "\n";
        }
      } elsif (grep(/\w+<\w+>/, $line)) {
        local(@pieces) = split(/\s/, $line);
        foreach $piece (@pieces) {
          local($prefix) = "";
          # special case for separating function name from templated first
          # parameter to it.
          if (grep(/\(\w+</, $piece)) {
            local(@chop_paren) = split(/\(/, $piece, 2);
            $prefix = $chop_paren[0].'(';
            $piece = $chop_paren[1];
          }
          if (grep(/\w+<\w+>/, $piece)) { $piece = &special_mangle($piece); }
          print OUTPUT_FILE "$prefix$piece ";
        }
        print OUTPUT_FILE "\n";
      } else {
          print OUTPUT_FILE $line;
      }
    }
    $i++;
  }
}

#
# generates an output name from the filename to be translated.
#
sub make_output_name {
  local($out_filename) = @_[0];
  local($d_dir_temp) = $d_dir;
#  print "OUTFILE NAME: ",$out_filename,"\n";
  # break down the filename at the slashes.
  local(@split_filename) = split(/[\\\/]/, $out_filename);
  # take the basename of the list of names.
  $out_filename = $split_filename[$#split_filename];
  local($hold_filename) = $out_filename;
  if (grep(!/\.cpp$/i, $out_filename) && grep(!/\.h$/i, $out_filename)
      && grep(!/\.c$/i, $out_filename) && grep(!/\.h$/i, $out_filename) ) {
    die("filename @_[0] not recognized as a C++ code file.");
  }
  # makes an instance of the file in a directory named after the instance type
  # that is located under the current directory.

  $d_dir_temp = join('/',$d_dir, $hold_filename);
  if ($d_option) {
     $d_dir_temp =~ s/-d//i;
     @split_filename = split(/[\\\/]/, $d_dir_temp);
#     exit;
  }

# try to create dir using the deepest dir given in filename input

    local($y) = 0;
    foreach (@split_filename) { $y++; }

    local($x) = 0;
    local($ret) = 0;
    local($dirs) = 0;

    if ($y >= 2) {
      foreach (@split_filename) {
       if ((($x > 0) && ($x < $y-1)) || (($d_option) && ($x < $y-1))) {
         if (!$dirs) { $dirs = @split_filename[$x]; }
         else { $dirs = $dirs."/".@split_filename[$x]; }
#         print "Creating... ",$dirs,"\n";
         $ret = mkdir($dirs, 0777);
         if (!ret) { die("a directory named $instance_dir could not be made."); }
       }
       $x++;
      }
      $out_filename = $dirs."/".$hold_filename;
    }
    else { $out_filename = "template/".$hold_filename;
         local($instance_dir) = "template";
         $ret = mkdir($instance_dir, 0777);
         if (!ret) { die("a directory named $instance_dir could not be made."); }
    }
#   print $out_filename, "\n";

#  local($instance_dir) = @split_filename[$x-2];
#  creates the directory.
#  local($ret) = mkdir($instance_dir, 0777);
#  if (!ret) { die("a directory named $instance_dir could not be made."); }

  $out_filename;  # return the new name.
}

#
# grabs the name of the placeholder type that will be replaced by
# the template instantiation type.
#
sub snag_place_holder {
  $place_holder = @_[0];
  chop $place_holder;

  local(@pieces) = split(/>\s*/, $place_holder, 2);

  # send back the parts not involved in the template statement.
  if (length($pieces[1])) {
     unshift(@file_array, $pieces[1]."\n");
  }
  $place_holder = $pieces[0];
  $place_holder =~ s/\s*template\s+<class\s+(\w+)$/\1/;
  if ($DEBUG_TEMPLATIZER) {
#    print "Replacing place holder \"$place_holder\" with \"$instance_type\".\n";
  }
}

#
# grabs the name of the object itself that will become an instantiated
# object in the type specified.  the global variable "object_name" is
# set by the subfunctions used here.
#
sub snag_object_name {
  local($next_line) = shift(@file_array);
  chop $next_line;
  &match_class_declaration($next_line)
    || &match_class_member_definition($next_line)
      || &match_function_definition($next_line);
}

#
# creates a mangled form of the name that includes the instantiation
# type.  the global variable "mangled_name" is set by this function.
#
sub mangle_name {
  local($to_grind) = @_[0];
  local($mangled_name) = "template__".$to_grind."__".$instance_type;
  if ($DEBUG_TEMPLATIZER) {
#    print "Replacing name \"$to_grind\" with \"$mangled_name\".\n";
  }
  $mangled_name;
}

#
# processes "#include" preprocessor directives to make sure if the filename
# is in there to include a C++ file (for the template code), then it gets
# converted to the new file name.
#

# this is a pretty bogus thing; it should not be used.

sub convert_inclusion {
  local($line) = @_[0];
  chop $line;
  local($temp) = $line;
  # extract out the name parts of the include declaration.
  $temp =~ s/\s*#include\s*([<"])([\w.]+)([>"])/\1 \2 \3/;
  local(@broken_up) = split(/ /, $temp);
  # strip off the punctuation from the name.
  local($incl_prefix) = @broken_up[1];
  $incl_prefix =~ s/["<](.*)[">]/\1/;
  $incl_prefix =~ s/\s//g;
  # return if it's not a code file being included.
  if (!grep(/.cpp/i, $incl_prefix)) { print OUTPUT_FILE $line, "\n"; return; }
  # strip to just the name without the ending.
  $incl_prefix =~ s/\.cpp$//i;
  # now get the name of the file we're processing.
  local($file_prefix) = $filename;
  # return if it's not a header file being examined.
  if (!grep(/.h/i, $file_prefix)) { print OUTPUT_FILE $line, "\n"; return; }
  # strip off the extension.
  $file_prefix =~ s/\.h$//i;
  # return if the names aren't equivalent--this means the include doesn't
  # refer to our new templated form of the code file.
  if ($incl_prefix ne $file_prefix) { print OUTPUT_FILE $line, "\n"; return FALSE; }
  # dump out a message about the removal.
  $line =~ s/^\s*//;
  print OUTPUT_FILE "/* removed unneeded template inclusion: $line */\n";
}

#
# extracts lines from the file until the curly brackets are matched up
# at level 0.
#
sub absorb_until_matched_block {
  $bracket_level = 0;
  $end_absorb = 0;
  @template_buffer = ();
  $hit_one=0;
  while (@file_array) {
    local($line) = shift @file_array;
    &look_for_curlies($line);
    if (($hit_one && ($bracket_level == 0)) || $end_absorb) { return; }
  }
}

#
# examines the parameters passed in for curly brackets and changes the
# counter if they are found.
#
sub look_for_curlies {
#  $hit_one = 0;  # records whether a bracket was found or not.
  local($line) = @_[0];
  @word = ();
  foreach $char (split(//, $line)) {
    if ($char eq '{') {
      $hit_one = 1;
      $bracket_level++;
    } elsif ($char eq '}') {
      $hit_one = 1;
      $bracket_level--;
    } elsif (($char eq ';') && ($hit_one==0)) {
      $end_absorb = 1;
    }


    if ($DEBUG_TEMPLATIZER) {
#      print "~$char~ ";
    }
    push(@word, $char);
    if (grep(!/\w/, $char)) {
      # don't split yet if it's a possible template char.
      if (grep(!/[<>]/, $char)) {
        local($real_word) = join("", @word);
        if ($DEBUG_TEMPLATIZER) {
#          print "adding a word $real_word\n";
        }
        push(@template_buffer, "$real_word");
        @word = ();
      }
    }
  }
}

#
# this goes through the buffer and replaces all occurrences of the name to
# replace with the instance name.
#
sub replace_place_holder {
  @new_template_buffer = @template_buffer;
  @template_buffer = ();

  foreach $i (0 .. $#new_template_buffer) {
    $word = $new_template_buffer[$i];
#    if ($DEBUG_TEMPLATIZER) {
#      print "<$i $word> ";
#      $old = $word;
#    }

    # replace a templated combination with the mangled version.
    $word =~ s/^${object_name}<${instance_type}>/${mangled_name}/;

#    if ($DEBUG_TEMPLATIZER) {
#      if ($old ne $word) {print "1 ... changed to $word.\n"; $old = $word; }
#    }

    if (grep(/^\w+<\w+>/, $word)) {
      # replace some other template with our stuff if we can.
      $word = &special_mangle($word);
    }

#    if ($DEBUG_TEMPLATIZER) {
#      if ($old ne $word) {print "2 ... changed to $word.\n"; $old = $word; }
#    }

    # replace the object's name with its mangled form.
    $word =~ s/^${object_name}/${mangled_name}/;

#    if ($DEBUG_TEMPLATIZER) {
#      if ($old ne $word) {print "3... changed to $word.\n"; $old = $word; }
#    }

    # replace the place holder with the instantiation type.
    $word =~ s/^${place_holder}/${instance_type}/;

#    if ($DEBUG_TEMPLATIZER) {
#      if ($old ne $word) {print "4... changed to $word.\n"; $old = $word; }
#    }

    push(@template_buffer, $word);
  }
}

#
# processes a general template usage, in the form X<Y>, where either
# X or Y are not ones that we think we need to replace.  it is assumed
# that it's safe to use the mangled form of the template.
#
sub special_mangle {
  local($word) = @_[0];
  # split the template form into pieces.
  local(@pieces) = split(/[<>]/, $word, 2);

  $pieces[1] =~ s/${place_holder}/${instance_type}/;
  $pieces[1] =~ s/>//;
  # hold onto the real instance type.
  local($hold_instance) = $instance_type;
  $instance_type = $pieces[1];
  # mangle the name in the template usage line.
  local($hold_mangled) = &mangle_name($pieces[0]);
  # restore the original instance type.
  $instance_type = $hold_instance;
  # returns the new mangled form.
  $hold_mangled;
}

#
# prints out the buffer we've accumulated to the output file.
#
sub dump_the_buffer {
  print OUTPUT_FILE @template_buffer;
}

#
# processes a class declaration and sets the object name for future use.
#
sub match_class_declaration {
  local($next_line) = @_[0];
# too strict!
#  if (grep(!/class\s.*\w+$/, $next_line)
#      && grep(!/class\s.*\w+\s*\{/, $next_line)
#      && grep(!/struct\s.*\w+$/, $next_line)
#      && grep(!/struct\s.*\w+\s*\{/, $next_line)) {
#    return 0;
#  }

  if (grep(!/class\s+\w+/, $next_line) && grep(!/struct\s+\w+/, $next_line) ) {
    return 0;
  }

  if ($DEBUG_TEMPLATIZER) {
#    print "matched class decl in $next_line\n";
  }

  if (grep(/class\s+\w+.*:/, $next_line)
      || grep(/struct\s+\w+.*:/, $next_line)) {
    # parses an inheriting class decl.
    if ($DEBUG_TEMPLATIZER) {
#      print "in inheritance case on $next_line\n";
    }
    local(@pieces) = split(/:/, $next_line, 2);
    # push the rest of the line back into the input array.
    if ($DEBUG_TEMPLATIZER) {
#      print "going to unshift $pieces[1]...\n";
    }
    unshift(@file_array, ": ".$pieces[1]." ");
    $next_line = $pieces[0];
  } elsif (grep(/class\s.*\w+\s*\{/, $next_line)
      || grep(/struct\s.*\w+\s*\{/, $next_line)) {
    # parses a non-inheriting declaration with bracket on same line.
    if ($DEBUG_TEMPLATIZER) {
#      print "in special case on $next_line\n";
    }
    # special case for continued stuff on same line.
    local(@pieces) = split(/{/, $next_line, 2);
    # push the rest of the line back into the input array.
    unshift(@file_array, " { ".$pieces[1]." ");
    $next_line = $pieces[0];
  }
  if ($DEBUG_TEMPLATIZER) {
#    print "matched class declaration... $next_line\n";
  }
  local(@pieces) = split(/\s/, $next_line);
  $object_name = $pieces[$#pieces];
  $mangled_name = &mangle_name($object_name);
  foreach $posn (0 .. $#pieces - 1) { print OUTPUT_FILE "$pieces[$posn] "; }
  print OUTPUT_FILE "$mangled_name\n";
  1;
}

#
# processes the implementation of a class member and sets the object
# name for future use.
#
sub match_class_member_definition {
  local($next_line) = @_[0];
  local($junk);
  if (grep(!/\w+<\w+>::/, $next_line)) {
    return 0;
  }
  if ($DEBUG_TEMPLATIZER) {
#    print "matched class member definition... $next_line\n";
  }
  local(@pieces) = split(/>::/, $next_line, 2);
  # checks for spaces in the first part of the split.  if there is one,
  # it means we don't have a simple object thing.
  if (grep(/\s/, $pieces[0])) {
    if ($DEBUG_TEMPLATIZER) {
#      print "matched a space in the first part of supposed object name... $pieces[0]\n";
    }
    if (grep(/^\w+<\w+>/, $pieces[0])) {
      if ($DEBUG_TEMPLATIZER) {
#        print "matched a template usage in first part of name...";
      }
      # replace some other template with our stuff if we can.
      $pieces[0] = &special_mangle($pieces[0]);
    }
    if ($DEBUG_TEMPLATIZER) {
#      print "now our first bit is: $pieces[0]\n";
    }
    local(@new_pieces) = split(/ /, $pieces[0]);
    $pieces[0] = $new_pieces[$#new_pieces];
    foreach $posn (0 .. $#new_pieces - 1) {
      $new_pieces[$posn] =~ s/${place_holder}/${instance_type}/g;
      print OUTPUT_FILE "$new_pieces[$posn] ";
    }
  }
  unshift(@file_array, "::\n".$pieces[1]."\n");
  $object_name = $pieces[0];
  $object_name =~ s/(\W*)(\w+)<(\w+)/\2/;
  if (length($1)) { print OUTPUT_FILE "$1"; }
  if ($3 ne $place_holder) {
    die("The placeholder does not match on this line: $next_line");
  }
  $mangled_name = &mangle_name($object_name);
  print OUTPUT_FILE "$mangled_name\n";
  1;  # return success.
}

#
# processes a function template by making sure it fits the format and
# then setting up the variables for the replacement.  since function templates
# are so simple, the object name is not changed; only the place_holder is
# changed to the instance type.
#
sub match_function_definition {
  local($next_line) = @_[0];

  if (grep(!/^\s*\w+\s+.*/, $next_line) ) {
    if ($DEBUG_TEMPLATIZER) {
      print "failed on funcdef for ", $next_line, "!\n";
    }
    return 0;
  }

# old broken code:...
#  if (grep(!/^\s*\w+\s+.*\(.*\)\s*/, $next_line) ) {
#print "failed on funcdef for ", $next_line, "!\n";
#    return 0;
#  }

#  if ($DEBUG_TEMPLATIZER) {
#    print "matched function definition on $next_line\n";
#  }

#  if ($DEBUG_TEMPLATIZER) {
#   print "stuffing back into the file array $next_line.\n";
#  }
  # put the line back because it's nearly right for being instantiated.
  unshift(@file_array, "inline ".$next_line."\n");
  # come up with a very rare name that will not be matched in the text.
  $object_name = "hogga_wogga_nunky_budget_weeny_teeny_kahini_beany";
  $mangled_name = &mangle_name($object_name);
  1;  # return a success.
}
