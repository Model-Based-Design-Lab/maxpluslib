#include "game/strategyvector.h"
#include "game/mpgameautomaton.h"
#include <algorithm>

#include "strategyvectortest.h"
#include "testing.h"

using namespace MaxPlus;

StrategyVectorTest::StrategyVectorTest() = default;

void StrategyVectorTest::Run() { testSimpleTest(); }

void StrategyVectorTest::SetUp() {}
void StrategyVectorTest::TearDown() {}

void StrategyVectorTest::testSimpleTest() {

    std::cout << "Running test: SimpleTest" << std::endl;

    MaxPlusGameAutomatonWithRewards mpa;

    // One FSM state, three tokens:
    CId fsm_s0 = 0;

    MPARState *s1 = mpa.addState(makeMPAStateLabel(fsm_s0, 0));
    MPARState *s2 = mpa.addState(makeMPAStateLabel(fsm_s0, 1));
    MPARState *s3 = mpa.addState(makeMPAStateLabel(fsm_s0, 2));

    mpa.addEdge(*s1, makeRewardEdgeLabel(MPDelay(3.0), CString("A"), 1.0), *s2);
    mpa.addEdge(*s1, makeRewardEdgeLabel(MPDelay(3.0), CString("A"), 1.0), *s3);
    mpa.addEdge(*s2, makeRewardEdgeLabel(MPDelay(1.0), CString("A"), 1.0), *s1);
    mpa.addEdge(*s3, makeRewardEdgeLabel(MPDelay(7.0), CString("A"), 1.0), *s1);
    mpa.addV0(s1);
    mpa.addV1(s2);
    mpa.addV1(s3);

    // Get a strategy vector on the given game.
    StrategyVector<MPAStateLabel, MPAREdgeLabel> vec;
    vec.initializeRandomStrategy(mpa);

    const MPARState *result = vec.getSuccessor(s1);
    ASSERT_THROW(result == s2 || result == s3);
}
