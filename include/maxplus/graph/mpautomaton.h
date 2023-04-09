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

#include "maxplus/base/fsm/fsm.h"
#include "maxplus/algebra/mptype.h"

namespace MaxPlus {

using namespace ::FSM::Labeled;
/**
 * MPA state is labeled with the FSM state ID, and the token number.
 */
using MPAStateLabel = struct MPAStateLabel {
    CId id;
    unsigned int tokenNr;
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

inline bool operator==(MPAStateLabel s, MPAStateLabel t) {
    return s.id == t.id && s.tokenNr == t.tokenNr;
}

/**
 * Compare MPA state labels using a lexicographical ordering.
 */
inline bool operator<(MPAStateLabel s, MPAStateLabel t) {
    if (s.id < t.id) {
        return true;
    }
    if (s.id > t.id) {
        return false;
    }
    return s.tokenNr < t.tokenNr;
}

/**
 * An MPA edge is labeled with a delay and a scenario name.
 */
using MPAEdgeLabel = struct MPAEdgeLabel {
    MPDelay delay;
    CString *scenario{nullptr};
};

/**
 * Create a new edge label
 * @param stateId FSM state id
 * @param tokenId token number
 */
inline MPAEdgeLabel makeMPAEdgeLabel(MPDelay delay, CString *scenario) {
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

/**
 * A max-plus automaton
 */
class MaxPlusAutomaton : public ::FSM::Labeled::FiniteStateMachine<MPAStateLabel, MPAEdgeLabel> {
public:
    // Destructor.
     ~MaxPlusAutomaton() override= default;;
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
inline MPAREdgeLabel makeRewardEdgeLabel(MPDelay d, const CString& sc, CDouble r) {
    MPAREdgeLabel el ={d, sc, r};
    return el;
}

// Types of states, edges, sets and cycle of an MPA with rewards.
using MPARState = ::FSM::Labeled::State<MPAStateLabel, MPAREdgeLabel>;
using MPAREdge = ::FSM::Labeled::Edge<MPAStateLabel, MPAREdgeLabel>;
using MPARSetOfStates = ::FSM::Labeled::SetOfStates<MPAStateLabel, MPAREdgeLabel>;
using MPARSetOfEdges = ::FSM::Abstract::SetOfEdges;
using MPARCycle = std::list<const ::FSM::Abstract::Edge*>;

/**
 * A max-plus automaton with rewards. In addition to the usual max-plus automaton,
 * its edges are labeled with rewards; a quantified amount of 'progress'.
 */
class MaxPlusAutomatonWithRewards
    : virtual public ::FSM::Labeled::FiniteStateMachine<MPAStateLabel, MPAREdgeLabel> {
public:
    // Destructor.
     ~MaxPlusAutomatonWithRewards() override= default;
    // compute the maximum cycle ratio of delay over progress
    CDouble calculateMCR();
    // compute the maximum cycle ratio of delay over progress and also return a critical cycle
    CDouble calculateMCRAndCycle(MPARCycle **cycle);
};

} // namespace MaxPlus

#endif