/*****************************************************************************\
*                                                                             *
*  Name   : value_tagger                                                      *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Scoots through the entire known code base and builds a list of all the   *
*  outcome (and filter) values for that tree.  A manifest of the names is     *
*  produced.  Most of the behavior is driven by the ini file whose name is    *
*  passed on the command line.                                                *
*    Note that the set of items that can be searched for can be specified     *
*  in the ini file, although they must follow the format of:                  *
*      pattern(name, value, description)                                      *
*  where the "pattern" is the search term and the other three items specify   *
*  the enumerated value to be marked.                                         *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <algorithms/shell_sort.h>
#include <application/application_shell.h>
#include <application/command_line.h>
#include <application/hoople_main.h>
#include <application/windoze_helper.h>
#include <basis/environment.h>
#include <basis/functions.h>
#include <basis/utf_conversion.h>
#include <configuration/ini_configurator.h>
#include <filesystem/byte_filer.h>
#include <filesystem/directory_tree.h>
#include <filesystem/filename.h>
#include <loggers/combo_logger.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <structures/set.h>
#include <structures/string_array.h>
#include <structures/string_table.h>
#include <timely/time_stamp.h>
#include <textual/parser_bits.h>

#include <sys/stat.h>
#ifdef __WIN32__
  #include <io.h>
#endif

#undef LOG
#define LOG(s) EMERGENCY_LOG(program_wide_logger::get(), astring(s))

using namespace algorithms;
using namespace application;
using namespace basis;
using namespace configuration;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;
using namespace timely;

const int LONGEST_SEPARATION = 128;
  // the longest we expect a single line of text to be in definition blocks.
  // if the definition of an outcome or whatever is farther away than this
  // many characters from a comment start, we will no longer consider the
  // line to be commented out.  this pretty much will never happen unless it's
  // intentionally done to break this case.

const char *SKIP_VALUE_PHRASE = "SKIP_TO_VALUE";
  // the special phrase we use to indicate that values should jump to
  // a specific number.

////////////////////////////////////////////////////////////////////////////

// this object records all the data that we gather for the defined items.
class item_record
{
public:
  astring _name;
  int _value;
  astring _description;
  astring _path;
  astring _extra_tag;  //!< records special info for links.

  item_record(const astring &name = astring::empty_string(), int value = 999,
      const astring &description = astring::empty_string(),
      const astring &path = astring::empty_string(),
      const astring &extra_tag = astring::empty_string())
  : _name(name), _value(value), _description(description), _path(path),
    _extra_tag(extra_tag) {}
};

////////////////////////////////////////////////////////////////////////////

class search_record
{
public:
  search_record(const astring &search = astring::empty_string(),
      bool is_link = false, search_record *link = NIL)
  : _search(search), _no_modify(false), _is_link(is_link), _our_link(link),
    _current_value(0), _value_increment(1) {}

  // these properties are available for both real or linked records.
  astring _search;  // our term to search for in the files.
  bool _no_modify;  // true if values should not be automatically incremented.
  astring _tag;  // extra information attached to this type.
  
  bool is_link() const { return _is_link; }
    // returns true if this object is leeching off another object for data.

  search_record *our_link() const { return _our_link; }
    // returns the object that this object is a mere shadow of.

  symbol_table<item_record> &definitions() {
    if (is_link()) return _our_link->_definitions;
    else return _definitions;
  }
  
  int &current_value() {
    if (is_link()) return _our_link->_current_value;
    else return _current_value;
  }

  int &value_increment() {
    if (is_link()) return _our_link->_value_increment;
    else return _value_increment;
  }

  int_set &out_of_band() {
    if (is_link()) return _our_link->_out_of_band;
    else return _out_of_band;
  }

private:
  bool _is_link;  // true if this object links to another.
  search_record *_our_link;  // the search we share for our values.
  symbol_table<item_record> _definitions;
    // the definitions that we found in the code.
  int _current_value;  // the next value to use for our term.
  int _value_increment;
    // how much to add for each new value, if this is an incrementing search.
  int_set _out_of_band;
    // values we've seen that were premature.  we always want to honor this
    // set, if it exists, but there will be nothing in it if the search has
    // completely standard non-incrementing type.  this could be varied by
    // a non-incrementer linking to a standard incrementer.
};

//! a table of terms that we will search for in the code.
class active_searches : public symbol_table<search_record>
{};

////////////////////////////////////////////////////////////////////////////

// this class provides us a way to easily sort our items based on value.

class simple_sorter {
public:
  int _index;
  int _value;
  simple_sorter(int index = 0, int value = 0) : _index(index), _value(value) {}
  bool operator < (const simple_sorter &to_compare) const
    { return _value < to_compare._value; }
  bool operator == (const simple_sorter &to_compare) const
    { return _value == to_compare._value; }
};

class sorting_array : public array<simple_sorter> {};

////////////////////////////////////////////////////////////////////////////

class value_tagger : public application_shell
{
public:
  value_tagger();
  virtual ~value_tagger();
  DEFINE_CLASS_NAME("value_tagger");
  int execute();
  int print_instructions_and_exit();

  bool process_tree(const astring &path);
    // called on each directory hierarchy that we need to process.

  bool process_file(const astring &path);
    // examines the file specified to see if it matches our needs.

  bool parse_define(const astring &scanning, int indy, astring &name,
          int &value, astring &description, int &num_start, int &num_end);
    // processes the string in "scanning" to find parentheses surrounding
    // the "name", "value" and "description".  the "description" field may
    // occupy multiple lines, so all are gathered together to form one
    // unbroken string.  the "num_start" and "num_end" specify where the
    // numeric value was found, in case it needs to be patched.

private:
  ini_configurator *_ini;  // the configuration for what we'll scan.
  string_table _dirs;  // the list of directories.
  string_table _dirs_seen;  // full list of already processed directories.
  filename _manifest_filename;  // the name of the manifest we'll create.
  byte_filer _manifest;  // the actual file we're building.
  active_searches _search_list;  // tracks our progress in scanning files.
  int_array _search_ordering;
    // lists the terms in the order they should be applied.  initially this
    // carries the first pass items, but later will be reset for second pass.
  int_array _postponed_searches;
    // lists the searches that must wait until the main search is done.
  string_table _modified_files;  // the list of files that we touched.
};

////////////////////////////////////////////////////////////////////////////

value_tagger::value_tagger()
: application_shell(),
  _ini(NIL),
  _dirs_seen(10)
{
}

value_tagger::~value_tagger()
{
  WHACK(_ini);
}

int value_tagger::print_instructions_and_exit()
{
  LOG(a_sprintf("%s usage:", filename(_global_argv[0]).basename().raw().s()));
  LOG("");

  LOG("\
This utility scans a code base for outcome and filter definitions.  It will\n\
only scan the header files (*.h) found in the directories specified.  The\n\
single parameter is expected to be an INI filename that contains the scanning\n\
configuration.  The INI file should be formatted like this (where the $HOME\n\
can be any variable substitution from the environment):");
  LOG("");
  LOG("\
[manifest]\n\
output=$HOME/manifest.txt\n\
\n\
[searches]\n\
DEFINE_OUTCOME=1\n\
DEFINE_FILTER=1\n\
\n\
[directories]\n\
$HOME/source/lib_src/library/basis\n\
$HOME/source/lib_src/library\n\
$HOME/source/lib_src/communication/sockets\n\
$HOME/source/lib_src/communication\n\
$HOME/source/lib_src\n\
$HOME/source/app_src\n\
$HOME/source/test_src\n\
\n\
[DEFINE_OUTCOME]\n\
first=0\n\
increment=-1\n\
\n\
[DEFINE_FILTER]\n\
first=-1\n\
increment=1\n\
no_modify=1\n\
\n\
[DEFINE_API_OUTCOME]\n\
no_modify=1\n\
link=DEFINE_OUTCOME\n\
tag=API\n\
\n\
  The \"first\" field defines the starting value that should be assigned to\n\
items.\n\
  The \"increment\" field specifies what to add to a value for the next item.\n\
  The optional \"no_modify\" flag means that the values should not be auto-\n\
incremented; their current value will be used.\n\
  The optional \"link\" field defines this type of item as using the current\n\
values for another type of item.  In this case, API_OUTCOME will use the\n\
values for OUTCOME to share its integer space, but API_OUTCOME is not auto-\n\
incremented even though OUTCOME is.  This causes the values for OUTCOME and\n\
API_OUTCOME to be checked for uniqueness together, but only OUTCOME will be\n\
auto-incremented.  Note that only one level of linking is supported currently.\n\
  The optional \"tag\" can be used to distinguish the entries for a particular\n\
search type if needed.  This is most helpful for links, so that they can be\n\
distinguished from their base type.\n\
\n\
");

  return 23;
}

astring header_string(const astring &build_number)
{
  return a_sprintf("\
#ifndef GENERATED_VALUES_MANIFEST\n\
#define GENERATED_VALUES_MANIFEST\n\
\n\
// This file contains all outcomes and filters for this build.\n\
\n\
// Generated for build %s on %s\n\
\n\
", build_number.s(), time_stamp::notarize(true).s());
}

astring footer_string(const byte_array &full_config_file)
{
  return a_sprintf("\n\
// End of definitions.\n\
\n\
\n\
// The following is the full configuration for this build:\n\
\n\
/*\n\
\n\
%s\n\
*/\n\
\n\
\n\
#endif // outer guard.\n\
", (char *)full_config_file.observe());
}

