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
#include <utility>

namespace MaxPlus::SMPLS {

using ModeMatrices = std::map<MPString, std::unique_ptr<MaxPlus::Matrix>>;

class EdgeLabeledModeFSM : public ::MaxPlus::FSM::Labeled::FiniteStateMachine<CId, MPString> {
public:
    EdgeLabeledModeFSM() = default;
    // put the destructor deliberately into the cc source to ensure the class vtable is accessible
    // see: <https://stackoverflow.com/questions/3065154/undefined-reference-to-vtable>
    ~EdgeLabeledModeFSM() override;

    // Delete copy/move constructors and assignment operators
    EdgeLabeledModeFSM(const EdgeLabeledModeFSM &) = delete;
    EdgeLabeledModeFSM &operator=(const EdgeLabeledModeFSM &) = delete;
    EdgeLabeledModeFSM(EdgeLabeledModeFSM &&) = delete;
    EdgeLabeledModeFSM &operator=(EdgeLabeledModeFSM &&) = delete;

    virtual void removeDanglingStates();
};

class SMPLS {
public:
    // the mode automaton
    EdgeLabeledModeFSM elsFSM;


    [[nodiscard]] std::unique_ptr<MaxPlusAutomaton> convertToMaxPlusAutomaton() const;

    // transposes all matrices of the SMPLS
    void transposeMatrices();
    // returns the mode matrices
    [[nodiscard]] const ModeMatrices &getModeMatrices() const { return this->mm; }
    void addModeMatrix(const MPString &modeName, std::unique_ptr<MaxPlus::Matrix> m) {
        this->mm.emplace(modeName, std::move(m));
    }

private:
    // the mode matrices
    ModeMatrices mm;

};

using ListOfMatrices = std::list<std::unique_ptr<Matrix>>;

class DissectedModeMatrix {
public:
    DissectedModeMatrix() = default;
    ModeMatrices& getCore() { return core; }
    [[nodiscard]] const ModeMatrices& getCore() const { return core; }
    ListOfMatrices& getEventRows() { return eventRows; }
    [[nodiscard]] const ListOfMatrices& getEventRows() const { return eventRows; }
private:
    ModeMatrices core;
    ListOfMatrices eventRows;
};

using Mode = InputAction;
using Event = OutputAction;
using EventList = std::list<Event>;
using EventOutcome = MPString;

using ModeEventPair = std::pair<Mode, Event>;
using EventOutcomePair = std::pair<Event, EventOutcome>;

class SMPLSwithEvents : public SMPLS {
public:
    std::list<ModeEventPair> sigma;    // relation between mode and event
    std::list<EventOutcomePair> gamma; // relation between event and outcome

    std::unique_ptr<IOAutomaton> ioa;

    SMPLSwithEvents() { this->ioa = std::make_unique<IOAutomaton>(); }

    explicit SMPLSwithEvents(std::unique_ptr<IOAutomaton> ioa) : ioa(std::move(ioa)) {}

    /**
     * checks the consistency rules for SMPLS with events
     */
    bool isConsistent();

    void saveDeterminizedIOAtoFile(const MPString &file);

    /**
     * creates a max-plus automaton from SMPLS with events
     */
    std::unique_ptr<MaxPlusAutomaton> convertToMaxPlusAutomaton();

    /*
     * goes through the gamma relation and finds the event of the outcome
     */
    [[nodiscard]] Event findEventByOutcome(const EventOutcome &outcome) const;

private:
    std::list<std::unique_ptr<DissectedModeMatrix>> disMatrices;
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
    [[nodiscard]] Event findEventByMode(const Mode &mode) const;

    /**
     * smpls with events needs to prepare the matrices to be able to perform analysis.
     * this includes adding rows and columns of -inf and 0 based on the spec
     * allowing the system to analyze processing or conveying event timings
     */
    void prepareMatrices(const IOAState &s,
                         std::multiset<Event> &eventList,
                         IOASetOfEdgeRefs &visitedEdges);

    DissectedModeMatrix* findDissectedModeMatrix(const MPString &sName);

    /**
     * recursive part of isConsistent
     */
    void isConsistentUtil(const IOAState &s,
                          EventList &eventList,
                          const ::MaxPlus::FSM::Abstract::SetOfStateRefs &finalStates,
                          MPString &errMsg,
                          std::map<IOAStateRef, EventList> &visited);

    void determinizeUtil(const IOAState &s,
                         IOASetOfStateRefs &visited,
                         const FSM::Abstract::SetOfStateRefs &finalStates,
                         MPString &errMsg,
                         std::ofstream &outfile);

    static bool compareEventLists(EventList &l1, EventList &l2);

    void dissectModeMatrices();
};

} // namespace MaxPlus::SMPLS

#endif