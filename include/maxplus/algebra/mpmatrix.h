/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   mpmatrix.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   March 23, 2009
 *
 *  Function        :   MaxPlus matrices
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
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#ifndef MAXPLUS_ALGEBRA_MATRIX_H_INCLUDED
#define MAXPLUS_ALGEBRA_MATRIX_H_INCLUDED

#include "maxplus/base/analysis/mcm/mcmgraph.h"
#include "mptype.h"
#include <memory>
#include <unordered_set>
#include <vector>

class CString;

namespace MaxPlus {

using namespace Graphs;

/**
 * Vector, represents a max-plus column vector.
 */
class Vector {
public:
    explicit Vector(unsigned int size = 0, MPTime value = MP_MINUS_INFINITY);

    explicit Vector(std::vector<MPTime> *v);

    ~Vector();

    Vector(Vector &&) = default;
    Vector &operator=(Vector &&) = delete;

    [[nodiscard]] inline unsigned int getSize() const {
        return static_cast<unsigned int>(this->table.size());
    }

    [[nodiscard]] inline MPTime get(unsigned int row) const { return this->table[row]; }

    void put(unsigned int row, MPTime value);

    void toString(CString &outString, CDouble scale = 1.0) const;

    Vector(const Vector &);

    Vector &operator=(const Vector &);

    [[nodiscard]] MPTime norm() const;

    void negate();
    MPTime normalize();

    [[nodiscard]] Vector add(MPTime increase) const;

    void add(MPTime increase, Vector &result) const;

    void maximum(const Vector &vecB, Vector &result) const;

    [[nodiscard]] Vector add(const Vector &vecB) const;

    void add(const Vector &vecB, Vector &res) const;

    Vector &operator+=(MPTime increase) {
        this->add(increase, *this);
        return *this;
    }

    Vector &operator-=(MPTime decrease) {
        assert(!decrease.isMinusInfinity());
        this->add(-decrease, *this);
        return *this;
    }

    [[nodiscard]] bool compare(const Vector &v) const;

    Vector &incrementalMaximum(const Vector &vec) {
        this->maximum(vec, *this);
        return *this;
    }

    /**
     * Get minimal finite element
     * returns the smallest among the finite elements in the vector or
     * MP_MINUS_INFINITY if no finite elements exist
     * itsPosition returns the index of the (a) smallest finite element is set
     * to a pointer to unsigned int, otherwise set or defaults to NULL
     */
    MPTime minimalFiniteElement(unsigned int *itsPosition_Ptr = nullptr) const;

private:
    std::vector<MPTime> table;
};

enum class MatrixFill { MinusInfinity, Zero, Identity };

class Matrix {
public:
    /**
     * Construct a max-plus matrix of \p N by \p N using the default fill pattern (minus infinity).
     * @param N Number of rows and columns
     */
    explicit Matrix(unsigned int N);

    /**
     * Construct a max-plus matrix of \p nrows by \p nr_cols using the given fill pattern.
     * @param nr_rows Number of rows
     * @param nr_cols Number of columns
     * @param fill Fill pattern for matrix
     */
    Matrix(unsigned int nr_rows, unsigned int nr_cols, MatrixFill fill = MatrixFill::MinusInfinity);

    Matrix(unsigned int nr_rows, unsigned int nr_cols, unsigned int nr_el);

    Matrix(Matrix &&) = default;
    Matrix &operator=(Matrix &&) = default;
    Matrix(const Matrix &) = default;

    Matrix &operator=(const Matrix &);

    virtual ~Matrix();

    [[nodiscard]] inline unsigned int getRows() const { return this->szRows; }

    [[nodiscard]] inline unsigned int getCols() const { return this->szCols; }

    [[nodiscard]] unsigned int getSize() const;

    [[nodiscard]] MPTime get(unsigned int row, unsigned int column) const;
    [[nodiscard]] Vector getRowVector(unsigned int row) const;

    void put(unsigned int row, unsigned int column, MPTime value);

    void paste(unsigned int top_row, unsigned int left_column, const Matrix *pastedMatrix);

    void pasteRowVector(unsigned int top_row, unsigned int left_column, const Vector *pastedVector);

    [[nodiscard]] virtual std::shared_ptr<Matrix> createCopy() const;

    [[nodiscard]] virtual std::shared_ptr<Matrix> getTransposedCopy() const;

    [[nodiscard]] Matrix transpose() const;

    [[nodiscard]] virtual Matrix getSubMatrix(const std::list<unsigned int> &rowIndices,
                                              const std::list<unsigned int> &colIndices) const;

    [[nodiscard]] virtual std::shared_ptr<Matrix>
    getSubMatrixPtr(const std::list<unsigned int> &rowIndices,
                    const std::list<unsigned int> &colIndices) const;

    [[nodiscard]] Matrix getSubMatrix(const std::list<unsigned int> &indices) const;

    [[nodiscard]] virtual std::shared_ptr<Matrix>
    getSubMatrixPtr(const std::list<unsigned int> &indices) const;

    [[nodiscard]] Matrix getSubMatrixNonSquare(const std::list<unsigned int> &indices) const;

    [[nodiscard]] Matrix getSubMatrixNonSquareRows(const std::list<unsigned int>& rowIndices) const;

    [[nodiscard]] virtual std::shared_ptr<Matrix>
    getSubMatrixNonSquareRowsPtr(const std::list<unsigned int>& rowIndices) const;

