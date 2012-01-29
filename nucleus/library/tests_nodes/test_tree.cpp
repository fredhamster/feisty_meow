/*
*  Name   : test_tree                                                         *
*  Author : Chris Koeritz                                                     *
*  Purpose:                                                                   *
*    Tests out the tree class.                                                *
**
* Copyright (c) 1993-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <loggers/console_logger.h>
#include <nodes/node.h>
#include <nodes/tree.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace nodes;
using namespace loggers;
using namespace structures;
using namespace unit_test;

//#define DEBUG_TEST_TREE
  // uncomment if you want the noisy streams version.

const int test_iterations = 20;

class bogotre : public packable, public tree
{
public:
  bogotre(const char *start = NIL) : who_cares(42), i_sure_dont('l'),
        another_useless_int(23) {
    astring to_init(start);
    if (to_init.length() < 1) to_init += "ack";
    to_init.stuff(the_actual_string, minimum(to_init.length()+1, 500));
  }
  DEFINE_CLASS_NAME("bogotre");
  virtual ~bogotre() {}
  virtual void pack(byte_array &packed_form) const;
  virtual bool unpack(byte_array &to_unpack);
  virtual int packed_size() const;
  virtual abyte *held() const { return (abyte *)the_actual_string; }
  virtual void print() const {
#ifdef DEBUG_TEST_TREE
    printf(the_actual_string); 
#endif
  }

private:
  char the_actual_string[500];
  int who_cares;
  char i_sure_dont;
  int another_useless_int;
};

//////////////

// forward.
typedef bogotre larch;
typedef void (applier)(larch *apply_to);
typedef tree::iterator traveller;

class test_tree : public virtual unit_base, virtual public application_shell
{
public:
  test_tree() : application_shell() {}
  DEFINE_CLASS_NAME("test_tree");
  virtual int execute();
  static void print_node(larch *curr_node);
  static larch *next(larch *&move, larch *hook, traveller &skip);
  static void apply(larch *apply_to, applier *to_apply,
          tree::traversal_directions order);
};

//////////////

#undef UNIT_BASE_THIS_OBJECT
#define UNIT_BASE_THIS_OBJECT (*dynamic_cast<unit_base *>(application_shell::single_instance()))

int bogotre::packed_size() const
{ return strlen(the_actual_string) + 1 + sizeof(int) * 2 + sizeof(abyte); }

void bogotre::pack(byte_array &packed_form) const
{
  FUNCDEF("pack");
  astring(the_actual_string).pack(packed_form);
  structures::attach(packed_form, who_cares);
  structures::attach(packed_form, i_sure_dont);
  structures::attach(packed_form, another_useless_int);
}

bool bogotre::unpack(byte_array &packed_form)
{
  FUNCDEF("unpack");
  // 5 is the magic knowledge of minimum packed string.
//hmmm: make the minimum packed size a property of packables?
  ASSERT_FALSE(packed_form.length() <
      int(1 + sizeof(who_cares) + sizeof(i_sure_dont) + sizeof(another_useless_int)),
      "size of package should be correct");
  astring unpacked;
  ASSERT_TRUE(unpacked.unpack(packed_form), "should be able to retrieve string");
  ASSERT_TRUE(structures::detach(packed_form, who_cares), "should retrieve who_cares");
  ASSERT_TRUE(structures::detach(packed_form, i_sure_dont), "should retrieve i_sure_dont");
  ASSERT_TRUE(structures::detach(packed_form, another_useless_int),
      "should retrieve another_...");

  ASSERT_EQUAL(who_cares, 42, "bogotre_unpack - right value held in first int");
  ASSERT_EQUAL(i_sure_dont, 'l', "bogotre_unpack - right character held");
  ASSERT_EQUAL(another_useless_int, 23, "bogotre_unpack - right value held in second int");
  return true;
}

//////////////

/*
bogotre *togen(char *to_store)
{ bogotre *to_return = new bogotre(astring(to_store).s()); return to_return; }
*/

void test_tree::print_node(larch *curr_node)
{
  FUNCDEF("print_node");
  ASSERT_TRUE(curr_node, "tree shouldn't be nil");
  bogotre *real_curr = dynamic_cast<bogotre *>(curr_node);
  ASSERT_TRUE(real_curr, "contents shouldn't be nil");
  astring to_examine((char *)real_curr->held());
#ifdef DEBUG_TEST_TREE
  to_examine += " ";
  printf(to_examine.s());
//remove it again if we reenable the cut.
#endif
//  if (to_examine == to_look_for) real_curr->cut();
}

#undef UNIT_BASE_THIS_OBJECT
#define UNIT_BASE_THIS_OBJECT (*this)

//////////////

larch *test_tree::next(larch *&move, larch *formal(hook), traveller &skip)
{ move = dynamic_cast<larch *>(skip.next()); return move; }

