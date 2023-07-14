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

#include "algebra/mpsparsematrix.h"
#include "algebra/mpmatrix.h"
#include "algebra/mptype.h"
#include "base/exception/exception.h"
#include <cmath>
#include <numeric>

namespace MaxPlus {

unsigned int Sizes::sum() const {
    return std::accumulate(this->cbegin(), this->cend(), static_cast<unsigned int>(0));
}

Sizes Sizes::refineWith(const Sizes &s) const {
    Sizes result;
    auto it = this->cbegin();
    auto is = s.cbegin();
    unsigned int rt = *it;
    unsigned int rs = *is;
    while (it != this->cend()) {
        if (rs == rt) {
            result.push_back(rs);
            it++;
            is++;
            if (it != this->cend()) {
                rt = *it;
            }
            if (is != s.cend()) {
                rs = *is;
            }
        } else {
            if (rs < rt) {
                result.push_back(rs);
                is++;
                rt -= rs;
                if (is != s.cend()) {
                    rs = *is;
                }
            } else {
                result.push_back(rt);
                it++;
                rs -= rt;
                if (it != this->cend()) {
                    rt = *it;
                }
            }
        }
    }
    return result;
}

/**
 * Construct a max-plus vector of size
 */
SparseVector::SparseVector(unsigned int size, MPTime value) : size(size), table(0) {
    if (size > 0) {
        this->table.emplace_back(size, value);
    }
}

/**
 * Construct a max-plus vector from an std:vector
 */
SparseVector::SparseVector(const std::vector<MPTime> &v) :
    size(static_cast<unsigned int>(v.size())), table(0) {
    unsigned int k = 0;
    while (k < v.size()) {
        unsigned int cnt = 0;
        while (k + cnt < static_cast<unsigned int>(v.size()) && v[k + cnt] == v[k]) {
            cnt++;
        }
        this->table.emplace_back(cnt, v[k]);
        k += cnt;
    }
}

/**
 * copy constructor
 */
SparseVector::SparseVector(const SparseVector &other) = default;

SparseVector::SparseVector(const unsigned int size,
                           const std::vector<std::pair<unsigned int, MPTime>> &v) :
    size(size), table(v) {}

SparseVector::SparseVector(const Vector &v, const Sizes &sz) {
    if (v.getSize() != sz.size()) {
        throw CException("Incompatible sizes");
    }
    this->size = 0;
    for (unsigned int i = 0; i < v.getSize(); i++) {
        this->table.emplace_back(sz[i], v.get(i));
        this->size += sz[i];
    }
}

/**
 * vector assignment
 */
SparseVector &SparseVector::operator=(const SparseVector &other) {
    // check self assignment
    if (this != &other) {
        if (this->getSize() != other.getSize()) {
            throw CException("Vectors of different size in"
                             "SparseVector::operator=");
        }
        this->table = other.table;
    }
    return *this;
}

SparseVector SparseVector::UnitVector(unsigned int size, unsigned int n) {
    SparseVector result(size);
    result.put(n, MPTime(0.0));
    return result;
}

/**
 * vector negate
 */
void SparseVector::negate() {
    for (auto &e : this->table) {
        if (e.second == MP_MINUSINFINITY) {
            throw CException("Cannot negate vectors with MP_MINUSINFINITY elements in"
                             "SparseVector::negate");
        }
        e.second = -e.second;
    }
}

/**
 * calculate vector norm
 */
MPTime SparseVector::norm() const {
    MPTime maxEl = MP_MINUSINFINITY;
    for (const auto &e : this->table) {
        maxEl = MP_MAX(maxEl, e.second);
    }
    return maxEl;
}

/**
 * normalize vector
 */
MPTime SparseVector::normalize() {
    MPTime maxEl = this->norm();

    if (maxEl.isMinusInfinity()) {
        throw CException("Cannot normalize vector with norm MP_MINUSINFINITY"
                         "SparseVector::normalize");
    }
    for (auto &e : this->table) {
        MPTime x_i = e.second; // MPTime handles -INF correctly
        x_i = x_i - maxEl;     // overloaded using MP_PLUS
        e.second = x_i;
    }

    return maxEl;
}

/**
 * add scalar to vector
 */
SparseVector SparseVector::add(MPTime increase) const {
    std::vector<std::pair<unsigned int, MPTime>> newTable;
    unsigned int k = 0;
    while (k < this->table.size()) {
        newTable.emplace_back(this->table[k].first, this->table[k].second + increase);
    }
    return {this->getSize(), newTable};
}

/**
 * element-wise combination
 */
SparseVector SparseVector::combine(const SparseVector &vecB, MPTime f(MPTime a, MPTime b)) const {
    assert(vecB.getSize() == this->getSize());

    std::vector<std::pair<unsigned int, MPTime>> newTable;
    unsigned int k1 = 0;
    unsigned int k2 = 0;
    unsigned int m1 = 0;
    unsigned int m2 = 0;

    // invariant m1 elements if this->table[k1] have been covered and m2 elements
    // of vecB.table[k2] m1 < this->table[k1].first && m2 < vecB.table[k1].first
    if (k1 < this->table.size()) {
        m1 = this->table[k1].first;
    }
    if (k2 < vecB.table.size()) {
        m2 = vecB.table[k2].first;
    }
    while (k1 < this->table.size()) {
        unsigned int m = (m1 < m2) ? m1 : m2;
        newTable.emplace_back(m, f(this->table[k1].second, vecB.table[k2].second));
        m1 -= m;
        m2 -= m;
        if (m1 == 0) {
            k1++;
            if (k1 < this->table.size()) {
                m1 = this->table[k1].first;
            }
        }
        if (m2 == 0) {
            k2++;
            if (k2 < vecB.table.size()) {
                m2 = vecB.table[k2].first;
            }
        }
    }
    return {this->getSize(), newTable};
}

bool SparseVector::forall(const SparseVector &vecB, bool f(MPTime a, MPTime b)) const {
    assert(vecB.getSize() == this->getSize());

    unsigned int k1 = 0;
    unsigned int k2 = 0;
    unsigned int m1 = 0;
    unsigned int m2 = 0;

    // invariant m1 elements if this->table[k1] have been covered and m2 elements
    // of vecB.table[k2] m1 < this->table[k1].first && m2 < vecB.table[k1].first
    if (k1 < this->table.size()) {
        m1 = this->table[k1].first;
    }
    if (k2 < vecB.table.size()) {
        m2 = vecB.table[k2].first;
    }
    while (k1 < this->table.size()) {
        unsigned int m = (m1 < m2) ? m1 : m2;
        if (!f(this->table[k1].second, vecB.table[k2].second)) {
            return false;
        }
        m1 -= m;
        m2 -= m;
        if (m1 == 0) {
            k1++;
            if (k1 < this->table.size()) {
                m1 = this->table[k1].first;
            }
        }
        if (m2 == 0) {
            k2++;
            if (k2 < vecB.table.size()) {
                m2 = vecB.table[k2].first;
            }
        }
    }
    return true;
}

/**
 * max of vectors
 */
SparseVector SparseVector::maximum(const SparseVector &vecB) const {
    return this->combine(vecB, [](MPTime a, MPTime b) { return MP_MAX(a, b); });
}

/**
 * Destructor of max-plus vector
 */
SparseVector::~SparseVector() = default;

/**
 * Put an entry into the vector. Grows vector if necessary
 */
void SparseVector::put(unsigned int row, MPTime value) {
    // find insertion place
    auto insert = this->find(row);
    unsigned int k = insert.first;
    unsigned int rc = insert.second;
    if (k < this->table.size()) {
        // we have found the spot in the list
        if (this->table[k].second == value) {
            return;
        }
        MPTime oldval = this->table[k].second;
        unsigned int before = rc;
        unsigned int after = this->table[k].first - rc - 1;
        if (before > 0) {
            this->table.insert(this->table.begin() + k, std::make_pair(before, oldval));
            k++;
        }
        this->table[k].first = 1;
        this->table[k].second = value;
        if (after > 0) {
            this->table.insert(this->table.begin() + k + 1, std::make_pair(after, oldval));
            k++;
        }
    } else {
        // the spot exceeds the current size
        unsigned int before = rc;
        if (before > 0) {
            this->table.emplace_back(before, MP_MINUSINFINITY);
            k++;
        }
        this->table.emplace_back(1, value);
        this->size += before + 1;
    }
}

std::pair<unsigned int, unsigned int> SparseVector::find(unsigned int row) {
    // find insertion place
    unsigned int k = 0;
    unsigned int rc = row;
    while (k < this->table.size() && rc >= this->table[k].first) {
        rc -= this->table[k].first;
        k++;
    }
    return std::make_pair(k, rc);
}

// fill the vector from startRow (including) to endRow (excluding) with value
void SparseVector::putAll(unsigned int startRow, unsigned int endRow, MPTime value) {
    // find insertion place
    auto insertStart = this->find(startRow);
    auto insertEnd = this->find(endRow);
    if (insertEnd.first >= this->table.size() && insertEnd.second > 0) {
        throw CException("Range exceeds vector size in SparseVector::putAll");
    }
    // we have found the spot in the list
    MPTime oldValPre = this->table[insertStart.first].second;
    MPTime oldValPost;
    unsigned int postRemain = 0;
    if (insertEnd.first >= this->table.size()) {
        postRemain = 0;
    } else {
        oldValPost = this->table[insertEnd.first].second;
        postRemain = this->table[insertEnd.first].first - insertEnd.second;
    }
    auto last = insertEnd.first < this->table.size() ? this->table.begin() + insertEnd.first + 1
                                                     : this->table.end();
    this->table.erase(this->table.begin() + insertStart.first, last);
    unsigned int i = insertStart.first;
    if (insertStart.second > 0) {
        this->table.insert(this->table.begin() + i, std::make_pair(insertStart.second, oldValPre));
        i++;
    }
    if (endRow - startRow > 0) {
        this->table.insert(this->table.begin() + i, std::make_pair(endRow - startRow, value));
        i++;
    }
    if (postRemain > 0) {
        this->table.insert(this->table.begin() + i, std::make_pair(postRemain, oldValPost));
        i++;
    }
}

// fill the vector from startRow (including) with the given vector
void SparseVector::insertVector(unsigned int startRow, const SparseVector &v) {
    // find insertion place
    unsigned int endRow = startRow + v.getSize();
    auto insertStart = this->find(startRow);
    auto insertEnd = this->find(endRow);
    if (insertEnd.first >= this->table.size() && insertEnd.second > 0) {
        throw CException("Range exceeds vector size in SparseVector::putAll");
    }
    // we have found the spot in the list
    MPTime oldValPre = this->table[insertStart.first].second;
    MPTime oldValPost;
    unsigned int postRemain = 0;
    if (insertEnd.first >= this->table.size()) {
        postRemain = 0;
    } else {
        oldValPost = this->table[insertEnd.first].second;
        postRemain = this->table[insertEnd.first].first - insertEnd.second;
    }
    auto last = insertEnd.first < this->table.size() ? this->table.begin() + insertEnd.first + 1
                                                     : this->table.end();
    this->table.erase(this->table.begin() + insertStart.first, last);
    unsigned int i = insertStart.first;
    if (insertStart.second > 0) {
        this->table.insert(this->table.begin() + i, std::make_pair(insertStart.second, oldValPre));
        i++;
    }
    if (endRow - startRow > 0) {
        this->table.insert(this->table.begin() + i, v.table.begin(), v.table.end());
        i += static_cast<unsigned int>(v.table.size());
    }
    if (postRemain > 0) {
        this->table.insert(this->table.begin() + i, std::make_pair(postRemain, oldValPost));
        i++;
    }
}

MPTime SparseVector::get(unsigned int row) const {
    unsigned int k = 0;
    unsigned int rc = row;
    while (k < this->table.size() && rc >= this->table[k].first) {
        rc -= this->table[k].first;
        k++;
    }
    return this->table[k].second;
}

/**
 * String representation of vector
 */
void SparseVector::toString(CString &outString, CDouble scale) const {
    outString = "[";
    for (auto k = this->table.begin(); k < this->table.end(); k++) {
        if (k != this->table.begin()) {
            outString += "; ";
        }
        outString += CString((*k).first) + " * " + timeToString(scale * (*k).second) + " ";
    }
    outString += "]";
}

/**
 * add vectors
 */
SparseVector SparseVector::add(const SparseVector &vecB) const {
    return this->combine(vecB, [](MPTime a, MPTime b) { return a + b; });
}

/**
 * Compare vectors up to MP_EPSILON
 */
bool SparseVector::compare(const SparseVector &v) const {
    return this->forall(v, [](MPTime a, MPTime b) {
        return fabs(static_cast<CDouble>(a) - static_cast<CDouble>(b))
               <= static_cast<CDouble>(MP_EPSILON);
    });
}

SparseVector SparseVector::operator+=(MPTime increase) const { return this->add(increase); }

SparseVector SparseVector::operator-=(MPTime decrease) const {
    if (decrease.isMinusInfinity()) {
        throw CException("Cannot subtract minus infinity in"
                         "SparseVector::operator-=");
    }
    return this->add(-decrease);
}

void SparseVector::compress() {
    std::vector<std::pair<unsigned int, MPTime>> newTable;
    unsigned int k = 0;
    while (k < this->table.size()) {
        unsigned int m = k + 1;
        unsigned int cnt = this->table[k].first;
        while (m < this->table.size() && this->table[m].second == this->table[k].second) {
            cnt += this->table[m].first;
            m++;
        }
        newTable.emplace_back(cnt, this->table[k].second);
        k = m;
    }
    this->table = newTable;
}

MPTime SparseVector::innerProduct(const SparseVector &v) const {
    assert(v.getSize() == this->getSize());
    MPTime result = MP_MINUSINFINITY;

    unsigned int k1 = 0;
    unsigned int k2 = 0;
    unsigned int m1 = 0;
    unsigned int m2 = 0;

    // invariant m1 elements if this->table[k1] have been covered and m2 elements
    // of vecB.table[k2] m1 < this->table[k1].first && m2 < vecB.table[k1].first
    if (k1 < this->table.size()) {
        m1 = this->table[k1].first;
    }
    if (k2 < v.table.size()) {
        m2 = v.table[k2].first;
    }
    while (k1 < this->table.size()) {
        unsigned int m = (m1 < m2) ? m1 : m2;
        result = MP_MAX(result, this->table[k1].second + v.table[k2].second);
        m1 -= m;
        m2 -= m;
        if (m1 == 0) {
            k1++;
            if (k1 < this->table.size()) {
                m1 = this->table[k1].first;
            }
        }
        if (m2 == 0) {
            k2++;
            if (k2 < v.table.size()) {
                m2 = v.table[k2].first;
            }
        }
    }
    return result;
}

bool SparseVector::operator==(const SparseVector &v) const {
    return this->forall(v, [](MPTime a, MPTime b) { return a == b; });
}

Vector SparseVector::maxRanges(const Ranges &ranges) const {
    Vector result(static_cast<unsigned int>(ranges.size()));
    unsigned int k = 0;
    unsigned int l = 0;
    unsigned int idx = 0;
    for (unsigned int m = 0; m < ranges.size(); m++) {
        unsigned int end = ranges[m].first + ranges[m].second;
        MPTime value = MP_MINUSINFINITY;
        while (idx < end) {
            value = MP_MAX(value, this->table[k].second);
            if (idx + this->table[k].first - l > end) {
                // move to the end of the range
                l += end - idx;
                idx = end;
            } else {
                // move to the end of this vector's interval
                idx += this->table[k].first - l;
                k++;
                l = 0;
            }
        }
        result.put(m, value);
    }
    return result;
}

Vector SparseVector::sample(const Indices &i) const {
    Vector result(static_cast<unsigned int>(i.size()));
    unsigned int k = 0;
    unsigned int idx = 0;
    for (unsigned int m = 0; m < i.size(); m++) {
        while (idx + this->table[k].first <= i[m]) {
            idx += this->table[k].first;
            k++;
        }
        result.put(m, this->table[k].second);
    }
    return result;
}

Sizes SparseVector::getSizes() const {
    Sizes result;
    for (const auto e : this->table) {
        result.push_back(e.first);
    }
    return result;
}

SparseMatrix::SparseMatrix(unsigned int rowSize, unsigned int colSize, MPTime value) :
    rowSize(rowSize), columnSize(colSize), isTransposed(false) {
    if (this->columnSize > 0) {
        this->table.emplace_back(this->columnSize, SparseVector(this->rowSize, value));
    }
}

SparseMatrix::SparseMatrix(const SparseMatrix &M) = default;

SparseMatrix::~SparseMatrix() = default;

MPTime SparseMatrix::get(unsigned int row, unsigned int column) const {
    unsigned int r = this->isTransposed ? column : row;
    unsigned int c = this->isTransposed ? row : column;
    unsigned int k = 0;
    unsigned int cc = c;
    while (k < this->table.size() && cc >= this->table[k].first) {
        cc -= this->table[k].first;
        k++;
    }
    return this->table[k].second.get(r);
}

void SparseMatrix::put(unsigned int row, unsigned int column, MPTime value) {
    // find insertion place
    unsigned int r = this->isTransposed ? column : row;
    unsigned int c = this->isTransposed ? row : column;
    unsigned int k = 0;
    unsigned int cc = c;
    while (k < this->table.size() && cc >= this->table[k].first) {
        cc -= this->table[k].first;
        k++;
    }
    if (k < this->table.size()) {
        // we have found the spot in the list
        SparseVector oldval = this->table[k].second;
        unsigned int before = cc;
        unsigned int after = this->table[k].first - cc - 1;
        if (before > 0) {
            this->table.insert(this->table.begin() + k, std::make_pair(before, oldval));
            k++;
        }
        this->table[k].first = 1;
        this->table[k].second = oldval;
        this->table[k].second.put(r, value);
        if (after > 0) {
            this->table.insert(this->table.begin() + k + 1, std::make_pair(after, oldval));
            k++;
        }
    } else {
        // the spot exceeds the current size
        unsigned int before = cc;
        if (before > 0) {
            this->table.emplace_back(before, SparseVector(this->rowSize, MP_MINUSINFINITY));
            k++;
        }
        SparseVector newVector(this->rowSize, MP_MINUSINFINITY);
        newVector.put(r, value);
        this->table.emplace_back(1, newVector);
        this->rowSize += before + 1;
    }
}

SparseMatrix &SparseMatrix::operator=(const SparseMatrix &other) {
    // check self assignment
    if (this != &other) {
        if (this->getRowSize() != other.getRowSize()
            || this->getColumnSize() != other.getColumnSize()) {
            throw CException("Matrices of different size in"
                             "SparseMatrix::operator=");
        }
        this->table = other.table;
        this->isTransposed = other.isTransposed;
    }
    return *this;
}

MPTime SparseMatrix::norm() const {
    MPTime maxEl = MP_MINUSINFINITY;
    for (const auto &e : this->table) {
        maxEl = MP_MAX(maxEl, e.second.norm());
    }
    return maxEl;
}

void SparseMatrix::transpose() { this->isTransposed = !this->isTransposed; }

SparseMatrix SparseMatrix::transposed() const {
    SparseMatrix result(*this);
    result.transpose();
    return result;
}

void SparseMatrix::doTranspose() {
    SparseMatrix result(this->getRowSize(), this->getColumnSize());
    unsigned int cb = 0;
    for (auto &k : this->table) {
        unsigned int ce = cb + k.first;
        unsigned int rb = 0;
        for (auto &l : k.second.table) {
            unsigned int re = rb + l.first;
            result.putAll(cb, ce, rb, re, l.second);
            rb = re;
        }
        cb = ce;
    }
    result.isTransposed = !this->isTransposed;
    (*this) = result;
}

std::pair<unsigned int, unsigned int> SparseMatrix::find(unsigned int col) {
    // find insertion place
    unsigned int k = 0;
    unsigned int cc = col;
    while (k < this->table.size() && cc >= this->table[k].first) {
        cc -= this->table[k].first;
        k++;
    }
    return std::make_pair(k, cc);
}

void SparseMatrix::putAll(unsigned int startRow,
                          unsigned int endRow,
                          unsigned int startColumn,
                          unsigned int endColumn,
                          MPTime value) {
    // find insertion place
    unsigned int sr = this->isTransposed ? startColumn : startRow;
    unsigned int er = this->isTransposed ? endColumn : endRow;
    unsigned int sc = this->isTransposed ? startRow : startColumn;
    unsigned int ec = this->isTransposed ? endRow : endColumn;
    auto insertStart = this->find(sc);
    auto insertEnd = this->find(ec);
    if (insertEnd.first >= this->table.size() && insertEnd.second > 0) {
        throw CException("Range exceeds matrix size in SparseMatrix::putAll");
    }
    // we have found the spot in the list
    SparseVector oldValPre = this->table[insertStart.first].second;
    SparseVector oldValPost(oldValPre.getSize());
    unsigned int postRemain = 0;
    if (insertEnd.first < this->table.size()) {
        oldValPost = this->table[insertEnd.first].second;
        postRemain = this->table[insertEnd.first].first - insertEnd.second;
    } else {
        postRemain = 0;
    }

    auto last = insertEnd.first < this->table.size() ? this->table.begin() + insertEnd.first + 1
                                                     : this->table.end();
    std::vector<std::pair<unsigned int, SparseVector>> v(this->table.begin() + insertStart.first,
                                                         last);
    this->table.erase(this->table.begin() + insertStart.first, last);

    unsigned int i = insertStart.first;
    if (insertStart.second > 0) {
        this->table.insert(this->table.begin() + i, std::make_pair(insertStart.second, oldValPre));
        i++;
    }
    if (ec - sc > 0) {
        unsigned int remaining = ec - sc;
        unsigned int j = 0;
        // insert all new rows here as copies of the old ones with putAll, efficient
        // as possible...
        while (remaining > 0) {
            unsigned int num = v[j].first;
            if (remaining < num) {
                num = remaining;
            }
            SparseVector newCol = v[j].second;

            newCol.putAll(sr, er, value);
            this->table.insert(this->table.begin() + i, std::make_pair(num, newCol));
            i++;
            remaining -= num;
            if (num == v[j].first) {
                j++;
            }
        }
    }
    if (postRemain > 0) {
        this->table.insert(this->table.begin() + i, std::make_pair(postRemain, oldValPost));
        i++;
    }
}

void SparseMatrix::insertMatrix(unsigned int startRow,
                                unsigned int startColumn,
                                const SparseMatrix &M) {
    // find insertion place
    unsigned int endRow = startRow + M.getRowSize();
    unsigned int endColumn = startColumn + M.getColumnSize();
    unsigned int sr = this->isTransposed ? startColumn : startRow;
    unsigned int sc = this->isTransposed ? startRow : startColumn;
    unsigned int ec = this->isTransposed ? endRow : endColumn;
    auto insertStart = this->find(sc);
    auto insertEnd = this->find(ec);
    if (insertEnd.first >= this->table.size() && insertEnd.second > 0) {
        throw CException("Range exceeds matrix size in SparseMatrix::putAll");
    }
    // we have found the spot in the list
    SparseVector oldValPre = this->table[insertStart.first].second;
    SparseVector oldValPost(oldValPre.getSize());
    unsigned int postRemain = 0;
    if (insertEnd.first < this->table.size()) {
        oldValPost = this->table[insertEnd.first].second;
        postRemain = this->table[insertEnd.first].first - insertEnd.second;
    } else {
        postRemain = 0;
    }

    auto last = insertEnd.first < this->table.size() ? this->table.begin() + insertEnd.first + 1
                                                     : this->table.end();
    std::vector<std::pair<unsigned int, SparseVector>> v(this->table.begin() + insertStart.first,
                                                         last);
    this->table.erase(this->table.begin() + insertStart.first, last);

    unsigned int i = insertStart.first;
    if (insertStart.second > 0) {
        this->table.insert(this->table.begin() + i, std::make_pair(insertStart.second, oldValPre));
        i++;
    }
    if (ec - sc > 0) {
        unsigned int remaining = ec - sc;
        unsigned int j = 0;
        // insert all new rows here as copies of the old ones with inserted data,
        // efficient as possible...
        unsigned int m = 0;
        unsigned int n = 0;
        while (remaining > 0) {
            unsigned int num = v[j].first;
            if (M.table[m].first < num) {
                num = M.table[m].first;
            }
            if (remaining < num) {
                num = remaining;
            }

            SparseVector newCol = v[j].second;

            newCol.insertVector(sr, M.table[m].second);
            this->table.insert(this->table.begin() + i, std::make_pair(num, newCol));
            i++;

            remaining -= num;
            if (num == v[j].first) {
                j++;
            }

            n += num;
            if (n == M.table[m].first) {
                m++;
                n = 0;
            }
        }
    }
    if (postRemain > 0) {
        this->table.insert(this->table.begin() + i, std::make_pair(postRemain, oldValPost));
        i++;
    }
}

SparseVector SparseMatrix::multiply(const SparseVector &v) {
    assert(v.getSize() == this->getColumnSize());

    if (!this->isTransposed) {
        this->doTranspose();
    }
    SparseVector result(this->getRowSize());
    unsigned int i = 0;
    for (const auto &e : this->table) {
        result.putAll(i, i + e.first, e.second.innerProduct(v));
        i += e.first;
    }
    return result;
}

SparseMatrix SparseMatrix::multiply(const SparseMatrix &M) {
    assert(M.getRowSize() == this->getColumnSize());

    SparseMatrix Mr = M;

    // make sure this is in transposed representation and Mr is not.
    if (!this->isTransposed) {
        this->doTranspose();
    }
    if (Mr.isTransposed) {
        Mr.doTranspose();
    }

    SparseMatrix result(this->getRowSize(), Mr.getColumnSize());

    unsigned int rs = 0;
    for (const auto &e : this->table) {
        unsigned int cs = 0;
        for (const auto &mre : Mr.table) {
            result.putAll(rs, rs + e.first, cs, cs + mre.first, e.second.innerProduct(mre.second));
            cs += mre.first;
        }
        rs += e.first;
    }
    return result;
}

void SparseMatrix::compress() {

    for (auto &e : this->table) {
        e.second.compress();
    }

    std::vector<std::pair<unsigned int, SparseVector>> newTable;
    unsigned int k = 0;
    while (k < this->table.size()) {
        unsigned int m = k + 1;
        unsigned int cnt = this->table[k].first;
        while (m < this->table.size() && this->table[m].second == this->table[k].second) {
            cnt += this->table[m].first;
            m++;
        }
        newTable.emplace_back(cnt, this->table[k].second);
        k = m;
    }
    this->table = newTable;
}

/**
 * String representation of matrix
 */
void SparseMatrix::toString(CString &outString, CDouble scale) const {
    outString = "";
    if (this->isTransposed) {
        outString += "Transposed of ";
    }
    for (const auto &e : this->table) {
        if (e != *this->table.begin()) {
            outString += "\n";
        }
        CString s;
        e.second.toString(s, scale);
        outString += CString(e.first) + " * " + s;
    }
}

// eliminate identical rows and corresponding columns.
Matrix SparseMatrix::reduceRows() {
    if (!this->isTransposed) {
        this->doTranspose();
    }
    auto N = static_cast<unsigned int>(this->table.size());
    Matrix M(N, N);

    std::vector<std::pair<unsigned int, unsigned int>> ranges(N);
    unsigned int idx = 0;
    for (unsigned int k = 0; k < N; k++) {
        ranges[k] = std::make_pair(idx, this->table[k].first);
        idx += this->table[k].first;
    }
    for (unsigned int k = 0; k < N; k++) {
        Vector v = this->table[k].second.maxRanges(ranges);
        M.pasteRowVector(k, 0, &v);
    }
    return M;
}

/**********
 * return a matrix according to the coarsest partitioning of the rows and
 *columns such that the corresponding blocks contain the same value and the new
 *matrix has a single element for each such block.
 **********/
std::pair<Matrix, Sizes> SparseMatrix::reduceRowsAndColumns() {
    // ensure that we are in transposed form
    if (!this->isTransposed) {
        this->doTranspose();
    }
    Sizes cs;
    Sizes rs;
    // determine column sizes cs and row sizes rs that refine each of the rows
    for (const auto &e : this->table) {
        cs.push_back(e.first);
        if (e == *this->table.cbegin()) {
            rs = e.second.getSizes();
        } else {
            rs = rs.refineWith(e.second.getSizes());
        }
    }
    // determine a refinement of columns and all rows
    Sizes fs = rs.refineWith(cs);

    // create a new non-sparse matrix with an element for each of the blocks with
    // the value of that block
    auto N = static_cast<unsigned int>(fs.size());
    Matrix M(N, N);

    // determine the row/col indices of the first element of each block
    Indices idcs;
    unsigned int idx = 0;
    for (unsigned int k = 0; k < N; k++) {
        idcs.push_back(idx);
        idx += fs[k];
    }

    unsigned int k = 0;
    idx = 0;
    // for each of the rows of the new matrix / each of the indices in idcs
    for (unsigned int m = 0; m < idcs.size(); m++) {
        // find the table entry that includes the index idcs[m]
        while (idx + this->table[k].first <= idcs[m]) {
            idx += this->table[k].first;
            k++;
        }
        // make a row vector for the matrix by sampling the row at the given indices
        Vector v = this->table[k].second.sample(idcs);
        // place the samples row vector in the matrix
        M.pasteRowVector(m, 0, &v);
    }

    // return the resulting matrix.
    return std::make_pair(M, fs);
}

// identical rows can be eliminated any eigenvector must have identical values
// for those rows.
MPTime SparseMatrix::mpEigenvalue() {
    Matrix M = this->reduceRows();
    auto lambda = MPTime(M.mp_eigenvalue());
    return lambda;
}

Sizes SparseMatrix::sizes() const {
    Sizes result;
    for (const auto &e : this->table) {
        result.push_back(e.first);
    }
    return result;
}

std::pair<SparseMatrix::EigenvectorList, SparseMatrix::GeneralizedEigenvectorList>
SparseMatrix::mpGeneralizedEigenvectors() {
    Matrix M = this->reduceRows();
    auto evp = M.mp_generalized_eigenvectors();
    auto evs = evp.first;
    auto gevs = evp.second;
    SparseMatrix::EigenvectorList evl;
    Sizes sizes = this->sizes();
    for (const auto &evev : evs) {
        auto ev = evev.first;
        auto lambda = MPTime(evev.second);
        evl.push_back(std::make_pair(SparseVector(ev, sizes), static_cast<CDouble>(lambda)));
    }
    SparseMatrix::GeneralizedEigenvectorList gevl;
    for (const auto &evev : gevs) {
        auto ev = evev.first;
        Vector lambda = evev.second;
        gevl.push_back(std::make_pair(SparseVector(ev, sizes), SparseVector(lambda, sizes)));
    }
    return std::make_pair(evl, gevl);
}

SparseMatrix::EigenvectorList SparseMatrix::mpEigenvectors() {
    auto evp = this->mpGeneralizedEigenvectors();
    return evp.first;
}

SparseMatrix SparseMatrix::IdentityMatrix(unsigned int rowsAndCols) {
    SparseMatrix result(rowsAndCols, rowsAndCols);
    result.table.clear();
    for (unsigned int i = 0; i < rowsAndCols; i++) {
        result.table.emplace_back(1, SparseVector::UnitVector(rowsAndCols, i));
    }
    return result;
}

SparseMatrix SparseMatrix::combine(const SparseMatrix &M, MPTime f(MPTime a, MPTime b)) {
    assert(this->getColumnSize() == M.getColumnSize() && this->getRowSize() == M.getRowSize());
    if (this->isTransposed != M.isTransposed) {
        this->doTranspose();
    }

    SparseMatrix result(this->rowSize, this->columnSize);
    result.isTransposed = this->isTransposed;
    result.table.clear();

    unsigned int tInd = 0;
    unsigned int mInd = 0;
    unsigned int tRem = this->table[tInd].first;
    unsigned int mRem = M.table[mInd].first;
    while (tInd < this->table.size()) {
        unsigned int d = (tRem < mRem) ? tRem : mRem;
        SparseVector v = this->table[tInd].second.combine(M.table[mInd].second, f);
        result.table.emplace_back(d, v);
        tRem -= d;
        if (tRem == 0) {
            tInd++;
            if (tInd < this->table.size()) {
                tRem = this->table[tInd].first;
            }
        }
        mRem -= d;
        if (mRem == 0) {
            mInd++;
            if (mInd < M.table.size()) {
                mRem = M.table[mInd].first;
            }
        }
    }
    return result;
}

SparseMatrix SparseMatrix::add(const SparseMatrix &M) {
    return this->combine(M, [](MPTime a, MPTime b) { return a + b; });
}

SparseMatrix SparseMatrix::maximum(const SparseMatrix &M) {
    return this->combine(M, [](MPTime a, MPTime b) { return MP_MAX(a, b); });
}

SparseMatrix SparseMatrix::starClosure() {
    assert(this->getRowSize() == this->getColumnSize());
    auto mi = this->reduceRowsAndColumns();
    Matrix M = mi.first;
    Sizes szs = mi.second;
    Matrix MS = M.plusClosureMatrix();

    // expand MS with indices returned from reduceRowsAndColumns
    //(this->getRowSize(), this->getColumnSize());
    SparseMatrix result = SparseMatrix::expand(MS, szs, szs);

    return result.maximum(SparseMatrix::IdentityMatrix(this->getRowSize()));
}

SparseMatrix SparseMatrix::expand(const Matrix &M, const Sizes &rszs, const Sizes &cszs) {
    unsigned int rSize = rszs.sum();
    unsigned int cSize = cszs.sum();
    SparseMatrix result(rSize, cSize);
    result.table.clear();
    unsigned int ri = 0;
    for (const auto &rs : rszs) {
        SparseVector rv(M.getRowVector(ri), cszs);
        result.table.emplace_back(rs, rv);
        ri++;
    }
    return result;
}

} // namespace MaxPlus
