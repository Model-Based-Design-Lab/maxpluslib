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
using IOAState = ::MaxPlus::FSM::Labeled::State<CId, IOAEdgeLabel>;
using IOAStateRef = const IOAState *;
using IOAEdge = ::MaxPlus::FSM::Labeled::Edge<CId, IOAEdgeLabel>;
using IOAEdgeRef = ::MaxPlus::FSM::Labeled::EdgeRef<CId, IOAEdgeLabel>;
using IOASetOfStates = ::MaxPlus::FSM::Labeled::SetOfStates<CId, IOAEdgeLabel>;
using IOASetOfStateRefs = ::MaxPlus::FSM::Labeled::SetOfStateRefs<CId, IOAEdgeLabel>;
using IOASetOfEdges = ::MaxPlus::FSM::Labeled::SetOfEdges<CId, IOAEdgeLabel>;
using IOASetOfEdgeRefs = ::MaxPlus::FSM::Labeled::SetOfEdgeRefs<CId, IOAEdgeLabel>;

namespace MaxPlus::FSM::Labeled {

class IOAutomaton : public ::MaxPlus::FSM::Labeled::FiniteStateMachine<CId, IOAEdgeLabel> {
public:
    virtual ~IOAutomaton(){};
};

} // namespace MaxPlus::FSM::Labeled

#endif