int value_tagger::execute()
{
  FUNCDEF("execute");
  if (_global_argc < 2) {
    return print_instructions_and_exit();
  }

  log(time_stamp::notarize(true) + "value_tagger started.", basis::ALWAYS_PRINT);

  astring test_repository = environment::get("FEISTY_MEOW_DIR");
  if (!test_repository) {
    astring msg = "\
There is a problem with a required build precondition.  The following\r\n\
variables must be set before the build is run:\r\n\
\r\n\
  FEISTY_MEOW_DIR    This should point at the root of the build tree.\r\n\
\r\n\
There are also a few variables only required for CLAM-based compilation:\r\n\
\r\n\
  MAKEFLAGS         This should be set to \"-I $FEISTY_MEOW_DIR/clam\".\r\n\
\r\n\
Note that on Win32 platforms, these should be set in the System or User\r\n\
variables before running a build.\r\n";
#ifdef __WIN32__
    ::MessageBox(0, to_unicode_temp(msg),
        to_unicode_temp("Missing Precondition"), MB_ICONWARNING|MB_OK);
#endif
    non_continuable_error(class_name(), func, msg);
  }

  astring ini_file = _global_argv[1];  // the name of our ini file.
  _ini = new ini_configurator(ini_file, ini_configurator::RETURN_ONLY);

  // read the name of the manifest file to create.
  _manifest_filename = filename(_ini->load("manifest", "output", ""));
  if (!_manifest_filename.raw().length()) {
    non_continuable_error(class_name(), ini_file, "The 'output' file entry is missing");
  }
  _manifest_filename = parser_bits::substitute_env_vars(_manifest_filename);

  LOG(astring("Sending Manifest to ") + _manifest_filename);
  LOG("");

  filename(_manifest_filename).unlink();
    // clean out the manifest ahead of time.

  // read the list of directories to scan for code.
  string_table temp_dirs;
  bool read_dirs = _ini->get_section("directories", temp_dirs);
  if (!read_dirs || !temp_dirs.symbols()) {
    non_continuable_error(class_name(), ini_file,
        "The 'directories' section is missing");
  }
  for (int i = 0; i < temp_dirs.symbols(); i++) {
//log(astring("curr is ") + current);
    filename current = filename(parser_bits::substitute_env_vars(temp_dirs.name(i)));
    _dirs.add(current, "");
  }

  LOG(astring("Directories to scan..."));
  LOG(_dirs.text_form());

  astring rdir = environment::get("FEISTY_MEOW_DIR");
  astring fname;
  astring parmfile = environment::get("BUILD_PARAMETER_FILE");
  if (parmfile.t()) fname = parmfile;
  else fname = rdir + "/build.ini";

  // read the list of search patterns.
  string_table searches;
  bool read_searches = _ini->get_section("searches", searches);
  if (!read_searches || !searches.symbols()) {
    non_continuable_error(class_name(), ini_file,
        "The 'searches' section is missing");
  }

  LOG("Searching for...");
  LOG(searches.text_form());

  // now make sure that we get the configuration for each type of value.
  for (int i = 0; i < searches.symbols(); i++) {
    const astring &curr_name = searches.name(i);

    search_record *check_search = _search_list.find(curr_name);
    if (check_search) {
      non_continuable_error(class_name(), ini_file,
          astring("section ") + curr_name + " is being defined twice");
    }

    {
      // check for whether this section is linked to another or not.
      astring linked = _ini->load(curr_name, "link", "");
      search_record *our_link_found = NIL;
      if (linked.t()) {
        // we found that this should be linked to another item.
        our_link_found = _search_list.find(linked);
        if (!our_link_found) {
          non_continuable_error(class_name(), ini_file,
              astring("linked section ") + curr_name + " is linked to missing "
                  "section " + linked);
        }
        search_record new_guy(curr_name, true, our_link_found);
        _search_list.add(curr_name, new_guy);
      } else {
        // this section is a stand-alone section.
        search_record new_guy(curr_name);
        _search_list.add(curr_name, new_guy);
      }
    }

    // find our new search cabinet again so we can use it.
    search_record *curr_search = _search_list.find(curr_name);
    if (!curr_search) {
      non_continuable_error(class_name(), ini_file,
          astring("section ") + curr_name + " is missing from table "
              "after addition; logic error");
    }

    // specify some defaults first.
    int start = 0;
    int increm = 1;
    if (!curr_search->is_link()) {
      // a linked object doesn't get to specify starting value or increment.
      start = _ini->load(curr_name, "first", start);
      curr_search->current_value() = start;
      increm = _ini->load(curr_name, "increment", increm);
      curr_search->value_increment() = increm;
    } else {
      start = curr_search->our_link()->current_value();
      increm = curr_search->our_link()->value_increment();
    }

    int no_modify = _ini->load(curr_name, "no_modify", 0);
    if (no_modify) {
      curr_search->_no_modify = true;
    }

    astring tag = _ini->load(curr_name, "tag", "");
    if (tag.t()) {
      curr_search->_tag = tag;
    }

    a_sprintf to_show("%s: no_modify=%s", curr_name.s(),
         no_modify? "true" : "false");

    if (curr_search->is_link()) {
      // links show who they're hooked to.
      to_show += astring(" link=") + curr_search->our_link()->_search;
    } else {
      // non-links get to show off their start value and increment.
      to_show += a_sprintf(" start=%d increment=%d", start, increm);
    }
    if (tag.t()) {
      to_show += astring(" tag=") + curr_search->_tag;
    }
    LOG(to_show);
  }
  LOG("");

  // now gather some info about the build that we can plug into the manifest.

  byte_filer build_file(fname, "r");
  if (!build_file.good()) {
    non_continuable_error(class_name(), build_file.name(),
        "Could not find the build configuration; is FEISTY_MEOW_DIR set?");
  }
  byte_array full_config;
  build_file.read(full_config, 100000);  // a good chance to be big enough.
  build_file.close();

//log("got config info:");
//log((char *)full_config.observe());

  astring build_number;
  ini_configurator temp_ini(fname, configurator::RETURN_ONLY);
  build_number += temp_ini.load("version", "major", "");
  build_number += ".";
  build_number += temp_ini.load("version", "minor", "");
  build_number += ".";
  build_number += temp_ini.load("version", "revision", "");
  build_number += ".";
  build_number += temp_ini.load("version", "build", "");
  if (build_number.equal_to("...")) {
    non_continuable_error(class_name(), build_file.name(),
        "Could not read the build number; is build parameter file malformed?");
  }

//log(astring("got build num: ") + build_number);

  // now that we know what file to create, write the header blob for it.
  _manifest.open(_manifest_filename, "wb");
  if (!_manifest.good()) {
    non_continuable_error(class_name(), _manifest_filename,
        "Could not write to the manifest file!");
  }
  _manifest.write(header_string(build_number));

  // make sure we have the right ordering for our terms.  items that are
  // non-modify types must come before the modifying types.
  for (int i = 0; i < _search_list.symbols(); i++) {
    search_record &curr_reco = _search_list[i];
    if (curr_reco._no_modify)
      _search_ordering += i;
    else
      _postponed_searches += i;
  }

  // scan across each directory specified for our first pass.
  LOG("First pass...");
  for (int i = 0; i < _dirs.symbols(); i++) {
    if (_dirs.name(i).begins("#") || _dirs.name(i).begins(";")) continue;  // skip comment.
    LOG(astring("  Processing: ") + _dirs.name(i));
    bool ret = process_tree(_dirs.name(i));
    if (!ret) {
      LOG(astring("Problem encountered in directory ") + _dirs.name(i));
    }
  }
  LOG("");

  // second pass now.
  LOG("Second pass...");
  _search_ordering = _postponed_searches;  // recharge the list for 2nd pass.
  _dirs_seen.reset();  // drop any directories we saw before.
  for (int i = 0; i < _dirs.symbols(); i++) {
    if (_dirs.name(i).begins("#") || _dirs.name(i).begins(";")) continue;  // skip comment.
    LOG(astring("  Processing: ") + _dirs.name(i));
    bool ret = process_tree(_dirs.name(i));
    if (!ret) {
      LOG(astring("Problem encountered in directory ") + _dirs.name(i));
    }
  }
  LOG("");

  const astring quote = "\"";
  const astring comma = ",";

  // scoot across all the completed searches and dump results.
  for (int i = 0; i < _search_list.symbols(); i++) {
    search_record &curr_reco = _search_list[i];
    const astring &pattern = curr_reco._search;

    _manifest.write(astring("/* START ") + pattern + "\n");
    _manifest.write(astring("[") + pattern + "]\n");

    if (!curr_reco.is_link()) {
      // scoot across all definitions and print them out.

      // do the print out in order, as dictated by the sign of the increment.
      sorting_array sortie;
      for (int j = 0; j < curr_reco.definitions().symbols(); j++) {
        const item_record &rec = curr_reco.definitions().get(j);
        sortie += simple_sorter(j, rec._value);
      }
      shell_sort(sortie.access(), sortie.length(),
          negative(curr_reco.value_increment()));

      for (int j = 0; j < sortie.length(); j++) {
        int indy = sortie[j]._index;
        const item_record &rec = curr_reco.definitions().get(indy);
        astring to_write = "  ";
        if (rec._extra_tag.t()) {
          to_write += astring("(") + rec._extra_tag + ") ";
        }
        to_write += quote + rec._name + quote + comma + " ";
        to_write += quote + a_sprintf("%d", rec._value) + quote + comma + " ";
        to_write += quote + rec._description + quote + comma + " ";
        to_write += quote + rec._path + quote;
        to_write += "\n";
        _manifest.write(to_write);
      }
    } else {
      // this is just a link.
      astring to_write = "  Linked to search item ";
      to_write += curr_reco.our_link()->_search;
      to_write += "\n";
      _manifest.write(to_write);
    }

    _manifest.write(astring("END ") + pattern + " */\n\n");
  }

  _manifest.write(footer_string(full_config));

  // show all the modified files.
  if (_modified_files.symbols()) {
    const int syms = _modified_files.symbols();
    LOG("Modified Files:");
    LOG("===============");
    for (int i = 0; i < syms; i++) {
      LOG(_modified_files.name(i));
    }
  } else {
    LOG("No files needed modification for generated values.");
  }
  LOG("");

  log(time_stamp::notarize(true) + "value_tagger finished.", ALWAYS_PRINT);

  return 0;
}

