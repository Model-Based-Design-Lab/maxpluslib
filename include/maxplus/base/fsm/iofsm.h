#include "fsm.h"
#include "maxplus/base/string/cstring.h"

// Input/Output Automaton
using InputAction = CString;
using OutputAction = CString;
using IOAEdgeLabel = std::pair<InputAction, OutputAction>;
using IOAState = ::FSM::Labeled::State<CId, IOAEdgeLabel>;
using IOAEdge = ::FSM::Labeled::Edge<CId, IOAEdgeLabel>;
using IOASetOfStates = ::FSM::Labeled::SetOfStates<CId, CString>;
using IOASetOfStateRefs = ::FSM::Labeled::SetOfStateRefs<CId, CString>;
using IOASetOfEdges = ::FSM::Labeled::SetOfEdges<CId, CString>;
using IOASetOfEdgeRefs = ::FSM::Abstract::SetOfEdgeRefs;

namespace FSM::Labeled {

class IOAutomaton : public ::FSM::Labeled::FiniteStateMachine<CId, IOAEdgeLabel> {
public:
    virtual ~IOAutomaton(){};
};

} // namespace FSM::Labeled