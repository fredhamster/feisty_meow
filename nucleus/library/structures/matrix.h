#ifndef MATRIX_CLASS
#define MATRIX_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : matrix                                                            *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1993-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/array.h>
#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/guards.h>

namespace structures {

//! Represents a two-dimensional array of objects.
/*!
  The indices range from zero to (maximum_value - 1) for rows and columns.
*/

template <class contents>
class matrix : protected basis::array<contents>
{
public:
  matrix(int rows = 0, int cols = 0, contents *data = NIL);
    //!< the "data" array must have at least "rows" * "cols" contents in it.
  matrix(const matrix &to_copy);

  ~matrix() {}

  int rows() const { return _rows; }
  int columns() const { return _cols; }

  matrix &operator = (const matrix &to_copy);

  contents &get(int row, int column);
  const contents &get(int row, int column) const;
    //!< retrieves the contents from the specified "row" and "column".

  bool put(int row, int column, const contents &to_put);
    //!< stores "to_put" into the matrix at "row" and "column".

  contents *operator[] (int row);
    //!< dangerous: indexes by "row" into the underlying contents.
    /*!< the result can be used as a pointer to the whole row of data, or it
    can be indexed into by column to find a row/column position.  this is
    dangerous if one is not careful about ensuring that the column used is
    within bounds.  if the "row" parameter is out of bounds, then NIL is
    returned. */
  const contents *operator[] (int row) const;
    //!< dangerous: constant peek into data for "row" in underlying contents.

  matrix submatrix(int row, int column, int rows, int columns) const;
    //!< returns a submatrix of this one starting at "row" and "column".
    /*!< The returned matrix should contain "rows" rows and "columns" columns.
    if the size or position are out of bounds, an empty matrix is returned. */
  void submatrix(matrix &sub, int row, int column, int rows, int cols) const;
    //!< like submatrix() above, but stores into the "sub" parameter.

  matrix transpose() const;
    //!< provides the transposed form of this matrix.
  void transpose(matrix &resultant) const;
    //!< stores the transpose of this matrix in "resultant".

  basis::array<contents> get_row(int row);
    //!< return the vector at the "row", or an empty array if "row" is invalid.
  basis::array<contents> get_column(int column);
    //!< return the vector at the "column", or an empty array if it's invalid.

  bool insert_row(int before_row);
    //!< inserts a row before the "before_" parameter.
    /*!< all existing data is preserved in the matrix, although it may get
    moved over depending on where it's located with respect to "before_row". */
  bool insert_column(int before_column);
    //!< inserts a column before the "before_" parameter.

  void reset(int rows = 0, int columns = 0);
    //!< empties the matrix and changes its size.

  void redimension(int new_rows, int new_columns);
    //!< changes the size to contain "new_rows" by "new_columns" elements.
    /*!< data that was held previously stays in the array as long as its row
    and column indices are still valid. */

  bool zap_row(int row_to_zap);
  bool zap_column(int column_to_zap);
    //!< removes a row or column from the matrix.
    /*!< the matrix becomes one row or column less than before and all data
    from the deleted vector is lost. */

private:
  int _rows;  //!< number of rows in the matrix.
  int _cols;  //!< number of columns in the matrix.

