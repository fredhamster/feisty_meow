/*****************************************************************************\
*                                                                             *
*  Name   : parser_bits                                                       *
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

#include "parser_bits.h"

#include <basis/astring.h>
#include <basis/environment.h>
#include <basis/functions.h>

#include <ctype.h>
#include <stdio.h>

using namespace basis;

#undef LOG
#define LOG(prf) printf("%s\n", basis::astring(prf).s())

namespace textual {

parser_bits::line_ending parser_bits::platform_eol()
{
#ifdef __UNIX__
  // obviously a unix OS, unless someone's playing games with us.
  return LF_AT_END;
#elif defined(__WIN32__)
  // smells like DOS.
  return CRLF_AT_END;
#else
  // pick the unix default if we can't tell.
  return LF_AT_END;
#endif
}

const char *parser_bits::eol_to_chars(line_ending end)
{
  static const char *CRLF_AT_END_STRING = "\r\n";
  static const char *LF_AT_END_STRING = "\n";
  static const char *NO_ENDING_STRING = "";

  switch (end) {
    case CRLF_AT_END: return CRLF_AT_END_STRING;
    case NO_ENDING: return NO_ENDING_STRING;
    case LF_AT_END:  // fall-through to default.
    default: return LF_AT_END_STRING;
  }
}

const char *parser_bits::platform_eol_to_chars()
{ return eol_to_chars(platform_eol()); }

bool parser_bits::is_printable_ascii(char to_check)
{ return (to_check >= 32) && (to_check <= 126); }

bool parser_bits::white_space_no_cr(char to_check)
{ return (to_check == ' ') || (to_check == '\t'); }

bool parser_bits::is_eol(char to_check)
{ return (to_check == '\n') || (to_check == '\r'); }

bool parser_bits::white_space(char to_check)
{ return white_space_no_cr(to_check) || is_eol(to_check); }

void parser_bits::translate_CR_for_platform(astring &to_translate)
{
  line_ending plat_eol = platform_eol();
  bool last_was_lf = false;
  for (int i = 0; i <= to_translate.end(); i++) {
    if (to_translate[i] == '\r') {
      if (last_was_lf) continue;  // ignore two in a row.
      last_was_lf = true;
    } else if (to_translate[i] == '\n') {
      if (last_was_lf) {
        if (plat_eol != CRLF_AT_END) {
          // fix it, since there was not supposed to be an LF.
          to_translate.zap(i - 1, i - 1);
          i--;
        }
      } else {
        if (plat_eol == CRLF_AT_END) {
          // fix it, since we're missing an LF that we want.
          to_translate.insert(i, "\r");
          i++;
        }
      }
      last_was_lf = false;
    } else {
      // not the two power characters.
      last_was_lf = false;
    }
  }
}

bool parser_bits::is_hexadecimal(char look_at)
{
  return range_check(look_at, 'a', 'f')
      || range_check(look_at, 'A', 'F')
      || range_check(look_at, '0', '9');
}

bool parser_bits::is_hexadecimal(const char *look_at, int len)
{
  for (int i = 0; i < len; i++)
    if (!is_hexadecimal(look_at[i])) return false;
  return true;
}

bool parser_bits::is_hexadecimal(const astring &look_at, int len)
{ return is_hexadecimal(look_at.observe(), len); }

bool parser_bits::is_alphanumeric(char look_at)
{
  return range_check(look_at, 'a', 'z')
      || range_check(look_at, 'A', 'Z')
      || range_check(look_at, '0', '9');
}

bool parser_bits::is_alphanumeric(const char *look_at, int len)
{
  for (int i = 0; i < len; i++)
    if (!is_alphanumeric(look_at[i])) return false;
  return true;
}

bool parser_bits::is_alphanumeric(const astring &look_at, int len)
{ return is_alphanumeric(look_at.observe(), len); }

bool parser_bits::is_identifier(char look_at)
{
  return range_check(look_at, 'a', 'z')
      || range_check(look_at, 'A', 'Z')
      || range_check(look_at, '0', '9')
      || (look_at == '_');
}

bool parser_bits::is_identifier(const char *look_at, int len)
{
  if (is_numeric(look_at[0])) return false;
  for (int i = 0; i < len; i++)
    if (!is_identifier(look_at[i])) return false;
  return true;
}

bool parser_bits::is_identifier(const astring &look_at, int len)
{ return is_identifier(look_at.observe(), len); }

bool parser_bits::is_numeric(char look_at)
{
  return range_check(look_at, '0', '9') || (look_at == '-');
}

bool parser_bits::is_numeric(const char *look_at, int len)
{
  for (int i = 0; i < len; i++) {
    if (!is_numeric(look_at[i])) return false;
    if ( (i > 0) && (look_at[i] == '-') ) return false;
  }
  return true;
}

bool parser_bits::is_numeric(const astring &look_at, int len)
{ return is_numeric(look_at.observe(), len); }

astring parser_bits::substitute_env_vars(const astring &to_process,
    bool leave_unknown)
{
  astring editing = to_process;

//LOG(astring("input to subst env: ") + to_process);

  int indy;  // index of the dollar sign in the string.
  while (true) {
    indy = editing.find('$');
    if (negative(indy)) break;  // all done.
    int q;
    for (q = indy + 1; q < editing.length(); q++) {
      if (!parser_bits::is_identifier(editing[q]))
        break;  // done getting variable name.
    }
    if (q != indy + 1) {
      // we caught something in our environment variable trap...
      astring var_name = editing.substring(indy + 1, q - 1);
//LOG(astring("var name ") + var_name);
      astring value_found = environment::get(var_name);
//LOG(astring("val found ") + value_found);
      if (value_found.t()) {
        editing.zap(indy, q - 1);
        editing.insert(indy, value_found);
      } else {
        if (leave_unknown) {
          // that lookup failed.  let's mark it.
          editing[indy] = '?';
            // simple replacement, shows variables that failed.
        } else {
          // replace it with blankness.
          editing.zap(indy, q - 1);
        }
      }
    } else {
      // well, we didn't see a valid variable name, but we don't want to leave
      // the dollar sign in there.
      editing[indy] = '!';  // simple replacement, marks where syntax is bad.
    }

  }

//LOG(astring("output from subst env: ") + editing);

  return editing;
}

} //namespace.

