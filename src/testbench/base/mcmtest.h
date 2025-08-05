#pragma once

#include <algorithm>

#include "algebra/mptype.h"
#include "testing.h"

class MCMTest : public ::testing::Test {

public:
    MCMTest() {}
    virtual void Run();
    virtual void SetUp() {};
    virtual void TearDown() {};

    void test_dg();
    void test_howard();
    void test_karp();
    void test_yto();
    void test_prune();
};
