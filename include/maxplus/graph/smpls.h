/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   smpls.h
 *
 *  Author          :   Marc Geilen (<m.c.w.geilen@tue.nl>)
 *                  :   Alireza Mohammadkhani (<a.mohammadkhani@tue.nl>)
 *
 *  Date            :   April 26, 2010
 *
 *  Function        :   Switched Max-plus Linear Systems.
 *
 *  History         :
 *      22-05-24    :   Initial version copied over from SDF3
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

#ifndef MAXPLUS_GRAPH_SMPLS_H
#define MAXPLUS_GRAPH_SMPLS_H

#include "maxplus/algebra/mpmatrix.h"
#include "maxplus/base/fsm/fsm.h"
#include "maxplus/base/fsm/iofsm.h"
#include "maxplus/graph/mpautomaton.h"
#include <map>
#include <memory>

namespace FSM {

// TODO: change name to SMPLS terminology
using ScenarioMatrices = std::map<CString, std::shared_ptr<MaxPlus::Matrix>>;

// TODO: move to fsm.h?
class EdgeLabeledScenarioFSM : public ::FSM::Labeled::FiniteStateMachine<CId, CString> {
public:
    // put the destructor deliberately into the cc sourc to ensure the class vtable is accessible
    // see: <https://stackoverflow.com/questions/3065154/undefined-reference-to-vtable>
    virtual ~EdgeLabeledScenarioFSM();
    virtual void removeDanglingStates();
};
} // namespace FSM

namespace MaxPlus {
class SMPLS {
public:
    std::shared_ptr<FSM::ScenarioMatrices> sm = std::make_shared<FSM::ScenarioMatrices>();

    FSM::EdgeLabeledScenarioFSM *elsFSM = new FSM::EdgeLabeledScenarioFSM();

    [[nodiscard]] std::shared_ptr<MaxPlusAutomaton> convertToMaxPlusAutomaton() const;

    // No file loading or xml in this library
    // void loadAutomatonFromIOAFile(CString fileName);
    // /*
    // Loads max-plus matrices from an mpt file into sm
    // */
    // void loadMPMatricesFromMPTFile(CString file);
    // void loadAutomatonFromDispatchingFile(CString fileName);
    // transposes all matrices of the SMPLS
    void transposeMatrices();
};

class DissectedScenarioMatrix {
public:
    FSM::ScenarioMatrices core;
    std::list<std::shared_ptr<Matrix>> eventRows;

    DissectedScenarioMatrix() = default;
};
using Mode = InputAction;
using Event = OutputAction;
using EventOutcome = CString;

class SMPLSwithEvents : public SMPLS {
public:
    std::list<std::pair<Mode, Event>> *sigma =
            new std::list<std::pair<Mode, Event>>(); // relation between mode and event
    std::list<std::pair<Event, EventOutcome>> *gamma =
            new std::list<std::pair<Event, EventOutcome>>(); // relation between event and outcome

    IOAutomaton *ioa;

    SMPLSwithEvents() { this->ioa = new IOAutomaton(); }

    explicit SMPLSwithEvents(IOAutomaton *ioa) : ioa(ioa) {  }
    // TODO: control the way sigma and gamma are filled
    void addToSigma(std::pair<Mode *, Event *> *p) {}
    void addToGamma(std::pair<Event *, EventOutcome *> *p) {}

    /**
     * checks the consistency rules for SMPLS with events
     */
    bool isConsistent();

    void saveDeterminizedIOAtoFile(const CString& file);

    /**
     * creates a max-plus automaton from SMPLS with events
     */
    std::shared_ptr<MaxPlusAutomaton> convertToMaxPlusAutomaton();

	// no loading in this library
    // /*
    // Loads an IOAutomaton file and stores in elsFMS
    // */
    // void loadIOAutomatonFromIOAFile(CString fileName);

    // void loadIOAutomatonFromDispatchingFile(CString fileName);
    // /*
    // Loads events from an activity file and fills gamma and sigma
    // */
    // void loadActivities(CString file);

    /*
     * goes through the gamma relation and finds the event of the outcome
     */
    [[nodiscard]] Event findEventByOutcome(const EventOutcome& outcome) const;

private:
    std::list<std::shared_ptr<DissectedScenarioMatrix>> *disMatrices = new std::list<std::shared_ptr<DissectedScenarioMatrix>>();
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

    std::shared_ptr<DissectedScenarioMatrix> findDissectedScenarioMatrix(const CString& sName);

    /**
     * recursive part of isConsistent
     */
    void isConsistentUtil(const IOAState &s,
                          std::list<Event> &eventList,
                          const IOASetOfStates &finalStates,
                          CString *errMsg,
                          std::map<const IOAState *, std::list<Event> *> &visited);

    void determinizeUtil(const IOAState &s,
                         IOASetOfStateRefs& visited,
                         const IOASetOfStates &finalStates,
                         CString *errMsg,
                         std::ofstream &outfile);

    static bool compareEventLists(std::list<Event>& l1, std::list<Event>& l2);

    void dissectScenarioMatrices();
};

} // namespace MaxPlus

#endif