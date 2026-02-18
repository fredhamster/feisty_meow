/*****************************************************************************\
*                                                                             *
*  Name   : test_state_machine                                                *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Sets up a simple state machine for a pattern matcher.                    *
*                                                                             *
*******************************************************************************
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

// This simple state machine recognizes all strings that are of the form
// *.[cho], which means any string with either .o, .c, or .h on the end.

#include <application/application_shell.h>
#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <configuration/application_configuration.h>
#include <filesystem/byte_filer.h>
#include <loggers/console_logger.h>
#include <loggers/critical_events.h>
#include <loggers/program_wide_logger.h>
#include <processes/state_machine.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

#include <string.h>

using namespace application;
using namespace basis;
using namespace configuration;
//using namespace mathematics;
using namespace filesystem;
using namespace loggers;
using namespace processes;
//using namespace structures;
//using namespace textual;
using namespace timely;
using namespace unit_test;

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), astring(s))

//////////////

enum recognizer_states { GET_ANY = 1, GOT_DOT, GOT_CHO, RECOGNIZED, REJECTED };

//#undef LOG
//#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

class test_state_machine : public application_shell
{
public:
  test_state_machine();
  DEFINE_CLASS_NAME("test_state_machine");
  int execute();
  void setup_state_machine(transition_map &recog);
  void print_instructions();
};

astring state_text(recognizer_states to_show)
{
  switch (to_show) {
    case GET_ANY: return "get_any";
    case GOT_DOT: return "got_dot";
    case GOT_CHO: return "got_cho";
    case RECOGNIZED: return "recognized";
    case REJECTED: return "rejected";
    default: return "unknown!";
  }
}

#define TEST(action) \
  if (!action) \
    deadly_error(class_name(), "setup", a_sprintf("failed on %s", #action))

void test_state_machine::setup_state_machine(transition_map &recog)
{
  TEST(recog.add_state(GET_ANY));
  TEST(recog.add_state(GOT_DOT));
  TEST(recog.add_state(GOT_CHO));
  TEST(recog.add_state(RECOGNIZED));
  TEST(recog.add_state(REJECTED));

  TEST(recog.set_start(GET_ANY));

// bogus for test!
//TEST(recog.add_range_transition(GET_ANY, 29378, 0, 0));

  TEST(recog.add_range_transition(GET_ANY, REJECTED, 0, 0));
  TEST(recog.add_range_transition(GET_ANY, GET_ANY, 1, '.'-1));
  TEST(recog.add_range_transition(GET_ANY, GOT_DOT, '.', '.'));
  TEST(recog.add_range_transition(GET_ANY, GET_ANY, '.'+1, 255));

  // we wouldn't need so many cases if the update function made the
  // jump out of GOT_DOT when an unsatisfactory pulse is seen?
  TEST(recog.add_range_transition(GOT_DOT, REJECTED, '\0', '\0'));
  TEST(recog.add_range_transition(GOT_DOT, GET_ANY, 1, '.'-1));
  TEST(recog.add_range_transition(GOT_DOT, GOT_DOT, '.', '.'));
  TEST(recog.add_range_transition(GOT_DOT, GET_ANY, '.'+1, 'c'-1));
  TEST(recog.add_range_transition(GOT_DOT, GOT_CHO, 'c', 'c'));
  TEST(recog.add_range_transition(GOT_DOT, GET_ANY, 'c'+1, 'h'-1));
  TEST(recog.add_range_transition(GOT_DOT, GOT_CHO, 'h', 'h'));
  TEST(recog.add_range_transition(GOT_DOT, GET_ANY, 'h'+1, 'o'-1));
  TEST(recog.add_range_transition(GOT_DOT, GOT_CHO, 'o', 'o'));
  TEST(recog.add_range_transition(GOT_DOT, GET_ANY, 'o'+1, 255));
/// if update was doing it; TEST(recog.add_simple_transition(GOT_DOT, GET_ANY));

  TEST(recog.add_range_transition(GOT_CHO, RECOGNIZED, '\0', '\0'));
  TEST(recog.add_range_transition(GOT_CHO, GET_ANY, 1, '.'-1));
  TEST(recog.add_range_transition(GOT_CHO, GOT_DOT, '.', '.'));
  TEST(recog.add_range_transition(GOT_CHO, GET_ANY, '.'+1, 255));

  int hosed;
  if (recog.validate(hosed) != transition_map::OKAY)
    deadly_error(class_name(), "check_machine",
        astring(astring::SPRINTF, "invalid state_machine due to state %d", hosed).s());
}

test_state_machine::test_state_machine()
: application_shell()
{}

void test_state_machine::print_instructions()
{
  critical_events::alert_message("\
This program demonstrates the state machine class by constructing a machine\n\
that recognizes the regular expression *.[cho], which is all strings ending\n\
in a period followed by c, h, or o.  The first parameter is the name of a\n\
file to load strings from.\n");
}

int test_state_machine::execute()
{
  FUNCDEF("execute");
  transition_map recog;
  setup_state_machine(recog);

  astring filename;
  if (application::_global_argc < 2) {
    // automating for unit test with a simple input file.
    filename = "./input_data_state_machine.txt";
///    print_instructions();
///    return 1;
  } else {
    filename = astring(application::_global_argv[1]);
  }
///  int indy = filename.find(' ');
  if (!filename) {
    print_instructions();
    return 1;
  }
  ///filename.zap(indy, filename.end());

  byte_filer fil(filename.s(), "r");

//LOG(astring("filename is ") + filename.raw());

  while (!fil.eof()) {
    state_machine m;
    recog.reset(m);

    byte_array typed_string(901);
//need a getline function for byte filer...
    fil.getline(typed_string, 900);

    int len = int(strlen((char *)typed_string.access()));
    if (!len) continue;  // skip blank lines...

    int position = 0;
    while ( (m.current() != RECOGNIZED) && (m.current() != REJECTED) ) {
      recog.pulse(m, typed_string[position++]);
    }

    if (m.current() == RECOGNIZED) {
      LOG((char *)typed_string.access());
    } else if (m.current() == REJECTED) {
      LOG("rejected...");
    } else {
      LOG("unknown final state!");
    }
  }
  return 0;
}

//////////////

HOOPLE_MAIN(test_state_machine, )

