#ifndef BUILD_DEFAULTS_CLASS
#define BUILD_DEFAULTS_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : write_build_config                                                *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    This class creates a header file that will provide macros that govern    *
*  how the build is created under win32 using visual studio project files.    *
*  This file is not on other platforms, nor with the clam makefile system.    *
*                                                                             *
*******************************************************************************
* Copyright (c) 1995-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/application_shell.h>
#include <basis/astring.h>
#include <basis/enhance_cpp.h>
#include <basis/outcome.h>
#include <structures/set.h>
#include <structures/version_record.h>

class write_build_config : public application::application_shell
{
public:
  write_build_config();
  ~write_build_config();

  DEFINE_CLASS_NAME("write_build_config");

  int execute();
    //!< performs the main action of creating our build configuration header.

  const structures::string_set &exclusions();
    //!< returns the set of symbols that we will not include in the header.

  basis::outcome output_macro(const basis::astring &symbol, const basis::astring &value,
          basis::astring &accumulator);
    //!< sends a macro definition for "symbol" with "value" to "accumulator".

  basis::outcome output_decorated_macro(const basis::astring &symbol, const basis::astring &value,
          basis::astring &cfg_accumulator, basis::astring &ver_accumulator);
    //!< produces a new macro by adding a uniquifying string to "symbol".
    /*!< if the item is a version component, it will be output to the
    "ver_accumulator" but otherwise it goes to the "cfg_accumulator". */

  basis::outcome output_definition_macro(const basis::astring &embedded_value,
          basis::astring &accumulator);
    //!< parses a 'name=value' pair out of "embedded_value" and writes a macro.

  bool process_version_parts(const basis::astring &symbol, const basis::astring &value);
    //!< checks on "symbol" to see if it's a version component.  stores if so.
    /*!< if the string was a version component, then true is returned. */

  bool check_nesting(const basis::astring &to_check);
    //!< if "to_check" is a make conditional, the nesting level is adjusted.
    /*!< also if it's a conditional, true is returned, which means the line
    can be dropped. */

  bool write_output_file(const basis::astring &filename, const basis::astring &contents);
    //!< writes "contents" to "filename" if it differs from current contents.

private:
  basis::astring *_end_matter;  // stuff that isn't part of the real file.
  structures::version *_ver;  // accumulated when we see the right parts.
  int _nesting;  // how many levels of conditionals do we see?

  // not provided.
  write_build_config(const write_build_config &);
  write_build_config &operator =(const write_build_config &);
};

#endif