#define INBO (indy < scanning.length())
  // a macro that makes length checking less verbose.

// make sure we drop any spaces in between important bits.
#define SKIP_SPACES \
  while (INBO && parser_bits::white_space(scanning[indy])) indy++;

// return with a failure but say why it happened.
#define FAIL_PARSE(why) { \
  log(astring("failed to parse the string because ") + why + ".", ALWAYS_PRINT); \
  return false; \
}

bool value_tagger::parse_define(const astring &scanning, int indy,
    astring &name, int &value, astring &description, int &num_start,
    int &num_end)
{
  // prepare our result objects.
  name = ""; value = -1; description = ""; num_start = -1; num_end = -1;

  SKIP_SPACES;

  // look for starting parenthesis.
  if (!INBO || (scanning[indy] != '(') )
    FAIL_PARSE("the first parenthesis is missing");

  indy++;  // skip paren.
  SKIP_SPACES;

  // find the name of the item being defined.
  while (INBO && (scanning[indy] != ',') ) {
    name += scanning[indy];
    indy++;
  }

  indy++;  // skip the comma.
  SKIP_SPACES;

  astring num_string;
  num_start = indy;
  while (INBO && parser_bits::is_numeric(scanning[indy])) {
    num_string += scanning[indy];
    indy++;
  }
  num_end = indy - 1;
  value = num_string.convert(0);

  SKIP_SPACES;

  if (!INBO || (scanning[indy] != ',') )
    FAIL_PARSE("the post-value comma is missing");

  indy++;
  SKIP_SPACES;

  if (!INBO || (scanning[indy] != '"') )
    FAIL_PARSE("the opening quote for the description is missing");

  indy++;  // now we should be at raw text.

  // scan through the full description, taking into account that it might
  // be broken across multiple lines as several quoted bits.
  bool in_quote = true;  // we're inside a quote now.
  while (INBO && (scanning[indy] != ')') ) {
    const char curr = scanning[indy];
//hmmm: escaped quotes are not currently handled.
    if (curr == '"') in_quote = !in_quote;  // switch quoting state.
    else if (in_quote) description += curr;
    indy++;
  }

  return scanning[indy] == ')';
}

