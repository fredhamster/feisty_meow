/*****************************************************************************\
*                                                                             *
*  Name   : entity_data_bin tester                                            *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Checks that the entity_data_bin class is behaving as expected.           *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
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
#include <application/application_shell.h>
#include <loggers/console_logger.h>
#include <loggers/program_wide_logger.h>
#include <structures/static_memory_gremlin.h>
#include <octopus/entity_data_bin.h>
#include <octopus/entity_defs.h>
#include <tentacles/security_infoton.h>
#include <textual/string_manipulation.h>

#include <stdio.h>

using namespace application;
using namespace basis;
using namespace loggers;
using namespace octopi;
using namespace textual;

const int ITEM_COUNT = 10000;
  // the number of times to repeat each test operation.

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger().get(), astring(s))
#define BASE_LOG(s) EMERGENCY_LOG(program_wide_logger().get(), astring(s))

class test_bin : public application_shell
{
public:
  test_bin() : application_shell() {}
  DEFINE_CLASS_NAME("test_bin");
  int execute();
};

//////////////
  
int test_bin::execute()
{
  FUNCDEF("execute");
  char c = '\0';

  array<octopus_request_id> item_list;

  entity_data_bin *bing = new entity_data_bin(10 * MEGABYTE);

  enum test_types { ANY = 1, ENT, ID };

  for (int q = ANY; q <= ID; q++) {
LOG(a_sprintf("test type %d beginning...%c", q, c));
  // using c just shuts up warnings.
//LOG("note memory usage and hit a key:");
//c = getchar();

    program_wide_logger().get().eol(parser_bits::NO_ENDING);
    for (int i = 1; i <= ITEM_COUNT; i++) {
      // test the basic filling of the values in an entity.
      octopus_request_id req_id;
      int sequencer = randomizer().inclusive(1, MAXINT32 - 10);
      int add_in = randomizer().inclusive(0, MAXINT32 - 10);
      int process_id = randomizer().inclusive(0, MAXINT32 - 10);
      req_id._entity = octopus_entity(string_manipulation::make_random_name(),
          process_id, sequencer, add_in);
      req_id._request_num = randomizer().inclusive(1, MAXINT32 - 10);
      infoton *torp = new security_infoton;
      bing->add_item(torp, req_id);
      item_list += req_id;

      if (! (i % 50) ) {
        printf("^");
        fflush(NIL);
      }
    }
    program_wide_logger().get().eol(parser_bits::CRLF_AT_END);
    LOG("");

    int items_seen = 0;

    program_wide_logger().get().eol(parser_bits::NO_ENDING);
    if (q == ANY) {
      while (item_list.length()) {
        octopus_request_id id;
        infoton *found = bing->acquire_for_any(id);
        if (!found) 
          deadly_error(class_name(), "ANY", "item was missing");
        WHACK(found);
        items_seen++;
        if (! (items_seen % 50) ) {
          printf("v");
          fflush(NIL);
        }
        bool saw_it = false;
        for (int q = 0; q < item_list.length(); q++) {
          if (item_list[q] == id) {
            saw_it = true;
            item_list.zap(q, q);
            break;
          }
        }
        if (!saw_it)
          deadly_error(class_name(), "ANY", "didn't see id for the item");
      }
    } else if (q == ENT) {
      while (item_list.length()) {
        octopus_request_id id;
        infoton *found = bing->acquire_for_entity(item_list[0]._entity, id);
        if (!found) 
          deadly_error(class_name(), "ENT", "item was missing");
        WHACK(found);
        items_seen++;
        if (! (items_seen % 50) ) {
          printf("v");
          fflush(NIL);
        }
        bool saw_it = false;
        for (int q = 0; q < item_list.length(); q++) {
          if (item_list[q] == id) {
            saw_it = true;
            item_list.zap(q, q);
            break;
          }
        }
        if (!saw_it)
          deadly_error(class_name(), "ENT", "didn't see id for the item");
      }
    } else if (q == ID) {
      for (int j = 0; j < item_list.length(); j++) {
        infoton *found = bing->acquire_for_identifier(item_list[j]);
        if (!found) 
          deadly_error(class_name(), "ENT", "item was missing");
        WHACK(found);
        items_seen++;
        if (! (items_seen % 50) ) {
          printf("v");
          fflush(NIL);
        }
        item_list.zap(j, j);
        j--;  // skip back.
      }
    } else {
      deadly_error(class_name(), "looping", "bad enum value");
    }
    program_wide_logger().get().eol(parser_bits::CRLF_AT_END);
    LOG("");
    item_list.reset();
    item_list.shrink();

    if (bing->entities())
      deadly_error(class_name(), "check left", "there are still contents in table!");

    bing->clean_out_deadwood();

LOG(a_sprintf("test type %d ending...", q));
//LOG("note memory usage and hit a key:");
//c = getchar();
  }

  WHACK(bing);
LOG("done testing, zapped bin, now should be low memory.");
//c = getchar();

  LOG("octopus_entity:: works for those functions tested.");
  return 0;
}

HOOPLE_MAIN(test_bin, )

