/*****************************************************************************\
*                                                                             *
*  Name   : test_entity_data_bin_threaded                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2010-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/application_shell.h>
#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/mutex.h>
#include <configuration/application_configuration.h>
#include <loggers/console_logger.h>
#include <mathematics/chaos.h>
#include <octopus/entity_data_bin.h>
#include <octopus/entity_defs.h>
#include <octopus/unhandled_request.h>
#include <processes/ethread.h>
#include <processes/safe_roller.h>
#include <structures/amorph.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_array.h>
#include <textual/string_manipulation.h>
#include <timely/time_control.h>
#include <timely/time_stamp.h>
#include <unit_test/unit_base.h>

#include <stdio.h>

#ifdef __WIN32__
  #include <process.h>
#endif

using namespace application;
using namespace basis;
using namespace loggers;
using namespace mathematics;
using namespace octopi;
using namespace processes;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

//////////////

// synchronization for logged messages to avoid overwriting on the console.
SAFE_STATIC(mutex, __loggers_lock, )

// our macros for logging (with or without a timestamp).
#define LOG(s) { \
  auto_synchronizer critical_section(__loggers_lock()); \
  CLASS_EMERGENCY_LOG(program_wide_logger::get(), astring(s)); \
}

//////////////

// the base log feature just prints the text to the console with no carriage return or extra flair.
// it does count up how many characters have been printed though, and does an EOL when it seems like it would be reasonable (80 chars-ish).
// note that this code makes no attempt to worry about any other printing that's happening; it has a very egocentric view of what's on the
// terminal so far.
static int chars_printed = 0;
const int MAXIMUM_CHARS_PER_LINE = 108;
SAFE_STATIC(console_logger, ted, );
#define BASE_LOG(s) { \
  auto_synchronizer critical_section(__loggers_lock()); \
  /* we set the eol every time, since console_logger constructor doesn't currently provide. */ \
  ted().eol(parser_bits::NO_ENDING); \
  astring joe(s); \
  int len = joe.length(); \
  /* naive check for line length. */ \
  if (chars_printed + len > MAXIMUM_CHARS_PER_LINE) { \
    ted().log(astring("\n"), basis::ALWAYS_PRINT); \
    chars_printed = 0; \
  } \
  chars_printed += len; \
  ted().log(joe, basis::ALWAYS_PRINT); \
}
//hmmm: may want to make the line size selectable, if we keep some version of the above line handling code around.

//////////////

// protects our logging stream really, by keeping all the threads chomping at the bit rather
// than running right away.  when this flag switches to true, then *bam* they're off.
class bool_scared_ya : public root_object {
public:
  bool_scared_ya(bool init = false) : _value(init) {}
  bool_scared_ya &operator = (const bool_scared_ya &s1) { _value = s1._value; return *this; }
  bool_scared_ya &operator = (bool s1) { _value = s1; return *this; }
  virtual ~bool_scared_ya() {}
  operator bool() { return _value; }
  DEFINE_CLASS_NAME("bool_scared_ya");
private:
  bool _value;
};
SAFE_STATIC(bool_scared_ya, __threads_can_run_wild_and_free, (false));

//////////////

// global constants...

//const int DEFAULT_RUN_TIME = 80 * MINUTE_ms;
//const int DEFAULT_RUN_TIME = 2 * MINUTE_ms;
//const int DEFAULT_RUN_TIME = 28 * SECOND_ms;
const int DEFAULT_RUN_TIME = 4 * SECOND_ms;
  // the length of time to run the program.

// how much data is the entity data bin allowed to hold at one time.
const int MAXIMUM_DATA_PER_ENTITY = 5 * KILOBYTE;
//tiny limit to test having too much data.

// controls the timing of the thread that adds items.
const int MIN_ADDER_THREAD_PAUSE = 3;
const int MAX_ADDER_THREAD_PAUSE = 20;

// controls the timing of the item deleting thread.
// we currently have this biased to be slower than the adder, so things accumulate.
const int MIN_WHACKER_THREAD_PAUSE = 8;
const int MAX_WHACKER_THREAD_PAUSE = 70;

