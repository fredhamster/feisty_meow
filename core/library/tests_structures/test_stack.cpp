/*
*  Name   : t_stack
*  Author : Chris Koeritz
*  Purpose:
*    Tests out the stack object with both flat objects (byte_array) but also
*  deep objects (pointer to byte_array).  Both bounded and unbounded stacks
*  are tested for each.
**
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/hoople_main.h>
#include <basis/byte_array.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <loggers/program_wide_logger.h>
#include <mathematics/chaos.h>
#include <structures/stack.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

#ifdef DEBUG_STACK
  #define LOG(to_print) EMERGENCY_LOG(program_wide_logger::get(), to_print)
#endif

#include <stdlib.h>
#include <string.h>

using namespace application;
using namespace basis;
///using namespace configuration;
using namespace mathematics;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

//HOOPLE_STARTUP_CODE;

//#define DEBUG_STACK
  // uncomment for a noisier version.

const int test_iterations = 40;


class test_stack : public virtual unit_base, public virtual application_shell
{
public:
  test_stack() {}
  DEFINE_CLASS_NAME("test_stack");
  int execute();

  void test_stack_with_objects();
  void test_stack_with_pointers();

  void CHECK_STACK_RESULT(outcome retval, const astring &place);
  byte_array generate_flat(const char *to_store);
  byte_array *generate_deep(const char *to_store);
  astring text_form(stack<byte_array *> &s);
  astring text_form(stack<byte_array> &s);
};

// CHECK_STACK_RESULT: a function that takes care of error testing for a
// stack command.  if executing the command ends in full or empty, then
// that is reported.
void test_stack::CHECK_STACK_RESULT(outcome retval, const astring &place)
{
#ifdef DEBUG_STACK
  if (retval == common::IS_FULL)
    LOG(astring(astring::SPRINTF, "returned IS_FULL at %s", place.s()))
  else if (retval == common::IS_EMPTY)
    LOG(astring(astring::SPRINTF, "returned IS_EMPTY at %s", place.s()));
#else
  if (retval.value() || !place) {}
#endif
}

byte_array test_stack::generate_flat(const char *to_store)
{
  byte_array to_return = byte_array(int(strlen(to_store) + 1), (abyte *)to_store);
  return to_return;
}

byte_array *test_stack::generate_deep(const char *to_store)
{
  byte_array *to_return = new byte_array(int(strlen(to_store) + 1),
      (abyte *)to_store);
  return to_return;
}

astring test_stack::text_form(stack<byte_array *> &s)
{
  astring to_return;
  for (int i = 0; i < s.elements(); i++) {
    to_return += astring(astring::SPRINTF, "#%d: %s\n", i, s[i]->observe());
  }
  return to_return;
}

astring test_stack::text_form(stack<byte_array> &s)
{
  astring to_return;
  for (int i = 0; i < s.elements(); i++) {
    to_return += astring(astring::SPRINTF, "#%d: %s\n", i, s[i].observe());
  }
  return to_return;
}

void test_stack::test_stack_with_objects()
{
  FUNCDEF("test_stack_with_objects");
  for (int qq = 0; qq < test_iterations; qq++) {
#ifdef DEBUG_STACK
    LOG(astring(astring::SPRINTF, "index %d", qq));
#endif
    stack<byte_array > bounded_stack(3);
    stack<byte_array > unlimited_stack(0);
    chaos randomizer;

    {
#ifdef DEBUG_STACK
      LOG("testing the bounded stack first:");
#endif
      CHECK_STACK_RESULT(bounded_stack.push(generate_flat("the first line")), "first push");
      CHECK_STACK_RESULT(bounded_stack.push(generate_flat("the second line")), "second push");
      CHECK_STACK_RESULT(bounded_stack.push(generate_flat("the final and third")), "third push");
      byte_array gend = generate_flat("this shouldn't work");
      ASSERT_EQUAL(bounded_stack.push(gend).value(), common::IS_FULL,
          "the bounded stack push should catch IS_FULL");
#ifdef DEBUG_STACK
      LOG("pushing worked successfully...");
      LOG("printing the stack in element order");
#endif
      for (int i = 0; i < bounded_stack.size(); i++) {
#ifdef DEBUG_STACK
        LOG(astring(astring::SPRINTF, "depth %d has %s.", i,
            bounded_stack[i].observe()));
#endif
      }
#ifdef DEBUG_STACK
      LOG("now popping the stack all the way back.");
#endif
      int full_size = bounded_stack.size();
      for (int j = 0; j < full_size; j++) {
#ifdef DEBUG_STACK
        LOG(astring(astring::SPRINTF, "pop %d, stack size is %d, has %s", j+1,
            bounded_stack.size(), bounded_stack.top().observe()));
#endif
        byte_array found;
        bounded_stack.acquire_pop(found);
        astring got((char *)found.observe());
        switch (j) {
        case 0: 
          ASSERT_EQUAL(got, astring("the final and third"),
              "acquire_pop should have right contents at 2");
          break;
        case 1:
          ASSERT_EQUAL(got, astring("the second line"),
              "acquire_pop should have right contents at 1");
          break;
        case 2:
          ASSERT_EQUAL(got, astring("the first line"),
              "acquire_pop should have right contents at 0");
          break;
        }
      }
      ASSERT_EQUAL(bounded_stack.pop().value(), common::IS_EMPTY,
          "bounded pop should have right outcome");
    }

    {
#ifdef DEBUG_STACK
      LOG("testing the unbounded stack now:");
#endif
      for (int j = 0; j < 24; j++) {
        astring line(astring::SPRINTF, "{posn %d here}", j);
        CHECK_STACK_RESULT(unlimited_stack.push(generate_flat(line.s())), "unbound push");
      }
#ifdef DEBUG_STACK
      LOG("unbounded stack in element order:");
#endif
      for (int k = 0; k < unlimited_stack.size(); k++) {
#ifdef DEBUG_STACK
        LOG(astring(astring::SPRINTF, "#%d is %s", k, unlimited_stack[k].observe()));
#endif
      }
#ifdef DEBUG_STACK
      LOG("now popping fresh order:");
#endif
      while (unlimited_stack.size()) {
#ifdef DEBUG_STACK
        LOG(astring(astring::SPRINTF, "size %d, content %s",
            unlimited_stack.size(), unlimited_stack.top().observe()));
#endif
        unlimited_stack.pop();
      }
#ifdef DEBUG_STACK
      LOG("");
#endif
      ASSERT_EQUAL(unlimited_stack.pop().value(), common::IS_EMPTY,
          "unlimited pop should return empty as expected");
#ifdef DEBUG_STACK
      LOG("\
----------------------------------------------\n\
both types of stack exercises were successful.\n\
----------------------------------------------");
#endif
    }

#ifdef DEBUG_STACK
    LOG("now setting up some simple stacks...");
#endif

    {
#ifdef DEBUG_STACK
      LOG("bounded first...");
#endif
      stack<byte_array > bounder(randomizer.inclusive(30, 80));
#ifdef DEBUG_STACK
      LOG(astring(astring::SPRINTF, "length of bounder is max %d, curr %d", bounder.elements(), bounder.size()));
#endif
      int max = bounder.elements();
      for (int i = 0; i < max; i++) {
        ASSERT_EQUAL(bounder.size(), i, "the bounder size should be in step with loop");
        int byte_array_size = randomizer.inclusive(259, 287);
        byte_array to_stuff(byte_array_size);
        astring visible(astring::SPRINTF, "entry %d...", i);
        visible.stuff((char *)to_stuff.access(), visible.length() + 1);
        for (int j = visible.length() + 1; j < to_stuff.length(); j++)
          *(to_stuff.access() + j) = '\0';
#ifdef DEBUG_STACK
        LOG(astring(astring::SPRINTF, "pushing index %d", i));
#endif
        outcome ret = bounder.push(to_stuff);
        ASSERT_EQUAL(ret.value(), common::OKAY, "pushing should not fail in simple test");
      }
      ASSERT_EQUAL(bounder.elements(), bounder.size(), "bounder set should see size and max same");
#ifdef DEBUG_STACK
      LOG("inverting:");
#endif
      bounder.invert();
#ifdef DEBUG_STACK
      LOG(astring(astring::SPRINTF, "inverted is:\n%s", text_form(bounder).s()));
#endif
      while (bounder.size()) bounder.pop();
    }
  }
}

void test_stack::test_stack_with_pointers()
{
  FUNCDEF("test_stack_with_pointers")
  for (int qq = 0; qq < test_iterations; qq++) {
#ifdef DEBUG_STACK
    LOG(astring(astring::SPRINTF, "index %d", qq));
#endif
    stack<byte_array *> bounded_stack(3);
    stack<byte_array *> unlimited_stack(0);
    chaos randomizer;

    {
#ifdef DEBUG_STACK
      LOG("testing the bounded stack first:");
#endif
      CHECK_STACK_RESULT(bounded_stack.push(generate_deep("the first line")), "first push");
      CHECK_STACK_RESULT(bounded_stack.push(generate_deep("the second line")), "second push");
      CHECK_STACK_RESULT(bounded_stack.push(generate_deep("the final and third")), "third push");
      byte_array *gend = generate_deep("this shouldn't work");
      ASSERT_EQUAL(bounded_stack.push(gend).value(), common::IS_FULL,
          "the bounded stack push should catch IS_FULL");
      delete gend;
#ifdef DEBUG_STACK
      LOG("pushing worked successfully...");
      LOG("printing the stack in element order");
#endif
      for (int i = 0; i < bounded_stack.size(); i++) {
#ifdef DEBUG_STACK
        LOG(astring(astring::SPRINTF, "depth %d has: %s", i, bounded_stack[i]->observe()));
#endif
      }
#ifdef DEBUG_STACK
      LOG("now popping the stack all the way back.");
#endif
      int full_size = bounded_stack.size();
      for (int j = 0; j < full_size; j++) {
#ifdef DEBUG_STACK
        LOG(astring(astring::SPRINTF, "pop %d, stack size is %d, has %s", j+1, bounded_stack.size(), bounded_stack.top()->observe()));
#endif
        byte_array *found;
        bounded_stack.acquire_pop(found);
        ASSERT_TRUE(found, "acquire_pop test should not return nil");
        ASSERT_TRUE(found->observe(), "acquire_pop test should have non-nil observe");
        astring got((char *)found->observe());
        switch (j) {
        case 0:
          ASSERT_EQUAL(got, astring("the final and third"),
              "popping should have right contents at 2");
          break;
        case 1:
          ASSERT_EQUAL(got, astring("the second line"),
              "popping should have right contents at 1");
          break;
        case 2:
          ASSERT_EQUAL(got, astring("the first line"),
              "popping should have right contents at 0");
          break;
        }
        delete found;
      }
      ASSERT_EQUAL(bounded_stack.pop().value(), common::IS_EMPTY,
          "bounded pop failure in result");
    }

    {
#ifdef DEBUG_STACK
      LOG("testing the unbounded stack now:");
#endif
      for (int j = 0; j < 24; j++) {
        astring line(astring::SPRINTF, "{posn %d here}", j);
        CHECK_STACK_RESULT(unlimited_stack.push(generate_deep(line.s())), "unbound push");
      }
#ifdef DEBUG_STACK
      LOG("unbounded stack in element order:");
#endif
      for (int k = 0; k < unlimited_stack.size(); k++) {
#ifdef DEBUG_STACK
        LOG(astring(astring::SPRINTF, "#%d is %s", k, unlimited_stack[k]->observe()));
#endif
      }
#ifdef DEBUG_STACK
      LOG("\nnow popping order:");
#endif
      while (unlimited_stack.size()) {
#ifdef DEBUG_STACK
        LOG(astring(astring::SPRINTF, "size %d, content %s", unlimited_stack.size(), unlimited_stack.top()->observe()));
#endif
        byte_array *to_zap = unlimited_stack.top();
        unlimited_stack.pop();
        delete to_zap;
          // okay because the pointer was copied, and the reference from top
          // is not being relied on.
      }
#ifdef DEBUG_STACK
      LOG("");
#endif
      ASSERT_EQUAL(unlimited_stack.pop().value(), common::IS_EMPTY,
          "unlimited pop should return empty as expected");
#ifdef DEBUG_STACK
      LOG("\n\
----------------------------------------------\n\
both types of stack exercises were successful.\n\
----------------------------------------------");
#endif
    }

#ifdef DEBUG_STACK
    LOG("now setting up some simple stacks...");
#endif

    {
#ifdef DEBUG_STACK
      LOG("bounded first...");
#endif
      stack<byte_array *> bounder(randomizer.inclusive(30, 80));
#ifdef DEBUG_STACK
      LOG(astring(astring::SPRINTF, "length of bounder is max %d, curr %d",
          bounder.elements(), bounder.size()));
#endif
      int max = bounder.elements();
      for (int i = 0; i < max; i++) {
        ASSERT_EQUAL(bounder.size(), i, "bounder size should remain in step with loop");
        int byte_array_size = randomizer.inclusive(259, 287);
        byte_array *to_stuff = new byte_array(byte_array(byte_array_size));
        astring visible(astring::SPRINTF, "entry %d...", i);
        visible.stuff((char *)to_stuff->observe(), visible.length() + 1);
        for (int j = visible.length() + 1; j < to_stuff->length(); j++)
          *(to_stuff->access() + j) = '\0';
        ASSERT_EQUAL(bounder.push(to_stuff).value(), common::OKAY,
            "pushing should not fail in bound simple test");
      }
      ASSERT_EQUAL(bounder.elements(), bounder.size(), "bounder set must have size and max agree");
#ifdef DEBUG_STACK
      LOG("inverting:");
#endif
      bounder.invert();
#ifdef DEBUG_STACK
      LOG(astring(astring::SPRINTF, "inverted is:\n%s", text_form(bounder).s()));
#endif
      while (bounder.size()) {
        byte_array *to_zap = bounder.top();
        bounder.pop();
        delete to_zap;
      }
    }
  }
}

int test_stack::execute()
{
  test_stack_with_objects();
  test_stack_with_pointers();
  return final_report();
}

HOOPLE_MAIN(test_stack, )

