#include "fsm.h"
#include "base/string/cstring.h"

// Input/Output Automaton
typedef CString InputAction;
typedef CString OutputAction;
typedef std::pair<InputAction, OutputAction> IOAEdgeLabel;
typedef ::FSM::Labeled::State<CId, IOAEdgeLabel> IOAState;
typedef ::FSM::Labeled::Edge<CId, IOAEdgeLabel> IOAEdge;
typedef ::FSM::Labeled::SetOfStates<CId, CString> IOASetOfStates;
typedef ::FSM::Labeled::SetOfEdges<CId, CString> IOASetOfEdges;
typedef ::FSM::Abstract::SetOfEdgeRefs IOASetOfEdgeRefs;

class IOAutomaton : public
    ::FSM::Labeled::FiniteStateMachine<CId, IOAEdgeLabel>
{
public:
    virtual ~IOAutomaton() {};
};
