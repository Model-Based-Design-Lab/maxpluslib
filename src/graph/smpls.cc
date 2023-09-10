/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   smpls.cc
 *
 *  Author			:	Alireza Mohammadkhani (a.mohammadkhani@tue.nl)
 *
 *  Date            :   January 4, 2021
 *
 *  Function        :   Switching max plus linear system.
 *
 *  History         :
 *      04-01-21    :   Initial version.
 *      18-01-21    :   consistency check on IOA of SMPLS with events.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Happy coding!
 */
#include "graph/smpls.h"
#include <string>

using namespace FSM;

namespace MaxPlus {

/*
 * Loads an entitiy from its starting '{' to the corresponding '}'
 */
std::string loadEntity(ifstream &stream, string line) {
    string out = line;
    size_t openBracketCount = 0, closeBracketCount = 0;
    openBracketCount += std::count(line.begin(), line.end(), '{');
    closeBracketCount += std::count(line.begin(), line.end(), '}');

    // get the whole activity {} into a string
    while (getline(stream, line)) {

        openBracketCount += std::count(line.begin(), line.end(), '{');
        closeBracketCount += std::count(line.begin(), line.end(), '}');
        out += line;

        if (openBracketCount == closeBracketCount) {
            openBracketCount = 0;
            closeBracketCount = 0;
            break;
        }
    }
    return out;
}
MaxPlusAutomaton *SMPLS::convertToMaxPlusAutomaton() {
    MaxPlusAutomaton *mpa = new MaxPlusAutomaton();
    // transposeMatrices();
    //  create the FSM states for every pair of a state of the FSMSADF's FSM
    //  and an initial token

    ELSSetOfStates *I = elsFSM->getInitialStates();
    ELSSetOfStates *F = elsFSM->getFinalStates();
    ELSSetOfStates *Q = elsFSM->getStates();
    ELSSetOfStates::CIter q;
    for (q = Q->begin(); q != Q->end(); q++) {
        auto e = (*q)->getOutgoingEdges();
        unsigned int nrTokens = 0;
        if (e->size() > 0) {
            CString label = ((ELSEdge *)(*e->begin()))->label;

            auto _sm = sm->begin();
            for (; _sm != sm->end(); _sm++) {
                if ((*_sm).first == label) {
                    nrTokens = (*_sm).second->getCols();
                    break;
                }
            }
        } else {
            // this is a bit iffy, but I dont have time to make it solid now
            nrTokens = (*sm->begin()).second->getCols();
        }
        // create a state for (q, k)
        CId qId = ((ELSState *)(*q))->getLabel();
        bool isInitial = false;
        bool isFinal = false;
        // if els state is initial
        ELSSetOfStates::CIter i;
        for (i = I->begin(); i != I->end(); i++) {
            CId iId = ((ELSState *)(*i))->getLabel();
            if (iId == qId) {
                isInitial = true;
                break;
            }
        }
        // if els state is final
        ELSSetOfStates::CIter f;
        for (f = F->begin(); f != F->end(); f++) {
            CId fId = ((ELSState *)(*f))->getLabel();
            if (fId == qId) {
                isFinal = true;
                break;
            }
        }

        // create the states needed per transition matrix
        for (unsigned int k = 0; k < nrTokens; k++) {
            MPAState *s = new MPAState(make_mpastatelabel(qId, k));
            mpa->addState(s);
            // cout << "DEBUG adding state to mpa id: " << (CString)(s->getLabel().id) << ", tkn: "
            // <<  (s->getLabel().tokenNr)<< endl;
            if (isInitial)
                mpa->addInitialState(s);
            if (isFinal)
                mpa->addFinalState(s);
        }
    }
    // add the edges. For every edge (q1, q2) in the original fsm,
    // let s be the scenario of q2 and let M_s be the matrix of s
    // For every non -inf element d=M_s(k,m) add an edge from state (q1,k)
    // to (q2,m) labelled with d.

    // for every state of the fsm...
    for (q = Q->begin(); q != Q->end(); q++) {
        ELSState *q1 = (ELSState *)*q;
        CId q1Id = q1->getLabel();

        // for every outgoing edge of the state
        ELSSetOfEdges *t = (ELSSetOfEdges *)q1->getOutgoingEdges();
        ELSSetOfEdges::CIter e;
        for (e = t->begin(); e != t->end(); e++) {
            ELSEdge *tr = (ELSEdge *)*e;
            CId q2Id = ((ELSState *)tr->getDestination())->getLabel();
            CString sc = tr->label;
            Matrix *Ms = (*sm)[sc];
            size_t r = Ms->getRows();
            size_t c = Ms->getCols();

            // for every entry in the scenario matrix of that edge
            for (size_t row = 0; row < r; row++) {
                for (size_t col = 0; col < c; col++) {
                    MPDelay d = Ms->get((unsigned int)row, (unsigned int)col);

                    if (!MP_ISMINUSINFINITY(d)) {
                        MPAState *src =
                                mpa->getStateLabeled(make_mpastatelabel(q1Id, (unsigned int)col));
                        MPAState *dst =
                                mpa->getStateLabeled(make_mpastatelabel(q2Id, (unsigned int)row));
                        MPAEdgeLabel l = make_mpaedgelabel(d, &sc);
                        l.scenario = (CString *)&tr->label;
                        mpa->addEdge(src, l, dst);
                    }
                }
            }
        }
    }

    return mpa;
}

void SMPLS::convertToPoosl(CString file) {
    /*
    std::ofstream outfile(file);

    // The principle is that we have a controller that dispatches activities/modes by sending the
    "dispatch_MODENAME" message to the plant
    // and a plant that executes them when it receives the "dispatch_MODENAME" message and sends
    back "MODENAME_finished" outfile << "my text here!" << std::endl;

    outfile.close();
    */
}

/* this is not fully functional for now. you can use the version that it prints out
 * it also does not check for input actions that don't belong to the same event, which it should.
 */
void SMPLSwithEvents::saveDeterminizedIOAtoFile(CString file) {
    /**
     * Deteministic IOA is defined with:
     *	The I/O automaton has exactly one initial state;
     *  The final states have no outgoing transition;
     *	Branches in the I/O automaton are based on different outcomes of the same event;
     */

    if (this->ioa == NULL)
        throw CException("The automaton of smpls is not loaded!");

    std::ofstream outfile(file);
    outfile << "ioautomaton statespace{ \r\n";
    IOASetOfStates *I = (IOASetOfStates *)this->ioa->getInitialStates();

    IOASetOfStates *finalStates = (IOASetOfStates *)this->ioa->getFinalStates();

    CString errMsg = "";
    typename IOASetOfStates::CIter i = I->begin();
    IOAState *s = (IOAState *)(*i);
    i++;
    // we remove the rest of the initial states since only one is allowed
    for (; i != I->end();) {
        ioa->removeState((IOAState *)*i);
    }
    IOASetOfStates *visitedStates = new IOASetOfStates();
    determinizeUtil(s, visitedStates, finalStates, &errMsg, outfile);
    outfile << "}";
    outfile.close();
    delete (visitedStates);
}

/**
 * checks the consistency rules for SMPLS with events
 */

bool SMPLSwithEvents::isConsistent() {
    if (this->ioa == NULL)
        throw CException("The automaton of smpls is not loaded!");
    list<Event> *eventList = new list<Event>();

    IOASetOfStates *I = (IOASetOfStates *)this->ioa->getInitialStates();

    IOASetOfStates *finalStates = (IOASetOfStates *)this->ioa->getFinalStates();

    map<IOAState *, list<Event> *> *visited = new map<IOAState *, list<Event> *>;
    CString errMsg = "";
    typename IOASetOfStates::CIter i;
    for (i = I->begin(); i != I->end(); i++) {
        isConsistentUtil(((IOAState *)*i), eventList, finalStates, &errMsg, visited);
    }

    delete visited;
    delete eventList;
    if (errMsg != "") {
        cout << "IO Automaton is not consistent, error message: " << endl;
        cout << errMsg << endl;
        return false;
    }

    return true;
}

/**
 * creates a max-plus automaton from SMPLS with events
 */
MaxPlusAutomaton *SMPLSwithEvents::convertToMaxPlusAutomaton() {
    dissectScenarioMatrices();
    IOASetOfEdges *visitedEdges = new IOASetOfEdges();
    std::multiset<Event> *eventList = new multiset<Event>();

    // create the elsFsm with the same state structure but we change the matrices in a depth-first
    // search
    IOASetOfStates *I = (IOASetOfStates *)this->ioa->getStates();
    typename IOASetOfStates::CIter i;
    for (i = I->begin(); i != I->end(); i++) {
        ELSState *s = new ELSState(((ELSState *)*i)->getLabel());
        this->elsFSM->addState(s);
    }

    I = (IOASetOfStates *)this->ioa->getInitialStates();
    for (i = I->begin(); i != I->end(); i++) {
        prepareMatrices(((IOAState *)*i), eventList, visitedEdges);
        this->elsFSM->addInitialState(((ELSState *)*i));
    }
    I = (IOASetOfStates *)this->ioa->getFinalStates();
    for (i = I->begin(); i != I->end(); i++) {
        this->elsFSM->addFinalState(((ELSState *)*i));
    }
    // transposeMatrices();
    // makeMatricesSquare();
    return SMPLS::convertToMaxPlusAutomaton();
}

/*
 * after we generate the matrices through 'preapreMatrices', we have to make them
 * all square (to the size of the biggest matrix) by adding rows and cols of -infty
 */
void SMPLSwithEvents::makeMatricesSquare() {
    ScenarioMatrices::iterator it = sm->begin();
    for (; it != sm->end(); it++) {
        Matrix *m = it->second;

        m->addCols(biggestMatrixSize - m->getCols());
        m->addRows(biggestMatrixSize - m->getRows());
    }
}

// transposes all matrices of the SMPLS
void SMPLS::transposeMatrices() {
    ScenarioMatrices::iterator it = sm->begin();
    for (; it != sm->end(); it++) {
        it->second = it->second->getTransposedCopy();
    }
}
/*
 * goes through the gamma relation and finds the event of the outcome
 */
Event SMPLSwithEvents::findEventByOutcome(EventOutcome outcome) {
    list<pair<Event, EventOutcome>>::iterator gammaIterator;
    for (gammaIterator = gamma->begin(); gammaIterator != gamma->end(); gammaIterator++) {
        // finding the event in gamma
        if (((pair<Event, EventOutcome>)*gammaIterator).second == outcome) {
            return ((pair<Event, EventOutcome>)*gammaIterator).first;
        }
    }
    throw CException("Event of outcome " + outcome + " not found!");
}

/*
 * goes through the sigma relation and finds the event emitted by mode
 */
Event SMPLSwithEvents::findEventByMode(Mode mode) {
    // find the event name ...
    list<pair<Mode, Event>>::iterator sigmaIterator;
    for (sigmaIterator = sigma->begin(); sigmaIterator != sigma->end(); sigmaIterator++) {
        if (((pair<Mode, Event>)*sigmaIterator).first == mode) {
            return ((pair<Mode, Event>)*sigmaIterator).second;
        }
    }
    throw CException("Event of mode " + mode + " not found!");
}

/**
 * smpls with events needs to prepare the matrices to be able to perform analysis.
 * this includes adding rows and columns of -inf and 0 based on the spec
 * allowing the system to analyze processing or conveying event timings
 */
void SMPLSwithEvents::prepareMatrices(IOAState *s,
                                      multiset<Event> *eventList,
                                      IOASetOfEdges *visitedEdges) {
    IOASetOfEdges *adj = (IOASetOfEdges *)s->getOutgoingEdges();
    typename IOASetOfEdges::CIter i;
    for (i = adj->begin(); i != adj->end(); ++i) {
        IOAEdge *e = (IOAEdge *)(*i);
        if (visitedEdges->count(e) > 0)
            continue;

        // create a unique label for the new edge. this name will also be the scenario name for sm
        CString scenarioName = s->getId();
        scenarioName += ",";
        scenarioName += e->label.first + "," + e->label.second;

        // add the edge with unique name between the corresponding states
        elsFSM->addEdge(elsFSM->findState(s->getLabel()),
                        scenarioName,
                        elsFSM->findState(((IOAState *)e->getDestination())->getLabel()));

        // make a copy so that child node can not modify the parent nodes list of events
        // only adds and removes and passes it to it's children
        multiset<Event> *eList = new multiset<Event>(*eventList);

        OutputAction output = e->label.second;
        InputAction input = e->label.first;

        DissectedScenarioMatrix *dissm = new DissectedScenarioMatrix();
        Matrix *sMatrix; // matrix to be generated and assigned to this edge
        CString outputActionEventName = "";
        int emittingEventIndex = -1;
        int processingEventIndex = -1;
        int eventIndexCounter = 0;
        if (output != "") {
            // find the dissected matrix structure needed for this edge
            dissm = findDissectedScenarioMatrix(output);
            if (dissm == NULL)
                throw CException("scenario " + output + " not found in dissected matrices!");

            sMatrix = dissm->core->begin()->second->createCopy();

            // look if it has any events to emit
            if (dissm->eventRows->size() > 0) {
                outputActionEventName = findEventByMode(output);

                // ... and where it would sit in the sorted event multiset
                multiset<Event> *eTempList = new multiset<Event>(*eList);
                eTempList->insert(
                        outputActionEventName); // we insert in a copy to see where it would sit
                emittingEventIndex = (int)(eTempList->size() - 1);
                multiset<Event>::reverse_iterator it = eTempList->rbegin();
                for (; it != eTempList->rend(); it++) {
                    if (*it == outputActionEventName) {
                        break;
                    }
                    emittingEventIndex--;
                }
            }
        } else {
            sMatrix = new Matrix(numberOfResources, numberOfResources, MatrixFill::Identity);
        }
        if (input != "") // find the index of the event to be removed from the list
        {
            Event e = findEventByOutcome(input);
            // we have to find the first occurence from top
            processingEventIndex = (int)(distance(eList->begin(), eList->find(e)));
        }

        if (eList->size() > 0) {
            CString eventToBeRemoved;
            multiset<Event> *eTempList = new multiset<Event>(*eList);
            multiset<Event>::iterator eventIter;
            for (eventIter = eTempList->begin(); eventIter != eTempList->end(); eventIter++) {
                sMatrix->addCols(1);

                // this means we have an event in the list which is being conveyed to the next
                // matrix so we add the necessary rows and cols to facilitate that

                if (eventIndexCounter == processingEventIndex) // we add B_c and B_e
                {
                    uint cols = sMatrix->getCols();
                    uint y = 0;
                    for (; y < numberOfResources; y++) // B_c (refer to paper)
                    {
                        sMatrix->put(y, cols - 1, sMatrix->getMaxOfRow(y));
                    }
                    for (; y < numberOfResources; y++) // B_e (refer to paper)
                    {
                        sMatrix->put(
                                y, cols - 1, sMatrix->getMaxOfRowUntilCol(y, numberOfResources));
                    }

                    eList->erase(eList->lower_bound(
                            *eventIter)); // event is processed, remove from eList
                } else // or are we conveying this event to the next state vector? I_c(q) (refer to
                       // paper)
                {
                    sMatrix->addRows(1); // we already added a col of -infty, now we add the row
                    sMatrix->put(sMatrix->getRows() - 1,
                                 sMatrix->getCols() - 1,
                                 0); // and 0 in the intersection of that row and col
                }
                // if we need to add the event row at the bottom
                if ((eventIndexCounter == emittingEventIndex)
                    || (eventIndexCounter + 1 == (int)eTempList->size()
                        && emittingEventIndex
                                   == eventIndexCounter
                                              + 1)) // the second condition happens when we have
                                                    // reached the end of event list but we still
                                                    // need to add the event at the bottom
                {
                    sMatrix->addRows(1);
                    Matrix *eventRow = *dissm->eventRows->begin(); // for now we just assume max one
                                                                   // event emitted per mode
                    uint cols = eventRow->getCols();
                    uint x = 0;
                    for (; x < cols; x++) // A_e (refer to paper)
                    {
                        sMatrix->put(sMatrix->getRows() - 1, x, eventRow->get(0, x));
                    }
                    for (; x < sMatrix->getCols();
                         x++) // B_e (refer to paper), this happens when we add B_c before adding
                              // A_e, so we have to make sure B_e that's added here is max of row
                    {
                        if (sMatrix->getMaxOfColUntilRow(x, numberOfResources)
                            > MP_MINUSINFINITY) // we must only add B_e under cols that correspondto
                                                // B_c
                            sMatrix->put(sMatrix->getRows() - 1,
                                         x,
                                         eventRow->getMaxOfRowUntilCol(0, numberOfResources));
                    }
                }

                eventIndexCounter++;
            }
        } else if (output != "")
            if (dissm->eventRows->size() > 0) {
                sMatrix->addRows(1);
                Matrix *eventRow = *dissm->eventRows->begin();
                uint cols = eventRow->getCols(); // for now we just assume max one event emitted per
                                                 // mode

                for (uint x = 0; x < cols; x++) {
                    sMatrix->put(sMatrix->getRows() - 1, x, eventRow->get(0, x));
                }
            }
        // we store the events emitted by modes as we move along the automaton
        if (outputActionEventName != "") {
            eList->insert(outputActionEventName);
        }

        (*sm)[scenarioName] = sMatrix;
        visitedEdges->insert(e);

        // we nned this later to make all matrices square
        biggestMatrixSize = max(biggestMatrixSize, max(sMatrix->getCols(), sMatrix->getRows()));

        prepareMatrices((IOAState *)(e->getDestination()), eList, visitedEdges);
    }
}

DissectedScenarioMatrix *SMPLSwithEvents::findDissectedScenarioMatrix(CString sName) {
    DissectedScenarioMatrix *dis = NULL;
    list<DissectedScenarioMatrix *>::iterator i = disMatrices->begin();
    for (; i != disMatrices->end(); i++)
        if ((*i)->core->begin()->first == sName) {
            dis = *i;
            break;
        }
    return dis;
}

/**
 * recursive part of isConsistent
 */
void SMPLSwithEvents::isConsistentUtil(IOAState *s,
                                       list<Event> *eventList,
                                       IOASetOfStates *finalStates,
                                       CString *errMsg,
                                       map<IOAState *, list<Event> *> *visited) {
    map<IOAState *, list<Event> *>::iterator it = visited->find(s);
    if (it != visited->end()) // we have already visited this state but we must check for
                              // inconsistency before leaving the state
    {
        if (!compareEventLists(it->second, eventList)) {
            *errMsg = "Different paths leading to different events at state "
                      + to_string(s->getLabel());
        }
        // we have checked for inconsistency, no need to go further down this state
        return;
    } else
        visited->insert(make_pair(s, eventList));
    if (finalStates->count(s)) {
        if (eventList->size() > 0) {
            *errMsg = "Event " + (*eventList->begin())
                      + " has not been processed by the end of the word.\r\n";
            return;
        }
    } else // If current state is not final
    {
        IOASetOfEdges *adj = (IOASetOfEdges *)s->getOutgoingEdges();
        typename IOASetOfEdges::CIter i;
        for (i = adj->begin(); i != adj->end(); ++i) {
            // make a copy so that child node can not modify the parent nodes list
            // only adds and removes and passes it to its children
            list<Event> *eList = new list<Event>(*eventList);

            IOAEdge *e = (IOAEdge *)(*i);
            OutputAction output = e->label.second;
            InputAction input = e->label.first;
            if (input != "") {
                bool eventFound = false;
                list<Event>::iterator eventIter;
                for (eventIter = eList->begin(); eventIter != eList->end(); eventIter++) {
                    list<pair<Event, EventOutcome>>::iterator gammaIterator;
                    for (gammaIterator = gamma->begin(); gammaIterator != gamma->end();
                         gammaIterator++) {
                        auto p = ((pair<Event, EventOutcome>)*gammaIterator);
                        if (p.first == *eventIter) // finding the event in gamma
                        {
                            if (p.second == input) {
                                eList->erase(
                                        eventIter); // event is processed, remove from eventList
                                eventFound = true;
                                break;
                            }
                        }
                    }
                    if (eventFound)
                        break;
                }
                if (!eventFound) {
                    // we are processing a non-emitted event
                    *errMsg = "on edge: " + to_string(s->getLabel()) + " - "
                              + string(input + "," + output + ", Processing event outcome : "
                                       + input + " where the event is not emitted yet.\r\n");
                    return;
                }
            }
            // we store the events emitted by modes as we move along the automaton
            if (output != "") {
                // look if it has any events to emit
                list<pair<Mode, Event>>::iterator sigmaIterator;
                for (sigmaIterator = sigma->begin(); sigmaIterator != sigma->end();
                     sigmaIterator++) {
                    if (((pair<Mode, Event>)*sigmaIterator).first
                        == output) // this output action emits an event
                    {
                        auto a = ((pair<Mode, Event>)*sigmaIterator);
                        eList->push_back(a.second);
                    }
                }
            }
            IOAState *s2 = (IOAState *)(e->getDestination());

            isConsistentUtil(s2, eList, finalStates, errMsg, visited);

            // visited->insert(make_pair(s2, eList));
            // delete(eList);
        }
    }
}

/**
 * recursive part of determinize
 */
void SMPLSwithEvents::determinizeUtil(IOAState *s,
                                      IOASetOfStates *visited,
                                      IOASetOfStates *finalStates,
                                      CString *errMsg,
                                      ofstream &outfile) {
    /**
     * Deteministic IOA is defined with:
     *	exactly one initial state;
     *  final states that have no outgoing transition;
     *	and branches only based on different outcomes of the same event;
     */
    // implementation is incomplete yet. we have to remove unreachable states and their edges after
    // this
    if (visited->count(s))
        return;

    visited->insert(s);

    IOASetOfEdges *outEdge = (IOASetOfEdges *)s->getOutgoingEdges();
    IOASetOfEdges::CIter i = outEdge->begin();

    IOAEdge *e = (IOAEdge *)(*i);
    InputAction input = e->label.first;
    if (input == "") {
        IOAState *s2 = (IOAState *)e->getDestination();
        outfile << s->stateLabel << "-," << e->label.second << "->" << s2->stateLabel;
        ioa->removeEdge(e);

        i++;
        // we go through all the rest of the edges and remove them from the automaton
        for (; i != outEdge->end(); ++i) {
            e = (IOAEdge *)(*i);
            ioa->removeEdge(e);
            // since we only start with one initial state and remove the others, then the
            // destination state of this edge will be unreachable
            // ioa->removeState((IOAState*)e->getDestination());
        }

        if (finalStates->count(s2)) {
            outfile << " f\n";
        } else {

            outfile << "\n";
            determinizeUtil(s2, visited, finalStates, errMsg, outfile);
        }
    } else // we have an input action
    {
        Event ev = this->findEventByOutcome(input);
        // we go through all the edges
        for (; i != outEdge->end(); i++) {
            e = (IOAEdge *)(*i);
            input = e->label.first;
            // we remove the ones that dont have an input action from the automaton
            if (input == "") {
                ioa->removeEdge(e);
                // since we only start with one initial state and remove the others, then the
                // destination state of this edge will be unreachable
                // ioa->removeState((IOAState*)e->getDestination());
            } else {
                // only allow edges with the outcome of the same event
                if (this->findEventByOutcome(input) == ev) {
                    IOAState *s2 = (IOAState *)e->getDestination();
                    outfile << s->stateLabel << "-" << e->label.first << "," << e->label.second
                            << "->" << s2->stateLabel;

                    ioa->removeEdge(e);
                    if (finalStates->count(s2)) {
                        outfile << " f\n";
                    } else {
                        outfile << "\n";
                        determinizeUtil(s2, visited, finalStates, errMsg, outfile);
                    }
                } else {
                    ioa->removeEdge(e);
                }
            }
        }
    }
}
bool SMPLSwithEvents::compareEventLists(list<Event> *l1, list<Event> *l2) {
    bool result = true;

    if (l1->size() != l2->size()) {
        return false;
    }

    list<Event>::iterator it2 = l1->begin();
    for (; it2 != l1->end(); it2++) {
        result = false;
        list<Event>::iterator it3 = l2->begin();
        for (; it3 != l2->end(); it3++) {
            if (*it2 == *it3) {
                result = true;
                break;
            }
        }
    }
    return result;
}

void SMPLSwithEvents::dissectScenarioMatrices() {
    ScenarioMatrices::iterator s;
    for (s = sm->begin(); s != sm->end(); s++) {
        numberOfResources = 0;
        DissectedScenarioMatrix *dis = new DissectedScenarioMatrix();
        dis->core->insert(*s);

        Matrix *m = s->second; //->getTransposedCopy();
        std::list<uint> mSubIndices;
        numberOfResources = min(m->getRows(), m->getCols());
        for (uint x = 0; x < numberOfResources; x++) {
            mSubIndices.push_back(x);
            // numberOfResources++;
        }

        dis->core->begin()->second = m->getSubMatrixNonSquareRows(mSubIndices);
        if (m->getRows() > numberOfResources) // if we have event rows
        {
            mSubIndices.clear();
            for (uint x = numberOfResources; x < max(m->getRows(), m->getCols()); x++) {
                mSubIndices.push_back(x);
                Matrix *mEventRow = m->getSubMatrixNonSquareRows(mSubIndices);
                dis->eventRows->push_back(mEventRow);
                mSubIndices.clear();
            }
        }
        disMatrices->push_back(dis);
    }
}

void SMPLS::loadAutomatonFromIOAFile(CString fileName) {
    cout << "Loading automaton." << std::endl;
    // remember which state maps to which new state
    map<CId, CString> stateMap;
    elsFSM = new EdgeLabeledScenarioFSM();

    ifstream input_stream(fileName);
    if (!input_stream)
        cerr << "Can't open input file : " << fileName << endl;
    string line;

    CId stateId = 0;
    // extract all the text from the input file
    while (getline(input_stream, line)) {

        size_t i1 = line.find("-");
        size_t i2 = line.find("->");

        if (i1 != std::string::npos && i2 != std::string::npos) {
            if (i1 < i2) // if we have a valid line as in "state i f --edge--> state2 f"
            {
                string srcState = "";
                CString edge = "";
                std::string destState = "";

                CString srcStateDescription = line.substr(0, i1);
                srcStateDescription.trim();
                srcState = srcStateDescription.substr(0, srcStateDescription.find(" "));

                CString destStateDescription = line.substr(i2 + 2);
                destStateDescription.trim();
                destState = destStateDescription.substr(0, destStateDescription.find_first_of(" "));

                std::string edgeDescription = line.substr(i1, i2 - i1);
                edge = edgeDescription.substr(edgeDescription.find_first_not_of("-"));
                edge = edge.substr(0, edge.find("-"));
                edge.trim();

                // look for invalid edge desc
                size_t tempIndex1 = edge.find(";");
                size_t tempIndex2 = edge.find(",");

                if (tempIndex1 != std::string::npos
                    || tempIndex2 != std::string::npos) // we have an ioa
                    throw CException("invalid edge description for normal automaton. edge can not "
                                     "contain ';' or ','");

                // look if the state already exists
                bool duplicateState = false;
                ELSState *q1 = new ELSState(stateId);
                for (unsigned int i = 0; i < stateMap.size(); i++) {
                    if (stateMap[i] == srcState) {
                        duplicateState = true;
                        q1 = elsFSM->getStateLabeled(i);
                        break;
                    }
                }

                if (!duplicateState) {
                    elsFSM->addState(q1);
                    stateMap[stateId] = srcState;
                    stateId++;
                }
                // look for state annotations
                srcStateDescription.erase(0, srcState.length());
                if (srcStateDescription.find(" i") != std::string::npos
                    || srcStateDescription.find(" initial") != std::string::npos)
                    elsFSM->addInitialState(q1);
                if (srcStateDescription.find(" f") != std::string::npos
                    || srcStateDescription.find(" final") != std::string::npos)
                    elsFSM->addFinalState(q1);

                duplicateState = false;
                ELSState *q2 = new ELSState(stateId);
                for (unsigned int i = 0; i < stateMap.size(); i++) {
                    if (stateMap[i] == destState) {
                        duplicateState = true;

                        q2 = elsFSM->getStateLabeled(i);
                        break;
                    }
                }

                if (!duplicateState) {
                    elsFSM->addState(q2);
                    stateMap[stateId] = destState;
                    stateId++;
                }
                // look for state annotations
                destStateDescription.erase(0, destState.length());
                if (destStateDescription.find(" i") != std::string::npos
                    || destStateDescription.find(" initial") != std::string::npos)
                    elsFSM->addInitialState(q2);
                if (destStateDescription.find(" f") != std::string::npos
                    || destStateDescription.find(" final") != std::string::npos)
                    elsFSM->addFinalState(q2);

                elsFSM->addEdge(q1, edge, q2);
            } else {
                throw CException("invalid line in ioa file!");
            }
        }
    }
    cout << endl << "automaton states size: " << elsFSM->getStates()->size() << endl;
    cout << "automaton transitions size: " << elsFSM->getEdges()->size() << endl;
}

void SMPLS::loadAutomatonFromDispatchingFile(CString fileName) {
    cout << "Loading dispatching file." << std::endl;
    // remember which state maps to which new state
    map<CId, CString> stateMap;
    elsFSM = new EdgeLabeledScenarioFSM();

    ifstream input_stream(fileName);
    if (!input_stream)
        cerr << "Can't open input file : " << fileName << endl;
    CString line;

    int counter = 0;
    while (getline(input_stream, line)) {
        size_t i1 = line.find("{");
        if (i1 != std::string::npos)
            break;
        if (counter == 10000) {
            throw CException("ERROR : Could not find a { which indicates start of dispatching "
                             "after 10000 lines. are you sure this is a dispatching file?");
        }
        counter++;
    }

    CId stateId = 0;

    int j = 0;
    ELSState *lastState = nullptr;
    // extract all the text from the input file
    while (getline(input_stream, line)) {
        line.trim();
        if (line.find("}") != std::string::npos)
            break;
        if (line.size() > 0) {
            string srcState = "";
            CString edge = "";
            string destState = "";

            CString srcStateDescription = "s_" + std::to_string(j);
            srcStateDescription.trim();
            srcState = srcStateDescription.substr(0, srcStateDescription.find(" "));

            CString destStateDescription = "s_" + std::to_string(1 + j);
            destStateDescription.trim();
            destState = destStateDescription.substr(0, destStateDescription.find_first_of(" "));

            string edgeDescription = line;
            edge = edgeDescription.substr(edgeDescription.find_first_not_of("-"));
            edge = edge.substr(0, edge.find("-"));
            edge.trim();

            cout << "Edge: " << edge << std::endl;
            // look if the state already exists
            bool duplicateState = false;
            ELSState *q1 = new ELSState(stateId);
            for (unsigned int i = 0; i < stateMap.size(); i++) {
                if (stateMap[i] == srcState) {
                    duplicateState = true;
                    q1 = elsFSM->getStateLabeled(i);
                    break;
                }
            }

            if (!duplicateState) {
                elsFSM->addState(q1);
                stateMap[stateId] = srcState;
                stateId++;
            }

            if (j == 0)
                elsFSM->addInitialState(q1);

            duplicateState = false;
            ELSState *q2 = new ELSState(stateId);
            for (unsigned int i = 0; i < stateMap.size(); i++) {
                if (stateMap[i] == destState) {
                    duplicateState = true;

                    q2 = elsFSM->getStateLabeled(i);
                    break;
                }
            }

            if (!duplicateState) {
                elsFSM->addState(q2);
                stateMap[stateId] = destState;
                stateId++;
            }
            lastState = q2;

            elsFSM->addEdge(q1, edge, q2);
            j++;
        }
    }
    elsFSM->addFinalState(lastState);
    cout << endl << "automaton states size: " << elsFSM->getStates()->size() << endl;
    cout << "automaton transitions size: " << elsFSM->getEdges()->size() << endl;
}

void SMPLSwithEvents::loadIOAutomatonFromDispatchingFile(CString fileName) {
    cout << "Loading I/O automaton from dispatching file." << std::endl;
    // remember which state maps to which new state
    map<CId, CString> stateMap;
    ifstream input_stream(fileName);
    if (!input_stream)
        cerr << "Can't open input file : " << fileName << endl;
    CString line;
    int lineNumber = 0;
    CId stateId = 0;
    int counter = 0;
    while (getline(input_stream, line)) {
        size_t i1 = line.find("{");
        if (i1 != std::string::npos)
            break;
        if (counter == 10000) {
            throw CException("ERROR : Could not find a '{' which indicates start of dispatching, "
                             "after 10000 lines. are you sure this is a dispatching file?");
        }
        counter++;
    }
    int j = 0;

    IOAState *lastState = new IOAState(stateId);
    // extract all the text from the input file
    while (getline(input_stream, line)) {
        lineNumber++;
        string srcState = "";
        CString edge = "";
        string destState = "";
        CString inputAction = "";
        CString outputAction = "";
        line.trim();
        if (line.find("}") != std::string::npos)
            break;
        if (line.size() <= 0)
            continue;
        CString srcStateDescription = "s_" + std::to_string(j);
        srcStateDescription.trim();
        srcState = srcStateDescription.substr(0, srcStateDescription.find(" "));

        CString destStateDescription = "s_" + std::to_string(1 + j);
        destStateDescription.trim();
        destState = destStateDescription.substr(0, destStateDescription.find_first_of(" "));

        string edgeDescription = line;
        edge = edgeDescription.substr(edgeDescription.find_first_not_of("-"));
        edge = edge.substr(0, edge.find("-"));
        edge.trim();

        // look for the input;output pair on the edge description
        // the first edge description dictates whether we have a normal automaton or an IOAutomaton.
        // (we could also make a command line setting for that but meh..) we have two separators for
        // io edges: ";" and ","
        size_t tempIndex1 = edge.find(";");
        size_t tempIndex2 = edge.find(",");
        string delimiter = "";
        if (tempIndex2 != std::string::npos)
            delimiter = ",";
        else if (tempIndex1 != std::string::npos)
            delimiter = ";";

        if (delimiter != "") {
            // not sure if we have to split edge label at this stage or later. let's go with later.
            size_t delimiterIndex = edge.find(delimiter);
            inputAction = edge.substr(0, delimiterIndex);
            outputAction = edge.substr(delimiterIndex + 1, edge.length() - delimiterIndex + 1);
            inputAction.trim();
            outputAction.trim();
        } else {
            outputAction = edge;
        }
        // "" is the standard 'empty' io action here
        if (inputAction == "$")
            inputAction = "";
        if (outputAction == "#")
            outputAction = "";

        IOAEdgeLabel *edgeLabel = new IOAEdgeLabel(inputAction, outputAction);

        // look if the state already exists
        bool duplicateState = false;
        IOAState *q1 = new IOAState(stateId);
        for (unsigned int i = 0; i < stateMap.size(); i++) {
            if (stateMap[i] == srcState) {
                duplicateState = true;
                q1 = ioa->getStateLabeled(i);
                break;
            }
        }

        if (!duplicateState) {
            ioa->addState(q1);
            stateMap[stateId] = srcState;
            stateId++;
        }
        // look for state annotations
        srcStateDescription.erase(0, srcState.length());
        srcStateDescription.trim();
        if (j == 0)
            ioa->addInitialState(q1);

        duplicateState = false;
        IOAState *q2 = new IOAState(stateId);
        for (unsigned int i = 0; i < stateMap.size(); i++) {
            if (stateMap[i] == destState) {
                duplicateState = true;

                q2 = ioa->getStateLabeled(i);
                break;
            }
        }

        if (!duplicateState) {
            ioa->addState(q2);
            stateMap[stateId] = destState;
            stateId++;
        }
        // look for state annotations
        destStateDescription.erase(0, destState.length());
        destStateDescription.trim();
        if (destStateDescription.find("i") != std::string::npos
            || destStateDescription.find("initial") != std::string::npos)
            ioa->addInitialState(q2);
        if (destStateDescription.find("f") != std::string::npos
            || destStateDescription.find("final") != std::string::npos)
            ioa->addFinalState(q2);
        lastState = q2;
        // cout << "Edge: -" << inputAction << "," << outputAction << std::endl;
        ioa->addEdge(q1, *edgeLabel, q2);
        j++;
    }
    ioa->addFinalState(lastState);
    cout << endl << "IO Automaton states size: " << ioa->getStates()->size() << endl;
    cout << "IO Automaton transitions size: " << ioa->getEdges()->size() << endl;
}

void SMPLSwithEvents::loadIOAutomatonFromIOAFile(CString fileName) {
    cout << "Loading I/O automaton." << std::endl;
    // remember which state maps to which new state
    map<CId, CString> stateMap;
    ifstream input_stream(fileName);
    if (!input_stream)
        cerr << "Can't open input file : " << fileName << endl;
    string line;
    int lineNumber = 0;
    CId stateId = 0;
    // extract all the text from the input file
    while (getline(input_stream, line)) {
        lineNumber++;
        size_t i1 = line.find("-");
        size_t i2 = line.find("->");

        if (i1 != std::string::npos && i2 != std::string::npos) {
            if (i1 < i2) // if we have a valid line as in "state i f --edge--> state2 f"
            {
                string srcState = "";
                CString edge = "";
                string destState = "";
                CString inputAction = "";
                CString outputAction = "";

                CString srcStateDescription = line.substr(0, i1);
                srcStateDescription.trim();
                srcState = srcStateDescription.substr(0, srcStateDescription.find(" "));

                CString destStateDescription = line.substr(i2 + 2);
                destStateDescription.trim();
                destState = destStateDescription.substr(0, destStateDescription.find_first_of(" "));

                CString edgeDescription = line.substr(i1, i2 - i1);
                edge = edgeDescription.substr(edgeDescription.find_first_not_of("-"));
                edge = edge.substr(0, edge.find("-"));
                edge.trim();

                // look for the input;output pair on the edge description
                // the first edge description dictates whether we have a normal automaton or an
                // IOAutomaton. (we could also make a command line setting for that but meh..) we
                // have two separators for io edges: ";" and ","
                size_t tempIndex1 = edge.find(";");
                size_t tempIndex2 = edge.find(",");
                string delimiter = "";
                if (tempIndex2 != std::string::npos)
                    delimiter = ",";
                else if (tempIndex1 != std::string::npos)
                    delimiter = ";";
                else
                    throw CException("Can not split edge into input and output action. Edge "
                                     "description is invalid. at line: "
                                     + lineNumber);

                // not sure if we have to split edge label at this stage or later. let's go with
                // later.
                size_t delimiterIndex = edge.find(delimiter);
                inputAction = edge.substr(0, delimiterIndex);
                outputAction = edge.substr(delimiterIndex + 1, edge.length() - delimiterIndex + 1);
                inputAction.trim();
                outputAction.trim();

                // "" is the standard 'empty' io action here
                if (inputAction == "$")
                    inputAction = "";
                if (outputAction == "#")
                    outputAction = "";

                IOAEdgeLabel *edgeLabel = new IOAEdgeLabel(inputAction, outputAction);

                // look if the state already exists
                bool duplicateState = false;
                IOAState *q1 = new IOAState(stateId);
                for (unsigned int i = 0; i < stateMap.size(); i++) {
                    if (stateMap[i] == srcState) {
                        duplicateState = true;
                        q1 = ioa->getStateLabeled(i);
                        break;
                    }
                }

                if (!duplicateState) {
                    ioa->addState(q1);
                    stateMap[stateId] = srcState;
                    stateId++;
                }
                // look for state annotations
                srcStateDescription.erase(0, srcState.length());
                srcStateDescription.trim();
                if (srcStateDescription.find("i") != std::string::npos
                    || srcStateDescription.find("initial") != std::string::npos)
                    ioa->addInitialState(q1);
                if (srcStateDescription.find("f") != std::string::npos
                    || srcStateDescription.find("final") != std::string::npos)
                    ioa->addFinalState(q1);

                duplicateState = false;
                IOAState *q2 = new IOAState(stateId);
                for (unsigned int i = 0; i < stateMap.size(); i++) {
                    if (stateMap[i] == destState) {
                        duplicateState = true;

                        q2 = ioa->getStateLabeled(i);
                        break;
                    }
                }

                if (!duplicateState) {
                    ioa->addState(q2);
                    stateMap[stateId] = destState;
                    stateId++;
                }
                // look for state annotations
                destStateDescription.erase(0, destState.length());
                destStateDescription.trim();
                if (destStateDescription.find("i") != std::string::npos
                    || destStateDescription.find("initial") != std::string::npos)
                    ioa->addInitialState(q2);
                if (destStateDescription.find("f") != std::string::npos
                    || destStateDescription.find("final") != std::string::npos)
                    ioa->addFinalState(q2);

                ioa->addEdge(q1, *edgeLabel, q2);
            } else {
                throw CException("invalid line in ioa file! at line: " + lineNumber);
            }
        }
    }
}

void SMPLS::loadMPMatricesFromMPTFile(CString file) {

    cout << "Loading max-plus matrices." << std::endl;
    // Open file
    CDoc *appGraphDoc = CParseFile(file);
    sm = new ScenarioMatrices();

    // Locate the root element
    CNode *rootNode = CGetRootNode(appGraphDoc);

    // TODO : properly check if this is a valid mpt
    /*if (rootNode->name != temp)
    throw CException("Root element in file '" + file + "' is not "
            "mpt:MaxPlusSpecification");*/

    CNode *matrices = CGetChildNode(rootNode, "matrices");

    // go through every matrix in the xml
    // and create their Matrix object
    while (matrices != NULL) {
        // get number of cols and rows to initialize the matrix
        int rows = CGetNumberOfChildNodes(matrices, "rows");
        int cols = 0;
        CNode *rowNode = CGetChildNode(matrices, "rows");
        if (rowNode != NULL)
            cols = CGetNumberOfChildNodes(rowNode, "values");

        // init the matrix
        Matrix *matrix = new Matrix(rows, cols);

        // reset the ints to be used as indices afterward
        rows = 0;
        cols = 0;

        while (rowNode != NULL) {
            cols = 0;
            CNode *vNode = CGetChildNode(rowNode, "values");
            while (vNode != NULL) {
                string content = CGetNodeContent(vNode);

                if (content != "-Infinity") {
                    double value = atof(content.c_str());
                    matrix->put(rows, cols, value);
                }
                cols++;
                vNode = CNextNode(vNode, "values");
            }
            rows++;
            rowNode = CNextNode(rowNode, "rows");
        }

        // we transpose the matrix, this is needed when reading from LSAT mpt files.
        // Gaubert MPA uses transposed version
        // matrix = matrix->getTransposedCopy();
        CString matrixName = CGetAttribute(matrices, "name");
        (*sm)[matrixName] = matrix;
        matrices = CNextNode(matrices, "matrices");
    }
}

void SMPLSwithEvents::loadActivities(CString file) {
    cout << "Loading activities." << std::endl;
    // remember which state maps to which new state
    ifstream input_stream(file);
    if (!input_stream)
        cerr << "Can't open input file : " << file << endl;
    string line;

    CString lastActivity = "";
    // extract all the text from the input file line by line
    while (getline(input_stream, line)) {

        size_t i = line.find("activity");
        if (i != std::string::npos) {
            // get the activity name
            lastActivity = line.substr(i + 9);
            lastActivity.trim();
            lastActivity = lastActivity.erase(lastActivity.find_first_of("{"));
            lastActivity.trim();
            string activityString = loadEntity(input_stream, line);

            i = activityString.find("events");
            if (i != std::string::npos) {
                string eventString = activityString.substr(i + 6);
                i = eventString.find("{");
                eventString = eventString.substr(i + 1, eventString.find("}") - i - 1);
                eventString.erase(std::remove(eventString.begin(), eventString.end(), '\t'),
                                  eventString.end()); // lsat fills a lot of \t in files which make
                                                      // their way into here, we get rid of them

                stringstream ss(eventString);
                Event str;
                while (getline(ss, str, ',')) {
                    str.trim();
                    // the magic we have been waiting for happens here.
                    // add to sigma
                    sigma->insert(sigma->end(), make_pair(lastActivity, str));
                }
            }
        }

        i = line.find("event");
        if (i != std::string::npos) {
            // get the event name
            Event eventName = line.substr(i + 6);
            eventName.trim();
            eventName = eventName.erase(eventName.find_first_of(" "));

            eventName.trim();
            string eventString = loadEntity(input_stream, line);
            //
            i = eventString.find("{");
            eventString = eventString.substr(i + 1, eventString.find("}") - i - 1);
            eventString.erase(std::remove(eventString.begin(), eventString.end(), '\t'),
                              eventString.end()); // lsat fills a lot of \t in files which make
                                                  // their way into here, we get rid of them

            // go to outcomes
            eventString = eventString.substr(eventString.find("outcomes") + 8);
            eventString = eventString.substr(eventString.find(":") + 1);

            stringstream ss(eventString);
            EventOutcome str;
            while (getline(ss, str, ',')) {
                str.trim();
                // the magic we have been waiting for happens here.
                // add to gamma
                gamma->insert(gamma->end(), make_pair(eventName, str));
            }
        }
    }
}
} // namespace MaxPlus
