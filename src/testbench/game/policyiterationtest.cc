#include "game/policyiteration.h"
#include "game/mpgameautomaton.h"
#include "game/strategyvector.h"
#include <algorithm>

#include "policyiterationtest.h"
#include "testing.h"

using namespace MaxPlus;

PolicyIterationTest::PolicyIterationTest() = default;

void PolicyIterationTest::Run() {
    testPlayer1CycleTest();
    testPlayer1CycleTest2();
    testTwoPlayersTest();
    testSimpleTest();
    testInvalidInputGraphTest();
};

void PolicyIterationTest::SetUp() {};

void PolicyIterationTest::TearDown() {};

void PolicyIterationTest::testPlayer1CycleTest() {

    std::cout << "Running test: Player1CycleTest" << std::endl;

    MaxPlusGameAutomatonWithRewards mpa;

    // One FSM state, four tokens:
    CId fsm_s0 = 0;

    MPARStateRef s1 = mpa.addState(makeMPAStateLabel(fsm_s0, 0));
    MPARStateRef s2 = mpa.addState(makeMPAStateLabel(fsm_s0, 1));
    MPARStateRef s3 = mpa.addState(makeMPAStateLabel(fsm_s0, 2));
    MPARStateRef s4 = mpa.addState(makeMPAStateLabel(fsm_s0, 3));
    mpa.addV1(s1);
    mpa.addV1(s2);
    mpa.addV1(s3);
    mpa.addV1(s4);

    mpa.addEdge(*s1, makeRewardEdgeLabel(MPDelay(1.0), MPString("A"), 1.0), *s2);
    mpa.addEdge(*s2, makeRewardEdgeLabel(MPDelay(1.0), MPString("A"), 2.0), *s3);
    mpa.addEdge(*s3, makeRewardEdgeLabel(MPDelay(1.0), MPString("A"), 3.0), *s4);
    mpa.addEdge(*s4, makeRewardEdgeLabel(MPDelay(1.0), MPString("A"), 4.0), *s2);

    PolicyIteration<MPAStateLabel, MPAREdgeLabel> pi;
    PolicyIteration<MPAStateLabel, MPAREdgeLabel>::PolicyIterationResult result = pi.solve(mpa);

    double expected_ratio = 9.0 / 3.0;
    const std::map<const State<MPAStateLabel, MPAREdgeLabel> *, double> &values = result.values;

    ASSERT_DOUBLE_EQUAL(expected_ratio, values.at(s1));
    ASSERT_DOUBLE_EQUAL(expected_ratio, values.at(s2));
    ASSERT_DOUBLE_EQUAL(expected_ratio, values.at(s3));
    ASSERT_DOUBLE_EQUAL(expected_ratio, values.at(s4));
}

void PolicyIterationTest::testPlayer1CycleTest2() {

    std::cout << "Running test: Player1CycleTest2" << std::endl;

    MaxPlusGameAutomatonWithRewards mpa;

    // One FSM state, four tokens:
    CId fsm_s0 = 0;

    MPARStateRef s1 = mpa.addState(makeMPAStateLabel(fsm_s0, 0));
    MPARStateRef s2 = mpa.addState(makeMPAStateLabel(fsm_s0, 1));
    MPARStateRef s3 = mpa.addState(makeMPAStateLabel(fsm_s0, 2));
    MPARStateRef s4 = mpa.addState(makeMPAStateLabel(fsm_s0, 3));

    mpa.addV1(s1);
    mpa.addV1(s2);
    mpa.addV1(s3);
    mpa.addV1(s4);

    mpa.addEdge(*s1, makeRewardEdgeLabel(MPDelay(3.0), MPString("A"), 1.0), *s2);
    mpa.addEdge(*s2, makeRewardEdgeLabel(MPDelay(3.0), MPString("A"), 2.0), *s3);
    mpa.addEdge(*s3, makeRewardEdgeLabel(MPDelay(1.0), MPString("A"), 3.0), *s4);
    mpa.addEdge(*s4, makeRewardEdgeLabel(MPDelay(7.0), MPString("A"), 4.0), *s2);

    PolicyIteration<MPAStateLabel, MPAREdgeLabel> *pi =
            new PolicyIteration<MPAStateLabel, MPAREdgeLabel>();
    PolicyIteration<MPAStateLabel, MPAREdgeLabel>::PolicyIterationResult result = pi->solve(mpa);

    double expected_ratio = 9.0 / 11.0;
    std::map<const State<MPAStateLabel, MPAREdgeLabel> *, double> values = result.values;

    ASSERT_DOUBLE_EQUAL(expected_ratio, values[s1]);
    ASSERT_DOUBLE_EQUAL(expected_ratio, values[s2]);
    ASSERT_DOUBLE_EQUAL(expected_ratio, values[s3]);
    ASSERT_DOUBLE_EQUAL(expected_ratio, values[s4]);
}

