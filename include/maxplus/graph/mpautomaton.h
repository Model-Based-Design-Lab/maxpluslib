/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   mpautomaton.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *                  :   Bram van der Sanden (b.v.d.sanden@tue.nl)
 *
 *  Date            :   April 26, 2010
 *
 *  Function        :   Max plus automaton.
 *
 *  History         :
 *      26-04-10    :   Initial version.
 *      12-06-17    :   Max-plus library.
 *
 *
 *  Copyright 2023 Eindhoven University of Technology
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the “Software”),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#ifndef MAXPLUS_GRAPH_AUTOMATON_H_INCLUDED
#define MAXPLUS_GRAPH_AUTOMATON_H_INCLUDED

#include "maxplus/algebra/mptype.h"
#include "maxplus/base/fsm/fsm.h"
#include <list>
#include <mutex>
#include <thread>

namespace MaxPlus {

using namespace ::FSM::Labeled;
/**
 * MPA state is labeled with the FSM state ID, and the token number.
 */
using MPAStateLabel = struct MPAStateLabel {
    CId id;
    int tokenNr; // use -1 to indicate unassigned
};

/**
 * Create a new state label.
 * @param stateId FSM state id
 * @param tokenId token number
 */
inline MPAStateLabel makeMPAStateLabel(CId stateId, unsigned int tokenId) {
    MPAStateLabel sl;
    sl.id = stateId;
    sl.tokenNr = tokenId;
    return sl;
}

inline bool operator==(const MPAStateLabel &s, const MPAStateLabel &t) {
    return s.id == t.id && s.tokenNr == t.tokenNr;
}

/**
 * Compare MPA state labels using a lexicographical ordering.
 */
inline bool operator<(const MPAStateLabel &s, const MPAStateLabel &t) {
    if (s.id < t.id) {
        return true;
    }
    if (s.id > t.id) {
        return false;
    }
    return s.tokenNr < t.tokenNr;
}

inline CString toString(const MPAStateLabel &l) {
    return "(id: " + CString(l.id) + ", tokenNr: " + CString(l.tokenNr) + ")";
};

/**
 * An MPA edge is labeled with a delay and a scenario name.
 */
using MPAEdgeLabel = struct MPAEdgeLabel {
    MPDelay delay;
    CString scenario;
};

inline bool operator==(const MPAEdgeLabel &s, const MPAEdgeLabel &t) {
    if (s.delay != t.delay) {
        return false;
    }
    return s.scenario == t.scenario;
}

/**
 * Compare MPA state labels using a lexicographical ordering.
 */
inline bool operator<(const MPAEdgeLabel &s, const MPAEdgeLabel &t) {
    if (s.delay < t.delay) {
        return true;
    }
    if (s.delay > t.delay) {
        return false;
    }
    return s.scenario < t.scenario;
}

/**
 * Create a new edge label
 * @param stateId FSM state id
 * @param tokenId token number
 */
inline MPAEdgeLabel makeMPAEdgeLabel(MPDelay delay, CString &scenario) {
    MPAEdgeLabel el;
    el.delay = delay;
    el.scenario = scenario;
    return el;
}

// Types for edges and states and sets.
using MPAState = ::FSM::Labeled::State<MPAStateLabel, MPAEdgeLabel>;
using MPAEdge = ::FSM::Labeled::Edge<MPAStateLabel, MPAEdgeLabel>;
using MPASetOfStates = ::FSM::Labeled::SetOfStates<MPAStateLabel, MPAEdgeLabel>;
using MPASetOfEdges = ::FSM::Abstract::SetOfEdges;
using MPASequence = ::FSM::Labeled::ListOfEdges<MPAStateLabel, MPAEdgeLabel> ;
 

/**
 * A max-plus automaton
 */
class MaxPlusAutomaton : public ::FSM::Labeled::FiniteStateMachine<MPAStateLabel, MPAEdgeLabel> {
public:
    // Destructor.
    ~MaxPlusAutomaton() override = default;

