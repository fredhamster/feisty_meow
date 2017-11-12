/*****************************************************************************\
*                                                                             *
*  Name   : check_version                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1996-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "version_checker.h"

#include <application/windoze_helper.h>
#include <basis/astring.h>
#include <basis/environment.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/utf_conversion.h>
#include <configuration/application_configuration.h>
#include <filesystem/filename.h>
#include <loggers/critical_events.h>

#include <stdio.h>

using namespace basis;
using namespace configuration;
using namespace loggers;
using namespace structures;

#ifndef BOOT_STRAPPING
  // pull in the version specified for this build.
  #include <__build_version.h>
//why was this include "on hold"?
#else
  // plug in a fake version for our bootstrapping process.
  #define __build_FILE_VERSION "108.420.1024.10008"
#endif

#ifdef _MSC_VER
  #include <direct.h>
  #include <winver.h>
#endif

#if defined(_MSC_VER)
//#ifdef __WIN32__
  // ensures that we handle the data properly regardless of unicode settings.
  #ifdef UNICODE
    #define render_ptr(ptr) from_unicode_temp( (UTF16 *) ptr)
  #else
    #define render_ptr(ptr) ( (char *) ptr)
  #endif
#endif

//////////////

namespace versions {

version_checker::version_checker(const astring &library_file_name,
    const version &expected_version, const astring &version_complaint)
: _library_file_name(new astring(library_file_name)),
  _expected_version(new version(expected_version)),
  _version_complaint(new astring(version_complaint))
{}

version_checker::~version_checker()
{
  WHACK(_library_file_name);
  WHACK(_expected_version);
  WHACK(_version_complaint);
}

astring version_checker::text_form() const
{
  return astring(class_name()) + ": library_file_name=" + *_library_file_name
      + ", expected_version=" + _expected_version->text_form()
      + ", complaint_message=" + *_version_complaint;
}

bool version_checker::good_version() const
{
  astring version_disabler = environment::TMP();
  version_disabler += "/no_version_check.txt";
  FILE *always_okay = fopen(version_disabler.s(), "r");
  if (always_okay) {
    fclose(always_okay);
    return true;
  }

  version version_found = retrieve_version(*_library_file_name);
  if (version_found.compatible(*_expected_version)) return true;  // success.
  complain_wrong_version(*_library_file_name, *_expected_version,
      version_found);
  return false;
}

bool version_checker::loaded(const astring &library_file_name)
{
//#ifdef __WIN32__
#if defined(_MSC_VER)
  return bool(get_handle(library_file_name) != 0); 
#else
//temp code. 
  return true || library_file_name.t();
#endif
}

void *version_checker::get_handle(const astring &library_file_name)
{
//#ifdef __WIN32__
#if defined(_MSC_VER)
  return GetModuleHandle(to_unicode_temp(library_file_name));
#else
  if (library_file_name.t()) return NULL_POINTER; else return NULL_POINTER;
#endif
}

astring version_checker::module_name(const void *module_handle)
{
#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
  if (module_handle) {}
  return application_configuration::application_name();
#elif defined(_MSC_VER)
//#elif defined(__WIN32__)
  flexichar low_buff[MAX_ABS_PATH + 1];
  GetModuleFileName((HMODULE)module_handle, low_buff, MAX_ABS_PATH - 1);
  astring buff = from_unicode_temp(low_buff);
  buff.to_lower();
  return buff;
#else
  #pragma message("module_name unknown for this operating system.")
  return application_configuration::application_name();
#endif
}

astring version_checker::get_name(const void *to_find)
{ return module_name(to_find); }

bool version_checker::retrieve_version_info(const astring &filename,
    byte_array &to_fill)
{
  to_fill.reset();  // clear the buffer.

  // determine the required size of the version info buffer.
  int required_size;
#if defined(_MSC_VER)
//#ifdef __WIN32__
  un_long module_handle;  // filled with the dll or exe handle.
  required_size = GetFileVersionInfoSize(to_unicode_temp(filename), &module_handle);
#else
  required_size = 0 && filename.t();
#endif
  if (!required_size) return false;
  to_fill.reset(required_size);  // resize the buffer.
  
  // read the version info into our buffer.
  bool success = false;
#if defined(_MSC_VER)
//#ifdef __WIN32__
  success = GetFileVersionInfo(to_unicode_temp(filename), module_handle,
      required_size, to_fill.access());
#else
  success = false;
#endif
  return success;
}

bool version_checker::get_language(byte_array &version_chunk,
    basis::un_short &high, basis::un_short &low)
{
  high = 0;
  low = 0;
#if defined(_MSC_VER)
//#ifdef __WIN32__
  // determine the language that the version's written in.
  basis::un_int data_size;
  void *pointer_to_language_structure;
  // query the information from the version blob.
  if (!VerQueryValue(version_chunk.access(),
      to_unicode_temp("\\VarFileInfo\\Translation"),
      &pointer_to_language_structure, &data_size))
    return false;
  // get the low & high shorts of the structure.
  high = LOWORD(*(unsigned int *)pointer_to_language_structure);
  low = HIWORD(*(unsigned int *)pointer_to_language_structure);
#else
  high = 0 && version_chunk.length();
  low = 0;
#endif

  return true;
}

version version_checker::retrieve_version(const astring &filename)
{
//#ifdef UNIX
#if defined(__UNIX__) || defined(__GNU_WINDOWS__)

  // totally bogus stand-in; this just returns the version we were built with
  // rather than the version that's actually tagged on the file.

//hmmm: fix this!  get the version header back.
//  return version(__build_FILE_VERSION);
  return version();

#endif

  byte_array version_info_found(0, NULL_POINTER);
  if (!retrieve_version_info(filename, version_info_found))
    return version(0, 0, 0, 0);

  basis::un_short high, low;  // holds the language of the version data.
  if (!get_language(version_info_found, high, low))
    return version(0, 0, 0, 0);

  // retrieve the file version from version info using the appropriate
  // language.
  astring root_key(astring::SPRINTF, "\\StringFileInfo\\%04x%04x", high, low);
  astring file_version_key(root_key + astring("\\FileVersion"));

  astring version_string;
#ifdef _MSC_VER
  abyte *file_version_pointer;
  basis::un_int data_size;
  if (!VerQueryValue(version_info_found.access(),
      to_unicode_temp(file_version_key),
      (LPVOID *)&file_version_pointer, &data_size))
    return version(0, 0, 0, 0);
  version_string = render_ptr(file_version_pointer);
  // clean any spaces out of the string; people sometimes format these
  // very badly.
  for (int i = 0; i < version_string.length(); i++) {
    if (version_string[i] == ' ') {
      version_string.zap(i, i);
      i--;  // skip back a beat.
    }
  }
#else
  return version(0, 0, 0, 0);
//tmp.
#endif
  return version::from_text(version_string);
}

bool version_checker::get_record(const astring &filename, 
    version_record &to_fill)
{
  to_fill = version_record();
  byte_array version_info_found(0, NULL_POINTER);
  if (!retrieve_version_info(filename, version_info_found))
    return false;

  basis::un_short high, low;  // holds the language of the version data.
  if (!get_language(version_info_found, high, low))
    return false;

  // set the root key for all accesses of the version chunk.
  astring root_key(astring::SPRINTF, "\\StringFileInfo\\%04x%04x", high, low);

  // reports whether all lookups succeeded or not.
  bool total_success = true;

  // the various version pieces are retrieved...

//#ifdef __WIN32__
#ifdef _MSC_VER
  basis::un_int data_size;
  void *data_pointer;

  // file version.
  if (!VerQueryValue(version_info_found.access(), to_unicode_temp(root_key
      + astring("\\FileVersion")), &data_pointer, &data_size))
    total_success = false;
  else
    to_fill.file_version = version::from_text(render_ptr(data_pointer));

  // company name.
  if (!VerQueryValue(version_info_found.access(), to_unicode_temp(root_key
      + astring("\\CompanyName")), &data_pointer, &data_size))
    total_success = false;
  else
    to_fill.company_name = render_ptr(data_pointer);

  // file description.
  if (!VerQueryValue(version_info_found.access(), to_unicode_temp(root_key
      + astring("\\FileDescription")), &data_pointer, &data_size))
    total_success = false;
  else
    to_fill.description = render_ptr(data_pointer);

  // internal name.
  if (!VerQueryValue(version_info_found.access(), to_unicode_temp(root_key
      + astring("\\InternalName")), &data_pointer, &data_size))
    total_success = false;
  else
    to_fill.internal_name = render_ptr(data_pointer);

  // copyright info.
  if (!VerQueryValue(version_info_found.access(), to_unicode_temp(root_key
      + astring("\\LegalCopyright")), &data_pointer, &data_size))
    total_success = false;
  else
    to_fill.copyright = render_ptr(data_pointer);

  // trademark info.
  if (!VerQueryValue(version_info_found.access(), to_unicode_temp(root_key
      + astring("\\LegalTrademarks")), &data_pointer, &data_size))
    total_success = false;
  else
    to_fill.trademarks = render_ptr(data_pointer);

  // original file name.
  if (!VerQueryValue(version_info_found.access(), to_unicode_temp(root_key
      + astring("\\OriginalFilename")), &data_pointer, &data_size))
    total_success = false;
  else
    to_fill.original_name = render_ptr(data_pointer);

  // product name.
  if (!VerQueryValue(version_info_found.access(), to_unicode_temp(root_key
      + astring("\\ProductName")), &data_pointer, &data_size))
    total_success = false;
  else
    to_fill.product_name = render_ptr(data_pointer);

  // product version.
  if (!VerQueryValue(version_info_found.access(), to_unicode_temp(root_key
      + astring("\\ProductVersion")), &data_pointer, &data_size))
    total_success = false;
  else
    to_fill.product_version = version::from_text(render_ptr(data_pointer));
#else
  // hmmm: chunks missing in version check.
#endif

  return total_success;
}

void version_checker::complain_wrong_version(const astring &library_file_name,
    const version &expected_version, const version &version_found) const
{
  astring to_show("There has been a Version Mismatch: The module \"");
  // use embedded module handle to retrieve name of dll or exe.
  astring module_name = get_name(version::__global_module_handle());
  if (!module_name) module_name = "Unknown";
  to_show += module_name;
  to_show += astring("\" cannot load.  This is because the file \"");
  to_show += library_file_name;
  to_show += astring("\" was expected to have a version of [");

  to_show += expected_version.flex_text_form(version::DOTS);

  to_show += astring("] but it instead had a version of [");
  to_show += version_found.flex_text_form(version::DOTS);

  to_show += astring("].  ");
  to_show += *_version_complaint;
//#ifdef __UNIX__
#if defined(__UNIX__) || defined(__GNU_WINDOWS__)
  continuable_error("version checking", "failure", to_show.s());
#elif defined(_MSC_VER)
  MessageBox(0, to_unicode_temp(to_show),
      to_unicode_temp("version_checking::failure"), MB_OK);
#endif
}

void version_checker::complain_cannot_load(const astring &lib_file) const
{
  astring to_show("There has been a failure in Version Checking: The file \"");
  to_show += lib_file;
  to_show += astring("\" could not be loaded or found.  ");
  to_show += *_version_complaint;
  continuable_error("version checking", "loading dll", to_show.s());
}

} //namespace.