// bound the randomly chosen pause time for the cleanup thread.
const int MIN_TIDIER_THREAD_PAUSE = 60;
const int MAX_TIDIER_THREAD_PAUSE = 500;

// monk is kept asleep most of the time or he'd be trashing all our data too frequently.
const int MIN_MONK_THREAD_PAUSE = 14 * SECOND_ms;
const int MAX_MONK_THREAD_PAUSE = 28 * SECOND_ms;

// the range of new items added whenever the creator or destroyer threads are hit.
const int MINIMUM_ITEMS_HANDLED = 1;
const int MAXIMUM_ITEMS_HANDLED = 20;

const int DEFAULT_THREADS = 90;
  // the number of threads we create by default.

const int DATA_DECAY_TIME = 1 * MINUTE_ms;
  // how long we retain unclaimed data.

const int MONKS_CLEANING_TIME = 10 * SECOND_ms;
  // a very short duration for data to live.

//////////////

// global objects...

SAFE_STATIC(chaos, _rando, );
////chaos _rando;  // our randomizer.
/* replaces app_shell version with local static randomizer. */
#define randomizer() _rando()

entity_data_bin binger(MAXIMUM_DATA_PER_ENTITY);

//////////////

octopus_request_id create_request_id()
{
  // test the basic filling of the values in an entity.
  octopus_request_id req_id;
  if (randomizer().inclusive(1, 100) < 25) {
    // some of the time we make a totally random entity id.
    int sequencer = randomizer().inclusive(1, MAXINT32 - 10);
    int add_in = randomizer().inclusive(0, MAXINT32 - 10);
    int process_id = randomizer().inclusive(0, MAXINT32 - 10);
    req_id._entity = octopus_entity(string_manipulation::make_random_name(),
        process_id, sequencer, add_in);
  } else {
    // sometimes we use a less random identity.
    int sequencer = randomizer().inclusive(1, 3);
    int add_in = 12;
    int process_id = randomizer().inclusive(1, 4);
    req_id._entity = octopus_entity("boringentity",
        process_id, sequencer, add_in);
  }
  req_id._request_num = randomizer().inclusive(1, MAXINT32 - 10);
  return req_id;
}

//////////////

// this thread creates new items for the entity data bin.
// also known as the adder.
class ballot_box_stuffer : public ethread
{
public:
  ballot_box_stuffer() : ethread(MIN_ADDER_THREAD_PAUSE, ethread::TIGHT_INTERVAL) {
    FUNCDEF("constructor");
    LOG(">> new creator >>");
  }

  virtual ~ballot_box_stuffer() {
    FUNCDEF("destructor");
    LOG("<< creator exits <<");
  }

  DEFINE_CLASS_NAME("ballot_box_stuffer");

  void perform_activity(void *formal(data)) {
    FUNCDEF("perform_activity");
    if (!__threads_can_run_wild_and_free()) { sleep_time(MIN_ADDER_THREAD_PAUSE); return; }
    // add a new item to the cache.
    int how_many = randomizer().inclusive(MINIMUM_ITEMS_HANDLED,
        MAXIMUM_ITEMS_HANDLED);
    for (int i = 0; i < how_many; i++) {
      string_array random_strings;
      int string_count = randomizer().inclusive(1, 10);
      // we create a random classifier, just to use up some space.
      for (int q = 0; q < string_count; q++) {
        random_strings += string_manipulation::make_random_name();
      }
      // check exit sentry again just before adding.
      if (!__threads_can_run_wild_and_free()) { sleep_time(MIN_ADDER_THREAD_PAUSE); return; }
      unhandled_request *newbert = new unhandled_request(create_request_id(),
          random_strings);
      BASE_LOG("+");
      binger.add_item(newbert, create_request_id());
    }

    // snooze.
    int sleepy_time = randomizer().inclusive(MIN_ADDER_THREAD_PAUSE,
        MAX_ADDER_THREAD_PAUSE);
    time_control::sleep_ms(sleepy_time);
    // reset the thread's snooze timing.
    ethread::sleep_time(sleepy_time);
  }
};

