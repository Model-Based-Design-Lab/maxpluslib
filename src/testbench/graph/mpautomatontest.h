#pragma once

#include <algorithm>
#include <base/basic_types.h>

#include "testing.h"

#include "graph/mpautomaton.h"

class MPAutomatonTest : public ::testing::Test {

public:
    MPAutomatonTest();

    void SetUp() override;
    void TearDown() override;
    void Run() override;
    void testCreateFSM();
    void testDeterminizeFSM();
    void testMinimizeFSM();
    void testDetectCycleFSM();
    void testDFSFSM();
};
