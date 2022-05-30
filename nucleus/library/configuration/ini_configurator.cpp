/*****************************************************************************\
*                                                                             *
*  Name   : ini_configurator                                                  *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "ini_configurator.h"
#include "application_configuration.h"
#include "variable_tokenizer.h"

#include <application/windoze_helper.h>
#include <basis/astring.h>
#include <basis/environment.h>
#include <basis/functions.h>
#include <basis/mutex.h>
#include <basis/utf_conversion.h>
#include <filesystem/byte_filer.h>
#include <filesystem/directory.h>
#include <filesystem/filename.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_array.h>
#include <structures/string_table.h>
#include <structures/symbol_table.h>

#include <stdio.h>

#undef LOG
#define LOG(to_print) printf("%s::%s: %s\n", static_class_name(), func, astring(to_print).s())

using namespace basis;
using namespace filesystem;
using namespace structures;

namespace configuration {

//#define DEBUG_INI_CONFIGURATOR
  // uncomment for noisy version.

const int MAXIMUM_LINE_INI_CONFIG = 16384;

// a default we hope never to see in an ini file.
SAFE_STATIC_CONST(astring, ini_configurator::ini_str_fake_default, ("NoTomatoesNorPotatoesNorQuayle"))

ini_configurator::ini_configurator(const astring &ini_filename,
      treatment_of_defaults behavior, file_location_default where)
: configurator(behavior),
  _ini_name(new filename),
#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
  _parser(new ini_parser("", behavior)),
#endif
  _where(where),
  _add_spaces(false)
{
  FUNCDEF("constructor");
  name(ini_filename);  // set name properly.
//LOG(astring("calculated ini name as: '") + _ini_name->raw() + "'");
}

ini_configurator::~ini_configurator()
{
  WHACK(_ini_name);
#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
  WHACK(_parser);
#endif
}

astring ini_configurator::name() const { return _ini_name->raw(); }

void ini_configurator::refresh()
{
#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
  write_ini_file();
  WHACK(_parser);
  _parser = new ini_parser("", behavior());
#endif
}

void ini_configurator::name(const astring &name)
{
  *_ini_name = name;

  bool use_appdir = true;
    // true if we should put files where programs start for those filenames
    // that don't include a directory name.
  if (_where == OS_DIRECTORY) use_appdir = false;
  if (_where == ALL_USERS_DIRECTORY) use_appdir = false;
#ifdef _MSC_VER
  use_appdir = true;
#endif
  // we must create the filename if they specified no directory at all.
  if (!_ini_name->had_directory()) {
    if (use_appdir) {
      // this is needed in case there is an ini right with the file; our
      // standard is to check there first.
      *_ini_name = filename(application_configuration::application_directory(),
          _ini_name->basename());
    } else if (!use_appdir && (_where == ALL_USERS_DIRECTORY) ) {
      // when the location default is all users, we get that from the
      // environment.  for the OS dir choice, we leave out the path entirely.
      directory::make_directory(environment::get("ALLUSERSPROFILE")
          + "/" + application_configuration::software_product_name());
      *_ini_name = filename(environment::get("ALLUSERSPROFILE")
          + "/" + application_configuration::software_product_name(),
          _ini_name->basename());
    }
  }
#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
  // read in the file's contents.
  read_ini_file();
#endif
}

void ini_configurator::sections(string_array &list)
{
  list = string_array();
  // open our ini file directly as a file.
  byte_filer section8(*_ini_name, "rb");
  if (!section8.good()) return;  // not a healthy file.
  astring line_found;
  // iterate through the lines of the ini file and see if we can't find a
  // bunch of section names.
  while (section8.read(line_found, MAXIMUM_LINE_INI_CONFIG) > 0) {
    // is the line in the format "^[ \t]*\[\([^\]]+\)\].*$" ?
    // if it is in that format, we add the matched \1 into our list.
    line_found.strip_white_spaces();
    if (line_found[0] != '[') continue;  // no opening bracket.  skip line.
    line_found.zap(0, 0);  // toss opening bracket.
    int close_brack_indy = line_found.find(']');
    if (negative(close_brack_indy)) continue;  // no closing bracket.
    line_found.zap(close_brack_indy, line_found.end());
    list += line_found;
  }
}

//hmmm: refactor section_exists to use the sections call, if it's faser?
bool ini_configurator::section_exists(const astring &section)
{
#ifdef _MSC_VER
  string_table infos;
  // heavy-weight call here...
  return get_section(section, infos);
#else
  return _parser->section_exists(section);
#endif
}

#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
void ini_configurator::read_ini_file()
{
#ifdef DEBUG_INI_CONFIGURATOR
  FUNCDEF("read_ini_file");
#endif
  _parser->reset("");  // clear out our current contents.
  byte_filer ini_file;
  bool open_ret = ini_file.open(*_ini_name, "rb");  // simple reading.
#ifdef DEBUG_INI_CONFIGURATOR
  if (!open_ret) LOG(astring("failed to open ini file: ") + *_ini_name);
  if (!ini_file.good()) LOG(astring("ini file not good: ") + *_ini_name);
#endif
  if (!open_ret || !ini_file.good()) {
    return;  // failure.
  }
  int file_size = ini_file.length();  // get the file length.
  // read the file.
  astring contents(' ', file_size + 3);
  int bytes_read = ini_file.read((abyte *)contents.observe(), file_size);
  contents.zap(bytes_read + 1, contents.end());
  _parser->reset(contents);
}

void ini_configurator::write_ini_file()
{
#ifdef DEBUG_INI_CONFIGURATOR
  FUNCDEF("write_ini_file");
#endif

//hmmm: just set dirty flag and use that for deciding whether to write.
//hmmm: future version, have a thread scheduled to write.

  // open filer with new mode for cleaning.
  byte_filer ini_file;
  bool open_ret = ini_file.open(*_ini_name, "wb");
    // open the file for binary read/write and drop previous contents.
#ifdef DEBUG_INI_CONFIGURATOR
  if (!open_ret) LOG(astring("failed to open ini file: ") + *_ini_name);
  if (!ini_file.good()) LOG(astring("ini file not good: ") + *_ini_name);
#endif
  if (!open_ret || !ini_file.good()) return;  // failure.

  // output table's contents to text.
  astring text;
  _parser->restate(text, _add_spaces);
  ini_file.write((abyte *)text.observe(), text.length());
}
#endif //UNIX

bool ini_configurator::delete_section(const astring &section)
{
#ifdef _MSC_VER
  return put_profile_string(section, "", ""); 
#else
  // zap the section.
  bool to_return = _parser->delete_section(section);
  // schedule the file to write.
  write_ini_file();
  return to_return;
#endif
}

bool ini_configurator::delete_entry(const astring &section, const astring &ent)
{
#ifdef _MSC_VER
  return put_profile_string(section, ent, "");
#else
  // zap the entry.
  bool to_return = _parser->delete_entry(section, ent);
  // schedule the file to write.
  write_ini_file();
  return to_return;
#endif
}

bool ini_configurator::put(const astring &section, const astring &entry,
    const astring &to_store)
{
  FUNCDEF("put");
  if (!to_store.length()) return delete_entry(section, entry);
  else if (!entry.length()) return delete_section(section);
  else if (!section.length()) return false;
#ifdef _MSC_VER
  return put_profile_string(section, entry, to_store);
#else
  // write the entry.
  bool to_return = _parser->put(section, entry, to_store);
  // schedule file write.
  write_ini_file();
  return to_return;
#endif
}

bool ini_configurator::get(const astring &section, const astring &entry,
    astring &found)
{
#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
  return _parser->get(section, entry, found);
#else
  flexichar temp_buffer[MAXIMUM_LINE_INI_CONFIG];
  temp_buffer[0] = 0;
  get_profile_string(section, entry, ini_str_fake_default(),
      temp_buffer, MAXIMUM_LINE_INI_CONFIG - 1);
  found = from_unicode_temp(temp_buffer);
  return !(ini_str_fake_default() == found);
#endif
}

bool ini_configurator::get_section(const astring &section, string_table &info)
{
  FUNCDEF("get_section");
#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
  return _parser->get_section(section, info);
#else
  info.reset();
  const int buffer_size = 200000;

  flexichar low_buff[buffer_size + 3];
  int read_len = GetPrivateProfileSection(to_unicode_temp(section.observe()),
      low_buff, buffer_size - 1, to_unicode_temp(name()));
  if (!read_len) return false;  // assume the API means there was no section.

  low_buff[read_len] = '\1';  // signal beyond the end of the stuff.
  low_buff[read_len + 1] = '\0';  // make sure we're still zero terminated.

  bool last_was_nil = false;
  // this loop replaces all the embedded nils with separators to allow the
  // variable_tokenizer to retrieve all the strings from the section.
  for (int i = 0; i < read_len; i++) {
    if (!low_buff[i] && last_was_nil) {
      // termination condition; we got two nils in a row.
      // this is just paranoia; the length should tell us.
      break;
    } else if (!low_buff[i]) {
      low_buff[i] = '\1';  // replace with a separator.
      last_was_nil = true;
    } else last_was_nil = false;  // reset the nil flag.
  }

  // now convert to a simple astring.
  astring buff = from_unicode_temp(low_buff);
  int length = buff.length();
  buff.shrink();
  variable_tokenizer parser("\1", "=");
  parser.parse(buff);
  info = parser.table();
  return true;
#endif
}

bool ini_configurator::put_section(const astring &section,
    const string_table &info)
{
#ifdef _MSC_VER
  variable_tokenizer parser("\1", "=");
  parser.table() = info;
  astring flat = parser.text_form();
  flat += "\1\1";  // add terminating guard.
  int len = flat.length();
  for (int i = 0; i < len; i++) {
    if (flat[i] == '\1') {
      flat[i] = '\0';
      if (flat[i+1] == ' ') {
        // if the space character is next, shift it before the nil to avoid
        // keys with a preceding space.
        flat[i] = ' ';
        flat[i + 1] = '\0';
      }
    }
  }
  return WritePrivateProfileSection(to_unicode_temp(section),
      to_unicode_temp(flat), to_unicode_temp(name()));
#else
  // write the section.
  bool to_return = _parser->put_section(section, info);
  // schedule file write.
  write_ini_file();
  return to_return;
#endif
}

#ifdef _MSC_VER
bool ini_configurator::put_profile_string(const astring &section,
    const astring &entry, const astring &to_store)
{
  return bool(WritePrivateProfileString(to_unicode_temp(section),
      entry.length() ? (flexichar *)to_unicode_temp(entry) : NULL_POINTER,
      to_store.length() ? (flexichar *)to_unicode_temp(to_store) : NULL_POINTER,
      to_unicode_temp(name())));
}

void ini_configurator::get_profile_string(const astring &section,
    const astring &entry, const astring &default_value,
    flexichar *return_buffer, int buffer_size)
{
  GetPrivateProfileString(section.length() ?
      (flexichar *)to_unicode_temp(section) : NULL_POINTER,
      entry.length() ? (flexichar *)to_unicode_temp(entry) : NULL_POINTER,
      to_unicode_temp(default_value),
      return_buffer, buffer_size, to_unicode_temp(name()));
}
#endif

} //namespace.