//////////////

// this thread eliminates entries in the ballot box.
// also known as the whacker.
class vote_destroyer : public ethread
{
public:
  vote_destroyer() : ethread(MIN_WHACKER_THREAD_PAUSE, ethread::TIGHT_INTERVAL) {
    FUNCDEF("constructor");
    LOG(">> new destroyer >>");
  }

  virtual ~vote_destroyer() {
    FUNCDEF("destructor");
    LOG("<< destroyer exits <<");
  }

  DEFINE_CLASS_NAME("vote_destroyer");

  void perform_activity(void *formal(data)) {
    FUNCDEF("perform_activity");
    if (!__threads_can_run_wild_and_free()) { sleep_time(MIN_WHACKER_THREAD_PAUSE); return; }
    int how_many = randomizer().inclusive(MINIMUM_ITEMS_HANDLED,
        MAXIMUM_ITEMS_HANDLED);
    for (int i = 0; i < how_many; i++) {
      // check exit sentry again just before removing.
      if (!__threads_can_run_wild_and_free()) { sleep_time(MIN_WHACKER_THREAD_PAUSE); return; }
      // snag any old item and drop it on the floor.
      octopus_request_id id;
      infoton *found = binger.acquire_for_any(id);
      if (!found) break;  // nothing to whack there.
      BASE_LOG("-");
      WHACK(found);
    }
    // snooze.
    int sleepy_time = randomizer().inclusive(MIN_WHACKER_THREAD_PAUSE,
        MAX_WHACKER_THREAD_PAUSE);
    time_control::sleep_ms(sleepy_time);
    // re-schedule the thread.
    ethread::sleep_time(sleepy_time);
  }
};

//////////////

// this class makes sure the deadwood is cleaned out of the entity bin.
class obsessive_compulsive : public ethread
{
public:
  obsessive_compulsive() : ethread(MIN_TIDIER_THREAD_PAUSE, ethread::TIGHT_INTERVAL) {
    FUNCDEF("constructor");
    LOG(">> new cleaner >>");
  }

  virtual ~obsessive_compulsive() {
    FUNCDEF("destructor");
    LOG("<< cleaner exits <<");
  }

  DEFINE_CLASS_NAME("obsessive_compulsive");

  void perform_activity(void *formal(data)) {
    FUNCDEF("perform_activity");
    if (!__threads_can_run_wild_and_free()) { sleep_time(MIN_TIDIER_THREAD_PAUSE); return; }
    // make sure there's nothing rotting too long.
    binger.clean_out_deadwood(DATA_DECAY_TIME);
    // snooze.
    int sleepy_time = randomizer().inclusive(MIN_TIDIER_THREAD_PAUSE,
        MAX_TIDIER_THREAD_PAUSE);
    time_control::sleep_ms(sleepy_time);
    ethread::sleep_time(sleepy_time);
  }
};

//////////////

// this thread will destroy all data in the bins while cleaning furiously.
class monk_the_detective : public ethread
{
public:
  monk_the_detective() : ethread(MIN_MONK_THREAD_PAUSE, ethread::TIGHT_INTERVAL) {
    FUNCDEF("constructor");
    LOG(">> new monk >>");
  }

  virtual ~monk_the_detective() {
    FUNCDEF("destructor");
    LOG("<< monk exits <<");
  }

  DEFINE_CLASS_NAME("monk_the_detective");

