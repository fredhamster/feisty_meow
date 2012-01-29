#ifndef UNIT_BASE_GROUP
#define UNIT_BASE_GROUP

/*
*  Name   : unit_base tools for unit testing
*  Author : Chris Koeritz
**
* Copyright (c) 2009-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

//! Useful support functions for unit testing, especially within hoople.
/*!
  Provides a simple framework for unit testing the hoople classes.
*/

#include <basis/astring.h>
#include <basis/contracts.h>
#include <basis/enhance_cpp.h>
#include <basis/mutex.h>
#include <structures/string_table.h>

namespace unit_test {

// these macros can be used to put more information into the test name.
// using these is preferable to calling the class methods directly, since
// those cannot print out the original version of the formal parameters a
// and b for the test; the macros show the expressions that failed rather
// than just the values.
// note that these require the calling object to be derived from unit_base.
#define UNIT_BASE_THIS_OBJECT (*this)
  // the macro for UNIT_BASE_THIS_OBJECT allows tests to use a single unit_base object by
  // changing the value of UNIT_BASE_THIS_OBJECT to refer to the proper object's name.
#define ASSERT_EQUAL(a, b, test_name) { \
  BASE_FUNCTION(func); \
  UNIT_BASE_THIS_OBJECT.assert_equal(a, b, function_name, test_name, basis::astring(#a) + " must be equal to " + #b); \
}
#define ASSERT_INEQUAL(a, b, test_name) { \
  BASE_FUNCTION(func); \
  UNIT_BASE_THIS_OBJECT.assert_not_equal(a, b, function_name, test_name, basis::astring(#a) + " must be inequal to " + #b); \
}
#define ASSERT_TRUE(a, test_name) { \
  BASE_FUNCTION(func); \
  UNIT_BASE_THIS_OBJECT.assert_true(a, function_name, test_name, basis::astring(#a) + " must be true"); \
}
#define ASSERT_FALSE(a, test_name) { \
  BASE_FUNCTION(func); \
  UNIT_BASE_THIS_OBJECT.assert_false(a, function_name, test_name, basis::astring(#a) + " must be false"); \
}
// pointer versions for nicer syntax.
#define ASSERT_NULL(x, y) ASSERT_FALSE(x, y)
#define ASSERT_NON_NULL(x, y) ASSERT_TRUE(x, y)

class unit_base : public virtual basis::nameable
{
public:
  unit_base();
  virtual ~unit_base();

  DEFINE_CLASS_NAME("unit_base");

  int total_tests() const;  //!< the total count of tests that have been run.
  int passed_tests() const;  //!< count of successful tests run.
  int failed_tests() const;  //!< count of number of failed tests.

  void assert_equal(const basis::hoople_standard &a, const basis::hoople_standard &b,
      const basis::astring &class_name, const basis::astring &test_name,
      const basis::astring &diagnostic_info);
    //!< tests that the objects a and b are equal.
  void assert_not_equal(const basis::hoople_standard &a, const basis::hoople_standard &b,
      const basis::astring &class_name, const basis::astring &test_name,
      const basis::astring &diagnostic_info);
    //!< tests that objects a and b are NOT equal.

  void assert_equal(const basis::byte_array &a, const basis::byte_array &b,
      const basis::astring &class_name, const basis::astring &test_name,
      const basis::astring &diagnostic_info);
    //!< tests that the byte_arrays a and b are equal.
  void assert_not_equal(const basis::byte_array &a, const basis::byte_array &b,
      const basis::astring &class_name, const basis::astring &test_name,
      const basis::astring &diagnostic_info);
    //!< tests that the byte_arrays a and b are not equal.

  void assert_equal(int a, int b,
      const basis::astring &class_name, const basis::astring &test_name,
      const basis::astring &diagnostic_info);
    //!< tests that integers a and b are equal.
  void assert_not_equal(int a, int b,
      const basis::astring &class_name, const basis::astring &test_name,
      const basis::astring &diagnostic_info);
    //!< tests that integers a and b are NOT equal.

  void assert_equal(double a, double b,
      const basis::astring &class_name, const basis::astring &test_name,
      const basis::astring &diagnostic_info);
    //!< tests that doubles a and b are equal.
  void assert_not_equal(double a, double b,
      const basis::astring &class_name, const basis::astring &test_name,
      const basis::astring &diagnostic_info);
    //!< tests that doubles a and b are NOT equal.

  void assert_equal(const void *a, const void *b,
      const basis::astring &class_name, const basis::astring &test_name,
      const basis::astring &diagnostic_info);
    //!< tests that void pointers a and b are equal.
    /*!< note that you can use this to compare any two pointers as long as
    you cast them to (void *) first.  this reports whether the two have the
    same address in memory, but nothing about their contents. */
  void assert_not_equal(const void *a, const void *b,
      const basis::astring &class_name, const basis::astring &test_name,
      const basis::astring &diagnostic_info);
    //!< tests that void pointers a and b are NOT equal.

  void assert_true(bool result, 
      const basis::astring &class_name, const basis::astring &test_name,
      const basis::astring &diagnostic_info);
    //!< tests that the "result" is a true boolean.
  void assert_false(bool result, 
      const basis::astring &class_name, const basis::astring &test_name,
      const basis::astring &diagnostic_info);
    //!< tests that the "result" is a false boolean.
  
  // these two methods can be used in an ad hoc manner when using the above
  // assert methods is not helpful.  the "test_name" should be provided
  // like above, but the "diagnostic_info" can be phrased in any way needed
  // to describe the pass or fail event.
  void record_pass(const basis::astring &class_name,
          const basis::astring &test_name,
          const basis::astring &diagnostic_info);
    //!< very general recording of a successful test; better to use asserts.
  void record_fail(const basis::astring &class_name,
          const basis::astring &test_name,
          const basis::astring &diagnostic_info);
    //!< very general recording of a failed test; better to use asserts.

  int final_report();
    //!< generates a report of the total number of tests that succeeded.
    /*!< the return value of this can be used as the exit value of the overall test.  if there
    are any failures, then a failure result is returned (non-zero).  otherwise the successful
    exit status of zero is returned. */

private:
  basis::mutex c_lock;  //!< protects our objects for concurrent access.
  int c_total_tests;  //!< how many tests have been run?
  int c_passed_tests;  //!< how many of those passed?
  structures::string_table c_successful;  //!< successful test names.
  structures::string_table c_failed;  //!< failing test names.

  void write_cppunit_xml();
    //!< outputs a report file in cppunit format so CI engines can see results.

  void count_successful_test(const basis::astring &class_name, const basis::astring &test_name);
    //!< records one successful test.

  void count_failed_test(const basis::astring &class_name, const basis::astring &test_name, const basis::astring &diag);
    //!< records one failed test.

  void record_successful_assertion(const basis::astring &class_name, const basis::astring &test_name,
      const basis::astring &assertion_name);
    //!< used by happy tests to record their success.

  void record_failed_object_compare(const basis::hoople_standard &a,
      const basis::hoople_standard &b, const basis::astring &class_name,
      const basis::astring &test_name, const basis::astring &assertion_name);
  void record_failed_int_compare(int a, int b,
      const basis::astring &class_name, const basis::astring &test_name,
      const basis::astring &assertion_name);
  void record_failed_double_compare(double a, double b,
      const basis::astring &class_name, const basis::astring &test_name,
      const basis::astring &assertion_name);
  void record_failed_tf_assertion(bool result, bool expected_result,
      const basis::astring &class_name, const basis::astring &test_name,
      const basis::astring &assertion_name);
  void record_failed_pointer_compare(const void *a, const void *b,
      const basis::astring &class_name, const basis::astring &test_name,
      const basis::astring &assertion_name);
};

} //namespace.

#endif