void test_tree::apply(larch *apply_to, applier *to_apply,
    tree::traversal_directions order)
{
  larch *curr = NIL;
  for (traveller skippy = apply_to->start(order);
       next(curr, apply_to, skippy); ) to_apply(curr);
}

int test_tree::execute()
{
  FUNCDEF("execute");
  for (int qq = 0; qq < test_iterations; qq++) {
  larch *e1 = new larch("a");
  larch *e2 = new larch("b");
  larch *e3 = new larch("+");
  e3->attach(e1);
  e3->attach(e2);

  larch *e4 = new larch("c");
  larch *e5 = new larch("-");
  e5->attach(e3);
  e5->attach(e4);

  larch *e6 = new larch(">");
  larch *e7 = new larch("23");
  e6->attach(e5);
  e6->attach(e7);

  larch *e8 = new larch("d");
  larch *e9 = new larch("=");
  e9->attach(e8);
  e9->attach(e6);

#ifdef DEBUG_TEST_TREE
  printf("infix is ");
#endif
  apply(e9, print_node, tree::infix);
#ifdef DEBUG_TEST_TREE
  printf("\nprefix is ");
#endif
  apply(e9, print_node, tree::prefix);
#ifdef DEBUG_TEST_TREE
  printf("\npostfix is ");
#endif
  apply(e9, print_node, tree::postfix);
#ifdef DEBUG_TEST_TREE
  printf("\n");
  printf("branches is ");
#endif
  apply(e9, print_node, tree::to_branches);
#ifdef DEBUG_TEST_TREE
  printf("\n");
  printf("branches reversed is ");
#endif
  apply(e9, print_node, tree::reverse_branches);
#ifdef DEBUG_TEST_TREE
  printf("\n");
  printf("before first pack");
#endif
  byte_array packed_e9(0);
  int sizzle = e9->packed_size();
  e9->pack(packed_e9);
  ASSERT_EQUAL(sizzle, packed_e9.length(), "packed size should agree with results");
#ifdef DEBUG_TEST_TREE
  printf("after first pack, size is %d\n", packed_e9.length());
#endif
  larch *new_e9 = new larch();
  new_e9->unpack(packed_e9);
#ifdef DEBUG_TEST_TREE
  printf("New tree after unpacking is (infix order):\n");
#endif
  apply(new_e9, print_node, tree::infix);
#ifdef DEBUG_TEST_TREE
  printf("\n");
#endif
/*
#ifdef DEBUG_TEST_TREE
  printf("the following dumps are in the order: infix, prefix, postfix.\n\n");
  printf("now trying cut on the character '>':\n");
#endif
  to_look_for = ">";
  new_e9->apply(&print_node, tree::infix);
#ifdef DEBUG_TEST_TREE
  p("\n");
#endif
  new_e9->apply(&print_node, tree::prefix);
#ifdef DEBUG_TEST_TREE
  p("\n");
#endif
  new_e9->apply(&print_node, tree::postfix);
#ifdef DEBUG_TEST_TREE
  p("\nnow trying cut on the character +:\n");
#endif
  to_look_for = "+";
  new_e9->apply(&print_node, tree::infix);
#ifdef DEBUG_TEST_TREE
  p("\n");
#endif
  new_e9->apply(&print_node, tree::prefix);
#ifdef DEBUG_TEST_TREE
  p("\n");
#endif
  new_e9->apply(&print_node, tree::postfix);
#ifdef DEBUG_TEST_TREE
  p("\n");
#endif
  to_look_for = "";

#ifdef DEBUG_TEST_TREE
  p("okay, trying to resume at -\n");
#endif
  e5->resume(&print_node, tree::infix);
#ifdef DEBUG_TEST_TREE
  p("\n");
#endif
  e5->resume(&print_node, tree::prefix);
#ifdef DEBUG_TEST_TREE
  p("\n");
#endif
  e5->resume(&print_node, tree::postfix);
#ifdef DEBUG_TEST_TREE
  p("\n");
#endif
*/
#ifdef DEBUG_TEST_TREE
  printf("deleting\n");
#endif
  delete e9;
/*
printf("second pack\n");
  byte_array second_pack;
printf("packing\n");
  new_e9->pack(second_pack);
#ifdef DEBUG_TEST_TREE
  printf("after second pack, size is %d\n", size);
#endif
*/
  delete new_e9;
/*
  larch *newest_e9 = new larch(SELF_CLEANING);
  newest_e9->unpack(second_pack);
#ifdef DEBUG_TEST_TREE
  printf("after second unpack... tree is (infix):\n");
#endif
  newest_e9->apply(print_node, tree::infix);
  delete newest_e9;
#ifdef DEBUG_TEST_TREE
  p("\n");
#endif
*/
  }
  return final_report();
}

HOOPLE_MAIN(test_tree, )

