/*****************************************************************************\
*                                                                             *
*  Name   : marks_maker_javascript                                            *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Turns a link database in HOOPLE format into a web page, when given a     *
*  suitable template file.  The template file must have the phrase:           *
*        $INSERT_LINKS_HERE                                                   *
*  at the point where the generated links are supposed to be stored.          *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "bookmark_tree.h"

#include <application/command_line.h>
#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <filesystem/byte_filer.h>
#include <filesystem/filename.h>
#include <loggers/file_logger.h>
#include <structures/stack.h>
#include <structures/static_memory_gremlin.h>
#include <textual/list_parsing.h>
#include <timely/time_stamp.h>

using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace nodes;
using namespace structures;
using namespace textual;
using namespace timely;

//#define DEBUG_MARKS
  // uncomment to have more debugging noise.

#undef BASE_LOG
#define BASE_LOG(s) program_wide_logger::get().log(s, ALWAYS_PRINT)
#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), \
   a_sprintf("line %d: ", _categories._line_number) + s, ALWAYS_PRINT)

const int MAX_FILE_SIZE = 4 * MEGABYTE;
  // the largest file we'll read.

////////////////////////////////////////////////////////////////////////////

class marks_maker_javascript : public application_shell
{
public:
  marks_maker_javascript() : application_shell(), _need_closure(false),
        _loader_count(0), _link_spool(0), _functions_pending(0) {}
  DEFINE_CLASS_NAME("marks_maker_javascript");
  virtual int execute();
  int print_instructions(const filename &program_name);

  int write_marks_page(const astring &output_filename,
          const astring &template_filename);
    // given a tree of links, this writes out a web page to "output_filename"
    // using a template file "template_filename".

private:
  bookmark_tree _categories;  // our tree of categories.
  bool _need_closure;  // true if our <div> needs a closure.
  int _loader_count;  // count of the loader functions.
  int _link_spool;  // count of which link we're writing.
  stack<astring> _functions_pending;  // used for javascript node functions.

//this needs to gather any strings that would have gone into functions.
//instead, they need to be written into the current node's string.
//when a new function def would be seen, then we need to push a new node
//for accumulating the text.

  // these handle outputting text for categories and links.
  void write_category(inner_mark_tree *node, astring &output);
  void write_link(inner_mark_tree *node, const link_record &linko,
         astring &output);
};

////////////////////////////////////////////////////////////////////////////

int marks_maker_javascript::print_instructions(const filename &program_name)
{
  a_sprintf to_show("%s:\n\
This program needs three filenames as command line parameters.  The -i flag\n\
is used to specify the input filename, the -t flag specifies a template web\n\
page which is used as the wrapper around the links, and the -o flag specifies\n\
the web page to be created.  The input file is expected to be in the HOOPLE\n\
link database format.  The output file will be created from the template file\n\
by finding the phrase $INSERT_LINKS_HERE in it and replacing that with html\n\
formatted link and categories from the input file.  Another tag of $TODAYS_DATE\n\
will be replaced with the date when the output file is regenerated.\n\
The HOOPLE link format is documented here:\n\
    http://feistymeow.org/guides/link_database/format_manifesto.txt\n\
", program_name.basename().raw().s(), program_name.basename().raw().s());
  program_wide_logger::get().log(to_show, ALWAYS_PRINT);
  return 12;
}

void marks_maker_javascript::write_category(inner_mark_tree *node, astring &output)
{
  FUNCDEF("write_category");
  // output a javascript line for the category.

  int node_num = node->_uid;
  inner_mark_tree *parent = dynamic_cast<inner_mark_tree *>(node->parent());
  int parent_node = parent? parent->_uid : -1;
    // the parent node for root is a negative one.
  astring chewed_name = node->name();
  for (int i = chewed_name.end(); i >= 0; i--) {
    // escape any raw single quotes that we see.
    if (chewed_name[i] == '\'') {
      chewed_name.zap(i, i);
      chewed_name.insert(i, "\\'");
    }
  }
  output += a_sprintf("  b.add(%d, %d, '%s');\n", node_num, parent_node,
      chewed_name.s());
}

void marks_maker_javascript::write_link(inner_mark_tree *node, const link_record &linko, 
    astring &output)
{
  FUNCDEF("write_link");
  // write a javascript link definition.
  int parent_node = node->_uid;
  astring chewed_name = linko._description;
  for (int i = chewed_name.end(); i >= 0; i--) {
    // escape any raw single quotes that we see.
    if (chewed_name[i] == '\'') {
      chewed_name.zap(i, i);
      chewed_name.insert(i, "\\'");
    }
  }

  if (!linko._url) {
    // this just appears to be a comment line.
    if (!linko._description) return;  // it's a nothing line.

/*
//hmmm: probably not what we want.
//hmmm: why not, again?
    output += linko._description;
    output += "<br>";
    output += parser_bits::platform_eol_to_chars();
*/
    return;
  }

  // generate a function header if the number of links is a multiple of 100.
  if (! (_link_spool % 100) ) {
    if (_link_spool) {
      // close out the previous function and set a timeout.
      output += "  setTimeout('run_tree_loaders()', 0);\n";
      output += "}\n";
    }

    output += a_sprintf("function tree_loader_%d() {\n", _loader_count++);
  }
  _link_spool++;

  output += a_sprintf("  b.add(%d, %d, '%s', '%s');\n",
      linko._uid, parent_node, chewed_name.s(), linko._url.s());
}

