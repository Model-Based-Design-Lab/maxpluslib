#pragma once

#include <algorithm>

#include "algebra/mpsparsematrix.h"
#include "testing.h"

class SparseMatrixTest : public ::testing::Test {

public:
    SparseMatrixTest() = default;

    void SetUp() override {};
    void TearDown() override {};
    void Run() override;

    int test_Vectors();
    int test_StarClosure();
    int test_EigenVectors();
    int test_GetPutMatrix();
    int test_Addition();
    int test_Multiplication();
};
