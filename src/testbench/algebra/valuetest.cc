#include <algorithm>

#include "algebra/mptype.h"
#include "testing.h"
#include "valuetest.h"

using namespace MaxPlus;

void ValueTest::Run() {
    this->test_mpTime();
    this->test_Max();
    this->test_Min();
    this->test_BasicArithmetic();
};

// Test infinity operations.
void ValueTest::test_mpTime() {
    std::cout << "Running test: mpTime" << std::endl;
    ASSERT_EQUAL(1.0, static_cast<CDouble>(MPTime(1.0)));
}

/// Test max operator.
void ValueTest::test_Max() {
    std::cout << "Running test: Max" << std::endl;
    // Test normal CDouble values.
    CDouble a = 5.0;
    CDouble b = 3.14;
    CDouble max_ab = 5;

    ASSERT_EQUAL(static_cast<CDouble>(MPTime(max_ab)),
                 static_cast<CDouble>(MP_MAX(MPTime(a), MPTime(b))));
    ASSERT_EQUAL(static_cast<CDouble>(MPTime(max_ab)),
                 static_cast<CDouble>(MP_MAX(MPTime(b), MPTime(a))));
    ASSERT_EQUAL(static_cast<CDouble>(MPTime(b)),
                 static_cast<CDouble>(MP_MAX(MPTime(-a), MPTime(b))));
    ASSERT_EQUAL(static_cast<CDouble>(MPTime(a)),
                 static_cast<CDouble>(MP_MAX(MPTime(-b), MPTime(a))));

    // -Infinity case.
    ASSERT_THROW((MP_MAX(MP_MINUS_INFINITY, MP_MINUS_INFINITY)).isMinusInfinity());
    ASSERT_EQUAL(static_cast<CDouble>(MPTime(3.14)),
                 static_cast<CDouble>(MP_MAX(MPTime(3.14), MP_MINUS_INFINITY)));
}

/// Test min operator.
void ValueTest::test_Min() {
    std::cout << "Running test: Min" << std::endl;
    // Test normal CDouble values.
    ASSERT_EQUAL(static_cast<CDouble>(MPTime(3.14)),
                 static_cast<CDouble>(MP_MIN(MPTime(5.0), MPTime(3.14))));
    ASSERT_EQUAL(static_cast<CDouble>(MPTime(3.14)),
                 static_cast<CDouble>(MP_MIN(MPTime(3.14), MPTime(5.0))));
    ASSERT_EQUAL(static_cast<CDouble>(MPTime(-5.0)),
                 static_cast<CDouble>(MP_MIN(MPTime(-5.0), MPTime(3.14))));
    ASSERT_EQUAL(static_cast<CDouble>(MPTime(-3.14)),
                 static_cast<CDouble>(MP_MIN(MPTime(-3.14), MPTime(5.0))));

    // -Infinity case.
    ASSERT_THROW(MP_MIN(MP_MINUS_INFINITY, MP_MINUS_INFINITY).isMinusInfinity());
    ASSERT_THROW(MP_MIN(MPTime(-3.14), MP_MINUS_INFINITY).isMinusInfinity());
    ASSERT_THROW(MP_MIN(MPTime(3.14), MP_MINUS_INFINITY).isMinusInfinity());
}

/// Test basic arithmetic operations.
void ValueTest::test_BasicArithmetic() {
    std::cout << "Running test: BasicArithmetic" << std::endl;
    MPTime a(3.14);
    MPTime b(6.0);

    // Addition.
    ASSERT_EQUAL(static_cast<CDouble>(MPTime(9.14)), static_cast<CDouble>(a + b));
    ASSERT_EQUAL(static_cast<CDouble>(MPTime(9.14)), static_cast<CDouble>(b + a));
    // Subtraction.
    ASSERT_EQUAL(static_cast<CDouble>(MPTime(2.86)), static_cast<CDouble>(b - a));
    ASSERT_EQUAL(static_cast<CDouble>(MPTime(-2.86)), static_cast<CDouble>(a - b));
    // Multiplication.
    ASSERT_EQUAL(static_cast<CDouble>(MPTime(18.84)), static_cast<CDouble>(a * b));
    // Multiplication.
    ASSERT_EQUAL(static_cast<CDouble>(MPTime(18.84)), static_cast<CDouble>(b * a));
}
