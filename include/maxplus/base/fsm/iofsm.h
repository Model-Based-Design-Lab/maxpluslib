#ifndef MAXPLUS_BASE_FSM_IOFSM_H
#define MAXPLUS_BASE_FSM_IOFSM_H

#include "maxplus/base/basic_types.h"
#include "maxplus/base/fsm/fsm.h"
#include "maxplus/base/string/cstring.h"
#include <utility>

// Input/Output Automaton
using InputAction = MaxPlus::CString;
using OutputAction = MaxPlus::CString;
using IOAEdgeLabel = std::pair<InputAction, OutputAction>;
using IOAState = ::FSM::Labeled::State<CId, IOAEdgeLabel>;
using IOAStateRef = const IOAState*;
using IOAEdge = ::FSM::Labeled::Edge<CId, IOAEdgeLabel>;
using IOAEdgeRef = ::FSM::Labeled::EdgeRef<CId, IOAEdgeLabel>;
using IOASetOfStates = ::FSM::Labeled::SetOfStates<CId, MaxPlus::CString>;
using IOASetOfStateRefs = ::FSM::Labeled::SetOfStateRefs<CId, MaxPlus::CString>;
using IOASetOfEdges = ::FSM::Labeled::SetOfEdges<CId, MaxPlus::CString>;
using IOASetOfEdgeRefs = ::FSM::Abstract::SetOfEdgeRefs;

namespace FSM::Labeled {

class IOAutomaton : public ::FSM::Labeled::FiniteStateMachine<CId, IOAEdgeLabel> {
public:
    virtual ~IOAutomaton(){};
};

} // namespace FSM::Labeled

#endif