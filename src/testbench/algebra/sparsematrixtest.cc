#include <algorithm>

#include "algebra/mpsparsematrix.h"
#include "sparsematrixtest.h"
#include "testing.h"

#define ASSERT_EPSILON 0.001

using namespace MaxPlus;

void SparseMatrixTest::Run() {
    this->test_StarClosure();
    this->test_EigenVectors();
    this->test_GetPutMatrix();
    this->test_Addition();
    this->test_Multiplication();
};

int SparseMatrixTest::test_Vectors() {
    std::cout << "Running test: Vectors" << std::endl;

    SparseVector v1(1000);
    v1.put(100, MPTime(0));
    v1.put(500, MPTime(2));

    SparseVector v2(1000);
    v2.put(100, MPTime(3));
    v2.put(400, MPTime(8));

    MPTime p = v1.innerProduct(v2);

    ASSERT_APPROX_EQUAL((CDouble)p, 3, ASSERT_EPSILON);

    return 0;
}

int SparseMatrixTest::test_StarClosure() {
    std::cout << "Running test: SparseStarClosure" << std::endl;

    SparseMatrix M(200, 200);
    M.putAll(90, 100, 90, 100, MPTime(-1.0));
    M.putAll(100, 110, 100, 110, MPTime(0.0));
    M.put(100, 99, MPTime(0.0));
    M.compress();

    auto C = M.starClosure();
    ASSERT_APPROX_EQUAL(static_cast<CDouble>(C.get(93, 90)), -1.0, ASSERT_EPSILON);

    return 0;
}

int SparseMatrixTest::test_EigenVectors() {
    std::cout << "Running test: EigenVectors" << std::endl;

    SparseMatrix M(200, 200);
    M.putAll(90, 100, 90, 100, MPTime(-1.0));
    M.putAll(100, 110, 100, 110, MPTime(3.0));
    M.put(100, 99, MPTime(0.0));

    MPTime lambda = M.mpEigenvalue();

    ASSERT_APPROX_EQUAL(static_cast<CDouble>(lambda), 3.0, ASSERT_EPSILON);

    auto ev = M.mpGeneralizedEigenvectors();
    auto ev1 = ev.first;
    auto ev2 = ev.second;

    ASSERT_EQUAL(ev1.size(), 1);
    auto vl1 = *ev1.begin();
    auto v1 = vl1.first;
    ASSERT_APPROX_EQUAL(static_cast<CDouble>(v1.get(105)), 0.0, ASSERT_EPSILON);
    auto l1 = vl1.second;
    ASSERT_APPROX_EQUAL(l1, 3.0, ASSERT_EPSILON);

    ASSERT_EQUAL(ev2.size(), 1);
    auto vl2 = *ev2.begin();
    auto v2 = vl2.first;
    ASSERT_APPROX_EQUAL(static_cast<CDouble>(v2.get(102)), 1.0, ASSERT_EPSILON);
    auto l2 = vl2.second;
    ASSERT_APPROX_EQUAL(static_cast<CDouble>(l2.get(100)), 3.0, ASSERT_EPSILON);

    return 0;
}

int SparseMatrixTest::test_GetPutMatrix() {
    std::cout << "Running test: GetPutMatrix" << std::endl;

    SparseMatrix M(200, 200);
    M.putAll(90, 100, 90, 100, MPTime(-1.0));
    M.putAll(100, 110, 100, 110, MPTime(3.0));
    M.put(100, 99, MPTime(0.0));

    ASSERT_APPROX_EQUAL(static_cast<CDouble>(M.get(90, 90)), -1.0, ASSERT_EPSILON);
    ASSERT_APPROX_EQUAL(static_cast<CDouble>(M.get(104, 104)), 3.0, ASSERT_EPSILON);
    ASSERT_APPROX_EQUAL(static_cast<CDouble>(M.get(100, 99)), 0.0, ASSERT_EPSILON);

    return 0;
}

int SparseMatrixTest::test_Addition() {
    std::cout << "Running test: Addition" << std::endl;

    SparseMatrix M(200, 200);
    M.putAll(0, 100, 100, 200, MPTime(5.0));
    M.putAll(100, 200, 0, 100, MPTime(-3.0));
    M.put(100, 99, MPTime(0.0));

    SparseMatrix N(M);
    N.transpose();
    SparseMatrix S = M.add(N);

    ASSERT_MP_MINUSINFINITY(static_cast<CDouble>(S.get(5, 5)));
    ASSERT_APPROX_EQUAL(static_cast<CDouble>(S.get(130, 30)), 2.0, ASSERT_EPSILON);
    return 0;
}
int SparseMatrixTest::test_Multiplication() {
    std::cout << "Running test: Multiplication" << std::endl;

    SparseVector v1(200);
    v1.putAll(100, 125, MPTime(-5.0));

    SparseMatrix M(200, 200);
    M.putAll(90, 100, 90, 100, MPTime(-1.0));
    M.putAll(100, 110, 100, 110, MPTime(0.0));
    M.put(100, 99, MPTime(0.0));
    SparseVector v2 = M.multiply(v1);
    v2.compress();

    // v2: [100 * -mp_inf ; 10 * -5 ; 90 * -mp_inf ]
    ASSERT_MP_MINUSINFINITY(static_cast<CDouble>(v2.get(60)));
    ASSERT_APPROX_EQUAL(static_cast<CDouble>(v2.get(105)), -5.0, ASSERT_EPSILON);

    // matrix matrix multiplication
    M.compress();
    SparseMatrix P = M.multiply(M);
    SparseMatrix Q = P.multiply(P);
    SparseMatrix R = Q.multiply(Q);
    SparseMatrix S = R.multiply(R);

    // S transposed
    // 90 * [200 * -mp_inf ]
    // 9  * [90 * -mp_inf ; 10 * -16 ; 10 * -1 ; 90 * -mp_inf ]
    // 1  * [90 * -mp_inf ; 10 * -16 ; 10 * 0 ; 90 * -mp_inf ]
    // 10 * [100 * -mp_inf ; 10 * 0 ; 90 * -mp_inf ]
    // 90 * [200 * -mp_inf ]

    ASSERT_APPROX_EQUAL((CDouble)S.get(95, 95), -16.0, ASSERT_EPSILON);
    ASSERT_APPROX_EQUAL((CDouble)S.get(105, 95), -1.0, ASSERT_EPSILON);

    return 0;
}
