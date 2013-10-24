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

#include <application/hoople_main.h>
#include <basis/byte_array.h>
#include <mathematics/chaos.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <basis/mutex.h>
#include <structures/amorph.h>
#include <structures/static_memory_gremlin.h>
#include <loggers/console_logger.h>
#include <processes/ethread.h>
#include <processes/safe_roller.h>
#include <timely/time_control.h>
#include <timely/time_stamp.h>
#include <octopus/entity_data_bin.h>
#include <octopus/entity_defs.h>
#include <octopus/unhandled_request.h>
#include <application/application_shell.h>
#include <configuration/application_configuration.h>
#include <textual/string_manipulation.h>

#include <stdio.h>

#ifdef __WIN32__
  #include <process.h>
#endif

using namespace application;
using namespace loggers;
using namespace octopi;
using namespace processes;
using namespace timely;

// global constants...

// how much data is the entity data bin allowed to hold at one time.
const int MAXIMUM_DATA_PER_ENTITY = 1 * KILOBYTE;
//tiny limit to test having too much data.

// controls the timing of the thread that adds items.
const int MIN_ADDER_THREAD_PAUSE = 3;
const int MAX_ADDER_THREAD_PAUSE = 20;

// controls the timing of the item deleting thread.
const int MIN_WHACKER_THREAD_PAUSE = 8;
const int MAX_WHACKER_THREAD_PAUSE = 70;

// bound the randomly chosen pause time for the cleanup thread.
const int MIN_TIDIER_THREAD_PAUSE = 60;
const int MAX_TIDIER_THREAD_PAUSE = 500;

// monk is kept asleep most of the time or he'd be trashing
// all our data too frequently.
const int MIN_MONK_THREAD_PAUSE = 2 * MINUTE_ms;
const int MAX_MONK_THREAD_PAUSE = 4 * MINUTE_ms;

// the range of new items added whenever the creator thread is hit.
const int MINIMUM_ITEMS_ADDED = 1;
const int MAXIMUM_ITEMS_ADDED = 20;

const int DEFAULT_THREADS = 90;
  // the number of threads we create by default.

const int DEFAULT_RUN_TIME = 80 * MINUTE_ms;
//2 * MINUTE_ms;
  // the length of time to run the program.

const int DATA_DECAY_TIME = 1 * MINUTE_ms;
  // how long we retain unclaimed data.

const int MONKS_CLEANING_TIME = 10 * SECOND_ms;
  // a very short duration for data to live.

#define LOG(to_print) printf("%s\n", (char *)astring(to_print).s());
//CLASS_EMERGENCY_LOG(program_wide_logger().get(), to_print)
  // our macro for logging with a timestamp.

// global objects...

chaos _rando;  // our randomizer.

// replace app_shell version with local randomizer, so all the static
// functions can employ it also.
#define randomizer() _rando

entity_data_bin binger(MAXIMUM_DATA_PER_ENTITY);

