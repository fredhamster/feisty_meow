/*
*  Name   : registry_configurator                                             *
*  Author : Chris Koeritz                                                     *
**
* Copyright (c) 2004-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include "registry_config.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/utf_conversion.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_array.h>
#include <structures/string_table.h>

using namespace basis;
using namespace filesystem;
using namespace structures;

#ifdef _MSC_VER

  // this implementation only works on windows currently.
//hmmm: i suppose we could fake it with an ini file.

  #include <shlwapi.h>
#endif

#undef LOG
#ifdef DEBUG_REGISTRY_CONFIGURATOR
  #define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)
#else
  #define LOG(s) {}
#endif

//#define DEBUG_REGISTRY_CONFIGURATOR
  // uncomment for noisy version.

namespace configuration {

const int MAXIMUM_ENTRY_SIZE = 256 * KILOBYTE;
  // arbitrary maximum for entries we'll read.

const int MAXIMUM_NAME_SIZE = 16384;
  // the longest that value names can be in the registry.

// a default we hope never to see in the registry.
//SAFE_STATIC_CONST(astring, registry_configurator::reg_str_fake_default,
 //   ("bogus_never_should_see"));
const astring &registry_configurator::reg_str_fake_default()
{
  static astring _hidden = "bogus_never_should_see";
  return _hidden;
}

registry_configurator::registry_configurator(registry_hives hive,
      treatment_of_defaults behavior)
: configurator(behavior),
  _hive(hive)
{}

registry_configurator::~registry_configurator()
{}

#ifndef __WIN32__
// fake the platform dependent names.
void *HKEY_CLASSES_ROOT = NULL;
void *HKEY_CURRENT_USER = NULL;
void *HKEY_LOCAL_MACHINE = NULL;
void *HKEY_USERS = NULL;
void *HKEY_CURRENT_CONFIG = NULL;
#endif

void *registry_configurator::translate_hive(registry_hives hive)
{
  switch (hive) {
    case hkey_classes_root: return HKEY_CLASSES_ROOT;
    case hkey_current_user: return HKEY_CURRENT_USER;
    case hkey_local_machine: return HKEY_LOCAL_MACHINE;
    case hkey_users: return HKEY_USERS;
    case hkey_current_config: return HKEY_CURRENT_CONFIG;
    default: return 0;
  }
}

astring registry_configurator::fix_section(const astring &section)
{
  astring to_return = section;
  for (int i = 0; i < to_return.length(); i++) {
    if (to_return[i] == '/')
      to_return[i] = '\\';
  }
  return to_return;
}

bool registry_configurator::put(const astring &section_in, const astring &entry,
    const astring &to_store)
{
  FUNCDEF("put");
  astring section = fix_section(section_in);
  if (!to_store.length()) return delete_entry(section, entry);
  else if (!section.length()) return false;

#ifdef _MSC_VER
  HKEY key;
  long ret = RegOpenKeyEx((HKEY)translate_hive(_hive),
      to_unicode_temp(section), 0, KEY_WRITE, &key);
  if (ret != ERROR_SUCCESS) {
    LOG("failed to open the key, trying to create it.");
    DWORD dispose;  // the disposition of the call (created or existing).
    ret = RegCreateKeyEx((HKEY)translate_hive(_hive),
        to_unicode_temp(section), 0, NULL_POINTER, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL_POINTER, &key, &dispose);
    if (ret != ERROR_SUCCESS) {
      LOG("failed to create the key!!");
      return false;
    }
  }

  bool to_return = true;
  ret = RegSetValueEx(key, to_unicode_temp(entry), 0, REG_SZ,
      (byte *)to_store.s(), to_store.length() + 1);
  if (ret != ERROR_SUCCESS) {
    LOG(astring("failed to write the entry!"));
    to_return = false;
  }

  RegCloseKey(key);
  return to_return;
#else
  return false;
#endif
}

bool registry_configurator::get(const astring &section_in, const astring &entry,
    astring &found)
{
  FUNCDEF("get");
  found = "";
  if (!section_in) return false;
  if (!entry) {}  // not a problem.
  astring section = fix_section(section_in);
#ifdef _MSC_VER
  HKEY key;
  long ret = RegOpenKeyEx((HKEY)translate_hive(_hive),
      to_unicode_temp(section), 0, KEY_QUERY_VALUE, &key);
  if (ret != ERROR_SUCCESS) {
    LOG("failed to open the key!");
    return false;
  }

  DWORD type_seen;
  byte *data_seen = new byte[MAXIMUM_ENTRY_SIZE];
  DWORD length = MAXIMUM_ENTRY_SIZE - 1;
  ret = RegQueryValueEx(key, to_unicode_temp(entry), 0, &type_seen, data_seen,
      &length);
  if (ret != ERROR_SUCCESS) {
    LOG(astring("failed to read the entry!"));
    return false;
  }

  if (type_seen != REG_SZ) {
    LOG(astring("entry found was not of string type!"));
    RegCloseKey(key);
    return false;
  }

  data_seen[MAXIMUM_ENTRY_SIZE - 1] = '\0';
    // force last character to be null if data happened to be too big.
  found = astring((char *)data_seen);

  delete [] data_seen;

  RegCloseKey(key);
  return true;
#else
  return false;
#endif
}

bool registry_configurator::get_section(const astring &section_in,
    string_table &info)
{
  FUNCDEF("get_section");
  info.reset();
  if (!section_in.length()) return false;
  astring section = fix_section(section_in);
#ifdef _MSC_VER
  HKEY key;
  long ret = RegOpenKeyEx((HKEY)translate_hive(_hive),
      to_unicode_temp(section), 0, KEY_QUERY_VALUE, &key);
  if (ret != ERROR_SUCCESS) {
    LOG("failed to open the key!");
    return false;
  }

  DWORD type_seen;
  byte *data_seen = new byte[MAXIMUM_ENTRY_SIZE];
  flexichar *name_seen = new flexichar[MAXIMUM_NAME_SIZE];
  DWORD name_length;
  for (DWORD index = 0; true; index++) {
    DWORD length = MAXIMUM_ENTRY_SIZE - 1;
    name_length = MAXIMUM_NAME_SIZE - 1;
    LONG ret = RegEnumValue(key, index, name_seen, &name_length, 0,
        &type_seen, data_seen, &length);
    if (ret != ERROR_SUCCESS) break;  // no entry at that index.
    if (type_seen == REG_SZ) {
      // found an entry successfully and it's the right type.
      astring name = from_unicode_temp(name_seen);
      astring content = from_unicode_temp((flexichar *)data_seen);
      info.add(name, content);
    }
  }

  delete [] data_seen;
  delete [] name_seen;

  RegCloseKey(key);

  return true;
#else
  return false;
#endif
}

bool registry_configurator::section_exists(const astring &section_in)
{
  FUNCDEF("section_exists");
  if (!section_in.length()) return false;
  astring section = fix_section(section_in);
#ifdef _MSC_VER
  HKEY key;
  long ret = RegOpenKeyEx((HKEY)translate_hive(_hive),
      to_unicode_temp(section), 0, KEY_QUERY_VALUE, &key);
  if (ret != ERROR_SUCCESS) {
    LOG("failed to open the key!");
    return false;
  }
  RegCloseKey(key);
  return true;
#else
  return false;
#endif
}

bool registry_configurator::delete_section(const astring &section_in)
{
  FUNCDEF("delete_section");
  if (!section_in.length()) return false;
  astring section = fix_section(section_in);
//if the key doesn't exist, should that be a failure?
#ifdef _MSC_VER
  long ret = SHDeleteKey((HKEY)translate_hive(_hive),
      to_unicode_temp(section));
  if (ret != ERROR_SUCCESS) {
    LOG("failed to delete the key!");
    return false;
  }
  return true;
#else
  return false;
#endif
}

bool registry_configurator::delete_entry(const astring &section_in,
    const astring &entry)
{
  FUNCDEF("delete_entry");
  if (!section_in.length()) return false;
  astring section = fix_section(section_in);
  if (!entry) {}  // no problem.

#ifdef _MSC_VER
  HKEY key;
  long ret = RegOpenKeyEx((HKEY)translate_hive(_hive),
      to_unicode_temp(section), 0, KEY_SET_VALUE, &key);
  if (ret != ERROR_SUCCESS) {
    LOG("failed to open the key!");
    return false;
  }

  bool to_return = true;
  ret = RegDeleteValue(key, to_unicode_temp(entry));
  if (ret != ERROR_SUCCESS) {
    LOG(astring("failed to delete the entry!"));
    to_return = false;
  }

  RegCloseKey(key);
  return to_return;
#else
  return false;
#endif
}

bool registry_configurator::put_section(const astring &section_in,
    const string_table &info)
{
  if (!section_in) return false;
  astring section = fix_section(section_in);
  bool failures = false;
  for (int i = 0; i < info.symbols(); i++) {
    bool worked = put(section, info.name(i), info[i]);
    if (!worked) failures = true;
  }
  return !failures;
}

} // namespace

