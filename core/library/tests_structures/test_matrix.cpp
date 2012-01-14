/*
*  Name   : test_matrix
*  Author : Chris Koeritz
**
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/guards.h>
#include <loggers/console_logger.h>
#include <structures/matrix.h>
#include <structures/static_memory_gremlin.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace mathematics;
using namespace filesystem;
using namespace loggers;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

//#define DEBUG_MATRIX
  // uncomment for noisier version.

const int DIM_ROWS = 10;
const int DIM_COLS = 10;

// fills the matrix "to_stuff" with the pure version of the exemplar.
#define STUFF_MATRIX(to_stuff, max_row, max_col) \
  to_stuff.reset(max_row, max_col); \
  for (int r = 0; r < max_row; r++) \
    for (int c = 0; c < max_col; c++) \
      to_stuff.put(r, c, test_pure.get(r, c))

//////////////

// forward.
class my_int_matrix;

//////////////

// this class exhibits an old bug where the matrix was zeroing out its
// contents for a same size resize.  the zeroing allowed hell to spew forth.
class diggulite
{
public:
  diggulite() {}
  virtual ~diggulite() {}
};

//////////////

class test_matrix : virtual public unit_base, virtual public application_shell
{
public:
  test_matrix();

  DEFINE_CLASS_NAME("test_matrix");

  void log(const astring &to_print) { application_shell::log(to_print); }
    // override to avoid redoing all the logs here.

  int execute();
    // performs main body of test.

  static astring dump_matrix(const my_int_matrix &to_print);
    // creates a nice form of the matrix "to_print".

  void print_matrix(const my_int_matrix &to_print);
    // dumps "to_print" to the diagnostic output.

  void test_out_submatrix(const my_int_matrix &source);
    //!< runs some tests on the submatrix() function.

  void test_out_redimension();
    //!< checks out the redimension() method for resizing the array.

  void test_out_resizing_virtual_objects();
    //!< checks that a matrix of non-simple objects doesn't have bad problems.

  void test_out_zapping(const my_int_matrix &test_pure);
    //!< tries out zap operations.

  void test_out_inserting(const my_int_matrix &test_pure);
    //!< checks the insert row and column methods.
};

//////////////

class my_int_matrix : public int_matrix, virtual public hoople_standard
{
public:
  my_int_matrix(int r = 0, int c = 0) : int_matrix(r, c) {}
  my_int_matrix(const int_matrix &init) : int_matrix(init) {}

  DEFINE_CLASS_NAME("my_int_matrix");
  
  virtual bool equal_to(const equalizable &s2) const {
    const my_int_matrix *sec = dynamic_cast<const my_int_matrix *>(&s2);
    if (!sec) return false;
    if (rows() != sec->rows()) return false;
    if (columns() != sec->columns()) return false;
    for (int r = 0; r < this->rows(); r++)
      for (int c = 0; c < this->columns(); c++)
        if ((*this)[r][c] != (*sec)[r][c]) return false;
    return true;
  }

  virtual void text_form(base_string &state_fill) const {
    state_fill.assign(test_matrix::dump_matrix(*this));
  }
};

//////////////

test_matrix::test_matrix() : application_shell() {}

astring test_matrix::dump_matrix(const my_int_matrix &to_print)
{
  astring text;
  for (int t = 0; t < to_print.rows(); t++) {
    text += astring(astring::SPRINTF, "[%d] ", t);
    for (int c = 0; c < to_print.columns(); c++)
      text += astring(astring::SPRINTF, "%03d ", int(to_print[t][c]));
    text += parser_bits::platform_eol_to_chars();
  }
  return text;
}

void test_matrix::print_matrix(const my_int_matrix &to_print)
{ log(astring("\n") + dump_matrix(to_print)); }

void test_matrix::test_out_submatrix(const my_int_matrix &source)
{
  FUNCDEF("test_out_submatrix")
  my_int_matrix test2(source);

  for (int s = 0; s < DIM_ROWS; s++)
    for (int c = 0; c < DIM_COLS; c++)
      ASSERT_EQUAL(source[s][c], test2[s][c], "computed matrices should be same after copy");

#ifdef DEBUG_MATRIX
  log("before submatrix:");
  print_matrix(test2);
#endif
  my_int_matrix chunk(test2.submatrix(2, 3, 3, 2));
  my_int_matrix chunk_comparator(3, 2);
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 2; c++)
      chunk_comparator[r][c] = test2[r+2][c+3];
  ASSERT_EQUAL(chunk, chunk_comparator, "submatrix should grab proper contents");
#ifdef DEBUG_MATRIX
  log("after submatrix, chunk of the matrix has:");
  print_matrix(chunk);
#endif
}

void test_matrix::test_out_redimension()
{
  FUNCDEF("test_out_redimension")
  my_int_matrix computed(7, 14);
  for (int x1 = 0; x1 < 7; x1++) {
    for (int y1 = 0; y1 < 14; y1++) {
      if ( (x1 * y1) % 2) computed[x1][y1] = 1 + x1 * 100 + y1;
      else computed.put(x1, y1, 1 + x1 * 100 + y1);
    }
  }

  for (int x2 = 6; x2 >= 0; x2--) {
    for (int y2 = 13; y2 >= 0; y2--) {
      ASSERT_EQUAL(computed[x2][y2], 1 + x2 * 100 + y2,
          "computed matrix should have proper computed values");
    }
  }

  computed.redimension(3, 5);
  ASSERT_FALSE( (computed.rows() != 3) || (computed.columns() != 5),
      "redimension should not get size wrong");
  for (int x3 = 2; x3 >= 0; x3--) {
    for (int y3 = 4; y3 >= 0; y3--) {
      ASSERT_EQUAL(computed[x3][y3], 1 + x3 * 100 + y3,
          "computed matrix should still have right values");
    }
  }

  computed.redimension(0, 0);
  ASSERT_FALSE(computed.rows() || computed.columns(),
      "redimension to zero should see matrix as empty");

  computed.reset(12, 20);
  ASSERT_FALSE( (computed.rows() != 12) || (computed.columns() != 20),
      "resize should compute proper size");
}

void test_matrix::test_out_resizing_virtual_objects()
{
  FUNCDEF("test_out_resizing_virtual_objects")
  // this test block ensures that the matrix doesn't blow up from certain
  // resizing operations performed on a templated type that has a virtual
  // destructor.
  matrix<diggulite> grids;
  grids.reset();
  grids.redimension ( 0, 1 );
  grids.redimension ( 1, 1 );
  grids.reset(1, 1);
  ASSERT_TRUE(true, "no explosions should occur due to virtual contents");
}

void test_matrix::test_out_zapping(const my_int_matrix &test_pure)
{
  FUNCDEF("test_out_zapping")
  // this block tests the zapping ops.
  my_int_matrix test_zap;
  STUFF_MATRIX(test_zap, DIM_ROWS, DIM_COLS);

#ifdef DEBUG_MATRIX
  log("matrix before zappage:");
  print_matrix(test_zap);
#endif

  my_int_matrix compare_1 = test_zap;
  ASSERT_EQUAL(compare_1, test_zap, "assignment works right");
  test_zap.zap_row(5);
  // make same changes but with different ops so we can compare.
  for (int r = 6; r < DIM_ROWS; r++)
    for (int c = 0; c < DIM_COLS; c++)
      compare_1[r - 1][c] = compare_1[r][c];
  compare_1.zap_row(DIM_ROWS - 1);  // lose the last row now.
  ASSERT_EQUAL(compare_1, test_zap, "zapping should work regardless of path");

#ifdef DEBUG_MATRIX
  log("matrix after zappage of row 5:");
  print_matrix(test_zap);
#endif

  // reset the array again.
  STUFF_MATRIX(test_zap, DIM_ROWS, DIM_COLS);
  my_int_matrix compare_2 = test_zap;
  test_zap.zap_column(3);
  // now make those same changes in our compare array.
  for (int r = 0; r < DIM_ROWS; r++)
    for (int c = 4; c < DIM_COLS; c++)
      compare_2[r][c - 1] = compare_2[r][c];
  compare_2.zap_column(DIM_COLS - 1);  // lose the last row now.
  ASSERT_EQUAL(compare_2, test_zap, "second zapping should work regardless of path");

#ifdef DEBUG_MATRIX
  log("matrix after zappage of column 3:");
  print_matrix(test_zap);
#endif

  // reset test_zap again.
  STUFF_MATRIX(test_zap, DIM_ROWS, DIM_COLS);
  my_int_matrix compare_3(test_zap.submatrix(1, 1, DIM_ROWS - 2, DIM_COLS - 2));
  test_zap.zap_column(0);
  test_zap.zap_row(0);
  test_zap.zap_row(test_zap.rows() - 1);
  test_zap.zap_column(test_zap.columns() - 1);
  ASSERT_EQUAL(test_zap, compare_3, "zapping and submatrix should compute same result");

#ifdef DEBUG_MATRIX
  log("matrix after zap of row 0, col 0, last row, last col");
  print_matrix(test_zap);
#endif
}

void test_matrix::test_out_inserting(const my_int_matrix &test_pure)
{
  FUNCDEF("test_out_inserting")
  // this block tests the inserting ops.
  my_int_matrix test_insert;
  STUFF_MATRIX(test_insert, 4, 4);

#ifdef DEBUG_MATRIX
  log("matrix before inserting:");
  print_matrix(test_insert);
#endif

  my_int_matrix compare_1(test_insert);
  test_insert.insert_row(2);
  compare_1.insert_row(4);
  for (int r = 3; r >= 2; r--)
    for (int c = 0; c < 4; c++) 
      compare_1[r + 1][c] = compare_1[r][c];
  for (int c = 0; c < 4; c++)
    compare_1[2][c] = 0;
  ASSERT_EQUAL(test_insert, compare_1, "inserting row should create expected array");

#ifdef DEBUG_MATRIX
  log("matrix after insert of row 2:");
  print_matrix(test_insert);
#endif

  // reset test_insert again.
  STUFF_MATRIX(test_insert, 5, 6);

#ifdef DEBUG_MATRIX
  log("reset matrix before inserting:");
  print_matrix(test_insert);
#endif

  my_int_matrix compare_2(test_insert);
  test_insert.insert_column(3);
  compare_2.insert_column(6);
  for (int r = 0; r < 5; r++)
    for (int c = 5; c >= 3; c--) 
      compare_2[r][c + 1] = compare_2[r][c];
  for (int r = 0; r < 5; r++)
    compare_2[r][3] = 0;
  ASSERT_EQUAL(test_insert, compare_2, "inserting column should create expected array");

#ifdef DEBUG_MATRIX
  log("matrix after insert of column 3:");
  print_matrix(test_insert);
#endif

  // reset test_insert again.
  STUFF_MATRIX(test_insert, 3, 3);
  my_int_matrix compare_3(5, 5);
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++)
      compare_3[r + 1][c + 1] = test_insert[r][c];
  for (int r = 0; r < 5; r++) { compare_3[r][0] = 0; compare_3[r][4] = 0; }
  for (int c = 0; c < 5; c++) { compare_3[0][c] = 0; compare_3[4][c] = 0; }

#ifdef DEBUG_MATRIX
  log("matrix before inserting:");
  print_matrix(test_insert);
#endif

  test_insert.insert_column(0);

#ifdef DEBUG_MATRIX
  log("insert col at 0");
  print_matrix(test_insert);
#endif

  test_insert.insert_row(test_insert.rows());

#ifdef DEBUG_MATRIX
  log("insert row at rows()");
  print_matrix(test_insert);
#endif

  test_insert.insert_column(test_insert.columns());

#ifdef DEBUG_MATRIX
  log("insert col at cols()");
  print_matrix(test_insert);
  log("insert row at 0...");
#endif

  test_insert.insert_row(0);

  ASSERT_EQUAL(test_insert, compare_3,
      "inserting some rows and columns should create expected array");

#ifdef DEBUG_MATRIX
  log(astring("matrix after insert of col 0, last row, last col, row 0"));
  print_matrix(test_insert);
#endif
}

int test_matrix::execute()
{
  FUNCDEF("execute");

  my_int_matrix test_pure(DIM_ROWS, DIM_COLS);  // kept without modification.
  for (int r = 0; r < DIM_ROWS; r++)
    for (int c = 0; c < DIM_COLS; c++)
      test_pure[r][c] = r * DIM_COLS + c;

  my_int_matrix test1 = test_pure;  // first copy to work with.

  test1.reset();
  ASSERT_FALSE(test1.rows() || test1.columns(), "after reset matrix should be empty");

  test_out_submatrix(test_pure);

  test_out_redimension();

  test_out_resizing_virtual_objects();

  test_out_zapping(test_pure);

  test_out_inserting(test_pure);

  return final_report();
}

HOOPLE_MAIN(test_matrix, )

