#ifndef COMMAND_LINE_CLASS
#define COMMAND_LINE_CLASS

/*****************************************************************************\
*                                                                             *
* Name   : command_line                                                       *
* Author : Chris Koeritz                                                      *
*                                                                             *
*******************************************************************************
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/contracts.h>
#include <filesystem/filename.h>

namespace application {

//! This class parses the command line passed to the main() function.
/*!
  The main function might be called WinMain or be implemented by CWinApp in MS-Windows, but all
  of these types of applications will still have flags passed to them.  This class constructs a
  list of parameters from the command line provided by the OS.  A parameter is either a command
  flag or a value string.  Flag characters take the form of "-l" or "-Q" or "-als" (which has
  three flags).  Value strings are other strings found on the command line that do not start with
  the separator character (usually '-').  Flag characters are all broken up into separate entries.
  A special kind of flag uses a double separator to allow multiple character flag names (e.g.,
  the string "--follow" where the flag name is "follow"), as in GNU software.
  This class knows about the convention of a paramter of just '--' being used to
  indicate that the rest of the line is all normal parameters and has no intentional flags
  in them.  This allows passing of character strings that would otherwise be misinterpreted
  as flags rather than literal input.

(not implemented)
  A special feature is provided to allow parsing of flag characters followed
  directly by the value (as in "-fbroiler.h", where the flag character is 'f'
  and the value is "broiler.h".
(not implemented)
*/

// forward declarations.
class internal_cmd_line_array_of_parms;

class command_parameter : public virtual basis::root_object
{
public:
  enum parameter_types { VALUE, CHAR_FLAG, STRING_FLAG, BOGUS_ITEM };

  command_parameter(parameter_types type = BOGUS_ITEM);
    //!< default constructor initializes to mostly blank state.

  command_parameter(parameter_types type, const basis::astring &text);
    //!< constructs a parameter of "type" where the value is "text".
    /*!< if the "type" is CHAR_FLAG, then this should be a string of
    one character.  for STRING_FLAG, the length is arbitrary. */

  command_parameter(const command_parameter &to_copy);

  ~command_parameter();

  DEFINE_CLASS_NAME("command_parameter");

  command_parameter &operator =(const command_parameter &to_copy);

  parameter_types type() const { return _type; }
    //!< observes the type of the parameter.
  void type(parameter_types new_type) { _type = new_type; }
    //!< modifies the type of the parameter.

  const basis::astring &text() const;
    //!< observes the string contents.
  void text(const basis::astring &new_text);
    //!< modifies the string contents.

private:
  parameter_types _type;
  basis::astring *_text;
};

//////////////

class command_line
{
public:
  command_line(int argc, char *argv[]);
    //!< takes command line parameters in the form of "argc" and "argv".
    /*!< this is suitable for most C++ main programs.  the first "argv" string (element zero) is
    ignored because it is assumed that it is the program name.  that means that the array of
    command parameters here will be (argc - 1) in length, and that index zero of our array has
    the first "real" parameter that was passed to the program (i.e., not it's name).
    note that the unaltered command parameters of argc and argv become available in the global
    variables _global_argc and _global_argv. */

  command_line(const basis::astring &to_parse);
    //!< takes a string form of the command line.
    /*!< this is the form rendered by GetCommandLine() in Win32.  on certain
    win32 platforms, this may not return a full path for the program_name()
    method.  this uses the separate_command_line() method to pick out the
    relevant pieces and supports embedded, escaped quotes. */

  virtual ~command_line();

  DEFINE_CLASS_NAME("command_line");

  filesystem::filename program_name() const;
    //!< Returns the program name found in the command line.

  static void separate_command_line(const basis::astring &cmd_line, basis::astring &app,
          basis::astring &parms);
    //!< breaks apart a command line in "cmd_line" into "app" and "parms".
    /*!< when given a full command line, where the application to run is the
    first chunk and its parameters (if any) are subsequent chunks, this will
    store the application name in "app" and the rest of the parameters in
    "parms".  this expects any paths in the "cmd_line" that contain spaces
    to be surrounded by quotes.  if there are any quote characters that are
    escaped, they are considered to be embedded in the parameter string; they
    will not be considered as matching any pending closing quotes. */

