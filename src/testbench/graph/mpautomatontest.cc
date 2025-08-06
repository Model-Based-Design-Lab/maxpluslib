#include <algorithm>
#include <base/basic_types.h>
#include <memory>

#include "base/fsm/fsm.h"
#include "mpautomatontest.h"

#include "graph/mpautomaton.h"
#include "testing.h"

#define ASSERT_EPSILON 0.001

using namespace MaxPlus;

// NOLINTBEGIN(*magic-numbers,*simplify-boolean-expr)

MPAutomatonTest::MPAutomatonTest() = default;

void MPAutomatonTest::SetUp() {}

void MPAutomatonTest::TearDown() {}

void MPAutomatonTest::Run() {
    testCreateFSM();
    testDeterminizeFSM();
    testMinimizeFSM();
    testDFSFSM();
    testDetectCycleFSM();
}

void MPAutomatonTest::testCreateFSM() { // NOLINT(*to-static)

    std::cout << "Running test: CreateFSM\n";

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
        const auto *e = dynamic_cast<EdgeRef<MPAStateLabel, MPAREdgeLabel>>(i);

        ASSERT_THROW(((e->getDestination())) == s2 || ((e->getDestination())) == s3);
    }

    mpa.setInitialState(*s1);

    const auto *s = mpa.checkStateLabeled(makeMPAStateLabel(0, 1));
    ASSERT_THROW(s != nullptr);

    const auto *e = mpa.findEdge(makeMPAStateLabel(0, 0),
                          makeRewardEdgeLabel(MPTime(3.0), MPString("A"), 1.0),
                          makeMPAStateLabel(0, 1));
    ASSERT_THROW(e != nullptr);

    ASSERT_EQUAL_NOPRINT(s1, mpa.getInitialState());
}

void MPAutomatonTest::testDeterminizeFSM() { // NOLINT(*to-static)

    std::cout << "Running test: DeterminizeFSM\n";

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

    auto determinizedAutomaton = mpa.determinizeEdgeLabels();
    std::unique_ptr<MaxPlusAutomatonWithRewards> mpaDeterminized = std::unique_ptr<MaxPlusAutomatonWithRewards>(dynamic_cast<MaxPlusAutomatonWithRewards *>(determinizedAutomaton.release()));

    ASSERT_THROW(mpaDeterminized->getStates().size() == 2);
    ASSERT_THROW(mpaDeterminized->getEdges().size() == 3);

    CDouble mcr = mpa.calculateMCR();
    std::vector<const MPAREdge *> cycle;
    CDouble mcr1 = mpaDeterminized->calculateMCRAndCycle(&cycle);

    ASSERT_APPROX_EQUAL(mcr, mcr1, ASSERT_EPSILON);
    ASSERT_EQUAL(cycle.size(), 2);
}

void MPAutomatonTest::testMinimizeFSM() { // NOLINT(*to-static)

    std::cout << "Running test: MinimizeFSM\n";

    FSM::Labeled::FiniteStateMachine<int, int> fsa;

    const auto *s0 = fsa.addState(3);
    const auto *s1 = fsa.addState(5);
    const auto *s2 = fsa.addState(5);

    fsa.addEdge(*s0, 2, *s1);
    fsa.addEdge(*s1, 2, *s2);
    fsa.addEdge(*s2, 2, *s2);

    fsa.setInitialState(*s0);

    const auto fsaMin = std::unique_ptr<FSM::Labeled::FiniteStateMachine<int, int>>(
            dynamic_cast<FSM::Labeled::FiniteStateMachine<int, int>*>(fsa.minimizeEdgeLabels().release()));

    std::cout << "Nr states: " << fsaMin->getStates().size() << "\n";
    std::cout << "Nr edges: " << fsaMin->getEdges().size() << "\n";

    ASSERT_EQUAL(fsaMin->getStates().size(), 2);
    ASSERT_EQUAL(fsaMin->getEdges().size(), 2);
}

void MPAutomatonTest::testDFSFSM() { // NOLINT(*to-static)

    std::cout << "Running test: Depth-First Search FSM\n";

    {
        FSM::Labeled::FiniteStateMachine<int, int> fsa;

        const auto *s0 = fsa.addState(0);
        const auto *s1 = fsa.addState(1);
        const auto *s2 = fsa.addState(2);
        const auto *s3 = fsa.addState(3);
        const auto *s4 = fsa.addState(4);

        fsa.setInitialState(*s0);
        fsa.addInitialState(*s2);

        fsa.addEdge(*s0, 2, *s1);
        fsa.addEdge(*s1, 2, *s1);
        fsa.addEdge(*s2, 2, *s1);
        fsa.addEdge(*s2, 2, *s3);
        fsa.addEdge(*s3, 2, *s2);
        fsa.addEdge(*s4, 2, *s3);

        bool foundCycle = false;
        FSM::Abstract::SetOfStateRefs statesFound;

        FSM::Abstract::DepthFirstSearchLambda DFS(fsa);

        DFS.setOnEnterLambda([&statesFound](FSM::Abstract::StateRef s) {
            ASSERT(!statesFound.includesState(s));
            statesFound.insert(s);
        });
        DFS.setOnSimpleCycleLambda(
                [&foundCycle](const FSM::Abstract::DepthFirstSearch::DfsStack &) {
                    foundCycle = true;
                });

        // from all initial states

        DFS.DoDepthFirstSearch();
        ASSERT(foundCycle);
        ASSERT(statesFound.size() == 4);
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

        bool foundCycle = false;
        FSM::Abstract::SetOfStateRefs statesFound;

        FSM::Abstract::DepthFirstSearchLambda DFS(fsa);

        DFS.setOnEnterLambda([&statesFound](FSM::Abstract::StateRef s) {
            ASSERT(!statesFound.includesState(s));
            statesFound.insert(s);
        });
        DFS.setOnSimpleCycleLambda(
                [&foundCycle](const FSM::Abstract::DepthFirstSearch::DfsStack &) {
                    foundCycle = true;
                });

        // from all initial states

        DFS.DoDepthFirstSearch(s2);
        ASSERT(!foundCycle);
        ASSERT(statesFound.size() == 2);
    }
}

void MPAutomatonTest::testDetectCycleFSM() { // NOLINT(*to-static)

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
        const auto *e = fsa.addEdge(*s3, 2, *s1);

        fsa.setEdgeLabel(e, 5);

        FSM::Abstract::DetectCycle DC(fsa);
        bool hasCycle = DC.checkForCycles();

        ASSERT(hasCycle);
    }
}

// NOLINTEND(*magic-numbers,*simplify-boolean-expr)
