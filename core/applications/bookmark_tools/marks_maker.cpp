//////////////
// Name   : marks_maker
// Author : Chris Koeritz
//////////////
// Copyright (c) 2005-$now By Author.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation:
//     http://www.gnu.org/licenses/gpl.html
// or under the terms of the GNU Library license:
//     http://www.gnu.org/licenses/lgpl.html
// at your preference.  Those licenses describe your legal rights to this
// software, and no other rights or warranties apply.
// Please send updates for this code to: fred@gruntose.com -- Thanks, fred.
//////////////

#include "bookmark_tree.h"

#include <application/hoople_main.h>
#include <application/command_line.h>
#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <filesystem/byte_filer.h>
#include <filesystem/filename.h>
#include <loggers/file_logger.h>
#include <timely/time_stamp.h>
#include <structures/static_memory_gremlin.h>
#include <textual/list_parsing.h>
#include <textual/string_manipulation.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace nodes;
using namespace structures;
using namespace textual;
using namespace timely;

#define DEBUG_MARKS
  // uncomment to have more debugging noise.

#undef BASE_LOG
#define BASE_LOG(s) program_wide_logger::get().log(s, ALWAYS_PRINT)
#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

const int MAX_FILE_SIZE = 4 * MEGABYTE;
  // the largest file we'll read.

const int SPACING_CHUNK = 4;
  // number of spaces per indentation level.

const int MAX_URL_DISPLAYED = 58;
const int MAX_DESCRIP_DISPLAYED = 72;

////////////////////////////////////////////////////////////////////////////

class marks_maker : public application_shell
{
public:
  marks_maker();

  enum output_style {
    ST_HUMAN_READABLE,
    ST_MOZILLA_MARKS,
//    ST_JAVASCRIPT_BASED... separate implementation currently.
  };

  int write_marks_page(const astring &output_filename,
          const astring &template_filename, output_style way);
    // given a tree of links, this writes out a web page to "output_filename"
    // using a template file "template_filename".

  DEFINE_CLASS_NAME("marks_maker");
  int print_instructions(const filename &program_name);
  virtual int execute();

private:
  bookmark_tree c_categories;  // our tree of categories.
  int c_current_depth;  // current indentation depth in list.
  output_style c_style;  // style of marks to write, set after construction.

  void increase_nesting(astring &output);
    // adds a new level of nesting to the text.

  void decrease_nesting(astring &output);
    // drops out of a level of nesting in the text.

  astring recurse_on_node(inner_mark_tree *nod);
    // the main recursive method that spiders down the tree.  it is important that it builds
    // the string through composition rather than being given a string reference, since it
    // expands all sub-trees as it goes.

  void inject_javascript_function(astring &output);
    // replaces a special phrase in the template file with our javascript-based link opener.

  void write_category_start(const astring &name, int node_depth, astring &output);
    // outputs the text for categories and adjusts the indentation level.

  void write_category_end(int depth, astring &output);
    // closes a category appropriately for the nesting depth.

  void write_link(inner_mark_tree *node, const link_record &linko,
         astring &output, int depth);
    // outputs the text for web links.
};

////////////////////////////////////////////////////////////////////////////

marks_maker::marks_maker()
: application_shell(),
  c_current_depth(0),
  c_style(ST_HUMAN_READABLE)
{}

int marks_maker::print_instructions(const filename &program_name)
{
  a_sprintf to_show("%s:\n\
This program needs three filenames as command line parameters.  The -i flag\n\
is used to specify the input filename, the -t flag specifies a template web\n\
page which is used as the wrapper around the links, and the -o flag specifies\n\
the web page to be created.  The input file is expected to be in the HOOPLE\n\
link database format.  The output file will be created from the template file\n\
by finding the phrase $INSERT_LINKS_HERE in it and replacing that with html\n\
formatted link and categories from the input file.  Another tag of $TODAYS_DATE\n\
will be replaced with the date when the output file is regenerated.  A final\n\
tag of $INSERT_JAVASCRIPT_HERE is replaced with a link opening function.\n\
Note that an optional -s flag can specify a value of \"human\" readable\n\
or \"mozilla\" bookmarks style to specify the style of the output file\n\
generated.\n\
The HOOPLE link format is documented here:\n\
    http://hoople.org/guides/link_database/format_manifesto.txt\n\
", program_name.basename().raw().s(), program_name.basename().raw().s());
  program_wide_logger::get().log(to_show, ALWAYS_PRINT);
  return 12;
}

