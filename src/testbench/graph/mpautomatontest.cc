#include <algorithm>
#include <base/basic_types.h>
#include <memory>

#include "base/fsm/fsm.h"
#include "mpautomatontest.h"

#include "graph/mpautomaton.h"
#include "testing.h"

#define ASSERT_EPSILON 0.001

using namespace MaxPlus;

MPAutomatonTest::MPAutomatonTest() = default;

void MPAutomatonTest::SetUp() {}

void MPAutomatonTest::TearDown() {}

void MPAutomatonTest::Run() {
    testCreateFSM();
    testDeterminizeFSM();
    testMinimizeFSM();
    testDetectCycleFSM();
}

void MPAutomatonTest::testCreateFSM() {

    std::cout << "Running test: CreateFSM" << std::endl;

    MaxPlusAutomatonWithRewards mpa;

    MPARStateRef s1 = mpa.addState(makeMPAStateLabel(0, 0));
    MPARStateRef s2 = mpa.addState(makeMPAStateLabel(0, 1));
    MPARStateRef s3 = mpa.addState(makeMPAStateLabel(0, 2));

    mpa.addEdge(*s1, makeRewardEdgeLabel(MPTime(3.0), MPString("A"), 1.0), *s2);
    mpa.addEdge(*s1, makeRewardEdgeLabel(MPTime(3.0), MPString("A"), 1.0), *s3);
    mpa.addEdge(*s2, makeRewardEdgeLabel(MPTime(1.0), MPString("A"), 1.0), *s1);
    mpa.addEdge(*s3, makeRewardEdgeLabel(MPTime(7.0), MPString("A"), 1.0), *s1);

    auto es = dynamic_cast<const FSM::Abstract::SetOfEdgeRefs &>(s1->getOutgoingEdges());
    for (const auto &i : es) {
        auto *e = dynamic_cast<EdgeRef<MPAStateLabel, MPAREdgeLabel>>(i);

        ASSERT_THROW(((e->getDestination())) == s2 || ((e->getDestination())) == s3);
    }

    mpa.setInitialState(*s1);

    auto s = mpa.checkStateLabeled(makeMPAStateLabel(0, 1));
    ASSERT_THROW(s != nullptr);

    auto e = mpa.findEdge(makeMPAStateLabel(0, 0),
                          makeRewardEdgeLabel(MPTime(3.0), MPString("A"), 1.0),
                          makeMPAStateLabel(0, 1));
    ASSERT_THROW(e != nullptr);

    ASSERT_EQUAL_NOPRINT(s1, mpa.getInitialState());
}

void MPAutomatonTest::testDeterminizeFSM() {

    std::cout << "Running test: DeterminizeFSM" << std::endl;

    MaxPlusAutomatonWithRewards mpa;

    MPARStateRef s1 = mpa.addState(makeMPAStateLabel(0, 0));
    MPARStateRef s2 = mpa.addState(makeMPAStateLabel(0, 1));
    MPARStateRef s3 = mpa.addState(makeMPAStateLabel(0, 2));

    // s1 -- (3,A,1) -> s2
    mpa.addEdge(*s1, makeRewardEdgeLabel(MPTime(3.0), MPString("A"), 1.0), *s2);
    // s1 -- (3,A,1) -> s3
    mpa.addEdge(*s1, makeRewardEdgeLabel(MPTime(3.0), MPString("A"), 1.0), *s3);
    // s2 -- (1,B,1) -> s1
    mpa.addEdge(*s2, makeRewardEdgeLabel(MPTime(1.0), MPString("B"), 1.0), *s1);
    // s3 -- (7,A,1) -> s1
    mpa.addEdge(*s3, makeRewardEdgeLabel(MPTime(7.0), MPString("A"), 1.0), *s1);

    auto es = dynamic_cast<const FSM::Abstract::SetOfEdgeRefs &>(s1->getOutgoingEdges());
    for (const auto &i : es) {
        const auto *e = dynamic_cast<EdgeRef<MPAStateLabel, MPAREdgeLabel>>(i);

        ASSERT_THROW((e->getDestination()) == s2 || (e->getDestination()) == s3);
    }

    mpa.setInitialState(*s1);

    std::shared_ptr<MaxPlusAutomatonWithRewards> mpaDeterminized =
            std::dynamic_pointer_cast<MaxPlusAutomatonWithRewards>(mpa.determinizeEdgeLabels());

    ASSERT_THROW(mpaDeterminized->getStates().size() == 2);
    ASSERT_THROW(mpaDeterminized->getEdges().size() == 3);

    CDouble mcr = mpa.calculateMCR();
    std::shared_ptr<std::vector<const MPAREdge *>> cycle;
    CDouble mcr1 = mpaDeterminized->calculateMCRAndCycle(&cycle);

    ASSERT_APPROX_EQUAL(mcr, mcr1, ASSERT_EPSILON);
    ASSERT_EQUAL(cycle->size(), 2);
}

void MPAutomatonTest::testMinimizeFSM() {

    std::cout << "Running test: MinimizeFSM\n";

    FSM::Labeled::FiniteStateMachine<int, int> fsa;

    const auto *s0 = fsa.addState(3);
    const auto *s1 = fsa.addState(5);
    const auto *s2 = fsa.addState(5);

    fsa.addEdge(*s0, 2, *s1);
    fsa.addEdge(*s1, 2, *s2);
    fsa.addEdge(*s2, 2, *s2);

    fsa.setInitialState(*s0);

    const auto fsaMin = std::dynamic_pointer_cast<FSM::Labeled::FiniteStateMachine<int, int>>(
            fsa.minimizeEdgeLabels());

    std::cout << "Nr states: " << fsaMin->getStates().size() << std::endl;
    std::cout << "Nr edges: " << fsaMin->getEdges().size() << std::endl;

    ASSERT_EQUAL(fsaMin->getStates().size(), 2);
    ASSERT_EQUAL(fsaMin->getEdges().size(), 2);
}

void MPAutomatonTest::testDetectCycleFSM() {

    std::cout << "Running test: DetectCycles\n";

    {
        FSM::Labeled::FiniteStateMachine<int, int> fsa;

        const auto *s0 = fsa.addState(3);
        const auto *s1 = fsa.addState(5);
        const auto *s2 = fsa.addState(5);
        const auto *s3 = fsa.addState(4);

        fsa.addEdge(*s3, 2, *s1);
        fsa.addEdge(*s1, 2, *s2);
        fsa.addEdge(*s2, 2, *s0);
        fsa.addEdge(*s3, 2, *s2);
        fsa.addEdge(*s1, 2, *s0);

        FSM::Abstract::DetectCycle DC(fsa);
        bool hasCycle = DC.checkForCycles();

        ASSERT(!hasCycle);
    }

    {
        FSM::Labeled::FiniteStateMachine<int, int> fsa;

        const auto *s2 = fsa.addState(5);
        const auto *s3 = fsa.addState(4);
        const auto *s0 = fsa.addState(3);
        const auto *s1 = fsa.addState(5);

        fsa.addEdge(*s0, 2, *s1);
        fsa.addEdge(*s1, 2, *s2);
        fsa.addEdge(*s2, 2, *s3);
        fsa.addEdge(*s3, 2, *s1);

        FSM::Abstract::DetectCycle DC(fsa);
        bool hasCycle = DC.checkForCycles();

        ASSERT(hasCycle);
    }
}
