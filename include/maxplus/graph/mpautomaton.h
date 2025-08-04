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

namespace MaxPlus {

using namespace ::MaxPlus::FSM::Labeled;
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

inline MPString toString(const MPAStateLabel &l) {
    return "(id: " + MPString(l.id) + ", tokenNr: " + MPString(l.tokenNr) + ")";
};

/**
 * An MPA edge is labeled with a delay and a mode name.
 */
using MPAEdgeLabel = struct MPAEdgeLabel {
    MPDelay delay;
    MPString mode;
};

/**
 * Create a new edge label
 * @param stateId FSM state id
 * @param tokenId token number
 */
inline MPAEdgeLabel makeMPAEdgeLabel(MPDelay delay, const MPString &mode) {
    MPAEdgeLabel el;
    el.delay = delay;
    el.mode = mode;
    return el;
}

inline bool operator==(const MPAEdgeLabel &s, const MPAEdgeLabel &t) {
    if (s.delay != t.delay) {
        return false;
    }
    return s.mode == t.mode;
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
    return s.mode < t.mode;
}

/**
 * Create a new edge label
 * @param stateId FSM state id
 * @param tokenId token number
 */
inline MPAEdgeLabel makeMPAEdgeLabel(MPDelay delay, MPString &mode) {
    MPAEdgeLabel el;
    el.delay = delay;
    el.mode = mode;
    return el;
}

// Types for edges and states and sets.
using MPAState = ::FSM::Labeled::State<MPAStateLabel, MPAEdgeLabel>;
using MPAStateRef = ::FSM::Labeled::StateRef<MPAStateLabel, MPAEdgeLabel>;
using MPAEdge = ::FSM::Labeled::Edge<MPAStateLabel, MPAEdgeLabel>;
using MPAEdgeRef = ::FSM::Labeled::EdgeRef<MPAStateLabel, MPAEdgeLabel>;
using MPAPath = std::list<MPAEdgeRef>;
using MPASetOfStates = ::FSM::Labeled::SetOfStates<MPAStateLabel, MPAEdgeLabel>;
using MPASetOfStateRefs = ::FSM::Labeled::SetOfStateRefs<MPAStateLabel, MPAEdgeLabel>;
using MPASetOfEdges = ::FSM::Labeled::SetOfEdges<MPAStateLabel, MPAEdgeLabel>;
using MPASetOfEdgeRefs = ::FSM::Labeled::SetOfEdgeRefs<MPAStateLabel, MPAEdgeLabel>;

/**
 * A max-plus automaton
 */
class MaxPlusAutomaton : public ::FSM::Labeled::FiniteStateMachine<MPAStateLabel, MPAEdgeLabel> {
public:
    // Destructor.
    ~MaxPlusAutomaton() override = default;
};

/**
 * An edge label type for a max-plus automaton with rewards
 */
using MPAREdgeLabel = struct MPAREdgeLabel {
    MPDelay delay;
    const MPString mode;
    CDouble reward{0.0};
};

/**
 * Support for easy construction of a edge label with rewards.
 */
inline MPAREdgeLabel makeRewardEdgeLabel(MPDelay d, const MPString &sc, CDouble r) {
    MPAREdgeLabel el = {d, sc, r};
    return el;
}

inline bool operator==(const MPAREdgeLabel &s, const MPAREdgeLabel &t) {
    if (s.delay != t.delay) {
        return false;
    }
    if (s.mode != t.mode) {
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
    if (s.mode < t.mode) {
        return true;
    }
    if (s.mode > t.mode) {
        return false;
    }
    return s.reward < t.reward;
}

inline MPString toString(const MPAREdgeLabel &l) {
    return "(delay: " + MPString(l.delay) + ", mode: " + MPString(l.mode)
           + ", reward: " + MPString(l.reward) + ")";
};

// Types of states, edges, sets and cycle of an MPA with rewards.
using MPARState = ::FSM::Labeled::State<MPAStateLabel, MPAREdgeLabel>;
using MPARStateRef = ::FSM::Labeled::StateRef<MPAStateLabel, MPAREdgeLabel>;
using MPAREdge = ::FSM::Labeled::Edge<MPAStateLabel, MPAREdgeLabel>;
using MPAREdgeRef = ::FSM::Labeled::EdgeRef<MPAStateLabel, MPAREdgeLabel>;
using MPARSetOfStates = ::FSM::Labeled::SetOfStates<MPAStateLabel, MPAREdgeLabel>;
using MPARSetOfEdges = ::FSM::Abstract::SetOfEdges;
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
    CDouble calculateMCRAndCycle(std::shared_ptr<std::vector<MPAREdgeRef>> *cycle);
};

} // namespace MaxPlus

#endif