  int entries() const;
    //!< Returns the number of fields found on the command line.
    /*!< This does not include the program name found; that's only
    accessible through the program_name() method. */

  const command_parameter &get(int field) const;
    //!< Returns the parameter at the "field" specified.
    /*!< The "field" ranges from zero through "entries() - 1" inclusive.  if
    an invalid index is used, then the type will be BOGUS_ITEM. */

  bool zap(int field);
    //!< eats the entry at position "field".
    /*!< this is useful for cleaning out entries that have already been dealt
    with. */

  // note: in the following, if "case_sense" is true, then the searches are
  //       case-sensitive.  otherwise, case of the flags is not a concern.
  //       the returned values always retain the original case.

  bool find(char option_character, int &index, bool case_sense = true) const;
    //!< Returns true if the "option_character" is found in the parameters.
    /*!< The search starts at the "index" specified, and if the item is found,
    its location is returned in "index" and the function returns true.
    Otherwise false is returned and the "index" is not modified. */
  bool find(const basis::astring &option_string, int &index,
          bool case_sense = true) const;
    //!< Returns true if the "option_string" is found in the parameters.

  bool get_value(char option_character, basis::astring &value,
          bool case_sense = true) const;
    //!< retrieves the "value" found for the option flag specified.
    /*!< this is useful for command lines with standard spacing.  for example,
    if the command line is "-Q query.bop --Linkage plorgs.txt", then this
    function would return "query.bop" for a search on 'Q' and the find()
    method below would return "plorgs.txt" for the string flag search on
    "Linkage". */
  bool get_value(const basis::astring &option_string, basis::astring &value,
          bool case_sense = true) const;
    //!< retrieves the "value" found for the "option_string" specified.

//is this useful?  it's kind of like what we need for special flags (like
//  -fgob.h, where gob.h is a value parameter) but needs to terminate
//differently for that to work.
  basis::astring gather(int &index) const;
    //!< coalesces parameters together until the next option flag.
    /*!< Returns a string constructed from the concatenation of the strings
    for the parameters at all indices in the list starting at "index" until
    an option character is found.  Note that this means an empty string
    will be returned if the parameter at "index" has an option character,
    or if "index" is greater than or equal to "elements()".
    After gather, "index" is set to the last location included in the
    string.  "index" is set to the last index in the list if "index" was
    past the end to begin with or if strings are gathered up to the last
    index.  otherwise, "index" is unchanged if nothing was gathered. */

  basis::astring text_form() const;
    //!< returns a string with all the information we have for the command line.

  static structures::string_array get_command_line();
    //!< returns the command line passed to the program as a list of strings.
    /*!< the string at index zero is the program name.  this is just a useful
    helper function and is not normally needed by users of the command_line
    object. */

private:
  internal_cmd_line_array_of_parms *_implementation;  //!< held parameters.
  filesystem::filename *_program_name;  //!< the name of this program.

  void parse_string_array(const structures::string_array &to_parse);
    //!< pulls all the strings in "to_parse" into the command_parameter list.

  // forbidden:
  command_line(const command_line &to_copy);
  command_line &operator =(const command_line &to_copy);

  static const command_parameter &cmdline_blank_parm();
};

//////////////

// this declares a program-wide command-line argument storage area.

extern int _global_argc;
extern char **_global_argv;
//! this macro allocates space for the command-line storage areas.
#define DEFINE_ARGC_AND_ARGV int _global_argc = 0; char **_global_argv = NIL

//! this macro assigns our command-line parameters for this program.
#define SET_ARGC_ARGV(argc, argv) { \
  application::_global_argc = argc; \
  application::_global_argv = argv; \
}

} //namespace.

#endif