int marks_maker_javascript::execute()
{
  FUNCDEF("execute");
  SETUP_COMBO_LOGGER;

  command_line cmds(_global_argc, _global_argv);  // process the command line parameters.
  astring input_filename;  // we'll store our link database name here.
  astring output_filename;  // where the web page we're creating goes.
  astring template_filename;  // the wrapper html code that we'll stuff.
  if (!cmds.get_value('i', input_filename, false))
    return print_instructions(cmds.program_name());
  if (!cmds.get_value('o', output_filename, false))
    return print_instructions(cmds.program_name());
  if (!cmds.get_value('t', template_filename, false))
    return print_instructions(cmds.program_name());

  BASE_LOG(astring("input file: ") + input_filename);
  BASE_LOG(astring("output file: ") + output_filename);
  BASE_LOG(astring("template file: ") + template_filename);

  int ret = _categories.read_csv_file(input_filename);
  if (ret) return ret;

  ret = write_marks_page(output_filename, template_filename);
  if (ret) return ret;
  
  return 0;
}

int marks_maker_javascript::write_marks_page(const astring &output_filename,
    const astring &template_filename)
{
  FUNCDEF("write_marks_page");
  astring long_string;
    // this is our accumulator of links.  it is the semi-final result that will
    // be injected into the template file.

  // add our target layer so that we can write to a useful place.
  long_string += "<div class=\"marks_target\" id=\"martarg\">Marks Target</div>\n";

  // add the tree style and creation of the tree object into the text.
  long_string += "\n<div class=\"dtree\">\n";
  long_string += "<script type=\"text/javascript\">\n";
  long_string += "<!--\n";

  long_string += "function open_mark(url) {\n";
  long_string += "  window.open(url, '', '');\n";
  long_string += "}\n";

  // the list of functions is used for calling into the tree loaders
  // without blocking.
  long_string += "  b = new dTree('b');\n";
///  long_string += "  b.config.inOrder = true;\n";
  long_string += "  b.config.useCookies = false;\n";
  long_string += "  b.config.folderLinks = false;\n";

  // traverse the tree in prefix order.
  tree::iterator itty = _categories.access_root().start(tree::prefix);
  tree *curr = NULL_POINTER;
  while ( (curr = itty.next()) ) {
    inner_mark_tree *nod = (inner_mark_tree *)curr;
    // print out the category on this node.
    write_category(nod, long_string);

    // print the link for all of the ones stored at this node.
    for (int i = 0; i < nod->_links.elements(); i++) {
      link_record *lin = nod->_links.borrow(i);
      write_link(nod, *lin, long_string);
    }
  }

  // close the block of script in the output.
  long_string += "  setTimeout('run_tree_loaders()', 0);\n";
  long_string += "}\n\n";

  long_string += a_sprintf("function tree_loader_%d()"
      "{ setTimeout('run_tree_loaders()', 0); }\n", _loader_count++);

  long_string += "\nconst max_funcs = 1000;\n";
  long_string += "var loader_functions = new Array(max_funcs);\n";
  long_string += "var curr_func = 0;\n";
  long_string += "var done_rendering = false;\n\n";

  long_string += a_sprintf("for (var i = 0; i < %d; i++) {\n", _loader_count);
  long_string += "  loader_functions[curr_func++] "
        "= 'tree_loader_' + i + '()';\n";
  long_string += "}\n";

  long_string += "var run_index = 0;\n";
  long_string += "function run_tree_loaders() {\n";
  long_string += "  if (done_rendering) return;\n";
  long_string += "  if (run_index >= curr_func) {\n";

  long_string += "    if (document.getElementById) {\n";
  long_string += "      x = document.getElementById('martarg');\n";
  long_string += "      x.innerHTML = '';\n";
  long_string += "      x.innerHTML = b;\n";
  long_string += "    } else { document.write(b); }\n";
//not a very graceful degradation.  we should use the other options from:
// http://www.quirksmode.org/js/layerwrite.html
  long_string += "    done_rendering = true;\n";
  long_string += "    return;\n";
  long_string += "  }\n";
  long_string += "  var next_func = loader_functions[run_index++];\n";
  long_string += "  setTimeout(next_func, 0);\n";
  long_string += "}\n";

  long_string += a_sprintf("  run_tree_loaders();\n", _loader_count);

  long_string += "//-->\n";
  long_string += "</script>\n";
  long_string += "<p><a href=\"javascript: b.openAll();\">open all</a> | "
      "<a href=\"javascript: b.closeAll();\">close all</a></p>\n";
  long_string += "</div>\n";

  byte_filer template_file(template_filename, "r");
  astring full_template;
  if (!template_file.good())
    non_continuable_error(class_name(), func, "the template file could not be opened");
  template_file.read(full_template, MAX_FILE_SIZE);
  template_file.close();

  // javascript output needs some extra junk added to the header section.
  int indy = full_template.ifind("</title>");
  if (negative(indy))
    non_continuable_error(class_name(), func, "the template file is missing "
        "a <head> declaration");
//hmmm: the path here must be configurable!
  full_template.insert(indy + 8, "\n\n"
      "<link rel=\"StyleSheet\" href=\"/yeti/javascript/dtree/dtree.css\" "
          "type=\"text/css\" />\n"
      "<script type=\"text/javascript\" src=\"/yeti/javascript/"
          "dtree/dtree.js\"></script>\n");

  // replace the tag with the long string we created.
  bool found_it = full_template.replace("$INSERT_LINKS_HERE", long_string);
  if (!found_it)
    non_continuable_error(class_name(), func, "the template file is missing "
        "the insertion point");
  full_template.replace("$TODAYS_DATE", time_stamp::notarize(true));

  filename outname(output_filename);
  if (outname.exists()) {
    non_continuable_error(class_name(), func, astring("the output file ")
        + output_filename + " already exists.  It would be over-written if "
        "we continued.");
  }

  byte_filer output_file(output_filename, "w");
  if (!output_file.good())
    non_continuable_error(class_name(), func, "the output file could not be opened");
  // write the newly generated web page out now.
  output_file.write(full_template);
  output_file.close();

// show the tree.
//  BASE_LOG("");
//  BASE_LOG(_categories.access_root().text_form());

  BASE_LOG(a_sprintf("wrote %d links in %d categories.",
      _categories.link_count(), _categories.category_count()));
  BASE_LOG(astring(""));

  return 0;
}

////////////////////////////////////////////////////////////////////////////

HOOPLE_MAIN(marks_maker_javascript, )

