#include <algorithm>

#include "algebra/mpmatrix.h"
#include "matrixtest.h"
#include "testing.h"

using namespace MaxPlus;

void MatrixTest::Run() {
    this->test_SetMPTimeInMatrix();
    this->test_PasteMatrix();
    this->test_SubMatrix();
    this->test_Equality();
    this->test_Addition();
};

int MatrixTest::test_SetMPTimeInMatrix() {
    std::cout << "Running test: SetMPTimeInMatrix" << std::endl;
    unsigned int N = 2;
    Matrix m(N, N, MatrixFill::MinusInfinity);
    ASSERT_EQUAL(N, m.getRows());
    ASSERT_EQUAL(N, m.getCols());
    ASSERT_EQUAL(N, m.getSize());
    ASSERT_EQUAL(static_cast<CDouble>(MP_MINUSINFINITY), static_cast<CDouble>(m.get(0, 0)));

    // Change entry and restore.
    MPTime newVal(3.0);

    // Change entry.
    m.put(0, 0, newVal);

    for (unsigned int row = 0; row < N; row++) {
        for (unsigned int column = 0; column < N; column++) {
            if (row == 0 && column == 0) {
                ASSERT_EQUAL(static_cast<CDouble>(newVal), static_cast<CDouble>(m.get(0, 0)));
            } else {
                ASSERT_EQUAL(static_cast<CDouble>(MP_MINUSINFINITY),
                             static_cast<CDouble>(m.get(row, column)));
            }
        }
    }

    return 0;
}

int MatrixTest::test_PasteMatrix() {
    std::cout << "Running test: PasteMatrix" << std::endl;

    Matrix m(3, 3, MatrixFill::MinusInfinity);
    Matrix mSub(2, 2, MatrixFill::Zero);

    m.paste(0, 0, &mSub);
    ASSERT_EQUAL(0.0, static_cast<CDouble>(m.get(0, 0)));
    ASSERT_EQUAL(0.0, static_cast<CDouble>(m.get(0, 1)));
    ASSERT_EQUAL(static_cast<CDouble>(MP_MINUSINFINITY), static_cast<CDouble>(m.get(0, 2)));
    ASSERT_EQUAL(0.0, static_cast<CDouble>(m.get(1, 0)));
    ASSERT_EQUAL(0.0, static_cast<CDouble>(m.get(1, 1)));
    ASSERT_EQUAL(static_cast<CDouble>(MP_MINUSINFINITY), static_cast<CDouble>(m.get(1, 2)));
    ASSERT_EQUAL(static_cast<CDouble>(MP_MINUSINFINITY), static_cast<CDouble>(m.get(2, 0)));
    ASSERT_EQUAL(static_cast<CDouble>(MP_MINUSINFINITY), static_cast<CDouble>(m.get(2, 1)));
    ASSERT_EQUAL(static_cast<CDouble>(MP_MINUSINFINITY), static_cast<CDouble>(m.get(2, 2)));

    return 0;
}

int MatrixTest::test_SubMatrix() {
    std::cout << "Running test: SubMatrix" << std::endl;

    Matrix m(5, 5, MatrixFill::Identity);
    std::list<unsigned int> l = {0, 1};

    Matrix sub = m.getSubMatrix(l, l);
    ASSERT_EQUAL(0.0, static_cast<CDouble>(sub.get(0, 0)));
    ASSERT_EQUAL(static_cast<CDouble>(MP_MINUSINFINITY), static_cast<CDouble>(sub.get(0, 1)));
    ASSERT_EQUAL(static_cast<CDouble>(MP_MINUSINFINITY), static_cast<CDouble>(sub.get(1, 0)));
    ASSERT_EQUAL(0.0, static_cast<CDouble>(sub.get(1, 1)));

    return 0;
}

int MatrixTest::test_Equality() {
    // Equality is not implemented!
    // Matrix m1(3, 3, MatrixFill::MinusInfinity);
    // Matrix m2(3, 3, MatrixFill::MinusInfinity);
    // ASSERT_THROW(m1==m2);

    return 0;
}

int MatrixTest::test_Addition() {
    std::cout << "Running test: Addition" << std::endl;

    Matrix m(3, 3, MatrixFill::Identity);
    Matrix mResult = m.add(MPTime(3.0));

    Matrix m2(3, 3, MatrixFill::Identity);
    Matrix mResult2(3, 3, MatrixFill::MinusInfinity);

    m2.add(MPTime(3.0), mResult2);

    for (int row = 0; row < 3; row++) {
        for (int column = 0; column < 3; column++) {
            if (row == column) {
                ASSERT_EQUAL(3.0, static_cast<CDouble>(mResult.get(row, column)));
                ASSERT_EQUAL(3.0, static_cast<CDouble>(mResult2.get(row, column)));
            } else {
                ASSERT_EQUAL(static_cast<CDouble>(MP_MINUSINFINITY),
                             static_cast<CDouble>(mResult.get(row, column)));
                ASSERT_EQUAL(static_cast<CDouble>(MP_MINUSINFINITY),
                             static_cast<CDouble>(mResult2.get(row, column)));
            }
        }
    }

    return 0;
}
