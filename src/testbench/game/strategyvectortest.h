#pragma once

#include "game/mpgameautomaton.h"
#include "game/strategyvector.h"
#include "testing.h"
#include <algorithm>

class StrategyVectorTest : public ::testing::Test {

public:
    StrategyVectorTest();

    void Run() override;
    void SetUp() override;
    void TearDown() override;

    void testSimpleTest();
};