bool value_tagger::process_file(const astring &path)
{
  byte_filer examining(path, "rb");
  if (!examining.good()) {
    log(astring("Error reading file: ") + path, ALWAYS_PRINT);
    return false;
  }
  examining.seek(0, byte_filer::FROM_END);
  int fsize = int(examining.tell());
  examining.seek(0, byte_filer::FROM_START);

  astring contents('\0', fsize + 20);
  int bytes_read = examining.read((abyte *)contents.access(), fsize);
    // read the file directly into a big astring.
  examining.close();
  contents[bytes_read] = '\0';
  contents.shrink();  // drop any extra stuff at end.

  bool modified = false;  // set to true if we need to write the file back.

  // check if the file matches our phrases of interest.
  bool matched = false;
  for (int q = 0; q < _search_list.symbols(); q++) {
    search_record &curr_reco = _search_list[q];
    if (contents.contains(curr_reco._search)) {
//_manifest.write(astring("MATCH-") + curr_pattern + ": " + path + "\n" ); //temp
      matched = true;
      break;
    }
  }

  if (!matched) return true;

  // now we have verified that there's something interesting in this file.
  // go through to find the interesting bits.

  // we do this in the search ordering that we established earlier, so we
  // will tag the values in the proper order.
  for (int x = 0; x < _search_ordering.length(); x++) {
    int q = _search_ordering[x];  // get our real index.
    search_record &curr_reco = _search_list[q];
    const astring &curr_pattern = curr_reco._search;
///log(astring("now seeking ") + curr_pattern);
    int start_from = 0;  // where searches will start from.

    while (true) {
      // search forward for next match.
      int indy = contents.find(curr_pattern, start_from);
      if (negative(indy)) break;  // no more matches.
      start_from = indy + 5;  // ensure we'll skip past the last match.

      // make sure our deadly pattern isn't in front; we don't want to
      // process the actual definition of the macro in question.
//log(a_sprintf("indy=%d [indy-1]=%c [indy-2]=%c", indy, contents[indy-1], contents[indy-2]));
      if ( (indy > 3) && (contents[indy-1] == ' ')
          && (contents[indy-2] == 'e') ) {
        int def_indy = contents.find("#define", indy, true);
//log(astring("checking ") + curr_pattern + a_sprintf(": defindy %d, ", def_indy) + path + "\n" );

        if (non_negative(def_indy) && (absolute_value(indy - def_indy) < 12) ) {
          // they're close enough that we probably need to skip this
          // occurrence of our search term.
//_manifest.write(astring("DEMATCH-") + curr_pattern + ": had the #define! " + path + "\n" );
          continue;
        }
      }

      // make sure we don't include commented lines in consideration.
      int comm_indy = contents.find("//", indy, true);
      if (non_negative(comm_indy)) {
//log("found a comment marker");
        // we found a comment before the definition, but we're not sure how
        // far before.
        if (absolute_value(comm_indy - indy) < LONGEST_SEPARATION) {
//log("comment is close enough...");
          // they could be on the same line...  unless lines are longer than
          // our constant.
          bool found_cr = false;
          for (int q = comm_indy; q < indy; q++) {
            if (parser_bits::is_eol(contents[q])) {
              found_cr = true;
              break;
            }
          }
          if (!found_cr) {
            // if there's a comment before the definition and no carriage
            // returns in between, then this is just a comment.
//log(astring("DEMATCH-") + curr_pattern + ": had the comment! " + path + "\n" );
            continue;
          }
        }
      }

      // now we are pretty sure this is a righteous definition of an outcome,
      // and not the definition of the macro itself.
      int value, num_start, num_end;
      astring name, description;
      bool found_it = parse_define(contents, indy + curr_pattern.length(),
          name, value, description, num_start, num_end);
      if (!found_it) {
        log(astring("there was a problem parsing ") + curr_pattern + " in " + path, ALWAYS_PRINT);
        continue;
      }

      // handle the special keyword for changing the value.  this is useful
      // if you want a set of outcomes to start at a specific range.
      if (name.equal_to(SKIP_VALUE_PHRASE)) {
        LOG(astring("\tSkipping value for ") + curr_pattern
            + a_sprintf(" to %d because of request in\n\t", value) + path);
        curr_reco.current_value() = value;
      }
      while (true) {
        // make sure that the current value is not already in use.
        if (!curr_reco.out_of_band().member(curr_reco.current_value()))
          break;
        // if we had a match above, we need to adjust the current value.
        curr_reco.current_value() += curr_reco.value_increment();
      }
      if (name.equal_to(SKIP_VALUE_PHRASE)) {
        continue;  // keep going now that we vetted the current value.
      }

//must catch some conditions here for values:
//  for incrementing types, we can always just try to use the next value
//  once we know it wasn't already defined out of band?
//  for non-incrementing types, we need to ensure we haven't already seen
//  the thing.  do we just always add a value seen to out of band?
//  for mixed types, the incrementing side needs to not reuse out of band
//  values.  

      astring other_place;  // the other place it was defined.
      if (curr_reco.out_of_band().member(value) && curr_reco._no_modify) {
        // this is bad; we have already seen this value elsewhere...
        for (int x = 0; x < curr_reco.definitions().symbols(); x++) {
          // see if we can find the previous definition in our list.
          if (value == curr_reco.definitions()[x]._value)
            other_place = curr_reco.definitions()[x]._path;
        }
        non_continuable_error(class_name(), path,
            a_sprintf("There is a duplicate value here for %s=%d !  "
                "Also defined in %s.", name.s(), value, other_place.s()));
      }

      // we care sometimes that this value is different than the next
      // sequential one we'd assign.  if it's a non-modifying type of
      // search, then we can't change the assigned value anyway--we can
      // only report the error in re-using a value (above).
      if (!curr_reco._no_modify) {
        // check that the defined value matches the next one we'd assign.
        if (value != curr_reco.current_value()) {
          // patch the value with the appropriate one we've been tracking.
          modified = true;
          value = curr_reco.current_value();
          contents.zap(num_start, num_end);  // remove old fusty value.
          contents.insert(num_start, a_sprintf("%d", value));
          _modified_files.add(path, "");
        }
        // move the current value up (or down).
        curr_reco.current_value() += curr_reco.value_increment();
      } else {
        // non-modifying type of value here.
//anything to do?
      }

      curr_reco.out_of_band() += value;
        // we've vetted the value, and now we're definitely using it.

      // make sure they aren't trying to reuse the name for this item.
      item_record rec;
      bool found_name = false;  // we don't want to find name already there.
      if (curr_reco.definitions().find(name)) {
        rec = *curr_reco.definitions().find(name);
        found_name = true;
      }
      if (found_name) {
        // this is bad.  this means we are not unique.  remove the manifest
        // file due to this error.
        _manifest.close();  // close the file since we want to whack it.
        filename(_manifest_filename).unlink();
        non_continuable_error(class_name(), path,
            a_sprintf("There is a duplicate name here (%s)!  "
                "Also defined in %s.", name.s(), rec._path.s()));
      }

      // record the definition in the appropriate table.
      curr_reco.definitions().add(name, item_record(name, value,
          description, path, curr_reco._tag));

//log(curr_pattern + a_sprintf(": name=%s value=%d desc=[%s]\n", name.s(), value, description.s()));

    }
  }

  if (modified) {
    // rewrite the file, since we modified its contents.
    bool chmod_result = filename(path).chmod(filename::ALLOW_BOTH,
        filename::USER_RIGHTS);
/*
    int chmod_value;
#ifdef __UNIX__
    chmod_value = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
#elif defined(__WIN32__)
    chmod_value = _S_IREAD | _S_IWRITE;
#else
    //unknown.  let's try unix...
    chmod_value = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
#endif
    int chmod_result = chmod(path.s(), chmod_value);
*/
    if (!chmod_result) {
      log(astring("there was a problem changing permissions on ") + path
          + "; writing the new version might fail.", ALWAYS_PRINT);
    }

    byte_filer rewriting(path, "wb");
    rewriting.write(contents);
    rewriting.close();
  }

  return true;
}

