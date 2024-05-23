#include "maxplus/algebra/mpmatrix.h"
#include "maxplus/base/fsm/fsm.h"
#include "maxplus/base/fsm/iofsm.h"
#include "maxplus/graph/mpautomaton.h"
#include <map>
#include <memory>
#include <utility>



namespace MaxPlus::SMPLS {

using ModeMatrices = std::map<CString, std::shared_ptr<MaxPlus::Matrix>>;

class EdgeLabeledModeFSM : public ::FSM::Labeled::FiniteStateMachine<CId, CString> {
public:
    // put the destructor deliberately into the cc sourc to ensure the class vtable is accessible
    // see: <https://stackoverflow.com/questions/3065154/undefined-reference-to-vtable>
    virtual ~EdgeLabeledModeFSM();
    virtual void removeDanglingStates();
};

class SMPLS {
public:
	// the mode automaton
    EdgeLabeledModeFSM elsFSM;

	// the mode matrices
    ModeMatrices sm;

    [[nodiscard]] std::shared_ptr<MaxPlusAutomaton> convertToMaxPlusAutomaton() const;

    // transposes all matrices of the SMPLS
    void transposeMatrices();
};

using ListOfMatrices = std::list<std::shared_ptr<Matrix>>;

class DissectedModeMatrix {
public:
    ModeMatrices core;
    ListOfMatrices eventRows;
    DissectedModeMatrix() = default;
};

using Mode = InputAction;
using Event = OutputAction;
using EventOutcome = CString;

using ModeEventPair = std::pair<Mode,Event>;
using EventOutcomePair = std::pair<Event, EventOutcome>;

class SMPLSwithEvents : public SMPLS {
public:
    std::list<ModeEventPair> sigma; // relation between mode and event
    std::list<EventOutcomePair> gamma; // relation between event and outcome

    std::shared_ptr<IOAutomaton> ioa;

    SMPLSwithEvents() { this->ioa = std::make_shared<IOAutomaton>(); }

    explicit SMPLSwithEvents(std::shared_ptr<IOAutomaton> ioa) : ioa(std::move(ioa)) {  }

    /**
     * checks the consistency rules for SMPLS with events
     */
    bool isConsistent();

    void saveDeterminizedIOAtoFile(const CString& file);

    /**
     * creates a max-plus automaton from SMPLS with events
     */
    std::shared_ptr<MaxPlusAutomaton> convertToMaxPlusAutomaton();

    /*
     * goes through the gamma relation and finds the event of the outcome
     */
    [[nodiscard]] Event findEventByOutcome(const EventOutcome& outcome) const;

private:
    std::list<std::shared_ptr<DissectedModeMatrix>> disMatrices;
    uint numberOfResources = 0;
    uint biggestMatrixSize = 0;

    /*
     * after we generate the matrices through 'prepareMatrices', we have to make them
     * all square (to the size of the biggest matrix) by adding rows and cols of -infty
     */
    void makeMatricesSquare();

    /*
     * goes through the sigma relation and finds the event emitted by mode
     */
    [[nodiscard]] Event findEventByMode(const Mode& mode) const;

    /**
     * smpls with events needs to prepare the matrices to be able to perform analysis.
     * this includes adding rows and columns of -inf and 0 based on the spec
     * allowing the system to analyze processing or conveying event timings
     */
    void prepareMatrices(const IOAState &s,
                         std::multiset<Event>& eventList,
                         IOASetOfEdgeRefs& visitedEdges);

    std::shared_ptr<DissectedModeMatrix> findDissectedModeMatrix(const CString& sName);

    /**
     * recursive part of isConsistent
     */
    void isConsistentUtil(const IOAState &s,
                          std::list<Event> &eventList,
                          const IOASetOfStates &finalStates,
                          CString &errMsg,
                          std::map<const IOAState *, std::list<Event> *> &visited);

    void determinizeUtil(const IOAState &s,
                         IOASetOfStateRefs& visited,
                         const IOASetOfStates &finalStates,
                         CString *errMsg,
                         std::ofstream &outfile);

    static bool compareEventLists(std::list<Event>& l1, std::list<Event>& l2);

    void dissectModeMatrices();
};

} // namespace SMPSL