void marks_maker::increase_nesting(astring &output)
{
  FUNCDEF("increase_nesting");
  int spaces = SPACING_CHUNK * c_current_depth;
  c_current_depth++;
#ifdef DEBUG_MARKS
  LOG(a_sprintf("++increased depth to %d...", c_current_depth));
#endif
  output += string_manipulation::indentation(spaces);
  output += "<dl><p>";
  output += parser_bits::platform_eol_to_chars();
}

void marks_maker::decrease_nesting(astring &output)
{
  FUNCDEF("decrease_nesting");
  c_current_depth--;
#ifdef DEBUG_MARKS
  LOG(a_sprintf("--decreased depth to %d...", c_current_depth));
#endif
  int spaces = SPACING_CHUNK * c_current_depth;
  output += string_manipulation::indentation(spaces);
  output += "</dl><p>";
  output += parser_bits::platform_eol_to_chars();
}

void marks_maker::write_category_start(const astring &name, int node_depth, astring &output)
{
  FUNCDEF("write_category_start");

  // calculate proper heading number.
  int heading_num = node_depth + 1;
  astring heading = a_sprintf("%d", heading_num);
  // force a weird requirement for mozilla bookmarks, all headings must be set at 3.
  if (c_style == ST_MOZILLA_MARKS) heading = "3";

#ifdef DEBUG_MARKS
  LOG(astring("header [") + name + "] level " + a_sprintf("%d", node_depth));
#endif

  // output our heading.
  output += string_manipulation::indentation(c_current_depth * SPACING_CHUNK);
  output += "<dt><h";
  output += heading;
  output += ">";
  output += name;
  output += "</h";
  output += heading;
  output += ">";
  output += "</dt>";
  output += parser_bits::platform_eol_to_chars();

  increase_nesting(output);
}

void marks_maker::write_category_end(int depth, astring &output)
{
  FUNCDEF("write_category_end");
  decrease_nesting(output);
}

void marks_maker::write_link(inner_mark_tree *formal(node),
    const link_record &linko, astring &output, int depth)
{
  FUNCDEF("write_link");
  // write an html link definition.
  if (!linko._url) {
    // this just appears to be a comment line.

    if (!linko._description) return;  // it's a nothing line.

    output += linko._description;
    output += "<br>";
    output += parser_bits::platform_eol_to_chars();
    return;
  }

  astring chomped_url = linko._url;
  if (c_style != ST_MOZILLA_MARKS) {
    if (chomped_url.length() > MAX_URL_DISPLAYED) {
      chomped_url.zap(MAX_URL_DISPLAYED / 2,
          chomped_url.length() - MAX_URL_DISPLAYED / 2 - 1);
      chomped_url.insert(MAX_URL_DISPLAYED / 2, "...");
    }
  }

  astring description = linko._description;
  if (c_style != ST_MOZILLA_MARKS) {
    // this is chopping the tail off, which seems reasonable for a very long description.
    if (description.length() > MAX_DESCRIP_DISPLAYED) {
      description.zap(MAX_DESCRIP_DISPLAYED - 1, description.end());
      description += "...";
    }
  }

  // new output format, totally clean and simple.  description is there
  // in readable manner, and it's also a link.  plus, this takes up a fraction
  // of the space the old way used.
  astring indentulus = string_manipulation::indentation(c_current_depth * SPACING_CHUNK);
  output += indentulus;
  output += "<dt><li>";
  output += "<a href=\"";
  output += linko._url;
  output += "\">";
  output += description;
  output += "</a>";

  if (c_style != ST_MOZILLA_MARKS) {
    output += "&nbsp;&nbsp;&nbsp;";
    output += "<a href=\"javascript:open_mark('";
    output += linko._url;
    output += "')\">";
    output += "[launch]";
    output += "</a>";
  }

  output += "</li>";
  output += "</dt>";
  output += parser_bits::platform_eol_to_chars();
}

astring marks_maker::recurse_on_node(inner_mark_tree *nod)
{
  FUNCDEF("recurse_on_node");
  astring to_return;

  // print out the category on this node.
  write_category_start(nod->name(), nod->depth(), to_return);

  // print the link for all of the ones stored at this node.
  for (int i = 0; i < nod->_links.elements(); i++) {
    link_record *lin = nod->_links.borrow(i);
    write_link(nod, *lin, to_return, nod->depth());
  }

  // zoom down into sub-categories.
  for (int i = 0; i < nod->branches(); i++) {
    to_return += recurse_on_node((inner_mark_tree *)nod->branch(i));
  }

  // finish this category.
  write_category_end(nod->depth(), to_return);

  return to_return;
}

