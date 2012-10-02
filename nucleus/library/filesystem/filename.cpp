/*****************************************************************************\
*                                                                             *
*  Name   : filename                                                          *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1993-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

// implementation note: the filename is kept canonicalized.  any constructor
// or assignment operator should ensure this (except the blank constructor).

#include "filename.h"

#include <basis/byte_array.h>
#include <basis/functions.h>
#include <textual/parser_bits.h>

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef __UNIX__
  #include <unistd.h>
#endif
#ifdef __WIN32__
  #include <io.h>
#endif

#undef LOG
#define LOG(to_print) printf("%s::%s: %s\n", static_class_name(), func, astring(to_print).s())

using namespace basis;
using namespace structures;

class status_info : public stat
{
};

namespace filesystem {

#if defined(__WIN32__) || defined(__VMS__)
  const char DEFAULT_SEPARATOR = '\\';
#elif defined(__UNIX__)
  const char DEFAULT_SEPARATOR = '/';
#else
  #error "We have no idea what the default path separator is."
#endif

const char *NO_PARENT_DEFAULT = ".";
  // used when no directory name can be popped off.

filename::filename()
: astring(),
  _had_directory(false)
{}

filename::filename(const astring &name)
: astring(name),
  _had_directory(true)
{ canonicalize(); }

filename::filename(const astring &directory, const astring &name_of_file)
: astring(directory),
  _had_directory(true)
{
  // if the directory is empty, use the current directory.
  if (!directory) {
    *this = astring(NO_PARENT_DEFAULT);
    _had_directory = false;
  }
  // check for a slash on the end of the directory.  add one if there is none
  // currently.
  bool add_slash = false;
  if ( (directory[directory.end()] != '\\')
       && (directory[directory.end()] != '/') ) add_slash = true;
  if (add_slash) *this += DEFAULT_SEPARATOR;
  *this += name_of_file;
  canonicalize();
}

filename::filename(const filename &to_copy)
: astring(to_copy),
  _had_directory(to_copy._had_directory)
{ canonicalize(); }

filename::~filename() {}

astring filename::default_separator() { return astring(DEFAULT_SEPARATOR, 1); }

astring &filename::raw() { return *this; }

const astring &filename::raw() const { return *this; }

bool filename::good() const { return exists(); }

bool filename::unlink() const { return ::unlink(observe()) == 0; }

astring filename::null_device()
{
#ifdef __WIN32__
  return "null:";
#else
  return "/dev/null";
#endif
}

bool filename::separator(char is_it)
{ return (is_it == pc_separator) || (is_it == unix_separator); }

filename &filename::operator = (const filename &to_copy)
{
  if (this == &to_copy) return *this;
  (astring &)(*this) = to_copy;
  _had_directory = to_copy._had_directory;
  return *this;
}

filename &filename::operator = (const astring &to_copy)
{
  _had_directory = true;
  if (this == &to_copy) return *this;
  (astring &)(*this) = to_copy;
  canonicalize();
  return *this;
}

astring filename::pop()
{
  astring to_return = basename();
  filename parent_dir = parent();
  if (parent_dir.raw().equal_to(NO_PARENT_DEFAULT)) {
    // we haven't gone anywhere.
    return "";  // signal that nothing was removed.
  }
  *this = parent_dir;
  return to_return;
}

filename filename::parent() const { return dirname(); }

void filename::push(const astring &to_push)
{
  *this = filename(*this, to_push);
}

void filename::canonicalize()
{
  FUNCDEF("canonicalize");
  // turn all the non-default separators into the default.
  bool found_sep = false;
  for (int j = 0; j < length(); j++) {
    if (separator(get(j))) {
      found_sep = true;
      put(j, DEFAULT_SEPARATOR);
    }
  }

  // if there wasn't a single directory separator, then they must not have
  // specified any directory name for this filename (although it could itself
  // be a directory).
  if (!found_sep) _had_directory = false;

  // remove all occurrences of double separators except for the first
  // double set, which could be a UNC filename.  that's why the index below
  // starts at one rather than zero.
  bool saw_sep = false;
  for (int i = 1; i < length(); i++) {
    if (separator(get(i))) {
      if (saw_sep) {
        zap(i, i);
          // two in a row is no good, except for the first two.
        i--;  // skip back one and try again.
        continue;
      }
      saw_sep = true;
    } else saw_sep = false;
  }

#ifdef __WIN32__
  // on windows, we want to translate away from any cygwin or msys format into a more palatable
  // version that the rest of windows understands.
  // first, cygwin...
  const astring CYGDRIVE_PATH = astring(astring(DEFAULT_SEPARATOR, 1) + "cygdrive"
      + astring(DEFAULT_SEPARATOR, 1));
  // must be at least as long as the string we're looking for, plus a drive letter afterwards.
  if ( (length() > CYGDRIVE_PATH.length() + 1) && begins(CYGDRIVE_PATH) ) {
    zap(0, CYGDRIVE_PATH.length() - 1);  // whack the cygdrive portion plus two slashes.
    insert(1, ":");  // add a colon after the imputed drive letter.
LOG(astring("turned cygdrive string into: ") + *this);
  }
  // now we convert msys...
  if ( (length() >= 2) && (get(0) == DEFAULT_SEPARATOR)
        && textual::parser_bits::is_alpha(get(1)) ) {
    // we seem reasonably sure now that this is a windows path hiding in msys format, but
    // the next character needs to be a slash (if there is a next character) for it to be
    // the windows drive form.  otherwise it could be /tmp, which would obviously not be
    // intended as a windows path.
    if ( (length() == 2) || (get(2) == DEFAULT_SEPARATOR) ) {
      // cool, this should be interpretable as an msys path, except for those wacky types
      // of folks that might use a top-level single character directory name.  we cannot
      // help them, because we have made a design decision to support msys-style paths.
      // note that this would only affect someone if they were referring to their directory on
      // the current windows partition (c:, d:, etc.) without providing the drive letter,
      // if they had that single character directory name (e.g., c:\x, d:\q, etc.) and even
      // then only on the near defunct windows platform.
      zap(0, 0);  // take off initial slash.
      insert(1, ":");  // add the obligatory colon.
LOG(astring("turned msys string into: ") + *this);
    }
  } 
#endif

  // we don't crop the last separator if the name's too small.  for msdos
  // names, that would be chopping a slash off the c:\ style name.
  if (length() > 3) {
    // zap any separators that are hiding on the end.
    const int last = end();
    if (separator(get(last))) zap(last, last);
  } else if ( (length() == 2) && (get(1) == ':') ) {
    // special case for dos drive names.  we turn it back into a valid
    // directory rather than leaving it as just "X:".  that form of the name
    // means something else under dos/windows.
    *this += astring(DEFAULT_SEPARATOR, 1);
  }
}

char filename::drive(bool interact_with_fs) const
{
  // first guess: if second letter's a colon, first letter's the drive.
  if (length() < 2)
    return '\0';
  if (get(1) == ':')
    return get(0);
  if (!interact_with_fs)
    return '\0';

  // otherwise, retrieve the file system's record for the file.
  status_info fill;
  if (!get_info(&fill))
    return '\0';
  return char('A' + fill.st_dev);
}

astring filename::extension() const
{
  astring base(basename().raw());
  int posn = base.find('.', base.end(), true);
  if (negative(posn))
    return "";
  return base.substring(posn + 1, base.length() - 1);
}

astring filename::rootname() const
{
  astring base(basename().raw());
  int posn = base.find('.', base.end(), true);
  if (negative(posn))
    return base;
  return base.substring(0, posn - 1);
}

bool filename::get_info(status_info *to_fill) const
{
  int ret = stat(observe(), to_fill);
  if (ret)
    return false;
  return true;
}

bool filename::is_directory() const
{
  status_info fill;
  if (!get_info(&fill))
    return false;
  return !!(fill.st_mode & S_IFDIR);
}

bool filename::is_writable() const
{
  status_info fill;
  if (!get_info(&fill))
    return false;
  return !!(fill.st_mode & S_IWRITE);
}

bool filename::is_readable() const
{
  status_info fill;
  if (!get_info(&fill))
    return false;
  return !!(fill.st_mode & S_IREAD);
}

bool filename::is_executable() const
{
  status_info fill;
  if (!get_info(&fill))
    return false;
  return !!(fill.st_mode & S_IEXEC);
}

bool filename::is_normal() const
{
  status_info fill;
  if (!get_info(&fill))
    return false;
  bool weird = S_ISCHR(fill.st_mode)
      || S_ISBLK(fill.st_mode)
      || S_ISFIFO(fill.st_mode)
      || S_ISSOCK(fill.st_mode);
  return !weird;
}

int filename::find_last_separator(const astring &look_at) const
{
  int last_sep = -1;
  int sep = 0;
  while (sep >= 0) {
    sep = look_at.find(DEFAULT_SEPARATOR, last_sep + 1);
    if (sep >= 0) last_sep = sep;
  }
  return last_sep;
}

filename filename::basename() const
{
  astring basename = *this;
  int last_sep = find_last_separator(basename);
  if (last_sep >= 0) basename.zap(0, last_sep);
  return basename;
}

filename filename::dirname() const
{
  astring dirname = *this;
  int last_sep = find_last_separator(dirname);
  // we don't accept ripping off the first slash.
  if (last_sep >= 1) {
    // we can rip the slash and suffix off to get the directory name.  however,
    // this might be in the form X: on windows.  if they want the slash to
    // remain, they can use the dirname that appends it.
    dirname.zap(last_sep, dirname.end());
  } else {
    if (get(0) == DEFAULT_SEPARATOR) {
      // handle when we're up at the top of the filesystem.  on unix, once
      // you hit the root, you can keep going up but you still remain at
      // the root.  similarly on windoze, if there's no drive name in there.
      dirname = astring(DEFAULT_SEPARATOR, 1);
    } else {
      // there's no slash at all in the filename any more.  we assume that
      // the directory is the current one, if no other information is
      // available.  this default is already used by some code.
      dirname = NO_PARENT_DEFAULT;
    }
  }
  return dirname;
}

astring filename::dirname(bool add_slash) const
{
  astring tempname = dirname().raw();
  if (add_slash) tempname += DEFAULT_SEPARATOR;
  return tempname;
}

bool filename::exists() const
{
  if (is_directory())
    return true;
  if (!length())
    return false;
  return is_readable();
///  byte_filer opened(observe(), "rb");
///  return opened.good();
}

bool filename::legal_character(char to_check)
{
  switch (to_check) {
    case ':': case ';':
    case '\\': case '/':
    case '*': case '?': case '$': case '&': case '|':
    case '\'': case '"': case '`':
    case '(': case ')':
    case '[': case ']':
    case '<': case '>':
    case '{': case '}':
      return false;
    default: return true;
  }
}

void filename::detooth_filename(astring &to_clean, char replacement)
{
  for (int i = 0; i < to_clean.length(); i++) {
    if (!legal_character(to_clean[i]))
      to_clean[i] = replacement;
  }
}

int filename::packed_size() const
{
  return PACKED_SIZE_INT32 + astring::packed_size();
}

void filename::pack(byte_array &packed_form) const
{
  attach(packed_form, int(_had_directory));
  astring::pack(packed_form);
}

bool filename::unpack(byte_array &packed_form)
{
  int temp;
  if (!detach(packed_form, temp))
    return false;
  _had_directory = temp;
  if (!astring::unpack(packed_form))
    return false;
  return true;
}

void filename::separate(bool &rooted, string_array &pieces) const
{
  pieces.reset();
  const astring &raw_form = raw();
  astring accumulator;  // holds the names we find.
  rooted = raw_form.length() && separator(raw_form[0]);
  for (int i = 0; i < raw_form.length(); i++) {
    if (separator(raw_form[i])) {
      // this is a separator character, so eat it and add the accumulated
      // string to the list.
      if (i && accumulator.length()) pieces += accumulator;
      // now reset our accumulated text.
      accumulator = astring::empty_string();
    } else {
      // not a separator, so just accumulate it.
      accumulator += raw_form[i];
    }
  }
  if (accumulator.length()) pieces += accumulator;
}

void filename::join(bool rooted, const string_array &pieces)
{
  astring constructed_name;  // we'll make a filename here.
  if (rooted) constructed_name += DEFAULT_SEPARATOR;
  for (int i = 0; i < pieces.length(); i++) {
    constructed_name += pieces[i];
    if (!i || (i != pieces.length() - 1))
      constructed_name += DEFAULT_SEPARATOR;
  }
  *this = constructed_name;
}

bool filename::base_compare_prefix(const filename &to_compare,
    string_array &first, string_array &second)
{
  bool first_rooted;
  separate(first_rooted, first);
  bool second_rooted;
  to_compare.separate(second_rooted, second);
  if (first_rooted != second_rooted) {
    return false;
  }
  // that case should never be allowed, since there are some bits missing
  // in the name to be compared.
  if (first.length() > second.length())
    return false;

  // compare each of the pieces.
  for (int i = 0; i < first.length(); i++) {
#if defined(__WIN32__) || defined(__VMS__)
    // case-insensitive compare.
    if (!first[i].iequals(second[i]))
      return false;
#else
    // case-sensitive compare.
    if (first[i] != second[i])
      return false;
#endif
  }
  return true;
}

bool filename::compare_prefix(const filename &to_compare, astring &sequel)
{
  sequel = astring::empty_string();  // clean our output parameter.
  string_array first;
  string_array second;
  if (!base_compare_prefix(to_compare, first, second))
    return false;

  // create the sequel string.
  int extra_strings = second.length() - first.length();
  for (int i = second.length() - extra_strings; i < second.length(); i++) {
    sequel += second[i];
    if (i != second.length() - 1) sequel += DEFAULT_SEPARATOR;
  }

  return true;
}

bool filename::compare_prefix(const filename &to_compare)
{
  string_array first;
  string_array second;
  return base_compare_prefix(to_compare, first, second);
}

bool filename::base_compare_suffix(const filename &to_compare,
    string_array &first, string_array &second)
{
  bool first_rooted;
  separate(first_rooted, first);
  bool second_rooted;
  to_compare.separate(second_rooted, second);
  // that case should never be allowed, since there are some bits missing
  // in the name to be compared.
  if (first.length() > second.length())
    return false;

  // compare each of the pieces.
  for (int i = first.length() - 1; i >= 0; i--) {
//clean up this computation; the difference in lengths is constant--use that.
    int distance_from_end = first.length() - 1 - i;
    int j = second.length() - 1 - distance_from_end;
#if defined(__WIN32__) || defined(__VMS__)
    // case-insensitive compare.
    if (!first[i].iequals(second[j]))
      return false;
#else
    // case-sensitive compare.
    if (first[i] != second[j])
      return false;
#endif
  }
  return true;
}

bool filename::compare_suffix(const filename &to_compare, astring &prequel)
{
  prequel = astring::empty_string();  // clean our output parameter.
  string_array first;
  string_array second;
  if (!base_compare_suffix(to_compare, first, second))
    return false;

  // create the prequel string.
  int extra_strings = second.length() - first.length();
  for (int i = 0; i < extra_strings; i++) {
    prequel += second[i];
    if (i != second.length() - 1) prequel += DEFAULT_SEPARATOR;
  }
  return true;
}

bool filename::compare_suffix(const filename &to_compare)
{
  string_array first;
  string_array second;
  return base_compare_suffix(to_compare, first, second);
}

bool filename::chmod(int write_mode, int owner_mode) const
{
  int chmod_value = 0;
#ifdef __UNIX__
  if (write_mode & ALLOW_READ) {
    if (owner_mode & USER_RIGHTS) chmod_value |= S_IRUSR;
    if (owner_mode & GROUP_RIGHTS) chmod_value |= S_IRGRP;
    if (owner_mode & OTHER_RIGHTS) chmod_value |= S_IROTH;
  }
  if (write_mode & ALLOW_WRITE) {
    if (owner_mode & USER_RIGHTS) chmod_value |= S_IWUSR;
    if (owner_mode & GROUP_RIGHTS) chmod_value |= S_IWGRP;
    if (owner_mode & OTHER_RIGHTS) chmod_value |= S_IWOTH;
  }
////  chmod_value = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
#elif defined(__WIN32__)
  if (write_mode & ALLOW_READ) {
    chmod_value |= _S_IREAD;
  }
  if (write_mode & ALLOW_WRITE) {
    chmod_value |= _S_IWRITE;
  }
#else
  #error unsupported OS type currently.
#endif
  int chmod_result = ::chmod(raw().s(), chmod_value);
  if (chmod_result) {
//    LOG(astring("there was a problem changing permissions on ") + raw());
    return false;
  }
  return true;
}

} //namespace.

