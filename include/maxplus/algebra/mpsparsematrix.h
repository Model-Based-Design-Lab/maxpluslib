/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   mpsparsematrix.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   April 8, 2021
 *
 *  Function        :   Sparse MaxPlus matrices
 *
 *  History         :
 *      08-04-21    :   Initial version.
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
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#ifndef MAXPLUS_ALGEBRA_SPARSEMATRIX_H_INCLUDED
#define MAXPLUS_ALGEBRA_SPARSEMATRIX_H_INCLUDED

#include "mpmatrix.h"
#include "mptype.h"
#include <vector>

class CString;

namespace MaxPlus {

class Sizes : public std::vector<unsigned int> {
public:
    [[nodiscard]] Sizes refineWith(const Sizes &s) const;
    [[nodiscard]] unsigned int sum() const;
};

using Indices = std::vector<unsigned int>;

using Ranges = std::vector<std::pair<unsigned int, unsigned int>>;

/**
 * SparseVector, represents a sparse max-plus column vector efficiently.
 * represent compress identical values
 */
class SparseVector {
public:
    explicit SparseVector(unsigned int size = 0, MPTime value = MP_MINUS_INFINITY);

    explicit SparseVector(const std::vector<MPTime> &v);

    SparseVector(const SparseVector &);

    SparseVector(const Vector &, const Sizes &);

    SparseVector(SparseVector &&) = default;
    SparseVector &operator=(SparseVector &&) = default;

    ~SparseVector();

    static SparseVector UnitVector(unsigned int size, unsigned int n);

    [[nodiscard]] inline unsigned int getSize() const { return this->size; }

    [[nodiscard]] MPTime get(unsigned int row) const;

    void put(unsigned int row, MPTime value);
    void putAll(unsigned int startRow, unsigned int endRow, MPTime value);
    void insertVector(unsigned int startRow, const SparseVector &v);

    void toString(CString &outString, CDouble scale = 1.0) const;

    SparseVector &operator=(const SparseVector &);

    [[nodiscard]] MPTime norm() const;

    void negate();

    MPTime normalize();

    [[nodiscard]] MPTime innerProduct(const SparseVector &v) const;

    [[nodiscard]] SparseVector add(MPTime increase) const;

    [[nodiscard]] SparseVector maximum(const SparseVector &vecB) const;

    [[nodiscard]] SparseVector add(const SparseVector &vecB) const;

    SparseVector operator+=(MPTime increase) const;

    bool operator==(const SparseVector &v) const;

    SparseVector operator-=(MPTime decrease) const;

    [[nodiscard]] bool compare(const SparseVector &v) const;

    void compress();

private:
    friend class SparseMatrix;
    unsigned int size;
    std::vector<std::pair<unsigned int, MPTime>> table;
    SparseVector(unsigned int size, const std::vector<std::pair<unsigned int, MPTime>> &v);
    SparseVector combine(const SparseVector &vecB, MPTime f(MPTime a, MPTime b)) const;
    bool forall(const SparseVector &vecB, bool f(MPTime a, MPTime b)) const;
    std::pair<unsigned int, unsigned int> find(unsigned int row);
    [[nodiscard]] Vector maxRanges(const Ranges &ranges) const;
    [[nodiscard]] Vector sample(const Indices &i) const;
    [[nodiscard]] Sizes getSizes() const;
};

/**
 SparseMatrix, represents a sparse max-plus matrix efficiently.
 use sparse column vectors and lazy transpose
 compress identical vectors
 **/
class SparseMatrix {
public:
    explicit SparseMatrix(unsigned int rowSize = 0,
                          unsigned int colSize = 0,
                          MPTime value = MP_MINUS_INFINITY);

    SparseMatrix(const SparseMatrix &);

    SparseMatrix(SparseMatrix &&) = default;
    SparseMatrix &operator=(SparseMatrix &&) = default;

    ~SparseMatrix();

    static SparseMatrix IdentityMatrix(unsigned int rowsAndCols);

    [[nodiscard]] inline unsigned int getRowSize() const {
        return this->isTransposed ? this->columnSize : this->rowSize;
    }

    [[nodiscard]] inline unsigned int getColumnSize() const {
        return this->isTransposed ? this->rowSize : this->columnSize;
    }

    [[nodiscard]] MPTime get(unsigned int row, unsigned int column) const;

    void put(unsigned int row, unsigned int column, MPTime value);

    /*
    Put all elements of the matrix between rows startRow (inclusive) and endRow (exclusive)
    and columns rows startColumn (inclusive) and endColumn (exclusive) with the provided value
    */
    void putAll(unsigned int startRow,
                unsigned int endRow,
                unsigned int startColumn,
                unsigned int endColumn,
                MPTime value);

    void insertMatrix(unsigned int startRow, unsigned int startColumn, const SparseMatrix &M);

    SparseMatrix &operator=(const SparseMatrix &);

    [[nodiscard]] MPTime norm() const;

    void transpose();

    [[nodiscard]] SparseMatrix transposed() const;

    SparseMatrix add(const SparseMatrix &M);
    SparseMatrix maximum(const SparseMatrix &M);

    SparseMatrix multiply(const SparseMatrix &M);
    SparseVector multiply(const SparseVector &v);

    void compress();

    void toString(CString &outString, CDouble scale = 1.0) const;

    MPTime mpEigenvalue();

    using EigenvectorList = std::list<std::pair<SparseVector, CDouble>>;
    using GeneralizedEigenvectorList = std::list<std::pair<SparseVector, SparseVector>>;
    std::pair<EigenvectorList, GeneralizedEigenvectorList> mpGeneralizedEigenvectors();
    EigenvectorList mpEigenvectors();

    SparseMatrix starClosure();

private:
    // row size and column size of the matrix is not implicitly transposed, in which case they are
    // reversed
    unsigned int rowSize;
    unsigned int columnSize;
    bool isTransposed;
    // table contains column vectors (if isTransposed is false)
    // a pair (n, v) means that the vector v is repeated n times.
    std::vector<std::pair<unsigned int, SparseVector>> table;
    std::pair<unsigned int, unsigned int> find(unsigned int col);
    void doTranspose();
    SparseMatrix combine(const SparseMatrix &M, MPTime f(MPTime a, MPTime b));
    Matrix reduceRows();
    std::pair<Matrix, Sizes> reduceRowsAndColumns();
    static SparseMatrix expand(const Matrix &M, const Sizes &rsz_s, const Sizes &csz_s);
    [[nodiscard]] Sizes sizes() const;
};

} // namespace MaxPlus

#endif