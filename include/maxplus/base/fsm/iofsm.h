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
using IOAState = ::MaxPlus::FSM::Labeled::State<MaxPlus::CId, IOAEdgeLabel>;
using IOAStateRef = const IOAState *;
using IOAEdge = ::MaxPlus::FSM::Labeled::Edge<MaxPlus::CId, IOAEdgeLabel>;
using IOAEdgeRef = ::MaxPlus::FSM::Labeled::EdgeRef<MaxPlus::CId, IOAEdgeLabel>;
using IOASetOfStates = ::MaxPlus::FSM::Labeled::SetOfStates<MaxPlus::CId, IOAEdgeLabel>;
using IOASetOfStateRefs = ::MaxPlus::FSM::Labeled::SetOfStateRefs<MaxPlus::CId, IOAEdgeLabel>;
using IOASetOfEdges = ::MaxPlus::FSM::Labeled::SetOfEdges<MaxPlus::CId, IOAEdgeLabel>;
using IOASetOfEdgeRefs = ::MaxPlus::FSM::Labeled::SetOfEdgeRefs<MaxPlus::CId, IOAEdgeLabel>;

namespace MaxPlus::FSM::Labeled {

class IOAutomaton : public ::MaxPlus::FSM::Labeled::FiniteStateMachine<CId, IOAEdgeLabel> {
public:
    IOAutomaton() = default;
    ~IOAutomaton() override = default;

    // Delete copy/move constructors and assignment operators
    IOAutomaton(const IOAutomaton &) = delete;
    IOAutomaton &operator=(const IOAutomaton &) = delete;
    IOAutomaton(IOAutomaton &&) = delete;
    IOAutomaton &operator=(IOAutomaton &&) = delete;
};

} // namespace MaxPlus::FSM::Labeled

#endif