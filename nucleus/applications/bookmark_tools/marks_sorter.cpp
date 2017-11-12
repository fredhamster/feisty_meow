/*****************************************************************************\
*                                                                             *
*  Name   : marks_sorter                                                      *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Processes a link database in HOOPLE format and generates a new database  *
*  that is sorted and always uses category nicknames where defined.           *
*                                                                             *
*******************************************************************************
* Copyright (c) 2006-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "bookmark_tree.h"

#include <application/hoople_main.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <filesystem/byte_filer.h>
#include <filesystem/filename.h>
#include <loggers/combo_logger.h>
#include <loggers/critical_events.h>
#include <structures/static_memory_gremlin.h>
#include <textual/list_parsing.h>
#include <textual/parser_bits.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace nodes;
using namespace structures;
using namespace textual;

//#define DEBUG_MARKS
  // uncomment to have more debugging noise.

#undef BASE_LOG
#define BASE_LOG(s) program_wide_logger::get().log(s, ALWAYS_PRINT)
#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), \
   a_sprintf("line %d: ", _categories._line_number) + s)

const int MAX_FILE_SIZE = 4 * MEGABYTE;
  // the largest file we'll read.

////////////////////////////////////////////////////////////////////////////

class marks_sorter : public application_shell
{
public:
  marks_sorter()
      : application_shell(), _loader_count(0), _link_spool(0) {}
  DEFINE_CLASS_NAME("marks_sorter");
  virtual int execute();
  int print_instructions(const filename &program_name);

  int write_new_marks(const astring &output_filename);
    // given a tree of links, this writes out a new sorted file to the
    // "output_filename".

private:
  bookmark_tree _categories;  // our tree of categories.
  int _loader_count;  // count of the loader functions.
  int _link_spool;  // count of which link we're writing.
};

////////////////////////////////////////////////////////////////////////////

int marks_sorter::print_instructions(const filename &program_name)
{
  a_sprintf to_show("%s:\n\
This program needs two filenames as command-line parameters.  The -i flag\n\
is used to specify the input filename, which is expected to be in the HOOPLE\n\
link database format.  The -o flag specifies the new bookmarks file to be\n\
created, which will also be in the HOOPLE link format.\n\
The HOOPLE link format is documented here:\n\
    http://feistymeow.org/guides/link_database/format_manifesto.txt\n\
", program_name.basename().raw().s(), program_name.basename().raw().s());
  program_wide_logger::get().log(to_show, ALWAYS_PRINT);
  return 12;
}

int marks_sorter::execute()
{
  FUNCDEF("execute");
  SETUP_COMBO_LOGGER;

  command_line cmds(_global_argc, _global_argv);  // process the command line parameters.
  astring input_filename;  // we'll store our link database name here.
  astring output_filename;  // where the web page we're creating goes.
  if (!cmds.get_value('i', input_filename, false))
    return print_instructions(cmds.program_name());
  if (!cmds.get_value('o', output_filename, false))
    return print_instructions(cmds.program_name());

  BASE_LOG(astring("input file: ") + input_filename);
  BASE_LOG(astring("output file: ") + output_filename);

  filename outname(output_filename);
  if (outname.exists()) {
    non_continuable_error(class_name(), func, astring("the output file ")
        + output_filename + " already exists.  It would be over-written if "
        "we continued.");
  }

  int ret = _categories.read_csv_file(input_filename);
  if (ret) return ret;

  ret = write_new_marks(output_filename);
  if (ret) return ret;
  
  return 0;
}

int marks_sorter::write_new_marks(const astring &output_filename)
{
  FUNCDEF("write_new_marks");
  // open the output file for streaming out the new marks file.
  filename outname(output_filename);
  byte_filer output_file(output_filename, "w");
  if (!output_file.good())
    non_continuable_error(class_name(), func, "the output file could not be opened");

  bool just_had_return = false;  // did we just see a carriage return?
  bool first_line = true;  // is this the first line to be emitted?

  // traverse the tree in prefix order.
  tree::iterator itty = _categories.access_root().start(tree::prefix);
  tree *curr = NULL_POINTER;  // the current node.

  while ( (curr = itty.next()) ) {
    inner_mark_tree *nod = (inner_mark_tree *)curr;
    // set up a category printout for this node.
    string_array cat_list;
    cat_list += "C";
    cat_list += nod->name();
    inner_mark_tree *pare = (inner_mark_tree *)nod->parent();
    if (pare) {
      astring name_split, nick_split;
      _categories.break_name(pare->name(), name_split, nick_split);
      if (!nick_split) cat_list += name_split;
      else cat_list += nick_split;
    } else {
      cat_list += "";
    }

    // create a text line to send to the output file.
    astring tmp;
    list_parsing::create_csv_line(cat_list, tmp);
    tmp += "\n";
    if (!just_had_return && !first_line) {
      // generate a blank line before the category name.
      output_file.write(parser_bits::platform_eol_to_chars());
    }

    // reset the flags after we've checked them.
    just_had_return = false;
    first_line = false;

    output_file.write(tmp);
      // write the actual category definition.

    // print the links for all of the ones stored at this node.
    for (int i = 0; i < nod->_links.elements(); i++) {
      link_record *lin = nod->_links.borrow(i);
      if (!lin->_url) {
        // just a comment.
        astring descrip = lin->_description;
        if (descrip.contains("http:")) {
          // we'll clean the html formatting out that we added earlier.
          int indy = descrip.find('"');
          if (non_negative(indy)) {
            descrip.zap(0, indy);
            indy = descrip.find('"');
            if (non_negative(indy)) descrip.zap(indy, descrip.end());
          }
          descrip = astring("    ") + descrip;
             // add a little spacing.
        }
        if (descrip.t()) {
          output_file.write(astring("#") + descrip + "\n");
          just_had_return = false;
        } else {
          // this line's totally blank, so we'll generate a blank line.
          // we don't want to put in more than one blank though, so we check
          // whether we did this recently.
          if (!just_had_return) {
            output_file.write(parser_bits::platform_eol_to_chars());
            just_had_return = true;  // set our flag for a carriage return.
          }
        }
      } else {
        // should be a real link.
        string_array lnks;
        lnks += "L";
        lnks += lin->_description;
        // use just the nickname for the parent, if there is a nick.
        astring name_split;
        astring nick_split;
        _categories.break_name(nod->name(), name_split, nick_split);
        if (!nick_split) lnks += nod->name();
        else lnks += nick_split;
        lnks += lin->_url;
        list_parsing::create_csv_line(lnks, tmp);
        tmp += "\n";
        output_file.write(tmp);
        just_had_return = false;
      }
    }
  }

  output_file.close();

  BASE_LOG(a_sprintf("wrote %d links in %d categories.",
      _categories.link_count(), _categories.category_count()));
  BASE_LOG(astring());

  return 0;
}

////////////////////////////////////////////////////////////////////////////

HOOPLE_MAIN(marks_sorter, )

