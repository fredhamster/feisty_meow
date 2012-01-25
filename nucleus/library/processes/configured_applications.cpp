/*****************************************************************************\
*                                                                             *
*  Name   : configured_applications
*  Author : Chris Koeritz
*                                                                             *
*******************************************************************************
* Copyright (c) 2000 By Author.  This program is free software; you can       *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "configured_applications.h"

#include <basis/astring.h>
#include <basis/mutex.h>
#include <configuration/ini_configurator.h>
#include <configuration/section_manager.h>
#include <configuration/variable_tokenizer.h>
#include <loggers/program_wide_logger.h>
#include <structures/set.h>
#include <structures/string_table.h>
#include <textual/parser_bits.h>

using namespace basis;
using namespace configuration;
using namespace loggers;
using namespace structures;
using namespace textual;

namespace processes {

//#define DEBUG_APP_CONFIG
  // uncomment for noisier debugging version.

#undef LOG
#define LOG(to_print) program_wide_logger::get().log(to_print, ALWAYS_PRINT)

//////////////

const char *PRODUCT_HEADING() { return "product"; }
  // the string used for our startup entries as a prefix to the product.

const char *ASSIGN_TOKEN() { return "="; }
  // how we distinguish the key from the value for startup entries.

const char *SEPARATOR_TOKEN() { return ","; }
  // the character between separate key/value pairs in the startup string.

const char *SEPARATOR_TEXT() { return ", "; }
  // the string we use for the separator when printing it.

const char *PARMS_HEADING() { return "parms"; }
  // the tag for parameters in the startup entry.

const char *ONESHOT_HEADING() { return "oneshot"; }
  // the key name for startup entries' flag for once only execution.

//////////////

configured_applications::configured_applications(const astring &config_file,
    const astring &basename)
: _lock(new mutex),
  _config(new ini_configurator(config_file, ini_configurator::RETURN_ONLY,
      ini_configurator::APPLICATION_DIRECTORY)),
  _sector(new section_manager(*_config, astring(basename) + "_TOC",
      astring(PRODUCT_HEADING()) + "_"))
{
//  FUNCDEF("constructor");
  string_table startup_info;
  if (!find_section(STARTUP_SECTION(), startup_info)) {
    // if there's no startup section, we do nothing right now.
    LOG(astring("the startup section doesn't exist yet; adding it now."));
    astring entry = make_startup_entry(basename, "", false);
    startup_info.add(STARTUP_APP_NAME(), entry);
    add_section(STARTUP_SECTION(), startup_info);
  }
}

configured_applications::~configured_applications()
{
  WHACK(_sector);
  WHACK(_config);
  WHACK(_lock);
}

const char *configured_applications::STARTUP_SECTION()
{ return "PRIVATE_STARTUP_LNCH1.0"; }

const char *configured_applications::STARTUP_APP_NAME()
{ return "placeholder"; }

bool configured_applications::product_exists(const astring &product)
{
  auto_synchronizer l(*_lock);
  if (!_sector->section_exists(product)) return false;
  return true;
}

astring configured_applications::find_program(const astring &product,
    const astring &app_name, int &level)
{
  auto_synchronizer l(*_lock);
  astring heading = _sector->make_section_heading(product);
  astring found = _sector->config().load(heading, app_name, "");
//////////////
//overly specific bits here...
//hmmm: add this in as a specialization provided by real owner of class.
  if (!found) {
    // we didn't find the entry under the section we wanted to find it in.
    // there are a couple cases where we can kludge this section to a
    // different name, based on legacy requirements, and still find the
    // right item possibly.
    if (product.iequals("supervisor")) {
      // for some older installs, they say "supervisor" but mean "core".
      heading = _sector->make_section_heading("core");
      found = _sector->config().load(heading, app_name, "");
    } else if (product.iequals("lightlink")) {
      heading = _sector->make_section_heading("core");
      found = _sector->config().load(heading, app_name, "");
      if (!found) {
        // we can take one more remedial step for this phrase.
        heading = _sector->make_section_heading("server");
        found = _sector->config().load(heading, app_name, "");
      }
    }
  }
//end of overly specific.
//////////////
  found = parser_bits::substitute_env_vars(found);

  int comma_loc = found.find(",");
  if (negative(comma_loc)) return "";  // couldn't find our priority.
  level = found.convert(0);
  found.zap(0, comma_loc);

  return found;
}

bool configured_applications::add_program(const astring &product,
    const astring &app_name, const astring &full_path, int level)
{
#ifdef DEBUG_APP_CONFIG
  FUNCDEF("add_program");
#endif
  auto_synchronizer l(*_lock);
  bool existed = true;
  // lookup the section, if it exists.
  string_table info_table;
  if (!_sector->section_exists(product)) {
    existed = false;
  } else
    find_section(product, info_table);
#ifdef DEBUG_APP_CONFIG
  if (existed) {
    LOG(astring("section for ") + product + " found:");
    for (int i = 0; i < info_table.symbols(); i++)
      LOG(astring("key=") + info_table.name(i) + " value=" + info_table[i]);
  } else LOG(astring("section for ") + product + " not found.");
#endif
  // remove any existing entry.
  info_table.whack(app_name);
  // plug in our new entry.
  a_sprintf full_entry("%d,%s", level, full_path.s());
  info_table.add(app_name, full_entry);
#ifdef DEBUG_APP_CONFIG
  LOG(astring("new section for ") + product + " has:");
  for (int i = 0; i < info_table.symbols(); i++)
    LOG(astring("key=") + info_table.name(i) + " value=" + info_table[i]);
#endif
  // now call the proper storage function based on whether the section
  // existed before or not.
  if (existed) return replace_section(product, info_table);
  else return add_section(product, info_table);
}

bool configured_applications::remove_program(const astring &product,
    const astring &app_name)
{
//  FUNCDEF("remove_program");
  auto_synchronizer l(*_lock);
  // if the section's missing, there's nothing to remove...
  string_table info_table;
  if (!find_section(product, info_table)) return true;
  // the section did exist, so remove any existing entry.
  info_table.whack(app_name);
  // now call the proper storage function based on whether the section
  // existed before or not.
  return replace_section(product, info_table);
}

bool configured_applications::find_section(const astring &section_name,
    string_table &info_found)
{
//  FUNCDEF("find_section");
  info_found.reset();
  auto_synchronizer l(*_lock);
  if (!_sector->find_section(section_name, info_found)) {
    LOG(section_name + " was not found in the configuration.");
    return false;
  }
  return true;
}

bool configured_applications::add_section(const astring &section_name,
    const string_table &info_found)
{
  auto_synchronizer l(*_lock);
  return _sector->add_section(section_name, info_found);
}

bool configured_applications::replace_section(const astring &section_name,
    const string_table &info_found)
{
  auto_synchronizer l(*_lock);
  return _sector->replace_section(section_name, info_found);
}

astring configured_applications::make_startup_entry(const astring &product,
    const astring &parms, bool one_shot)
{
  return astring(PRODUCT_HEADING()) + ASSIGN_TOKEN() + product
      + SEPARATOR_TEXT() + PARMS_HEADING() + ASSIGN_TOKEN()
      + parms + SEPARATOR_TEXT() + ONESHOT_HEADING() + ASSIGN_TOKEN()
      + astring(astring::SPRINTF, "%d", one_shot);
}

bool configured_applications::parse_startup_entry(const astring &info,
    astring &product, astring &parms, bool &one_shot)
{
//  FUNCDEF("parse_startup_section");
  // parse the items that are in the entry for this program.
  variable_tokenizer entry_parser(SEPARATOR_TOKEN(), ASSIGN_TOKEN());
  entry_parser.parse(info);
  // grab the pertinent bits for the program to be started.
  product = entry_parser.find(PRODUCT_HEADING());
  parms = entry_parser.find(PARMS_HEADING());
//LOG(astring("parms=") + parms);
  astring once = entry_parser.find(ONESHOT_HEADING());
  one_shot = (bool)once.convert(0);
  // we require the product part at least.
  if (!product) return false;
  return true;
}

bool configured_applications::find_entry(const string_table &table,
    const astring &name, astring &location)
{
  // seek the entry in the table specified.
  astring *found = table.find(name);
  if (!found) return false;
  // found the entry using the name.
  location = *found;
  return true;
}

bool configured_applications::add_startup_entry(const astring &product,
    const astring &app_name, const astring &parameters, int one_shot)
{
//  FUNCDEF("add_startup_entry");
  auto_synchronizer l(*_lock);

  LOG(astring("product \"") + product + "\", application \"" + app_name
      + (one_shot? astring("\", OneShot") : astring("\", MultiUse")));

  string_table startup_info;
  if (!find_section(STARTUP_SECTION(), startup_info)) {
    // if there's no startup section, we can't go on.  that should have been
    // created during startup of this program.
    LOG(astring("internal startup section not found!"));
    return false;
  }

  astring new_entry = make_startup_entry(product, parameters,
      one_shot);
  startup_info.add(app_name, new_entry);
  if (!replace_section(STARTUP_SECTION(), startup_info))
    return false;
//hmmm: that's a bogus error; this is really an internal fup error.

  return true;
}

bool configured_applications::remove_startup_entry(const astring &product,
    const astring &app_name)
{
//  FUNCDEF("remove_startup_entry");
  auto_synchronizer l(*_lock);

  LOG(astring("product \"") + product + "\", application \"" + app_name + "\"");

  string_table startup_info;
  if (!find_section(STARTUP_SECTION(), startup_info)) {
    // if there's no startup section, we try to add one.
    add_section(STARTUP_SECTION(), startup_info);
    // if it still doesn't exist afterwards, we're hosed.
    if (!find_section(STARTUP_SECTION(), startup_info)) {
///      COMPLAIN_PRODUCT;
//massive fup of some unanticipated sort.
//complain.
      return false;
    }
  }

  // check that the entry already exists for this program.
  astring entry_found;
  if (!find_entry(startup_info, app_name, entry_found)) {
//    COMPLAIN_APPLICATION;
    LOG(astring("no entry was found for ") + app_name);
    return false;
  }

  startup_info.whack(app_name);
  if (!replace_section(STARTUP_SECTION(), startup_info)) {
//what happened with that?
    return false;
  }

  return true;
}

} //namespace.