  void perform_activity(void *formal(data)) {
    FUNCDEF("perform_activity");
    if (!__threads_can_run_wild_and_free()) { sleep_time(MIN_MONK_THREAD_PAUSE); return; }
    // one activation of monk has devastating consequences.  we empty out
    // the data one item at a time until we see no data at all.  after
    // cleaning each item, we ensure that the deadwood is cleaned out.
    auto_synchronizer l(binger.locker());
LOG(a_sprintf("monk sees %d items and will clean them all.", binger.items_held()));
    int check_count = 0;
    while (binger.items_held()) {
      // check exit sentry again just before obsessing over cleanliness.
      if (!__threads_can_run_wild_and_free()) { sleep_time(MIN_MONK_THREAD_PAUSE); return; }
      // grab one instance of any item in the bin.
      octopus_request_id id;
      infoton *found = binger.acquire_for_any(id);
      if (!found) break;  // nothing to see here.
      check_count++;
      BASE_LOG("-");
      WHACK(found);
      // also clean out things a lot faster than normal.  
      binger.clean_out_deadwood(MONKS_CLEANING_TIME);
//hmmm: interesting--deadwood cleaning above will possibly cause us not to clean out the number of items reported above for items_held().
    }
LOG(a_sprintf("monk manually cleaned %d items very carefully...", check_count));
LOG(a_sprintf("after this little light cleaning, monk sees %d items held.", binger.items_held()));
    // snooze.
    int sleepy_time = randomizer().inclusive(MIN_MONK_THREAD_PAUSE, MAX_MONK_THREAD_PAUSE);
    // reschedule the thread for the new snooze.  and note how we are not actually stuck waiting
    // for the whole sleep time, given how ethread works with timed threads.  an old implementation
    // actually slept uninterruptably for the whole snooze time, which was really off-putting and
    // rude.
    ethread::sleep_time(sleepy_time);
  }
};

//////////////

class test_entity_data_bin_threaded : public application_shell
{
public:
  test_entity_data_bin_threaded() : application_shell() {}

  DEFINE_CLASS_NAME("test_entity_data_bin_threaded");

  int execute();
};

int test_entity_data_bin_threaded::execute()
{
  FUNCDEF("execute");

  int duration = DEFAULT_RUN_TIME;
  if (application::_global_argc >= 2) {
    astring duration_string = application::_global_argv[1];
    if (duration_string.length()) {
      duration = duration_string.convert(DEFAULT_RUN_TIME);
      LOG(a_sprintf("user specified runtime duration of %d seconds.", duration));
      // convert from seconds to milliseconds.
      duration *= SECOND_ms;
    }
  }

  /* we could use a thread_cabinet here, but it's kind of funny that we're using a bare amorph and
  that its built-in reset() method is shutting down all the threads for us.  so leaving this as
  a nice example. */
  amorph<ethread> thread_list;

  for (int i = 0; i < DEFAULT_THREADS; i++) {
    ethread *t = NULL_POINTER;
    if (i == DEFAULT_THREADS - 1) {
      // last item gets special treatment; we reserve this space for monk.
      t = new monk_the_detective;
    } else if (i % 3 == 0) {
      t = new ballot_box_stuffer;
    } else if (i % 3 == 1) {
      t = new vote_destroyer;
    } else {  // i % 3 must = 2.
      t = new obsessive_compulsive;
    }
    thread_list.append(t);
    ethread *q = thread_list[thread_list.elements() - 1];
    if (q != t)
      deadly_error(class_name(), func, "amorph has incorrect pointer!");
    // start the thread we added.
    thread_list[thread_list.elements() - 1]->start(NULL_POINTER);
  }

  // set our sentinel variable to allow the threads to run now.
  __threads_can_run_wild_and_free() = true;

  time_stamp when_to_leave(duration);
  while (when_to_leave > time_stamp()) {
    time_control::sleep_ms(20);
  }

  __threads_can_run_wild_and_free() = false;

  /* we cancel all the threads first.  this gives them an opportunity to know 
  they should shut down, and they will all go about that at their own rate.  if we
  just killed the list with reset first, then the amorph would dutifully shut the
  threads down also, but it would do them sequentially which is way slower. */
  LOG("now cancelling all threads...");
  for (int j = 0; j < thread_list.elements(); j++) { thread_list[j]->cancel(); }
  LOG("now resetting thread list...");
  thread_list.reset();  // should whack all threads.
  LOG("...done exiting from all threads.");

//report the results:
// how many objects created.
// how many got destroyed.
// how many evaporated due to timeout.

  critical_events::alert_message(astring(class_name()) + ":: works for all functions tested.");
  return 0;
}

//////////////

HOOPLE_MAIN(test_entity_data_bin_threaded, )

