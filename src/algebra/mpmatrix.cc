/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   mpmatrix.cc
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   March 23, 2009
 *
 *  Function        :   MaxPlus vectors and matrices
 *
 *  History         :
 *      23-03-09    :   Initial version.
 *
 *
 *  Copyright 2023 Eindhoven University of Technology
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the “Software”),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "algebra/mpmatrix.h"
#include "algebra/mptype.h"
#include "base/analysis/mcm/mcmgraph.h"
#include "base/analysis/mcm/mcmyto.h"
#include "base/exception/exception.h"
#include <cmath>
#include <cstdlib>
#include <memory>

using namespace Graphs;

#define DEFAULT_SCALE 1.0e-06

namespace MaxPlus {

/**
 * Construct a max-plus vector of size
 */
Vector::Vector(unsigned int size, MPTime value) {
    this->table.resize(size);
    for (unsigned int k = 0; k < size; k++) {
        this->table[k] = value;
    }
}

/**
 * Construct a max-plus vector from an std:vector
 */
Vector::Vector(std::vector<MPTime> *v) {
    this->table.resize(v->size());
    for (unsigned int i = 0; i < v->size(); i++) {
        this->table[i] = v->at(i);
    }
}

/**
 * copy constructor
 */
Vector::Vector(const Vector &other) {
    this->table.resize(other.getSize());
    for (unsigned int row = 0; row < this->getSize(); row++) {
        this->table[row] = other.table[row];
    }
}

/**
 * vector assignment
 */
Vector &Vector::operator=(const Vector &other) {
    // self assignment check
    if (this != &other) {
        if (this->getSize() != other.getSize()) {
            throw CException("Vectors of different size in"
                             "Vector::operator=");
        }
        for (unsigned int row = 0; row < this->getSize(); row++) {
            this->table[row] = other.table[row];
        }
    }
    return *this;
}

/**
 * vector negate
 */
void Vector::negate() {
    for (unsigned int row = 0; row < this->getSize(); row++) {
        if (this->get(row) == MP_MINUSINFINITY) {
            throw CException("Cannot negate vectors with MP_MINUSINFINITY elements in"
                             "Vector::negate");
        }
        this->put(row, -this->get(row));
    }
}

/**
 * randomize
 */
void Vector::randomize(unsigned int max) {
    unsigned int M = this->getSize();

    for (unsigned int pos = 0; pos < M; pos++) {
        MPTime r = (MPTime)(rand() % 2);
        if (r == (MPTime)0) {
            put(pos, r);
        } else {
            r = (MPTime)(rand() % max);
            put(pos, r);
        }
    }
}

/**
 * LaTex String representation of vector
 */
void Vector::toLaTeXString(CString &outString, double scale) const {
    outString = "";
    unsigned int MR = this->getSize();
    unsigned int MC = 1;
    outString += "\\begin{bmatrix}\n";
    for (unsigned int i = 0; i < MR; i++) {
        for (unsigned int j = 0; j < MC; j++) {
            outString += timeToLaTeXString(this->get(i) * scale) + " ";
            if (j < MC - 1) {
                outString += "&";
            } else {
                outString += "\\\\";
            }
        }
        outString += "\n";
    }
    outString += "\\end{bmatrix}\n";
}

/**
 * Increases the number of cols of the matrix by n and fills the new elements with -\infty.
 */
void Matrix::addCols(uint n) {
    unsigned int rows = this->getRows();
    unsigned int cols = this->getCols();
    this->szCols = this->szCols + n;
    // unsigned int nels = this->getRows() * this->getCols();
    // this->table.resize(nels);
    std::vector<MPTime>::iterator it = this->table.begin() + cols;
    for (unsigned int r = 0; r < rows; r++) {
        for (unsigned int c = 0; c < n; c++) {
            it = this->table.insert(it, MP_MINUSINFINITY);
            // advance(it, 1);
        }
        if (r < rows - 1)
            advance(it, cols + n);
    }
}

/**
 *  randomizes all elements of an existing matrix
 */
void Matrix::randomize(unsigned int max) {
    for (unsigned int rowIndex = 0; rowIndex < getRows(); rowIndex++) {
        for (unsigned int colIndex = 0; colIndex < getCols(); colIndex++) {

            MPTime r = (MPTime)(rand() % 2);
            if (r == (MPTime)0) {
                this->put(rowIndex, colIndex, MP_MINUSINFINITY);
            } else {
                r = (MPTime)(rand() % max);
                this->put(rowIndex, colIndex, r);
            }
        }
    }
}
 

/**
 * Paste rowvector into matrix
 */
void Matrix::pasteRowVector(unsigned int top_row,
                            unsigned int left_column,
                            const Vector *pastedVector) {
    const unsigned int p_csz = pastedVector->getSize();
    const unsigned int right_column = left_column + p_csz - 1;
    assert(right_column < this->getCols());
    for (unsigned int col = left_column; col <= right_column; col++) {
        this->put(top_row, col, pastedVector->get(col));
    }
}

/**
 * Make submatrix with indices in list for non-square matrix. The new matrix only keeps the rows of
 * the original matrix with the selected indices.
 */
Matrix *Matrix::getSubMatrixNonSquareRows(const std::list<unsigned int> &rowIndices) const {
    unsigned int NR = (unsigned int)rowIndices.size();
    Matrix *newMatrix = new Matrix(NR, this->getCols());

    std::list<unsigned int>::const_iterator cit = rowIndices.begin();
    for (unsigned int r = 0; r < NR; r++, cit++) {
        unsigned int ci = (*cit);
        for (unsigned int c = 0; c < this->getCols(); c++) {
            newMatrix->put(r, c, this->get(ci, c));
        }
    }
    return newMatrix;
}

/**
 * returns the largest finite element of a col
 */
MPTime Matrix::getMaxOfCol(uint colNumber) const {
    MPTime largestEl(0), largestMag(0);
    if (colNumber > this->getCols())
        throw CException("Matrix getMaxOfRow input index out of bounds.");
    unsigned int MR = this->getRows();

    for (unsigned int r = 0; r < MR; r++) {
        MPTime t = this->get(r, colNumber);
        if (t <= MP_MINUSINFINITY)
            continue;

        MPTime mag (fabs((double)t));
        if (mag > largestMag) {
            largestEl = t;
            largestMag = mag;
        }
    }

    return largestEl;
}

/**
 * returns the largest finite element of a col until row
 */
MPTime Matrix::getMaxOfColUntilRow(uint colNumber, uint rowNumber) const {
    MPTime largestEl(0), largestMag  (0);
    if (colNumber > this->getCols())
        throw CException("Matrix getMaxOfRow input index out of bounds.");

    for (unsigned int r = 0; r < rowNumber; r++) {
        MPTime t = this->get(r, colNumber);
        if (t <= MP_MINUSINFINITY)
            continue;

        MPTime mag(fabs((double)t));
        if (mag > largestMag) {
            largestEl = t;
            largestMag = mag;
        }
    }

    return largestEl;
}
/**
 * returns the largest finite element of a row
 */
MPTime Matrix::getMaxOfRow(uint rowNumber) const {
    MPTime largestEl(0), largestMag(0);
    if (rowNumber > this->getRows())
        throw CException("Matrix getMaxOfRow input index out of bounds.");
    unsigned int MC = this->getCols();

    for (unsigned int c = 0; c < MC; c++) {
        MPTime t = this->get(rowNumber, c);
        if (t <= MP_MINUSINFINITY)
            continue;

        MPTime mag(fabs((double)t));
        if (mag > largestMag) {
            largestEl = t;
            largestMag = mag;
        }
    }

    return largestEl;
}

/**
 * returns the largest finite element of a row up to and including colNumber
 */
MPTime Matrix::getMaxOfRowUntilCol(uint rowNumber, uint colNumber) const {
    MPTime largestEl(0), largestMag(0);
    if (rowNumber > this->getRows())
        throw CException("Matrix getMaxOfRow input row index out of bounds.");
    if (colNumber > this->getCols())
        throw CException("Matrix getMaxOfRowUntilCol input col index out of bounds.");
    for (unsigned int c = 0; c < colNumber; c++) {
        MPTime t = this->get(rowNumber, c);
        if (t <= MP_MINUSINFINITY)
            continue;

        MPTime mag(fabs((double)t));
        if (mag > largestMag) {
            largestEl = t;
            largestMag = mag;
        }
    }
    return largestEl;
}

/**
 * calculate vector norm
 */
MPTime Vector::norm() const {
    MPTime maxEl = MP_MINUSINFINITY;
    for (unsigned int row = 0; row < this->getSize(); row++) {
        maxEl = MP_MAX(maxEl, this->get(row));
    }
    return maxEl;
}

/**
 * normalize vector
 */
MPTime Vector::normalize() {
    MPTime maxEl = this->norm();

    if (maxEl == MP_MINUSINFINITY) {
        throw CException("Cannot normalize vector with norm MP_MINUSINFINITY"
                         "Vector::normalize");
    }
    for (unsigned int row = 0; row < this->getSize(); row++) {
        MPTime x_i = this->get(row); // MPTime handles -INF correctly
        x_i = x_i - maxEl;           // overloaded using MP_PLUS
        this->put(row, x_i);
    }

    return maxEl;
}

/**
 * add scalar to vector
 */
Vector Vector::add(MPTime increase) const {
    unsigned int M = this->getSize();
    Vector result(M);
    this->add(increase, result);
    return result;
}

/**
 * add scalar to vector
 */
void Vector::add(MPTime increase, Vector &result) const {
    unsigned int M = this->getSize();
    assert(result.getSize() == M);

    for (unsigned int pos = 0; pos < M; pos++) {
        result.put(pos, this->get(pos) + increase); // uses MP_PLUS()
    }
}

/**
 * max of vectors
 */
void Vector::maximum(const Vector &vecB, Vector &result) const {
    unsigned int M = this->getSize();
    assert(vecB.getSize() == M);
    assert(result.getSize() == M);

    for (unsigned int pos = 0; pos < M; pos++) {
        result.put(pos, MP_MAX(this->get(pos), vecB.get(pos)));
    }
}

/**
 * Destructor of max-plus vector
 */
Vector::~Vector() = default;

/**
 * Put an entry into the vector. Grows vector if necessary
 */
void Vector::put(unsigned int row, MPTime value) {
    if (this->table.size() <= row) {
        this->table.resize(row + 1);
    }
    this->table[row] = value;
}

/**
 * String representation of vector
 */
void Vector::toString(CString &outString, CDouble scale) const {
    outString = "";
    for (unsigned int row = 0; row < this->getSize(); row++) {
        outString += timeToString(scale * this->get(row)) + " ";
    }
}

/**
 * Vector::add
 * add vectors
 */
void Vector::add(const Vector &vecB, Vector &res) const {
    assert(this->getSize() == vecB.getSize());
    assert(this->getSize() == res.getSize());
    for (unsigned int row = 0; row < this->getSize(); row++) {
        res.put(row, this->get(row) + vecB.get(row));
    }
}

/**
 * add vectors
 */

Vector Vector::add(const Vector &vecB) const {
    Vector res(this->getSize());
    this->add(vecB, res);
    return res;
}

/**
 * Get minimal finite element
 * returns the smallest among the finite elements in the vector or
 * MP_MINUSINFINITY if no finite elements exist
 * itsPosition returns the index of the (a) smallest finite element is set
 * to a pointer to unsigned int, otherwise set or defaults to nullptr
 */
MPTime Vector::minimalFiniteElement(unsigned int *itsPosition_Ptr) const {
    unsigned int itsPosition_tmp = 0;
    unsigned int *itsPosition = nullptr;
    if (itsPosition_Ptr != nullptr) {
        itsPosition = itsPosition_Ptr;
    } else {
        itsPosition = &itsPosition_tmp;
    }
    *itsPosition = this->getSize() + 1; // arbitrary value, invalid

    MPTime minEl = MP_MINUSINFINITY;
    bool minEl_initialized = false;
    for (unsigned int row = 0; row < this->getSize(); row++) {
        MPTime val = this->get(row);
        if (val.isMinusInfinity()) {
            continue;
        }
        if (!minEl_initialized || (val < minEl)) {
            minEl = val;
            minEl_initialized = true;
            *itsPosition = row;
        }
    }
    return minEl;
}

/**
 * Compare vectors up to MP_EPSILON
 */
bool Vector::compare(const Vector &v) const {
    if (this->getSize() != v.getSize()) {
        return false;
    }
    for (unsigned int k = 0; k < this->getSize(); k++) {
        if (fabs(static_cast<CDouble>(this->get(k) - v.get(k)))
            > static_cast<CDouble>(MP_EPSILON)) {
            return false;
        }
    }
    return true;
}

Matrix::Matrix(unsigned int nr_rows, unsigned int nr_cols, MatrixFill fill) :
    szRows(nr_rows), szCols(nr_cols) {

    init(fill);
}

/**
 * initialize matrix
 */
void Matrix::init(MatrixFill fill) {
    // Generate a table given the number of rows and columns.
    unsigned int nr_els = getRows() * getCols();
    this->table.resize(nr_els);
    // Fill the matrix according to the given fill pattern.
    auto zeroValue = MPTime(0.0);
    switch (fill) {
    case MatrixFill::MinusInfinity:
        for (unsigned int pos = 0; pos < nr_els; pos++) {
            this->table[pos] = MP_MINUSINFINITY;
        }
        break;
    case MatrixFill::Zero:
        for (unsigned int pos = 0; pos < nr_els; pos++) {
            this->table[pos] = zeroValue;
        }
        break;
    case MatrixFill::Identity:
        for (unsigned int rowIndex = 0; rowIndex < getRows(); rowIndex++) {
            for (unsigned int colIndex = 0; colIndex < getCols(); colIndex++) {
                if (rowIndex == colIndex) {
                    put(rowIndex, colIndex, zeroValue);
                } else {
                    put(rowIndex, colIndex, MP_MINUSINFINITY);
                }
            }
        }
        break;
    default:
        break;
    }
}

/**
 * Initialize matrix
 */
void Matrix::init() { init(MatrixFill::MinusInfinity); }

/**
 * Construct a square max-plus matrix of N by N
 */
Matrix::Matrix(unsigned int N) : szRows(N), szCols(N) { this->init(); }

/**
 *Creates a matrix with reserved memory
 */
Matrix::Matrix(unsigned int nr_rows, unsigned int nr_cols, unsigned int nr_el) :
    szRows(nr_rows), szCols(nr_cols) {

    this->table.reserve(nr_el);
    this->init();
}

/**
 * Destructor of MaxPlus matrix
 */
Matrix::~Matrix() = default;

/**
 * Increases the number of rows of the matrix by n and fills the new elements
 * with -\infty.
 */
void Matrix::addRows(uint n) {
    this->szRows = this->szRows + n;
    unsigned int nr_els = this->getRows() * this->getCols();
    this->table.resize(nr_els);
    for (unsigned int pos = nr_els - n * this->getCols(); pos < nr_els; pos++) {
        this->table[pos] = MP_MINUSINFINITY;
    }
}

/**
 * Get size of a square matrix
 */
unsigned int Matrix::getSize() const {
    assert(this->getRows() == this->getCols());
    return this->getRows();
}

/**
 * Get an entry from the matrix. Row and column index must be between 0 and size-1
 */
MPTime Matrix::get(unsigned int row, unsigned int column) const {
    if ((row >= this->getRows()) || (column >= this->getCols())) {
        throw CException("Index out of bounds in"
                         "Matrix::get");
    }
    return this->table[row * this->getCols() + column];
}

/**
 * Get a row from the matrix as a vector. Row index must be between 0 and row
 * size-1
 */
Vector Matrix::getRowVector(unsigned int row) const {
    unsigned int sz = this->getCols();
    Vector result(sz);
    for (unsigned int i = 0; i < sz; i++) {
        result.put(i, this->get(row, i));
    }
    return result;
}

/**
 * Put a value in the matrix. Row and column index must be between 0 and size-1
 */
void Matrix::put(unsigned int row, unsigned int column, MPTime value) {
    if ((row >= this->getRows()) || (column >= this->getCols())) {
        throw CException("Index out of bounds in"
                         "Matrix::put");
    }
    this->table[row * this->getCols() + column] = value;
}

/**
 * Paste sub matrix into matrix
 */
void Matrix::paste(unsigned int top_row, unsigned int left_column, const Matrix *pastedMatrix) {
    const unsigned int p_rsz = pastedMatrix->getRows();
    const unsigned int p_csz = pastedMatrix->getCols();
    const unsigned int bottom_row = top_row + p_rsz - 1;
    const unsigned int right_column = left_column + p_csz - 1;
    assert(bottom_row < this->getRows());
    assert(right_column < this->getCols());
    unsigned int p_row = 0;
    unsigned int p_col = 0;

    for (unsigned int row = top_row; row <= bottom_row; row++, p_row++) {
        p_col = 0;
        for (unsigned int col = left_column; col <= right_column; col++, p_col++) {
            this->put(row, col, pastedMatrix->get(p_row, p_col));
        }
    }
}

/**
 * Paste row vector into matrix
 */
void Matrix::pasteRowVector(unsigned int top_row,
                            unsigned int left_column,
                            const Vector *pastedVector) {
    const unsigned int p_csz = pastedVector->getSize();
    const unsigned int right_column = left_column + p_csz - 1;
    assert(right_column < this->getCols());
    for (unsigned int col = left_column; col <= right_column; col++) {
        this->put(top_row, col, pastedVector->get(col));
    }
}

/**
 * mp_multiply()
 * Matrix-vector multiplication.
 */
Vector Matrix::mp_multiply(const Vector &v) const {
    // Check size of the matrix and vector
    if (this->getCols() != v.getSize()) {
        throw CException("Matrix and vector are of unequal size in "
                         "Matrix::mp_multiply");
    }

    // Allocate space of the resulting vector
    Vector res(this->getRows());

    // Perform point-wise multiplication
    for (unsigned int i = 0; i < this->getRows(); i++) {
        MPTime m = MP_MINUSINFINITY;
        for (unsigned int k = 0; k < this->getCols(); k++) {
            m = MP_MAX(m, MP_PLUS(this->get(i, k), v.get(k)));
        }
        res.put(i, m);
    }
    return res;
}

/**
 * mp_multiply()
 * Matrix-matrix multiplication.
 */
Matrix Matrix::mp_multiply(const Matrix &m) const {
    // Check sizes of the matrices
    if (this->getCols() != m.getRows()) {
        throw CException("Matrices are of incompatible size in"
                         "Matrix::mp_multiply(Matrix)");
    }

    // Allocate space of the resulting matrix
    Matrix res(this->getRows(), m.getCols());

    // Perform point-wise multiplication
    for (unsigned int i = 0; i < this->getRows(); i++) {
        for (unsigned int j = 0; j < m.getCols(); j++) {
            MPTime mpt = MP_MINUSINFINITY;
            for (unsigned int k = 0; k < this->getCols(); k++) {
                mpt = MP_MAX(mpt, MP_PLUS(this->get(i, k), m.get(k, j)));
            }
            res.put(i, j, mpt);
        }
    }
    return res;
}

/**
 * mp_sub()
 * Matrix-matrix subtraction.
 */
Matrix Matrix::mp_sub(const Matrix &m) const {
    // Check sizes of the matrices
    if ((m.getRows() != this->getRows()) || (m.getCols() != this->getCols())) {
        throw CException("Matrices are of different size in"
                         "Matrix::mp_sub(Matrix&");
    }

    // Allocate space of the resulting matrix
    Matrix res(this->getRows(), this->getCols());

    // Perform element-wise subtraction
    for (unsigned int i = 0; i < this->getRows(); i++) {
        for (unsigned int j = 0; j < this->getCols(); j++) {
            res.put(i, j, this->get(i, j) - m.get(i, j));
        }
    }
    return res;
}

/**
 * mp_maximum()
 * Matrix-matrix maximization.
 */
Matrix Matrix::mp_maximum(const Matrix &m) const {
    // Check sizes of the matrices
    if ((m.getRows() != this->getRows()) || (m.getCols() != this->getCols())) {
        throw CException("Matrices are of different size in"
                         "Matrix::maximum(Matrix*, Matrix*");
    }

    // Allocate space of the resulting matrix
    Matrix res(this->getRows(), this->getCols());

    // Perform element-wise subtraction
    for (unsigned int i = 0; i < this->getRows(); i++) {
        for (unsigned int j = 0; j < this->getCols(); j++) {
            res.put(i, j, MP_MAX(this->get(i, j), m.get(i, j)));
        }
    }
    return res;
}
/**
 * mp_power()
 * Raise matrix to a positive integer power >= 1.
 */
Matrix Matrix::mp_power(const unsigned int p) const {

    // base case p==1
    if (p == 1) {
        return *this;
    }

    // check if p is odd
    if ((p % 2) == 1) {
        Matrix m_pow = this->mp_power(p - 1);
        return this->mp_multiply(m_pow);
    }
    //  p is even
    Matrix m_pow = this->mp_power(p / 2);
    return m_pow.mp_multiply(m_pow);
}

Matrix Matrix::transpose() const {
    unsigned int MR = this->getCols();
    unsigned int MC = this->getRows();
    Matrix newMatrix(MR, MC);
    for (unsigned int col = 0; col < MC; col++) {
        for (unsigned int row = 0; row < MR; row++) {
            newMatrix.put(row, col, this->get(col, row));
        }
    }
    return newMatrix;
}

/**
 * Make sub matrix with indices in list.
 */
Matrix Matrix::getSubMatrix(const std::list<unsigned int> &rowIndices,
                            const std::list<unsigned int> &colIndices) const {
    auto NR = static_cast<unsigned int>(rowIndices.size());
    auto NC = static_cast<unsigned int>(colIndices.size());
    Matrix newMatrix(NR, NC);

    auto rit = rowIndices.begin();
    for (unsigned int r = 0; r < NR; r++, rit++) {
        unsigned int ri = (*rit);
        auto cit = colIndices.begin();
        for (unsigned int c = 0; c < NC; c++, cit++) {
            unsigned int ci = (*cit);
            newMatrix.put(r, c, this->get(ri, ci));
        }
    }
    return newMatrix;
}

std::shared_ptr<Matrix> Matrix::getSubMatrixPtr(const std::list<unsigned int> &rowIndices,
                                                const std::list<unsigned int> &colIndices) const {
    auto NR = static_cast<unsigned int>(rowIndices.size());
    auto NC = static_cast<unsigned int>(colIndices.size());
    auto newMatrix = std::make_shared<Matrix>(NR, NC);

    auto rit = rowIndices.begin();
    for (unsigned int r = 0; r < NR; r++, rit++) {
        unsigned int ri = (*rit);
        auto cit = colIndices.begin();
        for (unsigned int c = 0; c < NC; c++, cit++) {
            unsigned int ci = (*cit);
            newMatrix->put(r, c, this->get(ri, ci));
        }
    }
    return newMatrix;
}

/**
 * Make sub matrix with indices in list from square matrix
 */
Matrix Matrix::getSubMatrix(const std::list<unsigned int> &indices) const {
    assert(this->getRows() == this->getCols());
    return this->getSubMatrix(indices, indices);
}

std::shared_ptr<Matrix> Matrix::getSubMatrixPtr(const std::list<unsigned int> &indices) const {
    assert(this->getRows() == this->getCols());
    return this->getSubMatrixPtr(indices, indices);
}

/**
 * Make sub matrix with indices in list for non-square matrix. The new matrix
 * only keeps the columns of the original matrix with the selected indices.
 */
Matrix Matrix::getSubMatrixNonSquare(const std::list<unsigned int> &colIndices) const {
    auto NC = static_cast<unsigned int>(colIndices.size());
    Matrix newMatrix(this->getRows(), NC);

    for (unsigned int r = 0; r < this->getRows(); r++) {
        auto cit = colIndices.begin();
        for (unsigned int c = 0; c < NC; c++, cit++) {
            unsigned int ci = (*cit);
            newMatrix.put(r, c, this->get(r, ci));
        }
    }
    return newMatrix;
}

/**
 * Matrix addition of scalar.
 */
Matrix Matrix::add(MPTime increase) const {
    unsigned int MR = this->getRows();
    unsigned int MC = this->getCols();
    Matrix result(MR, MC);

    this->add(increase, result);
    return result;
}

/**
 * Matrix addition of scalar with existing result matrix.
 */
void Matrix::add(MPTime increase, Matrix &result) const {
    unsigned int MR = this->getRows();
    unsigned int MC = this->getCols();
    if ((MR != result.getRows()) || (MC != result.getCols())) {
        throw CException("Matrices are of different size in"
                         "Matrix::add(Matrix*, MPTime, Matrix*");
    }
    for (unsigned int r = 0; r < MR; r++) {
        for (unsigned int c = 0; c < MC; c++) {
            result.put(r, c, this->get(r, c) + increase); // uses MP_PLUS()
        }
    }
}

/**
 * Matrix maximum with existing result matrix.
 */
void Matrix::maximum(const Matrix &matB, Matrix &result) const {
    unsigned int MR = this->getRows();
    unsigned int MC = this->getCols();
    if ((matB.getRows() != MR) || (matB.getCols() != MC) || (result.getRows() != MR)
        || (result.getCols() != MC)) {
        throw CException("Matrices are of different size in"
                         "Matrix::maximum(Matrix*, Matrix*, Matrix*");
    }

    for (unsigned int r = 0; r < MR; r++) {
        for (unsigned int c = 0; c < MC; c++) {
            result.put(r, c, MP_MAX(this->get(r, c), matB.get(r, c)));
        }
    }
}

/**
 * Matrix to string.
 */
void Matrix::toString(CString &outString, CDouble scale) const {
    outString = "";
    unsigned int MR = this->getRows();
    unsigned int MC = this->getCols();
    for (unsigned int i = 0; i < MR; i++) {
        for (unsigned int j = 0; j < MC; j++) {
            outString += timeToString(this->get(i, j) * scale) + " ";
        }
        outString += "\n";
    }
}

/**
 * Matrix to string.
 */
void Matrix::toMatlabString(CString &outString, CDouble scale) const {
    outString = "";
    unsigned int MR = this->getRows();
    unsigned int MC = this->getCols();
    outString += "[\n";
    for (unsigned int i = 0; i < MR; i++) {
        for (unsigned int j = 0; j < MC; j++) {
            outString += timeToMatlabString(this->get(i, j) * scale) + " ";
        }
        if (i < MR - 1) {
            outString += ";\n";
        } else {
            outString += "\n";
        }
    }
    outString += "]\n";
}

/**
 * Matrix to LaTex string.
 */
void Matrix::toLaTeXString(CString &outString, CDouble scale) const {
    outString = "";
    unsigned int MR = this->getRows();
    unsigned int MC = this->getCols();
    outString += "\\begin{bmatrix}\n";
    for (unsigned int i = 0; i < MR; i++) {
        for (unsigned int j = 0; j < MC; j++) {
            outString += timeToLaTeXString(this->get(i, j) * scale) + " ";
            if (j < MC - 1) {
                outString += "&";
            } else {
                outString += "\\\\";
            }
        }
        outString += "\n";
    }
    outString += "\\end{bmatrix}\n";
}

/**
 * Matrix return largest element.
 */
MPTime Matrix::largestFiniteElement() const {
    // finite element with the largest absolute value
    // all -INF => 0
    //
    auto largestEl = MPTime(0.0);
    auto largestMag = MPTime(0.0);
    unsigned int MR = this->getRows();
    unsigned int MC = this->getCols();

    for (unsigned int r = 0; r < MR; r++) {
        for (unsigned int c = 0; c < MC; c++) {
            if (this->get(r, c) == MP_MINUSINFINITY) {
                continue;
            }

            MPTime mag = this->get(r, c).fabs();
            if (mag > largestMag) {
                largestEl = this->get(r, c);
                largestMag = mag;
            }
        }
    }

    return largestEl;
}

/**
 * Matrix get minimal element
 */
MPTime Matrix::minimalFiniteElement() const {
    // smallest finite element
    // all -INF => -INF
    unsigned int MR = this->getRows();
    unsigned int MC = this->getCols();

    if (MR == 0 || MC == 0) {
        return MP_MINUSINFINITY;
    }

    MPTime minimalEl = MP_MINUSINFINITY;

    for (unsigned int r = 0; r < MR; r++) {
        for (unsigned int c = 0; c < MC; c++) {
            if (this->get(r, c) == MP_MINUSINFINITY) {
                continue;
            }

            if (minimalEl == MP_MINUSINFINITY) {
                minimalEl = this->get(r, c);
            } else {
                minimalEl = MP_MIN(minimalEl, this->get(r, c));
            }
        }
    }

    return minimalEl;
}

/**
 * Matrix plus closure.
 */
Matrix Matrix::plusClosureMatrix(MPTime posCycleThreshold) const {
    // notation: A^(+) = max(A, A^2, ...)
    return Matrix::allPairLongestPathMatrix(posCycleThreshold, false /*implyZeroSelfEdges*/);
}

/**
 * Matrix star closure.
 */
Matrix Matrix::starClosureMatrix(MPTime posCycleThreshold) const {
    // notation: A^(*) = max(E,A,A^2,...)
    // E - diagonal matrix with 'e'==0 on the diagonal

    return Matrix::allPairLongestPathMatrix(posCycleThreshold, true /*implyZeroSelfEdges*/);
}

/**
 * Matrix all pair longest path.
 */
Matrix Matrix::allPairLongestPathMatrix(MPTime posCycleThreshold, bool implyZeroSelfEdges) const {
    // Floyd-Warshall algorithm
    if (this->getRows() != this->getCols()) {
        throw CException("Matrix must be square in Matrix::allPaiLongestPathMatrix.");
    }
    unsigned int N = this->getRows();

    Matrix distMat = *this;

    // k - intermediate node
    for (unsigned int k = 0; k < N; k++) {
        for (unsigned int u = 0; u < N; u++) {
            for (unsigned int v = 0; v < N; v++) {
                MPTime extra = (implyZeroSelfEdges && u == v) ? MPTime(0) : MP_MINUSINFINITY;
                MPTime path_u2v = MP_MAX(distMat.get(v, u), extra);
                MPTime path_u2k = distMat.get(k, u);
                MPTime path_k2v = distMat.get(v, k);

                MPTime path_u2v_candidate = (path_u2k + path_k2v); // uses MP_PLUS()
                if (path_u2v_candidate > path_u2v) {
                    path_u2v = path_u2v_candidate;
                }
                distMat.put(v, u, path_u2v);
            }
        }
    }

    for (unsigned int k = 0; k < N; k++) {
        if (distMat.get(k, k) > posCycleThreshold) {
            CString tmp;
            distMat.toString(tmp, DEFAULT_SCALE);
            std::cout << tmp << std::endl;
            throw CException("Positive cycle!");
        }
    }

    return distMat;
}

/**
 * Matrix all pair longest path. Returns true if there is a positive cycle.
 * TODO: unify with the method above to have only one longest path
 * implementation
 */
bool Matrix::allPairLongestPathMatrix(MPTime posCycleThreshold,
                                      bool implyZeroSelfEdges,
                                      Matrix &res) const {
    // Floyd-Warshall algorithm
    if (this->getRows() != this->getCols()) {
        throw CException("Matrix must be square in Matrix::allPaiLongestPathMatrix.");
    }
    unsigned int N = this->getRows();

    if ((N != res.getRows()) || (N != res.getCols())) {
        throw CException("The matrix of the longest paths "
                         "should have the same size as the given matrix.");
    }

    // TODO: make / use copy operation
    for (unsigned int u = 0; u < N; u++) {
        for (unsigned int v = 0; v < N; v++) {
            res.put(u, v, this->get(u, v));
        }
    }

    // k - intermediate node
    for (unsigned int k = 0; k < N; k++) {
        for (unsigned int u = 0; u < N; u++) {
            for (unsigned int v = 0; v < N; v++) {
                MPTime extra = (implyZeroSelfEdges && u == v) ? MPTime(0) : MP_MINUSINFINITY;
                MPTime path_u2v = MP_MAX(res.get(v, u), extra);
                MPTime path_u2k = res.get(k, u);
                MPTime path_k2v = res.get(v, k);

                MPTime path_u2v_candidate = (path_u2k + path_k2v); // uses MP_PLUS()
                if (path_u2v_candidate > path_u2v) {
                    path_u2v = path_u2v_candidate;
                }
                res.put(v, u, path_u2v);
            }
        }
    }

    for (unsigned int k = 0; k < N; k++) {
        if (res.get(k, k) > posCycleThreshold) {
            return true;
        }
    }

    return false;
}

/**
 * class VectorList
 */

/**
 * VectorList::toString()
 */
void VectorList::toString(CString &outString, CDouble scale) const {
    outString = "";
    for (unsigned int i = 0; i < this->getSize(); i++) {
        CString vec_str;
        const Vector *v = &vectorRefAt(i);
        assert(v->getSize() == this->oneVectorSize);
        v->toString(vec_str, scale);
        outString += vec_str;
        outString += "\n";
    }
}

//  /**
//   * VectorList::findSimilar
//   * test if list contains a vector which differs to vecX by less than
//   threshold
//* implementation incomplete!
//   */
//  bool VectorList::findSimilar(const Vector &vecX, CDouble threshold) const
//  {

//      // similar - differs by a constant within a threshold
//      bool found = false;
//      assert(vecX.getSize() == this->oneVectorSize);
//      // test all vectors in list
//      for (unsigned int i = 0; i < this->size(); i++)
//      {
//          const Vector &vecY = this->vectorRefAt(i);
//          assert(vecY.getSize() == this->oneVectorSize);

//          // determine min and max difference
//          CDouble minDiff = 0;
//          CDouble maxDiff = 0;
//          bool min_def = false;
//          bool max_def = false;
//          for (unsigned int j = 0; j < this->oneVectorSize; j++)
//          {
//              bool vecX_inf = MP_ISMINUSINFINITY(vecX.get(j));
//              bool vecY_inf = MP_ISMINUSINFINITY(vecY.get(j));
//              if (vecX_inf || vecY_inf)
//              {
//                  if (vecX_inf && vecY_inf)
//                  {
//                      continue;
//                  }
//                  else
//                  {
//                      // Difference is infinity
//                      minDiff = MP_MINUSINFINITY;
//                      maxDiff = 0;
//                      break;
//                  }
//              }
//          }
//          return true;
//      }
// return false;
//  }

CDouble Matrix::mp_eigenvalue() const {
    // check if matrix is square.
    if (this->getRows() != this->getCols()) {
        throw CException("Matrix is not square in Matrix::mp_eigenvalue().");
    }

    uint sz = this->getRows();

    // convert matrix to mcm graph

    // Create a new MCM graph
    std::shared_ptr<MCMgraph> mcmGraph = std::make_shared<MCMgraph>();

    // store vector of nodes
    std::vector<MCMnode *> nodes(sz);

    // Generate ids by counting
    CId id = 0;
    for (uint i = 0; i != sz; i++) {
        // Create an MCM node for this state

        // Add the node to the MCM graph
        MCMnode *n = mcmGraph->addNode(id, true);
        id++;
        nodes[i] = n;
    }

    // Add edges to the MCM graph
    // for all states of the fsm...
    CId edgeId = 0;
    uint row = 0;
    uint col = 0;
    std::vector<MPTime>::const_iterator i;
    for (i = this->table.begin(); i != this->table.end(); i++) {
        // Add the edge to the MCM graph and the src and dst node
        mcmGraph->addEdge(edgeId, *nodes[col], *nodes[row], static_cast<CDouble>(*i), 1.0, true);
        edgeId++;

        col++;
        if (col == sz) {
            col = 0;
            row++;
        }
    }

    // prune the graph
    std::shared_ptr<MCMgraph> pruned = mcmGraph->pruneEdges();

    // compute MCM
    CDouble res = pruned->calculateMaximumCycleMeanKarpDouble();
    return res;
}

MCMgraph Matrix::mpMatrixToPrecedenceGraph() const {
    // check if matrix is square.
    if (this->getRows() != this->getCols()) {
        throw CException("Matrix is not square in Matrix::mpMatrixToPrecedenceGraph().");
    }

    uint sz = this->getRows();

    // convert matrix to precedence graph

    // Create a new MCM graph
    MCMgraph precGraph;

    // store vector of nodes
    std::vector<MCMnode *> nodes(sz);

    // Generate ids by counting
    CId id = 0;
    for (uint i = 0; i != sz; i++) {
        // Create an MCM node for state element i
        MCMnode *n = precGraph.addNode(id, true);
        id++;
        nodes[i] = n;
    }

    // Add edges to the MCM graph
    CId edgeId = 0;
    uint row = 0;
    uint col = 0;
    for (auto i : this->table) {
        // add edge is the value is not minus infinity
        if (!i.isMinusInfinity()) {
            // Add the edge to the MCM graph and the src and dst node
            precGraph.addEdge(edgeId, *nodes[col], *nodes[row], static_cast<CDouble>(i), 1.0, true);
            edgeId++;
        }
        // update row and col
        col++;
        if (col == sz) {
            col = 0;
            row++;
        }
    }

    return precGraph;
}

std::pair<Matrix::EigenvectorList, Matrix::GeneralizedEigenvectorList>
Matrix::mp_generalized_eigenvectors() const {
    // check if matrix is square.
    if (this->getRows() != this->getCols()) {
        throw CException("Matrix is not square in Matrix::mp_eigenvector().");
    }

    // gr = mpMatrixToPrecedenceGraph(M)
    //  compute precedence graph
    MCMgraph precGraph = this->mpMatrixToPrecedenceGraph();
    MCMgraphs sccs;

    // compute SCCs including single nodes without edges.
    stronglyConnectedMCMgraph(precGraph, sccs, true);

    // map from number of SCC to SCC
    std::map<uint, MCMgraph *> sccMapInv;

    // map from node of precGraph to its SCC
    std::map<MCMnode *, uint> sccMap;

    // map from nodes of precGraph to the cycle mean of its SCC
    std::map<MCMnode *, MPTime> cycleMeansMap;

    // vector such that element k is a node from precGraph in the critical path of
    // SCC k
    std::vector<MCMnode *> criticalNodes;

    // vector such that element k is the maximum cycle mean of SCC k
    std::vector<MPTime> cycleMeans;

    // SCC counter k
    uint k = 0;
    // for each SCC
    for (auto scci = sccs.cbegin(); scci != sccs.cend(); scci++, k++) {
        const std::shared_ptr<MCMgraph> &scc = *scci;
        sccMapInv[k] = scc.get();

        // MCM calculation requires node relabelling
        std::map<CId, CId> sccNodeIdMap;
        scc->relabelNodeIds(&sccNodeIdMap);

        if (scc->nrVisibleEdges() > 0) {
            // compute MCM mu and critical node n of scc
            const MCMnode *n = nullptr;
            auto mu = MPTime(scc->calculateMaximumCycleMeanKarpDouble(&n));
            criticalNodes.push_back(precGraph.getNode(sccNodeIdMap[n->id]));
            cycleMeans.push_back(mu);
        } else {
            // a SCC without edges is a single node and has no cycle mean
            criticalNodes.push_back(nullptr);
            cycleMeans.push_back(MP_MINUSINFINITY);
        }
        // for each node in the scc
        for (auto &nn : precGraph.getNodes()) {
            MCMnode *nnn = precGraph.getNode(sccNodeIdMap[nn.id]);
            // map node to SCC index
            sccMap[nnn] = k;
            // map node to the cycle mean of its SCC
            cycleMeansMap[nnn] = cycleMeans[k];
        }
    }

    // compute the eigenvectors
    Matrix::EigenvectorList eigenVectors;
    Matrix::GeneralizedEigenvectorList genEigenVectors;

    for (size_t k = 0; k < sccs.size(); k++) {
        // there is one for each SCC with a cycle mean larger than -inf
        if (!cycleMeans[k].isMinusInfinity()) {
            // eigenvector is formed by normalized longest paths from critical node to
            // all other nodes compute transitive cycle means such that all nodes in
            // SCC k and downstream SCCs get a cycle mean that is the maximum if all
            // (reflexive) upstream SCCs
            std::map<CId, MPTime> trCycleMeans;
            // initialize all nodes to undefined (represented by -DBL_MAX), except the
            // root node which is initialized with its own cycle mean
            for (unsigned int n = 0; n != precGraph.getNodes().size(); n++) {
                trCycleMeans[n] = (n == criticalNodes[k]->id) ? (cycleMeans[k]) : MP_MINUSINFINITY;
            }
            bool change = true;
            while (change) {
                change = false;
                // for all edges in precGraph
                for (const auto &e : precGraph.getEdges()) {
                    if (trCycleMeans[e.src->id] > trCycleMeans[e.dst->id]) {
                        change = true;
                        trCycleMeans[e.dst->id] =
                                MP_MAX(trCycleMeans[e.src->id], cycleMeans[sccMap[e.dst]]);
                    }
                }
            }

            // compute normalization map replace MP_MINUSINFINITY by -DBL_MAX
            std::map<CId, CDouble> muMap;
            for (unsigned int n = 0; n < trCycleMeans.size(); n++) {
                muMap[n] = (MPTime(trCycleMeans[n]).isMinusInfinity())
                                   ? -DBL_MAX
                                   : static_cast<CDouble>(trCycleMeans[n]);
            }

            // compute normalized longest paths
            std::map<CId, CDouble> lengths =
                    precGraph.normalizedLongestPaths(criticalNodes[k]->id, muMap);
            // make an eigenvector
            Vector v(this->getCols());
            for (auto length : lengths) {
                CId n = length.first;
                MPTime value =
                        ((MPTime(length.second)) <= MP_MINUSINFINITY ? MP_MINUSINFINITY
                                                                     : MPTime(length.second));
                v.put(static_cast<unsigned int>(n), value);
            }

            // check if it is a generalized eigenvalue
            bool isGeneralized = false;
            MPTime lambda = MP_MINUSINFINITY;
            Vector ev(this->getCols());
            int l = 0;
            for (auto &n : precGraph.getNodes()) {
                if (lambda == MP_MINUSINFINITY) {
                    lambda = trCycleMeans[n.id];
                } else {
                    if ((!lambda.isMinusInfinity()) && (!trCycleMeans[n.id].isMinusInfinity())
                        && (lambda != trCycleMeans[n.id])) {
                        isGeneralized = true;
                    }
                }
                ev.put(l, trCycleMeans[n.id]);
                l++;
            }

            if (isGeneralized) {
                genEigenVectors.push_back(std::make_pair(v, ev));
            } else {
                eigenVectors.push_back(std::make_pair(v, static_cast<CDouble>(lambda)));
            }
        }
    }

    return std::make_pair(eigenVectors, genEigenVectors);
}

Matrix::EigenvectorList Matrix::mpEigenvectors() const {
    auto gev = this->mp_generalized_eigenvectors();
    return gev.first;
}

} // namespace MaxPlus
