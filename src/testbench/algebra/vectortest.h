#pragma once

#include <algorithm>

#include "algebra/mpmatrix.h"

class VectorTest : public ::testing::Test {

public:
    VectorTest() {}

    virtual void Run();
    virtual void SetUp() {};
    virtual void TearDown() {};
    void test_Infinity();
};