  int compute_index(int row, int column) const;
    //!< returns the flat index that corresponds to the "row" and "column".
};

//////////////

//! A matrix of integers.
class int_matrix : public matrix<int>
{
public:
  int_matrix(int rows = 0, int cols = 0, int *data = NIL)
      : matrix<int>(rows, cols, data) {}
  int_matrix(const matrix<int> &to_copy) : matrix<int>(to_copy) {}
};

//! A matrix of strings.
class string_matrix : public matrix<basis::astring>
{
public:
  string_matrix(int rows = 0, int cols = 0, basis::astring *data = NIL)
      : matrix<basis::astring>(rows, cols, data) {}
  string_matrix(const matrix<basis::astring> &to_copy) : matrix<basis::astring>(to_copy) {}
};

//! A matrix of double floating point numbers.
class double_matrix : public matrix<double>
{
public:
  double_matrix(int rows = 0, int cols = 0, double *data = NIL)
      : matrix<double>(rows, cols, data) {}
  double_matrix(const matrix<double> &to_copy) : matrix<double>(to_copy) {}
};

//////////////

// implementation for longer methods...

//hmmm: the operations for zapping use extra memory.  they could easily
//      be done as in-place copies.

#undef static_class_name
#define static_class_name() "matrix"
  // used in bounds_halt macro.

template <class contents>
matrix<contents>::matrix(int r, int c, contents *dat)
: basis::array<contents>(r*c, dat), _rows(r), _cols(c) {}

template <class contents>
matrix<contents>::matrix(const matrix<contents> &to_copy)
: basis::array<contents>(0), _rows(0), _cols(0)
{ *this = to_copy; }

template <class contents>
matrix<contents> &matrix<contents>::operator = (const matrix &to_copy)
{
  if (&to_copy == this) return *this;
  basis::array<contents>::operator = (to_copy);
  _rows = to_copy._rows;
  _cols = to_copy._cols;
  return *this;
}

template <class contents>
contents *matrix<contents>::operator[] (int r)
{ return &basis::array<contents>::operator [] (compute_index(r, 0)); }

template <class contents>
const contents *matrix<contents>::operator[] (int r) const
{ return &basis::array<contents>::operator [] (compute_index(r, 0)); }

template <class contents>
const contents &matrix<contents>::get(int r, int c) const
{ return basis::array<contents>::get(compute_index(r, c)); }

template <class contents>
contents &matrix<contents>::get(int row, int column)
{ return basis::array<contents>::operator [] (compute_index(row, column)); }

template <class contents>
int matrix<contents>::compute_index(int row, int column) const
{ return column + row * _cols; }

template <class contents>
void matrix<contents>::reset(int rows_in, int columns_in)
{
  if ( (_rows == rows_in) && (_cols == columns_in) ) {
    // reuse space, but reset the objects to their initial state.
    for (int i = 0; i < basis::array<contents>::length(); i++)
      basis::array<contents>::operator [](i) = contents();
    return;
  }

  _rows = 0;
  _cols = 0;
  basis::array<contents>::reset(0);

  this->insert(0, rows_in * columns_in);
  _rows = rows_in;
  _cols = columns_in;
}

template <class contents>
void matrix<contents>::redimension(int new_rows, int new_columns)
{
  if ( (_rows == new_rows) && (_cols == new_columns) ) return;
  matrix<contents> new_this(new_rows, new_columns);
  for (int r = 0; r < minimum(new_rows, rows()); r++)
    for (int c = 0; c < minimum(new_columns, columns()); c++)
      new_this[r][c] = (*this)[r][c];
  *this = new_this;
}

template <class contents>
bool matrix<contents>::put(int row, int column, const contents &to_put)
{
  if ( (row >= rows()) || (column >= columns()) )
    return false;
  (operator [](row))[column] = to_put;
  return true;
}

template <class contents>
matrix<contents> matrix<contents>::submatrix(int row, int column, int rows_in,
    int columns_in) const
{
  matrix<contents> to_return;
  submatrix(to_return, row, column, rows_in, columns_in);
  return to_return;
}

template <class contents>
void matrix<contents>::submatrix(matrix<contents> &sub, int row, int column,
    int rows_in, int columns_in) const
{
  sub.reset();
  if ( (row >= rows()) || (row + rows_in >= rows()) ) return;
  if ( (column >= columns()) || (column + columns_in >= columns()) ) return;
  sub.reset(rows_in, columns_in);
  for (int r = row; r < row + rows_in; r++)
    for (int c = column; c < column + columns_in; c++)
      sub[r - row][c - column] = (*this)[r][c];
}

template <class contents>
matrix<contents> matrix<contents>::transpose() const
{
  matrix<contents> to_return;
  transpose(to_return);
  return to_return;
}

template <class contents>
void matrix<contents>::transpose(matrix<contents> &resultant) const
{
  resultant.reset(columns(), rows());
  for (int i = 0; i < rows(); i++)
    for (int j = 0; j < columns(); j++)
      resultant[j][i] = (*this)[i][j];
}

template <class contents>
basis::array<contents> matrix<contents>::get_row(int row)
{
  basis::array<contents> to_return;
  if (row >= rows()) return to_return;
  to_return.reset(columns());
  for (int i = 0; i < columns(); i++)
    to_return[i] = get(row, i);
  return to_return;
}

template <class contents>
basis::array<contents> matrix<contents>::get_column(int column)
{
  basis::array<contents> to_return;
  if (column >= columns()) return to_return;
  to_return.reset(rows());
  for (int i = 0; i < rows(); i++)
    to_return[i] = get(i, column);
  return to_return;
}

template <class contents>
bool matrix<contents>::zap_row(int row_to_zap)
{
  FUNCDEF("zap_row");
  bounds_halt(row_to_zap, 0, rows() - 1, false);
  const int start = compute_index(row_to_zap, 0);
  // this is only safe because the indices are stored in row-major order (which
  // i hope means the right thing).  in any case, the order is like so:
  //   1 2 3 4
  //   5 6 7 8
  // thus we can whack a whole row contiguously.
  basis::array<contents>::zap(start, start + columns() - 1);
  _rows--;
  return true;
}

template <class contents>
bool matrix<contents>::zap_column(int column_to_zap)
{
  FUNCDEF("zap_column");
  bounds_halt(column_to_zap, 0, columns() - 1, false);
  // this starts at the end, which keeps the indexes meaningful.  otherwise
  // the destruction interferes with finding the elements.
  for (int r = rows(); r >= 0; r--) {
    const int loc = compute_index(r, column_to_zap);
    basis::array<contents>::zap(loc, loc);
  }
  _cols--;
  return true;
}

template <class contents>
bool matrix<contents>::insert_row(int position)
{
  FUNCDEF("insert_row");
  bounds_halt(position, 0, rows(), false);
  // see comment in zap_row for reasoning about the below.
  basis::array<contents>::insert(compute_index(position, 0), columns());
  _rows++;
  // clear out those spaces.
  for (int c = 0; c < columns(); c++)
    put(position, c, contents());
  return true;
}

template <class contents>
bool matrix<contents>::insert_column(int position)
{
  FUNCDEF("insert_column");
  bounds_halt(position, 0, columns(), false);
  // similarly to zap_column, we must iterate in reverse.
  for (int r = rows(); r >= 0; r--)
    basis::array<contents>::insert(compute_index(r, position), 1);
  _cols++;
  // clear out those spaces.
  for (int r = 0; r < rows(); r++) 
    put(r, position, contents());
  return true;
}

#undef static_class_name

} //namespace.

#endif

