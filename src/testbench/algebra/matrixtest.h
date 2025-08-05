#pragma once

#include <algorithm>

#include "algebra/mpmatrix.h"
#include "testing.h"

using namespace MaxPlus;

class MatrixTest : public ::testing::Test {

public:
    MatrixTest() {}

    virtual void SetUp() {};
    virtual void TearDown() {};
    int test_SetMPTimeInMatrix();
    int test_PasteMatrix();
    int test_SubMatrix();
    int test_Equality();
    int test_Addition();
    virtual void Run();
};