octopus_request_id create_request_id()
{
  // test the basic filling of the values in an entity.
  octopus_request_id req_id;
  if (randomizer().inclusive(1, 100) < 25) {
    // some of the time we make a totally random entity id.
    int sequencer = randomizer().inclusive(1, MAXINT - 10);
    int add_in = randomizer().inclusive(0, MAXINT - 10);
    int process_id = randomizer().inclusive(0, MAXINT - 10);
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
  req_id._request_num = randomizer().inclusive(1, MAXINT - 10);
  return req_id;
}

// this thread creates new items for the entity data bin.
class ballot_box_stuffer : public ethread
{
public:
  ballot_box_stuffer() : ethread(0) {
    FUNCDEF("constructor");
    LOG("+creator");
  }

  virtual ~ballot_box_stuffer() {
    FUNCDEF("destructor");
    LOG("~creator");
  }

  DEFINE_CLASS_NAME("ballot_box_stuffer");

  void perform_activity(void *formal(data)) {
    FUNCDEF("perform_activity");
    while (!should_stop()) {
      // add a new item to the cache.
      int how_many = randomizer().inclusive(MINIMUM_ITEMS_ADDED,
          MAXIMUM_ITEMS_ADDED);
      for (int i = 0; i < how_many; i++) {
        string_array random_strings;
        int string_count = randomizer().inclusive(1, 10);
        // we create a random classifier, just to use up some space.
        for (int q = 0; q < string_count; q++) {
          random_strings += string_manipulation::make_random_name();
        }
        unhandled_request *newbert = new unhandled_request(create_request_id(),
            random_strings);
        binger.add_item(newbert, create_request_id());
      }
      // snooze.
      int sleepy_time = randomizer().inclusive(MIN_ADDER_THREAD_PAUSE,
          MAX_ADDER_THREAD_PAUSE);
      time_control::sleep_ms(sleepy_time);
    }
  }

};

// this thread eliminates entries in the ballot box.
class vote_destroyer : public ethread
{
public:
  vote_destroyer() : ethread(0) {
    FUNCDEF("constructor");
    LOG("+destroyer");
  }

  virtual ~vote_destroyer() {
    FUNCDEF("destructor");
    LOG("~destroyer");
  }

  DEFINE_CLASS_NAME("vote_destroyer");

  void perform_activity(void *formal(data)) {
    FUNCDEF("perform_activity");
    while (!should_stop()) {
      // snag any old item and drop it on the floor.
      octopus_request_id id;
      infoton *found = binger.acquire_for_any(id);
      WHACK(found);
      // snooze.
      int sleepy_time = randomizer().inclusive(MIN_WHACKER_THREAD_PAUSE,
          MAX_WHACKER_THREAD_PAUSE);
      time_control::sleep_ms(sleepy_time);
    }
  }
};

// this class makes sure the deadwood is cleaned out of the entity bin.
class obsessive_compulsive : public ethread
{
public:
  obsessive_compulsive() : ethread(0) {
    FUNCDEF("constructor");
    LOG("+cleaner");
  }

  virtual ~obsessive_compulsive() {
    FUNCDEF("destructor");
    LOG("~cleaner");
  }

  DEFINE_CLASS_NAME("obsessive_compulsive");

  void perform_activity(void *formal(data)) {
    FUNCDEF("perform_activity");
    while (!should_stop()) {
      // make sure there's nothing rotting too long.
      binger.clean_out_deadwood(DATA_DECAY_TIME);
      // snooze.
      int sleepy_time = randomizer().inclusive(MIN_TIDIER_THREAD_PAUSE,
          MAX_TIDIER_THREAD_PAUSE);
      time_control::sleep_ms(sleepy_time);
    }
  }
};

// this thread will destroy all data in the bins while cleaning furiously.
class monk_the_detective : public ethread
{
public:
  monk_the_detective() : ethread(0) {
    FUNCDEF("constructor");
    LOG("+monk");
  }

  virtual ~monk_the_detective() {
    FUNCDEF("destructor");
    LOG("~monk");
  }

  DEFINE_CLASS_NAME("monk_the_detective");

  void perform_activity(void *formal(data)) {
    FUNCDEF("perform_activity");
    while (!should_stop()) {
      // one activation of monk has devastating consequences.  we empty out
      // the data one item at a time until we see no data at all.  after
      // cleaning each item, we ensure that the deadwood is cleaned out.
      binger._ent_lock->lock();
LOG(a_sprintf("monk sees %d items.", binger.items_held()));
      while (binger.items_held()) {
        // grab one instance of any item in the bin.
        octopus_request_id id;
        infoton *found = binger.acquire_for_any(id);
        WHACK(found);
        // also clean out things a lot faster than normal.  
        binger.clean_out_deadwood(MONKS_CLEANING_TIME);
      }
      binger._ent_lock->unlock();
LOG(a_sprintf("after a little cleaning, monk sees %d items.", binger.items_held()));
      // snooze.
      int sleepy_time = randomizer().inclusive(MIN_MONK_THREAD_PAUSE,
          MAX_MONK_THREAD_PAUSE);
      time_control::sleep_ms(sleepy_time);
    }
  }
};

//////////////

class test_entity_data_bin_threaded : public application_shell
{
public:
  test_entity_data_bin_threaded() : application_shell(class_name()) {}

  DEFINE_CLASS_NAME("test_entity_data_bin_threaded");

  int execute();
};

int test_entity_data_bin_threaded::execute()
{
  FUNCDEF("execute");

  amorph<ethread> thread_list;

  for (int i = 0; i < DEFAULT_THREADS; i++) {
    ethread *t = NIL;
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
    thread_list[thread_list.elements() - 1]->start(NIL);
  }

  time_stamp when_to_leave(DEFAULT_RUN_TIME);
  while (when_to_leave > time_stamp()) {
    time_control::sleep_ms(100);
  }

//  LOG("now cancelling all threads....");

//  for (int j = 0; j < thread_list.elements(); j++) thread_list[j]->cancel();

//  LOG("now stopping all threads....");

//  for (int k = 0; k < thread_list.elements(); k++) thread_list[k]->stop();

//  LOG("resetting thread list....");

  thread_list.reset();  // should whack all threads.

  LOG("done exiting from all threads....");

//report the results:
// how many objects created.
// how many got destroyed.
// how many evaporated due to timeout.


  guards::alert_message("t_bin_threaded:: works for all functions tested.");
  return 0;
}

HOOPLE_MAIN(test_entity_data_bin_threaded, )