bool value_tagger::process_tree(const astring &path)
{
  directory_tree dir(path, "*.h");
  if (!dir.good()) return false;

  dir_tree_iterator *ted = dir.start(directory_tree::prefix);
    // create our iterator to perform a prefix traversal.

  filename curr_dir;  // the current path the iterator is at.
  string_array files;  // the filenames held at the iterator.

  while (directory_tree::current(*ted, curr_dir, files)) {
    // we have a good directory to process.

    // omit any subdirectories that exactly match directories we've already
    // scanned.  necessary to avoid redoing whole areas.
    if (!_dirs_seen.find(curr_dir)) {
      // deal with each matching header file we've found.
      for (int i = 0; i < files.length(); i++) {
        bool file_ret = process_file(filename(curr_dir.raw(), files[i]));
        if (!file_ret) {
          log(astring("There was an error while processing ") + files[i], ALWAYS_PRINT);
        }
      }

      _dirs_seen.add(curr_dir, "");
    }

    // go to the next place.
    directory_tree::next(*ted);
  }

  directory_tree::throw_out(ted);
  return true;
}

HOOPLE_MAIN(value_tagger, )

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
  #include <application/application_shell.cpp>
  #include <application/command_line.cpp>
  #include <application/windoze_helper.cpp>
  #include <basis/astring.cpp>
  #include <basis/common_outcomes.cpp>
  #include <basis/environment.cpp>
  #include <basis/guards.cpp>
  #include <basis/mutex.cpp>
  #include <basis/utf_conversion.cpp>
  #include <configuration/application_configuration.cpp>
  #include <configuration/configurator.cpp>
  #include <configuration/ini_configurator.cpp>
  #include <configuration/ini_parser.cpp>
  #include <configuration/table_configurator.cpp>
  #include <configuration/variable_tokenizer.cpp>
  #include <filesystem/byte_filer.cpp>
  #include <filesystem/directory.cpp>
  #include <filesystem/directory_tree.cpp>
  #include <filesystem/file_info.cpp>
  #include <filesystem/file_time.cpp>
  #include <filesystem/filename.cpp>
  #include <filesystem/filename_list.cpp>
  #include <filesystem/filename_tree.cpp>
  #include <filesystem/huge_file.cpp>
  #include <loggers/combo_logger.cpp>
  #include <loggers/console_logger.cpp>
  #include <loggers/critical_events.cpp>
  #include <loggers/file_logger.cpp>
  #include <loggers/program_wide_logger.cpp>
  #include <nodes/node.cpp>
  #include <nodes/packable_tree.cpp>
  #include <nodes/path.cpp>
  #include <nodes/tree.cpp>
  #include <structures/bit_vector.cpp>
  #include <structures/checksums.cpp>
  #include <structures/object_packers.cpp>
  #include <structures/static_memory_gremlin.cpp>
  #include <structures/string_hasher.cpp>
  #include <structures/string_table.cpp>
  #include <structures/version_record.cpp>
  #include <textual/byte_formatter.cpp>
  #include <textual/parser_bits.cpp>
  #include <textual/string_manipulation.cpp>
  #include <timely/earth_time.cpp>
  #include <timely/time_stamp.cpp>
#endif // __BUILD_STATIC_APPLICATION__

