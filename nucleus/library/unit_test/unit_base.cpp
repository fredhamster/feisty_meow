/*
*  Name   : unit test tools
*  Author : Chris Koeritz
**
* Copyright (c) 2009-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include "unit_base.h"

#include <application/application_shell.h>
#include <basis/astring.h>
#include <basis/functions.h>
#include <configuration/application_configuration.h>
#include <loggers/program_wide_logger.h>
#include <filesystem/byte_filer.h>
#include <filesystem/filename.h>
#include <structures/string_table.h>
#include <textual/xml_generator.h>

using namespace application;
using namespace basis;
using namespace configuration;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;

#define BASE_LOG(s) EMERGENCY_LOG(program_wide_logger::get(), s)

namespace unit_test {

const int EXPECTED_MAXIMUM_TESTS = 10008;  //!< maximum number of tests expected.

const char *name_for_bools(bool bv)
{ if (bv) return "true"; else return "false"; }

unit_base::unit_base()
: c_lock(),
  c_total_tests(0),
  c_passed_tests(0),
  c_successful(EXPECTED_MAXIMUM_TESTS),
  c_failed(EXPECTED_MAXIMUM_TESTS)
{
}

unit_base::~unit_base() {}

int unit_base::total_tests() const { return c_total_tests; }

int unit_base::passed_tests() const { return c_passed_tests; }

int unit_base::failed_tests() const
{ return c_total_tests - c_passed_tests; }

void unit_base::count_successful_test(const basis::astring &class_name, const basis::astring &test_name)
{
  auto_synchronizer synch(c_lock);
  outcome ret = c_successful.add(class_name + " -- " + test_name, "");
  if (ret == common::IS_NEW) {
    c_total_tests++;
    c_passed_tests++;
  }
}

void unit_base::record_pass(const basis::astring &class_name, const astring &test_name,
    const astring &diag)
{
  auto_synchronizer synch(c_lock);
  count_successful_test(class_name, test_name);
//hmmm: kind of lame bailout on printing this.
//      it gets very very wordy if it's left in.
#ifdef DEBUG
  astring message = astring("OKAY: ") + class_name + " in test [" + test_name + "]";
  BASE_LOG(message);
#endif
}

void unit_base::count_failed_test(const basis::astring &class_name, const basis::astring &test_name,
    const basis::astring &diag)
{
  auto_synchronizer synch(c_lock);
  outcome ret = c_failed.add(class_name + " -- " + test_name, diag);
  if (ret == common::IS_NEW) {
    c_total_tests++;
  }
}

void unit_base::record_fail(const basis::astring &class_name, const astring &test_name, const astring &diag)
{
  count_failed_test(class_name, test_name, diag);
  astring message = astring("\nFAIL: ") + class_name + " in test [" + test_name + "]\n" + diag + "\n";
  BASE_LOG(message);
}

void unit_base::record_successful_assertion(const basis::astring &class_name, const astring &test_name,
    const astring &assertion_name)
{
  record_pass(class_name, test_name,
      astring("no problem with: ") + assertion_name);
}

void unit_base::record_failed_object_compare(const hoople_standard &a, const hoople_standard &b,
    const basis::astring &class_name, const astring &test_name, const astring &assertion_name)
{
  astring a_state, b_state;
  a.text_form(a_state);
  b.text_form(b_state);
  record_fail(class_name, test_name,
      astring("Error in assertion ") + assertion_name + ":\n"
      + "==============\n"
      + a_state + "\n"
      + "=== versus ===\n"
      + b_state + "\n"
      + "==============");
}

void unit_base::record_failed_int_compare(int a, int b,
    const basis::astring &class_name, const astring &test_name, const astring &assertion_name)
{
  record_fail(class_name, test_name,
      astring("Error in assertion ") + assertion_name
      + a_sprintf(": inappropriate values: %d & %d", a, b));
}

void unit_base::record_failed_double_compare(double a, double b,
    const basis::astring &class_name, const astring &test_name, const astring &assertion_name)
{
  record_fail(class_name, test_name,
      astring("Error in assertion ") + assertion_name
      + a_sprintf(": inappropriate values: %f & %f", a, b));
}

void unit_base::record_failed_pointer_compare(const void *a, const void *b,
    const basis::astring &class_name, const astring &test_name, const astring &assertion_name)
{
  record_fail(class_name, test_name,
      astring("Error in assertion ") + assertion_name
      + a_sprintf(": inappropriate values: %p & %p", a, b));
}

void unit_base::record_failed_tf_assertion(bool result,
    bool expected_result, const basis::astring &class_name, const astring &test_name,
    const astring &assertion_name)
{
  record_fail(class_name, test_name, astring("Error in assertion ") + assertion_name
      + ": expected " + name_for_bools(expected_result)
      + " but found " + name_for_bools(result));
}

void unit_base::assert_equal(const hoople_standard &a, const hoople_standard &b,
    const basis::astring &class_name, const astring &test_name, const astring &diag)
{
  FUNCDEF("assert_equal");
  bool are_equal = (a == b);
  if (are_equal) record_successful_assertion(class_name, test_name, astring(func) + ": " + diag);
  else record_failed_object_compare(a, b, class_name, test_name, func);
}

void unit_base::assert_not_equal(const hoople_standard &a, const hoople_standard &b,
    const basis::astring &class_name, const astring &test_name, const astring &diag)
{  
  FUNCDEF("assert_not_equal");
  bool are_equal = (a == b);
  if (!are_equal) record_successful_assertion(class_name, test_name, astring(func) + ": " + diag);
  else record_failed_object_compare(a, b, class_name, test_name, func);
}

const char *byte_array_phrase = "byte_array[%d]";  // for printing a text form of byte_array.

void unit_base::assert_equal(const basis::byte_array &a, const basis::byte_array &b,
    const basis::astring &class_name, const basis::astring &test_name, const astring &diag)
{
  FUNCDEF("assert_equal");
  bool are_equal = (a == b);
  if (are_equal) record_successful_assertion(class_name, test_name, astring(func) + ": " + diag);
  else {
    astring a_s = a_sprintf(byte_array_phrase, a.length());
    astring b_s = a_sprintf(byte_array_phrase, b.length());
    record_failed_object_compare(a_s, b_s, class_name, test_name, func);
  }
}

void unit_base::assert_not_equal(const basis::byte_array &a, const basis::byte_array &b,
    const basis::astring &class_name, const basis::astring &test_name, const astring &diag)
{  
  FUNCDEF("assert_not_equal");
  bool are_equal = (a == b);
  if (!are_equal) record_successful_assertion(class_name, test_name, func);
  else {
    astring a_s = a_sprintf(byte_array_phrase, a.length());
    astring b_s = a_sprintf(byte_array_phrase, b.length());
    record_failed_object_compare(a_s, b_s, class_name, test_name, func);
  }
}

void unit_base::assert_equal(int a, int b, const basis::astring &class_name, const astring &test_name, const astring &diag)
{
  FUNCDEF("assert_equal integer");
  bool are_equal = a == b;
  if (are_equal) record_successful_assertion(class_name, test_name, astring(func) + ": " + diag);
  else record_failed_int_compare(a, b, class_name, test_name, astring(func) + ": " + diag);
}

void unit_base::assert_not_equal(int a, int b, const basis::astring &class_name, const astring &test_name, const astring &diag)
{
  FUNCDEF("assert_not_equal integer");
  bool are_inequal = a != b;
  if (are_inequal) record_successful_assertion(class_name, test_name, astring(func) + ": " + diag);
  else record_failed_int_compare(a, b, class_name, test_name, astring(func) + ": " + diag);
}

void unit_base::assert_equal(double a, double b, const basis::astring &class_name, const astring &test_name, const astring &diag)
{
  FUNCDEF("assert_equal double");
  bool are_equal = a == b;
  if (are_equal) record_successful_assertion(class_name, test_name, astring(func) + ": " + diag);
  else record_failed_double_compare(a, b, class_name, test_name, astring(func) + ": " + diag);
}

void unit_base::assert_not_equal(double a, double b, const basis::astring &class_name, const astring &test_name, const astring &diag)
{
  FUNCDEF("assert_not_equal double");
  bool are_inequal = a != b;
  if (are_inequal) record_successful_assertion(class_name, test_name, astring(func) + ": " + diag);
  else record_failed_double_compare(a, b, class_name, test_name, astring(func) + ": " + diag);
}


void unit_base::assert_equal(const void *a, const void *b,
    const basis::astring &class_name, const astring &test_name, const astring &diag)
{
  FUNCDEF("assert_equal void pointer");
  bool are_equal = a == b;
  if (are_equal) record_successful_assertion(class_name, test_name, astring(func) + ": " + diag);
  else record_failed_pointer_compare(a, b, class_name, test_name, astring(func) + ": " + diag);
}

void unit_base::assert_not_equal(const void *a, const void *b,
    const basis::astring &class_name, const astring &test_name, const astring &diag)
{
  FUNCDEF("assert_not_equal void pointer");
  bool are_inequal = a != b;
  if (are_inequal) record_successful_assertion(class_name, test_name, astring(func) + ": " + diag);
  else record_failed_pointer_compare(a, b, class_name, test_name, astring(func) + ": " + diag);
}

void unit_base::assert_true(bool result, const basis::astring &class_name, const astring &test_name, const astring &diag)
{
  FUNCDEF("assert_true");
  if (result) record_successful_assertion(class_name, test_name, astring(func) + ": " + diag);
  else record_failed_tf_assertion(result, true, class_name, test_name, astring(func) + ": " + diag);
}

void unit_base::assert_false(bool result, const basis::astring &class_name, const astring &test_name, const astring &diag)
{
  FUNCDEF("assert_false");
  if (!result) record_successful_assertion(class_name, test_name, astring(func) + ": " + diag);
  else record_failed_tf_assertion(result, false, class_name, test_name, astring(func) + ": " + diag);
}

int unit_base::final_report()
{
  auto_synchronizer synch(c_lock);
  int to_return = 0;  // return success until we know otherwise.

  astring keyword = "FAILURE";  // but be pessimistic about overall result at first..?

// BASE_LOG(a_sprintf("total tests %d passed tests %d", c_total_tests, c_passed_tests));

  // check whether we really did succeed or not.
  if (c_total_tests == c_passed_tests) keyword = "SUCCESS";  // success!
  else to_return = 12;  // a failure return.

  if (!c_total_tests) keyword = "LAMENESS (no tests!)";  // boring!

//  astring message = keyword + " for "
//    + application_configuration::application_name()
//    + a_sprintf(": %d of %d atomic tests passed.",
//      c_passed_tests, c_total_tests);
//  BASE_LOG(message);

  astring message = keyword + " for "
    + filename(application_configuration::application_name()).basename().raw()
    + a_sprintf(": %d of %d unit tests passed.",
      c_successful.symbols(), c_successful.symbols() + c_failed.symbols());
  BASE_LOG(message);

  // send an xml file out for the build engine to analyze.
  write_cppunit_xml();

  return to_return;
}

void unit_base::write_cppunit_xml()
{
  auto_synchronizer synch(c_lock);
  astring logs_dir = environment::get("LOGS_DIR");
  if (logs_dir == astring::empty_string()) logs_dir = "logs";  // uhhh.
  astring outfile = logs_dir + "/"
      + filename(application_configuration::application_name()).basename().raw()
      + ".xml";

//BASE_LOG(astring("outfile is ") + outfile);

  int id = 1;

  xml_generator report;
  string_table attribs;

  // we are emulating a cppunit xml output.

  report.open_tag("TestRun");

  report.open_tag("FailedTests");
  for (int i = 0; i < c_failed.symbols(); i++) {
    attribs.reset();
    attribs.add("id", a_sprintf("%d", id++));
    report.open_tag("FailedTest", attribs);
    attribs.reset();
//hmmm: does open_tag eat the attribs?  we could stop worrying about resetting.
    report.open_tag("Name");
    report.add_content(c_failed.name(i));
    report.close_tag("Name");

    report.open_tag("FailureType", attribs);
    report.add_content("Assertion");
    report.close_tag("FailureType");

    report.open_tag("Location", attribs);

    report.open_tag("File", attribs);
    report.add_content(application_configuration::application_name());
    report.close_tag("File");

    report.open_tag("Line", attribs);
    report.add_content("0");
    report.close_tag("Line");

    report.close_tag("Location");

    report.open_tag("Message");
    report.add_content(c_failed[i]);
    report.close_tag("Message");

    report.close_tag("FailedTest");
  }
  report.close_tag("FailedTests");

  report.open_tag("SuccessfulTests");
  for (int i = 0; i < c_successful.symbols(); i++) {
    attribs.reset();
    attribs.add("id", a_sprintf("%d", id++));
    attribs.reset();
    report.open_tag("Test", attribs);
    report.open_tag("Name");
    report.add_content(c_successful.name(i));
    report.close_tag("Name");
    report.close_tag("Test");
  }
  report.close_tag("SuccessfulTests");

  report.open_tag("Statistics");
  report.open_tag("Tests");
  report.add_content(a_sprintf("%d", c_failed.symbols() + c_successful.symbols()));
  report.close_tag("Tests");

  report.open_tag("FailuresTotal");
  report.add_content(a_sprintf("%d", c_failed.symbols()));
  report.close_tag("FailuresTotal");

  report.open_tag("Errors");
  report.add_content("0");
  report.close_tag("Errors");

  report.open_tag("Failures");
  report.add_content(a_sprintf("%d", c_failed.symbols()));
  report.close_tag("Failures");

  report.close_tag("Statistics");

  report.close_tag("TestRun");

  astring text_report = report.generate();
//  BASE_LOG(astring("got report\n") + text_report);

  byte_filer xml_out(outfile, "wb");
  xml_out.write(text_report);
  xml_out.close();
}

} //namespace.

