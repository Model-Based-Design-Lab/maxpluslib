#ifndef MAXPLUS_BASE_FSM_IOFSM_H
#define MAXPLUS_BASE_FSM_IOFSM_H

#include "maxplus/base/basic_types.h"
#include "maxplus/base/fsm/fsm.h"
#include "maxplus/base/string/cstring.h"
#include <utility>

// Input/Output Automaton
using InputAction = MaxPlus::MPString;
using OutputAction = MaxPlus::MPString;
using IOAEdgeLabel = std::pair<InputAction, OutputAction>;
using IOAState = ::FSM::Labeled::State<CId, IOAEdgeLabel>;
using IOAStateRef = const IOAState *;
using IOAEdge = ::FSM::Labeled::Edge<CId, IOAEdgeLabel>;
using IOAEdgeRef = ::FSM::Labeled::EdgeRef<CId, IOAEdgeLabel>;
using IOASetOfStates = ::FSM::Labeled::SetOfStates<CId, IOAEdgeLabel>;
using IOASetOfStateRefs = ::FSM::Labeled::SetOfStateRefs<CId, IOAEdgeLabel>;
using IOASetOfEdges = ::FSM::Labeled::SetOfEdges<CId, IOAEdgeLabel>;
using IOASetOfEdgeRefs = ::FSM::Labeled::SetOfEdgeRefs<CId, IOAEdgeLabel>;

namespace FSM::Labeled {

class IOAutomaton : public ::FSM::Labeled::FiniteStateMachine<CId, IOAEdgeLabel> {
public:
    virtual ~IOAutomaton(){};
};

} // namespace FSM::Labeled

#endif