void marks_maker::inject_javascript_function(astring &output)
{
  FUNCDEF("inject_javascript_function");
  astring scrip = "\n\
<script language=\"javascript1.2\">\n\
<!--\n\
function open_mark(url) {\n\
  if (typeof open_mark.next_num == 'undefined') {\n\
    // must initialize this before use.\n\
    open_mark.next_num = 0;\n\
  }\n\
  // pick the next number for our auto-generated name.\n\
  open_mark.next_num++;\n\
  winname = \"wingo\" + open_mark.next_num;\n\
  // open URL they asked for and give its window all permissions.\n\
  winner = window.open(url, winname);\n\
  // bring that new window into focus so they can see it.\n\
  winner.focus();\n\
}\n\
//-->\n\
</script>\n\
\n";

  bool found_it = output.replace("$INSERT_JAVASCRIPT_HERE", scrip);
  if (!found_it)
    non_continuable_error(class_name(), func, "the template file is missing "
        "the insertion point for '$INSERT_JAVASCRIPT_HERE'");
}

int marks_maker::write_marks_page(const astring &output_filename,
    const astring &template_filename, output_style style)
{
  FUNCDEF("write_marks_page");
  c_style = style;  // set the overall output style here.
  astring long_string;
    // this is our accumulator of links.  it is the semi-final result that will
    // be injected into the template file.

  // generate the meaty portion of the bookmarks.
  increase_nesting(long_string);
  inner_mark_tree *top = (inner_mark_tree *)&c_categories.access_root();
  long_string += recurse_on_node(top);
  decrease_nesting(long_string);

  byte_filer template_file(template_filename, "r");
  astring full_template;
  if (!template_file.good())
    non_continuable_error(class_name(), func, "the template file could not be opened");
  template_file.read(full_template, MAX_FILE_SIZE);
  template_file.close();

  // spice up the boring template with a nice link opening function.
  inject_javascript_function(full_template);

  // replace the tag with the long string we created.
  bool found_it = full_template.replace("$INSERT_LINKS_HERE", long_string);
  if (!found_it)
    non_continuable_error(class_name(), func, "the template file is missing "
        "the insertion point for '$INSERT_LINKS_HERE'");

  full_template.replace("$TODAYS_DATE", time_stamp::notarize(true));

  filename outname(output_filename);
  byte_filer output_file(output_filename, "w");
  if (!output_file.good())
    non_continuable_error(class_name(), func, "the output file could not be opened");
  // write the newly generated web page out now.
  output_file.write(full_template);
  output_file.close();

#ifdef DEBUG_MARKS
  // show the tree.
  BASE_LOG(astring());
  BASE_LOG(astring("the tree, sir..."));
  BASE_LOG(astring());
  BASE_LOG(c_categories.access_root().text_form());
#endif

  BASE_LOG(a_sprintf("wrote %d links in %d categories.",
      c_categories.link_count(), c_categories.category_count()));
  BASE_LOG(astring(""));

  return 0;
}

int marks_maker::execute()
{
  FUNCDEF("execute");
  SETUP_COMBO_LOGGER;

  command_line cmds(_global_argc, _global_argv);  // process the command line parameters.
  astring input_filename;  // we'll store our link database name here.
  astring output_filename;  // where the web page we're creating goes.
  astring template_filename;  // the wrapper html code that we'll stuff.
  astring style_used;  // type of output file style to create.
  if (!cmds.get_value('i', input_filename, false))
    return print_instructions(cmds.program_name());
  if (!cmds.get_value('o', output_filename, false))
    return print_instructions(cmds.program_name());
  if (!cmds.get_value('t', template_filename, false))
    return print_instructions(cmds.program_name());
  cmds.get_value('s', style_used, false);
  if (!style_used) style_used = "human";

  BASE_LOG(astring("input file: ") + input_filename);
  BASE_LOG(astring("output file: ") + output_filename);
  BASE_LOG(astring("template file: ") + template_filename);
  BASE_LOG(astring("style: ") + style_used);

  filename outname(output_filename);
  if (outname.exists()) {
    non_continuable_error(class_name(), func, astring("the output file ")
        + output_filename + " already exists.  It would be over-written if "
        "we continued.");
  }

  output_style styley = ST_HUMAN_READABLE;
  if (style_used == astring("mozilla")) styley = ST_MOZILLA_MARKS;

  int ret = c_categories.read_csv_file(input_filename);
  if (ret) return ret;

  ret = write_marks_page(output_filename, template_filename, styley);
  if (ret) return ret;
  
  return 0;
}

////////////////////////////////////////////////////////////////////////////

HOOPLE_MAIN(marks_maker, )

