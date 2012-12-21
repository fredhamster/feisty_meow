/*****************************************************************************\
*                                                                             *
*  Name   : version_ini editing support                                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1995-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "version_ini.h"

#include <basis/functions.h>
#include <configuration/application_configuration.h>
#include <configuration/ini_configurator.h>
#include <filesystem/byte_filer.h>
#include <filesystem/directory.h>
#include <filesystem/filename.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <structures/string_array.h>

#include <sys/stat.h>
#ifdef __WIN32__
  #include <io.h>
#endif

using namespace basis;
using namespace configuration;
using namespace filesystem;
using namespace loggers;
using namespace structures;

namespace versions {

// the following are all strings that are sought in the version.ini file.
const char *version_ini::VERSION_SECTION = "version";
  // the section that version entries are stored under in the INI file.
const char *version_ini::COMPANY_KEY="company";
const char *version_ini::COPYRIGHT_KEY="copyright";
const char *version_ini::LEGAL_INFO_KEY="legal_info";
const char *version_ini::PRODUCT_KEY="product_name";
const char *version_ini::WEB_SITE_KEY="web_site";

// not used anymore; now matched with the file version's first two digits.
//const version PRODUCT_VERSION(2, 0, 0, 0);
  // the current version of the entire product.

// these are field names in the INI file.
const char *version_ini::MAJOR = "major";
const char *version_ini::MINOR = "minor";
const char *version_ini::REVISION = "revision";
const char *version_ini::BUILD = "build";
const char *version_ini::DESCRIPTION = "description";
const char *version_ini::ROOT = "root";
const char *version_ini::NAME = "name";
const char *version_ini::EXTENSION = "extension";
const char *version_ini::OLE_AUTO = "ole_auto";

// this is the default version INI file name, if no other is specified.
const char *VERSION_INI_FILE = "/version.ini";

#undef LOG
#define LOG(t) CLASS_EMERGENCY_LOG(program_wide_logger::get(), t)

version_ini::version_ini(const astring &path_name)
: _loaded(false),
  _path_name(new filename(path_name)),
  _ini(new ini_configurator("", ini_configurator::RETURN_ONLY)),
  _held_record(new version_record)
{
  check_name(*_path_name);
  _ini->name(*_path_name);
}

version_ini::~version_ini()
{
  WHACK(_ini);
  WHACK(_path_name);
  WHACK(_held_record);
}

bool version_ini::ole_auto_registering()
{
  astring extension = _ini->load(VERSION_SECTION, OLE_AUTO, "");
  return (extension.lower().t());
}

bool version_ini::executable()
{
  astring extension = _ini->load(VERSION_SECTION, EXTENSION, "");
  if (extension.lower() == astring("exe")) return true;
  return false;
}

bool version_ini::library() { return !executable(); }

bool version_ini::writable() { return _path_name->is_writable(); }

void version_ini::check_name(filename &to_examine)
{
  // if it's just a directory name, add the file name.
  if (to_examine.is_directory()) {
    to_examine = astring(to_examine) + VERSION_INI_FILE;
    to_examine.canonicalize();
  }

  // add the directory explicitly (if it's not there already) or the ini
  // writer will get it from the windows directory.
  if ( (to_examine.raw()[0] != '.') && (to_examine.dirname().raw().equal_to(".")) ) {
    to_examine = astring("./") + to_examine;
    to_examine.canonicalize();
  }
}

bool version_ini::executable(const astring &path_name_in)
{
  filename path_name(path_name_in);
  check_name(path_name);
  ini_configurator temp_ini(path_name, ini_configurator::RETURN_ONLY);
  astring extension = temp_ini.load(VERSION_SECTION, EXTENSION, "");
  extension.to_lower();
  if (extension == astring("exe")) return true;
  return false;
}

bool version_ini::library(const astring &path_name)
{ return !executable(path_name); }

version version_ini::get_version()
{
  if (_loaded) return _held_record->file_version;
  get_record();
  return _held_record->file_version;
}

void version_ini::set_version(const version &to_write, bool write_ini)
{
  _held_record->file_version = to_write;  // copy the version we're given.

  // set the product version appropriately to the file version.
  _held_record->product_version = to_write;
  _held_record->product_version.set_component(version::REVISION, "0");
  _held_record->product_version.set_component(version::BUILD, "0");

  if (!write_ini) return;  // they don't want a change to the file.
  _ini->store(VERSION_SECTION, MAJOR, to_write.get_component(version::MAJOR));
  _ini->store(VERSION_SECTION, MINOR, to_write.get_component(version::MINOR));
  _ini->store(VERSION_SECTION, REVISION, to_write.get_component(version::REVISION));
  _ini->store(VERSION_SECTION, BUILD, to_write.get_component(version::BUILD));
}

version version_ini::read_version_from_ini()
{
  string_array parts;
  parts += _ini->load(VERSION_SECTION, MAJOR, "0");
  parts += _ini->load(VERSION_SECTION, MINOR, "0");
  parts += _ini->load(VERSION_SECTION, REVISION, "0");
  parts += _ini->load(VERSION_SECTION, BUILD, "0");
  return version(parts);
}

version_record &version_ini::access_record() { return *_held_record; }

version_record version_ini::get_record()
{
  FUNCDEF("get_record");
  if (_loaded) return *_held_record;
  version_record to_return;
  to_return.description = _ini->load(VERSION_SECTION, DESCRIPTION, "");
  to_return.file_version = read_version_from_ini();
  to_return.internal_name = _ini->load(VERSION_SECTION, NAME, "");
  to_return.original_name = _ini->load(VERSION_SECTION, ROOT, "");
  to_return.original_name += ".";

  // the dll type of extension is a hard default.  anything besides the exe
  // ending gets mapped to dll.
  astring extension = _ini->load(VERSION_SECTION, EXTENSION, "");
  extension.to_lower();
  if (extension.equal_to("dll")) {}
  else if (extension.equal_to("exe")) {}
  else extension.equal_to("dll");
  to_return.original_name += extension;

  to_return.product_version = to_return.file_version;
  to_return.product_version.set_component(version::REVISION, "0");
  to_return.product_version.set_component(version::BUILD, "0");

  to_return.product_name = _ini->load(VERSION_SECTION, PRODUCT_KEY, "");
  to_return.company_name = _ini->load(VERSION_SECTION, COMPANY_KEY, "");
  to_return.copyright = _ini->load(VERSION_SECTION, COPYRIGHT_KEY, "");
  to_return.trademarks = _ini->load(VERSION_SECTION, LEGAL_INFO_KEY, "");
  to_return.web_address = _ini->load(VERSION_SECTION, WEB_SITE_KEY, "");

  *_held_record = to_return;
  _loaded = true;
  return to_return;
}

void version_ini::set_record(const version_record &to_write, bool write_ini)
{
  *_held_record = to_write;
  if (write_ini) {
    _ini->store(VERSION_SECTION, DESCRIPTION, to_write.description);
    set_version(to_write.file_version, write_ini);
    _ini->store(VERSION_SECTION, ROOT, to_write.original_name);
    _ini->store(VERSION_SECTION, NAME, to_write.internal_name);
  }
  _loaded = true;  // we consider this to be the real version now.
}

//////////////

const astring version_rc_template = "\
#ifndef NO_VERSION\n\
#include <winver.h>\n\
#include <__build_version.h>\n\
#include <__build_configuration.h>\n\
#define BI_PLAT_WIN32\n\
  // force 32 bit compile.\n\
1 VERSIONINFO LOADONCALL MOVEABLE\n\
FILEVERSION __build_FILE_VERSION_COMMAS\n\
PRODUCTVERSION __build_PRODUCT_VERSION_COMMAS\n\
FILEFLAGSMASK 0\n\
FILEFLAGS VS_FFI_FILEFLAGSMASK\n\
#if defined(BI_PLAT_WIN32)\n\
  FILEOS VOS__WINDOWS32\n\
#else\n\
  FILEOS VOS__WINDOWS16\n\
#endif\n\
FILETYPE VFT_APP\n\
BEGIN\n\
  BLOCK \"StringFileInfo\"\n\
  BEGIN\n\
    // Language type = U.S. English(0x0409) and Character Set = Windows, Multilingual(0x04b0)\n\
    BLOCK \"040904b0\"              // Matches VarFileInfo Translation hex value.\n\
    BEGIN\n\
      VALUE \"CompanyName\", __build_company \"\\000\"\n\
#ifndef _DEBUG\n\
      VALUE \"FileDescription\", \"$file_desc\\000\"\n\
#else\n\
      VALUE \"FileDescription\", \"$file_desc (DEBUG)\\000\"\n\
#endif\n\
      VALUE \"FileVersion\", __build_FILE_VERSION \"\\000\" \n\
      VALUE \"ProductVersion\", __build_PRODUCT_VERSION \"\\000\" \n\
      VALUE \"InternalName\", \"$internal\\000\"\n\
      VALUE \"LegalCopyright\", __build_copyright \"\\000\"\n\
      VALUE \"LegalTrademarks\", __build_legal_info \"\\000\"\n\
      VALUE \"OriginalFilename\", \"$original_name\\000\"\n\
      VALUE \"ProductName\", __build_product_name \"\\000\"\n\
      $special_ole_flag\n\
    END\n\
  END\n\
\n\
  BLOCK \"VarFileInfo\"\n\
  BEGIN\n\
    VALUE \"Translation\", 0x0409, 0x04b0 // US English (0x0409) and win32 multilingual (0x04b0)\n\
  END\n\
END\n\
#endif\n";

//////////////

// replaces every occurrence of the keyword in "tag" with the "replacement".
#define REPLACE(tag, replacement) \
  new_version_entry.replace_all(tag, replacement); \

bool version_ini::write_rc(const version_record &to_write)
{
  astring new_version_entry(version_rc_template);

  // $file_ver -> w, x, y, z for version of the file.
  REPLACE("$file_ver", to_write.file_version.flex_text_form(version::COMMAS));

  // $prod_ver -> w, x, y, z for version of the product.
  REPLACE("$prod_ver", to_write.product_version.flex_text_form
      (version::COMMAS));

  // $company -> name of company.
  REPLACE("$company", to_write.company_name);

  // $file_desc -> description of file.
  astring description_release = to_write.description;
  REPLACE("$file_desc", description_release);
  astring description_debug = to_write.description
      + astring(" -- Debug Version");
  REPLACE("$file_desc", description_debug);

  // $file_txt_ver -> file version in form w.x.y.z.
  REPLACE("$file_txt_ver", to_write.file_version.flex_text_form(version::DOTS));

  // $internal -> internal name of the library, without extensions?
  REPLACE("$internal", to_write.internal_name);

  // $copyright -> copyright held by us.
  REPLACE("$copyright", to_write.copyright);

  // $legal_tm -> the legal trademarks that must be included, e.g. windows?
  REPLACE("$legal_tm", to_write.trademarks);

  // $original_name -> the file's name before possible renamings.
  REPLACE("$original_name", to_write.original_name);

  // $prod_name -> the name of the product that this belongs to.
  REPLACE("$prod_name", to_write.product_name);

  // $prod_txt_ver -> product version in form w.x.y.z.
  REPLACE("$prod_txt_ver", to_write.product_version
      .flex_text_form(version::DOTS, version::MINOR));

  astring special_filler;  // nothing by default.
  if (ole_auto_registering())
    special_filler = "VALUE \"OLESelfRegister\", \"\\0\"";
  REPLACE("$special_ole_flag", special_filler);

  astring root_part = "/";
  root_part += _ini->load(VERSION_SECTION, ROOT, "");

  astring rc_filename(astring(_path_name->dirname()) + root_part
      + "_version.rc");

  filename(rc_filename).chmod(filename::ALLOW_BOTH, filename::USER_RIGHTS);
    // make sure we can write to the file.

  byte_filer rc_file(rc_filename, "w");
  if (!rc_file.good()) return false;
  rc_file.write((abyte *)new_version_entry.s(), new_version_entry.length());
  rc_file.close();
  return true;
}

//////////////

const astring version_header_template = "\
#ifndef $lib_prefix_VERSION_HEADER\n\
#define $lib_prefix_VERSION_HEADER\n\
\n\
/*****************************************************************************\\\n\
*                                                                             *\n\
*  Name   : Version header for $lib_name\n\
*  Author : Automatically generated by version_stamper                        *\n\
*                                                                             *\n\
\\*****************************************************************************/\n\
\n\
#include <__build_version.h>\n\
#include <__build_configuration.h>\n\
#include <basis/version_checker.h>\n\
#include <basis/version_record.h>\n\
\n\
#ifdef __WIN32__\n\
\n\
// this macro can be used to check that the current version of the\n\
// $lib_name library is the same version as expected.  to use it, check\n\
// whether it returns true or false.  if false, the version is incorrect.\n\
#define CHECK_$lib_prefix() (version_checker(astring(\"$lib_prefix\")\\\n\
    + astring(\".dll\"), version(__build_SYSTEM_VERSION),\\\n\
      astring(\"Please contact $company_name for the latest DLL and \"\\\n\
        \"Executable files ($web_address).\")).good_version())\n\
\n\
#else\n\
\n\
// null checking for embedded or other platforms without versions.\n\
\n\
#define CHECK_$lib_prefix() 1\n\
\n\
#endif //__WIN32__\n\
\n\
#endif\n\
\n";

//////////////

bool version_ini::write_code(const version_record &to_write)
{
  astring root_part = _ini->load(VERSION_SECTION, ROOT, "");
  astring root = root_part.upper();  // make upper case for naming sake.
  astring name = _ini->load(VERSION_SECTION, NAME, "");
  // replace the macros here also.
  name.replace_all("$product_name", to_write.product_name);

  astring new_version_entry(version_header_template);

//some of the replacements are no longer needed.

  // $lib_prefix -> the first part of the library's name.
  REPLACE("$lib_prefix", root);
  REPLACE("$lib_prefix", root);
  REPLACE("$lib_prefix", root);
  REPLACE("$lib_prefix", root);
  REPLACE("$lib_prefix", root);
  REPLACE("$lib_prefix", root);
  REPLACE("$lib_prefix", root);

  // $lib_name -> the name of the library, as it thinks of itself.
  REPLACE("$lib_name", name);
  REPLACE("$lib_name", name);
  REPLACE("$lib_name", name);

  // $lib_version -> the current version for this library.
  REPLACE("$lib_version", to_write.file_version.flex_text_form(version::COMMAS));

  // $company_name -> the company that produces the library.
  REPLACE("$company_name", to_write.company_name);

  // $web_address -> the web site for the company.  not actually stored.
  REPLACE("$web_address", to_write.web_address);

  astring header_filename(_path_name->dirname().raw() + "/" + root_part
      + astring("_version.h"));

  filename(header_filename).chmod(filename::ALLOW_BOTH, filename::USER_RIGHTS);
    // make sure we can write to the file.

  byte_filer header(header_filename, "w");
  if (!header.good()) return false;
  header.write((abyte *)new_version_entry.s(), new_version_entry.length());
  header.close();
  return true;
}


// returns true if manipulated the full_string to replace its version
bool replace_version_entry(astring &full_string, const astring &look_for,
    const astring &new_ver)
{
  bool to_return = false;
  int posn = 0;  // where are we looking for this?

  while (posn < full_string.length()) {
    int ver_posn = full_string.find(look_for, posn);
    if (ver_posn < 0) break;  // nothing to modify.
    int quote_posn = full_string.find("\"", ver_posn);
    if (quote_posn < 0) break;  // malformed assembly we will not touch.
    int second_quote_posn = full_string.find("\"", quote_posn + 1);
    if (second_quote_posn < 0) break;  // more malformage.
    full_string.zap(quote_posn + 1, second_quote_posn - 1);
    full_string.insert(quote_posn + 1, new_ver);
    to_return = true;  // found a match.
    // skip to the next place.
    posn = quote_posn + new_ver.length();
  }

  return to_return;
}

bool version_ini::write_assembly(const version_record &to_write,
    bool do_logging)
{
  FUNCDEF("write_assembly");
  filename just_dir = _path_name->dirname();
//LOG(astring("dir is set to: ") + just_dir);
  directory dir(just_dir);
  filename to_patch;
//LOG(astring("dir has: ") + dir.files().text_form());
  if (non_negative(dir.files().find("AssemblyInfo.cpp")))
    to_patch = just_dir.raw() + "/AssemblyInfo.cpp";
  else if (non_negative(dir.files().find("AssemblyInfo.cs")))
    to_patch = just_dir.raw() + "/AssemblyInfo.cs";
  if (!to_patch.raw()) {
    // no assembly file yet.  see if there's one in a properties subdirectory.
    directory dir2(just_dir + "/Properties");
    if (non_negative(dir2.files().find("AssemblyInfo.cpp")))
      to_patch = just_dir.raw() + "/Properties/AssemblyInfo.cpp";
    else if (non_negative(dir2.files().find("AssemblyInfo.cs")))
      to_patch = just_dir.raw() + "/Properties/AssemblyInfo.cs";
  }

  if (to_patch.raw().t()) {
    // we have a filename to work on.
    filename(to_patch).chmod(filename::ALLOW_BOTH, filename::USER_RIGHTS);
      // make sure we can write to the file.
    byte_filer modfile(to_patch, "r+b");
    astring contents;
    modfile.read(contents, 1000000);  // read any file size up to that.
    while (contents[contents.end()] == '\032') {
      // erase any stray eof characters that may happen to be present.     
      contents.zap(contents.end(), contents.end());
    }
//LOG(astring("file contents are: \n") + contents);

//here's where to fixit.

    astring ver_string = to_write.file_version.flex_text_form(version::DOTS);
    bool did_replace = replace_version_entry(contents, "AssemblyVersionAttribute", ver_string);
    if (!did_replace) {
      did_replace = replace_version_entry(contents, "AssemblyVersion", ver_string);
    }
    if (!did_replace) return true;  // nothing to modify?
    did_replace = replace_version_entry(contents, "AssemblyFileVersion", ver_string);
    if (!did_replace) {
      did_replace = replace_version_entry(contents, "AssemblyFileVersionAttribute", ver_string);
    }
    // if we got to here, we at least replaced something...

/*
    int ver_posn = contents.find("AssemblyVersionAttribute", 0);
    // try again if that seek failed.
    if (ver_posn < 0)
      ver_posn = contents.find("AssemblyVersion", 0);
    if (ver_posn < 0) return true;  // nothing to modify.
//LOG(astring("found assembly version: ") + to_patch);
    int quote_posn = contents.find("\"", ver_posn);
    if (quote_posn < 0) return true;  // malformed assembly we will not touch.
//LOG(astring("found quote: ") + to_patch);
    int second_quote_posn = contents.find("\"", quote_posn + 1);
    if (second_quote_posn < 0) return true;  // more malformage.
//LOG(astring("found quote: ") + to_patch);
    contents.zap(quote_posn + 1, second_quote_posn - 1);
    contents.insert(quote_posn + 1, ver_string);
*/

//LOG(astring("writing new output file: ") + to_patch);
    modfile.seek(0);
    modfile.write(contents);
    modfile.truncate();  // chop off anything left from previous versions.
    if (do_logging) {
      // let the people know about this...
      filename dirbase = filename(modfile.name()).dirname().basename();
      filename just_base = filename(modfile.name()).basename();
      program_wide_logger::get().log(astring("    patching: ") + dirbase
          + "/" + just_base, basis::ALWAYS_PRINT);
    }
  }

  return true;
}

bool version_ini::one_stop_version_stamp(const astring &path,
    const astring &source_version, bool do_logging)
{
  astring path_name = path;
  if (path_name.equal_to("."))
    path_name = application_configuration::current_directory();

  // load the version record in from the ini file and cache it.
  version_ini source(path_name);
  source.get_record();

  if (source_version.t()) {
    // get the version structure from the passed file.
    version_ini main_version(source_version);
    version version_to_use = main_version.get_version();

    // stuff the version from the main source into this particular file.
    source.set_version(version_to_use, false);

    // stuff the other volatile records from the main version.
    version_record main = main_version.get_record();
    source.access_record().company_name = main.company_name;
    source.access_record().web_address = main.web_address;
    source.access_record().copyright = main.copyright;
    source.access_record().trademarks = main.trademarks;
    source.access_record().product_name = main.product_name;

    source.access_record().internal_name.replace("$product_name",
        source.get_record().product_name);
  }

  if (do_logging) {
    // report the current state.
   program_wide_logger::get().log(source.get_record().internal_name + " version "
       + source.get_version().text_form() + ".", ALWAYS_PRINT);
  }

  version_ini verini(path_name);
  verini.set_record(source.get_record(), false);

//  LOG(a_sprintf("The file \"%s\" contains this version information:",
//      path_name.s()));
//  LOG(verini.get_record().text_form());

  if (!verini.write_rc(verini.get_record())) {
    critical_events::alert_message(a_sprintf("Could not write the RC file in \"%s\".",
        filename(path_name).basename().raw().s()));
    return false;
  }

  if (verini.library() && !verini.write_code(verini.get_record())) {
    critical_events::alert_message(astring("Could not write the C++ header file for "
        "the directory \"")
        + filename(path_name).basename() + astring("\".\n"));
    return false;
  }

  if (!verini.write_assembly(verini.get_record(), do_logging)) {
    critical_events::alert_message(astring("Could not write the Assembly info file for "
        "the directory \"")
        + filename(path_name).basename() + astring("\".\n"));
    return false;
  }

  return true;
}

} //namespace.