    /**
     * Increases the number of rows of the matrix by n and fills the new elements with -\infty.
     */
    void addRows(uint n);

    /**
     * Increases the number of columns of the matrix by n and fills the new elements with -\infty.
     */
    void addCols(uint n);

    void toString(CString &outString, CDouble scale = 1.0) const;
    void toMatlabString(CString &outString, CDouble scale = 1.0) const;
    void toLaTeXString(CString &outString, CDouble scale = 1.0) const;

    // Algebraic operations.
    [[nodiscard]] Matrix add(MPTime increase) const;

    void add(MPTime increase, Matrix &result) const;

    [[nodiscard]] Matrix mp_sub(const Matrix &m) const;

    [[nodiscard]] Matrix mp_maximum(const Matrix &m) const;

    void maximum(const Matrix &matB, Matrix &result) const;

    [[nodiscard]] Vector mp_multiply(const Vector &v) const;

    [[nodiscard]] Matrix mp_multiply(const Matrix &m) const;

    [[nodiscard]] Matrix mp_power(unsigned int p) const;

    [[nodiscard]] CDouble mp_eigenvalue() const;

    using EigenvectorList = std::list<std::pair<Vector, CDouble>>;
    using GeneralizedEigenvectorList = std::list<std::pair<Vector, Vector>>;
    [[nodiscard]] std::pair<EigenvectorList, GeneralizedEigenvectorList>
    mp_generalized_eigenvectors() const;
    [[nodiscard]] EigenvectorList mpEigenvectors() const;

    Matrix &operator+=(MPTime increase) {
        this->add(increase, *this);
        return *this;
    }

    Matrix &operator-=(MPTime decrease) {
        assert(!decrease.isMinusInfinity());
        this->add(-decrease, *this);
        return *this;
    }

    void incrementalMaximum(const Matrix &matrix) { this->maximum(matrix, *this); }

    bool operator==(const Matrix &other);

    /**
     * Return the element having the largest abs() value.
     * @return element having the largest abs() value
     */
    [[nodiscard]] MPTime largestFiniteElement() const;
    [[nodiscard]] MPTime minimalFiniteElement() const;
    [[nodiscard]] MPTime getMaxOfCol(uint colNumber) const;
    [[nodiscard]] MPTime getMaxOfRow(uint rowNumber) const;

    [[nodiscard]] Matrix plusClosureMatrix(MPTime posCycleThreshold = MP_EPSILON) const;

    [[nodiscard]] Matrix starClosureMatrix(MPTime posCycleThreshold = MP_EPSILON) const;

    [[nodiscard]] Matrix allPairLongestPathMatrix(MPTime posCycleThreshold,
                                                  bool implyZeroSelfEdges) const;
    bool
    allPairLongestPathMatrix(MPTime posCycleThreshold, bool implyZeroSelfEdges, Matrix &res) const;

    [[nodiscard]] MCMgraph mpMatrixToPrecedenceGraph() const;

private:
    void init(MatrixFill fill);
    void init();

    Matrix();

    std::vector<MPTime> table;
    unsigned int szRows;
    unsigned int szCols;
};

/****************************************************
 * VectorList: usually represents a set of eigenvectors
 * More efficient than vector<MaxPlus::Vector>
 ****************************************************/

class VectorList : private std::vector<std::unique_ptr<Vector>> {
public:
    explicit VectorList(unsigned int oneVectorSizeInit);
    ~VectorList() = default;

    VectorList(VectorList &&) = default;
    VectorList &operator=(VectorList &&) = delete;

    // Implicit copying is not allowed
    //  => Intentionally private and not implemented
    VectorList(const VectorList &) = delete;
    VectorList &operator=(const VectorList &) = delete;

    [[nodiscard]] const Vector &vectorRefAt(unsigned int n) const; // vector at index 'n'
    Vector &vectorRefAt(unsigned int n);

    [[nodiscard]] const Vector &lastVectorRef() const; // last vector
    Vector &lastVectorRef();

    [[nodiscard]] unsigned int getSize() const; // vector count
    [[nodiscard]] unsigned int getOneVectorSize() const { return this->oneVectorSize; }

    void grow(); // append one vector place

    void toString(CString &outString, CDouble scale = 1.0) const;

    // bool findSimilar(const Vector& vec, CDouble threshold) const;
    //  similar - differs by a constant within a threshold

private:
    const unsigned int oneVectorSize;
};

inline VectorList::VectorList(unsigned int oneVectorSizeInit) : oneVectorSize(oneVectorSizeInit) {
    assert(oneVectorSize > 0);
}

inline const Vector &VectorList::vectorRefAt(unsigned int n) const { return *this->at(n); }

inline Vector &VectorList::vectorRefAt(unsigned int n) { return *this->at(n); }

inline const Vector &VectorList::lastVectorRef() const { return *this->at(this->size() - 1); }

inline Vector &VectorList::lastVectorRef() { return *this->at(this->size() - 1); }

inline unsigned int VectorList::getSize() const { return static_cast<unsigned int>(this->size()); }

inline void VectorList::grow() {
    auto last = static_cast<unsigned int>(this->size());
    this->resize(static_cast<size_t>(last + 1));
    this->insert(this->begin() + last, std::make_unique<Vector>(oneVectorSize, MP_MINUS_INFINITY));
}

} // namespace MaxPlus

#endif