    // prints a graphviz vizualisation of the MPA
    // use http://www.webgraphviz.com/ to see
    void printForGraphviz() {
        std::cout << "printing the max-plus automaton for graphviz \r\n";
        MPASetOfEdges::CIter e = this->getEdges()->begin();
        for (; e != this->getEdges()->end(); e++) {
            auto x = ((MPAEdge *)(*e))->label.scenario;

            std::cout << "\"" << (*e)->getSource()->getId() << "\" -> \""
                      << (*e)->getDestination()->getId() << "\" [ label = \"" << *x << ","
                      << ((MPAEdge *)(*e))->label.delay << "\" ];  \r\n";
        }
    }
};

/**
 * An edge label type for a max-plus automaton with rewards
 */
using MPAREdgeLabel = struct MPAREdgeLabel {
    MPDelay delay;
    const CString scenario;
    CDouble reward{0.0};
};

/**
 * Support for easy construction of a edge label with rewards.
 */
inline MPAREdgeLabel makeRewardEdgeLabel(MPDelay d, const CString &sc, CDouble r) {
    MPAREdgeLabel el = {d, sc, r};
    return el;
}

inline bool operator==(const MPAREdgeLabel &s, const MPAREdgeLabel &t) {
    if (s.delay != t.delay) {
        return false;
    }
    if (s.scenario != t.scenario) {
        return false;
    }
    return s.reward == t.reward;
}

/**
 * Compare MPA state labels using a lexicographical ordering.
 */
inline bool operator<(const MPAREdgeLabel &s, const MPAREdgeLabel &t) {
    if (s.delay < t.delay) {
        return true;
    }
    if (s.delay > t.delay) {
        return false;
    }
    if (s.scenario < t.scenario) {
        return true;
    }
    if (s.scenario > t.scenario) {
        return false;
    }
    return s.reward < t.reward;
}

inline CString toString(const MPAREdgeLabel &l) {
    return "(delay: " + CString(l.delay) + ", scenario: " + CString(l.scenario)
           + ", reward: " + CString(l.reward) + ")";
};

// Types of states, edges, sets and cycle of an MPA with rewards.
using MPARState = ::FSM::Labeled::State<MPAStateLabel, MPAREdgeLabel>;
using MPAREdge = ::FSM::Labeled::Edge<MPAStateLabel, MPAREdgeLabel>;
using MPARSetOfStates = ::FSM::Labeled::SetOfStates<MPAStateLabel, MPAREdgeLabel>;
using MPARSetOfEdges = ::FSM::Abstract::SetOfEdges;
using MPASequence = ::FSM::Labeled::ListOfEdges<MPAStateLabel, MPAEdgeLabel>;
using MPARCycle = std::list<const ::FSM::Abstract::Edge *>;

/**
 * A max-plus automaton with rewards. In addition to the usual max-plus automaton,
 * its edges are labeled with rewards; a quantified amount of 'progress'.
 */
class MaxPlusAutomatonWithRewards
    : virtual public ::FSM::Labeled::FiniteStateMachine<MPAStateLabel, MPAREdgeLabel> {
public:
    // Destructor.
    ~MaxPlusAutomatonWithRewards() override = default;
    std::shared_ptr<::FSM::Abstract::FiniteStateMachine> newInstance() override {
        return std::make_shared<MaxPlusAutomatonWithRewards>();
    }

    // compute the maximum cycle ratio of delay over progress
    CDouble calculateMCR();
    // compute the maximum cycle ratio of delay over progress and also return a critical cycle
    CDouble calculateMCRAndCycle(std::shared_ptr<std::vector<const MPAREdge *>> *cycle);
};

class MakespanCalculator {
public:
    /**
     * Compute the worst-case makespan of the maxplus automaton and produce the corresponding
     * sequence.
     * TODO : make  pair<CDouble, list<CString*>*> a struct
     */
    std::pair<CDouble, std::list<CString *> *> calculateWorstCaseMakespan(MaxPlusAutomaton *mpa);
    /**
     * Compute the best-case makespan of the maxplus automaton and produce the corresponding
     * sequence.
     */
    CDouble calculateBestCaseMakespan(MPASequence **sequence);

private:
    /*
     * bellman-ford algorithm on max-plus automaton mpa to find the minimum path
     */
    std::pair<CDouble, std::list<CString *> *>
    bellmanFordMin(MaxPlusAutomaton *mpa, MPAState *srcId, bool checkForNegativeCycle);
    /*
     * bellman-ford algorithm on max-plus automaton mpa to find the maximum path
     */
    std::pair<CDouble, std::list<CString *> *>
    bellmanFordMax(MaxPlusAutomaton *mpa, MPAState *srcId, bool checkForNegativeCycle);

    std::map<MPAState *, std::pair<CDouble, std::list<CString *> *>>
    bellmanFord(MaxPlusAutomaton *mpa, MPAState *srcId, bool checkForNegativeCycle);
};
} // namespace MaxPlus

#endif