void PolicyIterationTest::testTwoPlayersTest() {

    std::cout << "Running test: TwoPlayersTest" << std::endl;

    MaxPlusGameAutomatonWithRewards mpa;

    // One FSM state, four tokens:
    CId fsm_s0 = 0;

    MPARStateRef s1 = mpa.addState(makeMPAStateLabel(fsm_s0, 0));
    MPARStateRef s2 = mpa.addState(makeMPAStateLabel(fsm_s0, 1));
    MPARStateRef s3 = mpa.addState(makeMPAStateLabel(fsm_s0, 2));
    MPARStateRef s4 = mpa.addState(makeMPAStateLabel(fsm_s0, 3));
    MPARStateRef s5 = mpa.addState(makeMPAStateLabel(fsm_s0, 4));

    mpa.addV0(s1);
    mpa.addV0(s3);
    mpa.addV0(s5);
    mpa.addV1(s2);
    mpa.addV1(s4);

    mpa.addEdge(*s1, makeRewardEdgeLabel(MPDelay(1.0), MPString("A"), 0.0), *s4);
    mpa.addEdge(*s1, makeRewardEdgeLabel(MPDelay(1.0), MPString("A"), 0.0), *s2);
    mpa.addEdge(*s4, makeRewardEdgeLabel(MPDelay(1.0), MPString("A"), 0.0), *s5);
    mpa.addEdge(*s5, makeRewardEdgeLabel(MPDelay(2.0), MPString("A"), 1.0), *s4);
    mpa.addEdge(*s2, makeRewardEdgeLabel(MPDelay(0.0), MPString("A"), 3.0), *s3);
    mpa.addEdge(*s3, makeRewardEdgeLabel(MPDelay(2.0), MPString("A"), 4.0), *s2);

    PolicyIteration<MPAStateLabel, MPAREdgeLabel> *pi =
            new PolicyIteration<MPAStateLabel, MPAREdgeLabel>();
    PolicyIteration<MPAStateLabel, MPAREdgeLabel>::PolicyIterationResult result = pi->solve(mpa);

    double val1 = 7.0 / 2.0;
    double val2 = 1.0 / 3.0;
    std::map<const State<MPAStateLabel, MPAREdgeLabel> *, double> values = result.values;

    ASSERT_DOUBLE_EQUAL(val1, values[s1]);
    ASSERT_DOUBLE_EQUAL(val1, values[s2]);
    ASSERT_DOUBLE_EQUAL(val1, values[s3]);
    ASSERT_DOUBLE_EQUAL(val2, values[s4]);
    ASSERT_DOUBLE_EQUAL(val2, values[s5]);
}

void PolicyIterationTest::testSimpleTest() {

    std::cout << "Running test: SimpleTest" << std::endl;

    MaxPlusGameAutomatonWithRewards mpa;

    // One FSM state, three tokens:
    CId fsm_s0 = 0;

    MPARStateRef s1 = mpa.addState(makeMPAStateLabel(fsm_s0, 0));
    MPARStateRef s2 = mpa.addState(makeMPAStateLabel(fsm_s0, 1));
    MPARStateRef s3 = mpa.addState(makeMPAStateLabel(fsm_s0, 2));

    mpa.addEdge(*s1, makeRewardEdgeLabel(MPDelay(3.0), MPString("A"), 1.0), *s2);
    mpa.addEdge(*s1, makeRewardEdgeLabel(MPDelay(3.0), MPString("A"), 1.0), *s3);
    mpa.addEdge(*s2, makeRewardEdgeLabel(MPDelay(1.0), MPString("A"), 1.0), *s1);
    mpa.addEdge(*s3, makeRewardEdgeLabel(MPDelay(7.0), MPString("A"), 1.0), *s1);
    mpa.addV0(s1);
    mpa.addV1(s2);
    mpa.addV1(s3);

    PolicyIteration<MPAStateLabel, MPAREdgeLabel> *pi =
            new PolicyIteration<MPAStateLabel, MPAREdgeLabel>();
    PolicyIteration<MPAStateLabel, MPAREdgeLabel>::PolicyIterationResult result = pi->solve(mpa);
}

void PolicyIterationTest::testInvalidInputGraphTest() {

    std::cout << "Running test: InvalidInputGraphTest" << std::endl;

    MaxPlusGameAutomatonWithRewards mpa;

    // One FSM state, three tokens:
    CId fsm_s0 = 0;

    MPARStateRef s1 = mpa.addState(makeMPAStateLabel(fsm_s0, 0));
    MPARStateRef s2 = mpa.addState(makeMPAStateLabel(fsm_s0, 1));

    mpa.addEdge(*s1, makeRewardEdgeLabel(MPDelay(3.0), MPString("A"), 1.0), *s2);
    mpa.addV0(s1);
    mpa.addV1(s2);

    try {
        PolicyIteration<MPAStateLabel, MPAREdgeLabel> *pi =
                new PolicyIteration<MPAStateLabel, MPAREdgeLabel>();
        PolicyIteration<MPAStateLabel, MPAREdgeLabel>::PolicyIterationResult result =
                pi->solve(mpa);
    } catch (const std::runtime_error &) {
        // test passes
        // SUCCEED();
    }
}
