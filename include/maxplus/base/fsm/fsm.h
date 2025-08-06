/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   fsm.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   March 23, 2009
 *
 *  Function        :   generic Finite State Machine functionality
 *
 *  History         :
 *      23-03-09    :   Initial version.
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

#ifndef MAXPLUS_BASE_FSM_FSM_H
#define MAXPLUS_BASE_FSM_FSM_H

#include "maxplus/base/basic_types.h"
#include "maxplus/base/exception/exception.h"
#include "maxplus/base/string/cstring.h"
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <utility>

namespace MaxPlus::FSM {

// the abstract ancestor of FSM types
namespace Abstract {

// Unique ID for deterministic sets
class WithUniqueID {
public:
    WithUniqueID() : id(nextID++) {}

    [[nodiscard]] bool lessThan(const WithUniqueID &rhs) const { return this->id < rhs.id; }
    [[nodiscard]] bool operator==(const WithUniqueID &rhs) const { return this->id == rhs.id; }

    [[nodiscard]] CId getId() const { return this->id; }

private:
    static CId nextID;
    CId id;
};

// forward declaration of FSM state class
class State;
using StateRef = const State *;

// Edge of an FSM
class Edge : public WithUniqueID {
public:
    // constructor
    Edge(State &src, State &dst) : source(&src), destination(&dst) {}

    Edge(const Edge &) = default;
    Edge &operator=(const Edge &other) = delete;
    Edge(Edge &&) = default;
    Edge &operator=(Edge &&) = delete;

    // destructor
    virtual ~Edge() = default;
    ;

    // virtual access methods
    [[nodiscard]] virtual StateRef getSource() const { return this->source; }
    [[nodiscard]] virtual StateRef getDestination() const { return this->destination; }

private:
    // source and destination state of the edge
    State *source;
    State *destination;
};

// // Edge ordering for sets
// struct EdgeCompareLessThan {
//     bool operator()(const Edge &lhs, const Edge &rhs) const { return lhs.lessThan(rhs); };
// };

// A set of edges
// the set is assumed to have unique ownership of the edges
class SetOfEdges : public std::map<CId, std::unique_ptr<Edge>> {
public:
    void remove(const Edge &e) { this->erase(e.getId()); }
};

using EdgeRef = const Edge *;

struct EdgeRefCompareLessThan {
    bool operator()(EdgeRef lhs, EdgeRef rhs) const { return lhs->lessThan(*rhs); };
};

// A set of references to edges
class SetOfEdgeRefs : public std::set<EdgeRef, EdgeRefCompareLessThan> {
public:
    using CIter = SetOfEdgeRefs::const_iterator;
    SetOfEdgeRefs() = default;
    virtual ~SetOfEdgeRefs() = default;

    // Delete copy/move constructors and assignment operators
    SetOfEdgeRefs(const SetOfEdgeRefs &) = default;
    SetOfEdgeRefs &operator=(const SetOfEdgeRefs &) = default;
    SetOfEdgeRefs(SetOfEdgeRefs &&) noexcept = default;
    SetOfEdgeRefs &operator=(SetOfEdgeRefs &&) = default;
};

// Ancestor of all FSM states
class State : public WithUniqueID {
public:
    // constructor
    State() = default;

    // destructor
    virtual ~State() = default;

    State(const State &) = default;
    State &operator=(const State &other) = delete;
    State(State &&) noexcept = default;
    State &operator=(State &&) = delete;

    // add an outgoing edge to the state
    void addOutGoingEdge(Edge &e) {
        assert(e.getSource() == this);
        this->outgoingEdges.insert(&e);
    }

    // access the outgoing edges
    [[nodiscard]] virtual const SetOfEdgeRefs &getOutgoingEdges() const {
        return this->outgoingEdges;
    }

    // as a reference
    [[nodiscard]] virtual StateRef getReference() const { return this; }

    void insertOutgoingEdge(Edge &e) { this->outgoingEdges.insert(&e); }

    void removeOutgoingEdge(EdgeRef e) { this->outgoingEdges.erase(e); }

private:
    SetOfEdgeRefs outgoingEdges;
};

// A set of states
// the set is assumed to have unique ownership of the states
class SetOfStates : public std::map<CId, std::unique_ptr<State>> {
public:
    SetOfStates() = default;
    virtual ~SetOfStates() = default;

    // Delete copy/move constructors and assignment operators
    SetOfStates(const SetOfStates &) = delete;
    SetOfStates &operator=(const SetOfStates &) = delete;
    SetOfStates(SetOfStates &&) = delete;
    SetOfStates &operator=(SetOfStates &&) = delete;

    void remove(const State &s) { this->erase(s.getId()); }
    void addState(std::unique_ptr<State> s) { this->emplace(s->getId(), std::move(s)); }

    State &withId(const CId id) { return *this->at(id); }
};

struct StateRefCompareLessThan {
    bool operator()(const State *lhs, const State *rhs) const { return lhs->lessThan(*rhs); };
};

// A set of references to states
class SetOfStateRefs : public std::set<StateRef, StateRefCompareLessThan> {
public:
    SetOfStateRefs() = default;
    virtual ~SetOfStateRefs() = default;

    // copy/move constructors and assignment operators
    SetOfStateRefs(const SetOfStateRefs &) = default;
    SetOfStateRefs &operator=(const SetOfStateRefs &) = default;
    SetOfStateRefs(SetOfStateRefs &&) = default;
    SetOfStateRefs &operator=(SetOfStateRefs &&) = default;

    using CIter = SetOfStateRefs::const_iterator;
    bool includesState(const State *s) { return this->find(s) != this->end(); }
};

// A list of references to states
class ListOfStateRefs : public std::list<StateRef> {
public:
    using CIter = ListOfStateRefs::const_iterator;
    ListOfStateRefs() = default;
    virtual ~ListOfStateRefs() = default;

    // copy/move constructors and assignment operators
    ListOfStateRefs(const ListOfStateRefs &) = default;
    ListOfStateRefs &operator=(const ListOfStateRefs &) = default;
    ListOfStateRefs(ListOfStateRefs &&) = default;
    ListOfStateRefs &operator=(ListOfStateRefs &&) = default;
};

// forward declaration of reachable states strategy
class ReachableStates;

// The abstract ancestor of finite state machines
class FiniteStateMachine {
public:
    FiniteStateMachine() = default;
    virtual ~FiniteStateMachine() = default;

    FiniteStateMachine(const FiniteStateMachine &) = delete;
    FiniteStateMachine &operator=(const FiniteStateMachine &other) = delete;
    FiniteStateMachine(FiniteStateMachine &&) = default;
    FiniteStateMachine &operator=(FiniteStateMachine &&) = delete;
    virtual std::unique_ptr<FiniteStateMachine> newInstance() = 0;

    [[nodiscard]] virtual StateRef getInitialState() const = 0;
    [[nodiscard]] virtual const SetOfStateRefs &getInitialStates() const = 0;
    [[nodiscard]] virtual const SetOfStateRefs &getFinalStates() const = 0;
    [[nodiscard]] virtual const SetOfStates &getStates() const = 0;
    [[nodiscard]] virtual SetOfStateRefs getStateRefs() const = 0;
    [[nodiscard]] virtual const SetOfEdges &getEdges() const = 0;
};

//
// A generic DFS strategy on the target FSM
// overwrite the methods onEnterState, onLeaveState, onTransition and onSimpleCycle with
// the desired actions
//
class DepthFirstSearch {

public:
    DepthFirstSearch(const DepthFirstSearch &) = delete;
    DepthFirstSearch &operator=(const DepthFirstSearch &other) = delete;
    DepthFirstSearch(DepthFirstSearch &&) = delete;
    DepthFirstSearch &operator=(DepthFirstSearch &&) = delete;

    // supporting class for DFS stack items
    class DFSStackItem {
    public:
        // constructor
        explicit DFSStackItem(StateRef s) : state(s) {
            this->iter = s->getOutgoingEdges().begin();
        };

        // access state
        StateRef getState() { return this->state; }

        SetOfEdgeRefs::CIter getIter() { return this->iter; }

        // test if all outgoing edges have been done
        bool atEnd() { return this->iter == this->state->getOutgoingEdges().end(); }

        // move to the next edge
        void advance() { (this->iter)++; }

    private:
        StateRef state;
        SetOfEdgeRefs::CIter iter;
    };

    using DfsStack = std::list<DFSStackItem>;

private:
    DfsStack dfsStack;

public:
    virtual ~DepthFirstSearch() = default;
    using DFSStackCIter = DfsStack::const_iterator;

    virtual void onEnterState(StateRef s) {};

    virtual void onLeaveState(StateRef s) {};

    virtual void onTransition(const Edge &e) {};

    virtual void onSimpleCycle(DfsStack &stack) {};

    const FiniteStateMachine &getFSM() { return this->fsm; }

    explicit DepthFirstSearch(const FiniteStateMachine &targetFsm) : fsm(targetFsm) {};

    // Execute the depth first search
    void DoDepthFirstSearch(const SetOfStateRefs &startingStates, // NOLINT(*cognitive-complexity)
                            bool fullDFS = false) {
        // store visited states
        SetOfStateRefs visitedStates;
        SetOfStateRefs statesOnStack;

        this->_abort = false;

        // for each of the starting states
        auto nextStartingState = startingStates.begin();
        while (nextStartingState != startingStates.end()) {

            // skip states we have already visited
            while (nextStartingState != startingStates.end()
                   && visitedStates.includesState(*nextStartingState)) {
                nextStartingState++;
            }
            // if we did not find any state anymore
            if (nextStartingState == startingStates.end()) {
                break;
            }

            statesOnStack.clear();
            StateRef s = *nextStartingState;
            dfsStack.emplace_back(s);
            statesOnStack.insert(s);
            visitedStates.insert(s);
            this->onEnterState(s);

            while (!this->_abort && !(dfsStack.empty())) {
                DFSStackItem &si = dfsStack.back();
                if (!visitedStates.includesState(si.getState())) {
                    this->onEnterState(si.getState());
                    visitedStates.insert(si.getState());
                }
                // current item complete?
                if (si.atEnd()) {
                    // pop it from stack
                    this->onLeaveState(si.getState());
                    const auto *const s = si.getState();
                    statesOnStack.erase(s);
                    if (fullDFS) {
                        assert(visitedStates.includesState(s));
                        visitedStates.erase(s);
                    }
                    dfsStack.pop_back();
                } else {
                    // goto next edge
                    const auto *e = *(si.getIter());
                    si.advance();
                    StateRef dest = e->getDestination();
                    bool revisit = statesOnStack.includesState(dest);
                    if (revisit) {
                        // cycle found
                        this->onSimpleCycle(dfsStack);
                    } else {
                        if (!visitedStates.includesState(dest)) {
                            // if target state not visited before
                            dfsStack.emplace_back(dest);
                            this->onTransition(*e);
                            this->onEnterState(dest);
                            visitedStates.insert(dest);
                            statesOnStack.insert(dest);
                        }
                    }
                }
            }
        }
    }

    void DoDepthFirstSearch(const StateRef &startingState, bool fullDFS = false) {
        SetOfStateRefs stateSet;
        stateSet.insert(startingState);
        this->DoDepthFirstSearch(stateSet, fullDFS);
    }

    // Execute the depth first search
    void DoDepthFirstSearch(bool fullDFS = false) {
        this->DoDepthFirstSearch(this->fsm.getInitialStates(), fullDFS);
    }

    void abortDFS() { this->_abort = true; }

private:
    const FiniteStateMachine &fsm;
    bool _abort{};
};

class DepthFirstSearchLambda : public DepthFirstSearch {

public:
    DepthFirstSearchLambda(const DepthFirstSearchLambda &) = delete;
    DepthFirstSearchLambda &operator=(const DepthFirstSearchLambda &other) = delete;
    DepthFirstSearchLambda(DepthFirstSearchLambda &&) = delete;
    DepthFirstSearchLambda &operator=(DepthFirstSearchLambda &&) = delete;

private:
    using TOnEnterLambda = std::function<void(StateRef s)>;
    TOnEnterLambda _onEnterStateLambda;
    using TOnLeaveLambda = std::function<void(StateRef s)>;
    TOnLeaveLambda _onLeaveStateLambda;
    using TOnTransitionLambda = std::function<void(const Edge &e)>;
    TOnTransitionLambda _onTransitionLambda;
    using TOnSimpleCycleLambda = std::function<void(const DepthFirstSearch::DfsStack &stack)>;
    TOnSimpleCycleLambda _onSimpleCycleLambda;

public:
    ~DepthFirstSearchLambda() override = default;

    void onEnterState(StateRef s) override { this->_onEnterStateLambda(s); };

    void onLeaveState(StateRef s) override { this->_onLeaveStateLambda(s); };

    void onTransition(const Edge &e) override { this->_onTransitionLambda(e); };

    void onSimpleCycle(DepthFirstSearch::DfsStack &stack) override {
        this->_onSimpleCycleLambda(stack);
    };

    explicit DepthFirstSearchLambda(const FiniteStateMachine &targetFsm) :
        _onEnterStateLambda([](StateRef) {}),
        _onLeaveStateLambda([](StateRef) {}),
        _onTransitionLambda([](const Edge &) {}),
        _onSimpleCycleLambda([](const DepthFirstSearch::DfsStack &) {}),
        DepthFirstSearch(targetFsm) {};

    void setOnEnterLambda(TOnEnterLambda lambda) { this->_onEnterStateLambda = std::move(lambda); }

    void setOnLeaveLambda(TOnLeaveLambda lambda) { this->_onLeaveStateLambda = std::move(lambda); }

    void setOnTransitionLambda(TOnTransitionLambda lambda) {
        this->_onTransitionLambda = std::move(lambda);
    }

    void setOnSimpleCycleLambda(TOnSimpleCycleLambda lambda) {
        this->_onSimpleCycleLambda = std::move(lambda);
    }
};

// Check for cycles
class DetectCycle : public DepthFirstSearch {
public:
    bool hasCycle = false;

    explicit DetectCycle(const FiniteStateMachine &targetFsm) : DepthFirstSearch(targetFsm) {};

    ~DetectCycle() override = default;

    DetectCycle(const DetectCycle &) = delete;
    DetectCycle &operator=(const DetectCycle &other) = delete;
    DetectCycle(DetectCycle &&) = delete;
    DetectCycle &operator=(DetectCycle &&) = delete;

    bool checkForCycles() { return this->checkForCycles(nullptr); }

    bool checkForCycles(ListOfStateRefs *cycle) {
        this->cycle = cycle;
        const SetOfStateRefs states = this->getFSM().getStateRefs();
        this->DoDepthFirstSearch(states);
        return this->hasCycle;
    }

private:
    ListOfStateRefs *cycle = nullptr;

    void onSimpleCycle(DfsStack &stack) override {
        if (!this->hasCycle) {
            if (this->cycle != nullptr) {
                for (auto si : stack) {
                    this->cycle->push_back(si.getState());
                }
            }
            this->hasCycle = true;
            this->abortDFS();
        }
    }
};

// Reachable states strategy based on DFS
class ReachableStates : public DepthFirstSearch {
public:
    SetOfStateRefs result;

    explicit ReachableStates(FiniteStateMachine &targetFsm) : DepthFirstSearch(targetFsm) {};

    ~ReachableStates() override = default;

    ReachableStates(const ReachableStates &) = delete;
    ReachableStates &operator=(const ReachableStates &other) = delete;
    ReachableStates(ReachableStates &&) = delete;
    ReachableStates &operator=(ReachableStates &&) = delete;

    void onEnterState(StateRef s) override { this->result.insert(s); };
};

}; // namespace Abstract

namespace Labeled {

// forward declarations
template <typename StateLabelType, typename EdgeLabelType> class State;

template <typename StateLabelType, typename EdgeLabelType> class Edge : public Abstract::Edge {
public:
    Edge(State<StateLabelType, EdgeLabelType> &src,
         EdgeLabelType &lbl,
         State<StateLabelType, EdgeLabelType> &dst) :
        Abstract::Edge(src, dst), label(lbl) {}

    [[nodiscard]] const EdgeLabelType &getLabel() const { return this->label; }
    void setLabel(EdgeLabelType l) { this->label = l; }

private:
    EdgeLabelType label;
};

template <typename StateLabelType, typename EdgeLabelType>
using EdgeRef = const Edge<StateLabelType, EdgeLabelType> *;

template <typename StateLabelType, typename EdgeLabelType>
class SetOfEdges : public Abstract::SetOfEdges {
public:
    using CIter = typename Abstract::SetOfEdges::const_iterator;
};

template <typename StateLabelType, typename EdgeLabelType>
class SetOfEdgeRefs : public Abstract::SetOfEdgeRefs {
public:
};

template <typename StateLabelType, typename EdgeLabelType>
using StateRef = const State<StateLabelType, EdgeLabelType> *;

// template <typename StateLabelType, typename EdgeLabelType>
// class SetOfEdgeRefs : public Abstract::SetOfEdgeRefs {
// public:
//     using CIter = typename Abstract::SetOfEdgeRefs::const_iterator;
// };

template <typename StateLabelType, typename EdgeLabelType>
class SetOfStates : public Abstract::SetOfStates {
private:
    std::map<StateLabelType, State<StateLabelType, EdgeLabelType> *> labelIndex;

    void addToStateIndex(StateLabelType l, State<StateLabelType, EdgeLabelType> *s) {
        this->labelIndex[l] = s;
    }

public:
    State<StateLabelType, EdgeLabelType> &withLabel(StateLabelType l) {
        if (this->labelIndex.find(l) != this->labelIndex.end()) {
            return *this->labelIndex[l];
        }
        throw MaxPlus::MPException("error - state not found in FiniteStateMachine::_withLabel");
    }

    bool hasStateWithLabel(StateLabelType l) {
        return this->labelIndex.find(l) != this->labelIndex.end();
    }

    void addState(std::unique_ptr<State<StateLabelType, EdgeLabelType>> s) {
        Abstract::SetOfStates::addState(std::move(s));
        this->addToStateIndex(s->getLabel(), s.get());
    }
};

template <typename StateLabelType, typename EdgeLabelType>
class SetOfStateRefs : public Abstract::SetOfStateRefs {
public:
};

template <typename StateLabelType, typename EdgeLabelType> class State : public Abstract::State {
public:
    StateLabelType stateLabel;

    explicit State(const StateLabelType &withLabel) : Abstract::State(), stateLabel(withLabel) {}

    [[nodiscard]] const StateLabelType &getLabel() const { return this->stateLabel; }

    // return all next states reachable via an edge labelled l
    [[nodiscard]] std::unique_ptr<Abstract::SetOfStateRefs>
    nextStatesOfEdgeLabel(const EdgeLabelType l) const {

        auto result = std::make_unique<Abstract::SetOfStateRefs>();
        const auto &es = static_cast<const Abstract::SetOfEdgeRefs &>(this->getOutgoingEdges());
        for (const auto &i : es) {
            auto *e = static_cast<const Edge<StateLabelType, EdgeLabelType> *>(i);
            if (e->getLabel() == l) {
                result->insert(e->getDestination());
            }
        }
        return result;
    }

    // return an arbitrary next state reachable via an edge labelled l
    // or null if no such state exists
    State<StateLabelType, EdgeLabelType> *nextStateOfEdgeLabel(const EdgeLabelType l) {
        const auto &es = static_cast<const Abstract::SetOfEdgeRefs &>(this->getOutgoingEdges());
        for (const auto &i : es) {
            Edge<StateLabelType, EdgeLabelType> *e = i;
            if (e->getLabel() == l) {
                return e->getDestination();
            }
        }
        return nullptr;
    }
};

template <typename StateLabelType, typename EdgeLabelType>
class FiniteStateMachine : public Abstract::FiniteStateMachine {

private:
    SetOfStates<StateLabelType, EdgeLabelType> states;
    SetOfEdges<StateLabelType, EdgeLabelType> edges;
    // State<StateLabelType, EdgeLabelType> *initialState;
    SetOfStateRefs<StateLabelType, EdgeLabelType> initialStates;
    SetOfStateRefs<StateLabelType, EdgeLabelType> finalStates;

    State<StateLabelType, EdgeLabelType> &_getStateLabeled(const StateLabelType &s) {
        return this->states.withLabel(s);
    };

    State<StateLabelType, EdgeLabelType> &_getState(const State<StateLabelType, EdgeLabelType> &s) {
        return dynamic_cast<State<StateLabelType, EdgeLabelType> &>(this->states.withId(s.getId()));
    };

public:
    FiniteStateMachine() : Abstract::FiniteStateMachine() {};

    ~FiniteStateMachine() override = default;

    FiniteStateMachine(const FiniteStateMachine &) = default;
    FiniteStateMachine &operator=(const FiniteStateMachine &other) = default;
    FiniteStateMachine(FiniteStateMachine &&) noexcept = default;
    FiniteStateMachine &operator=(FiniteStateMachine &&) noexcept = default;

    std::unique_ptr<Abstract::FiniteStateMachine> newInstance() override {
        return std::make_unique<FiniteStateMachine<StateLabelType, EdgeLabelType>>();
    }

    // add state with the given label
    StateRef<StateLabelType, EdgeLabelType> addState(StateLabelType label) {
        bool added = false;
        auto sp = std::make_unique<State<StateLabelType, EdgeLabelType>>(label);
        auto &s = *sp;
        this->states.addState(std::move(sp));
        return &s;
    };

    Edge<StateLabelType, EdgeLabelType> *addEdge(const State<StateLabelType, EdgeLabelType> &src,
                                                 EdgeLabelType lbl,
                                                 const State<StateLabelType, EdgeLabelType> &dst) {
        // lookup state again to drop const qualifier
        auto &mySrc = dynamic_cast<State<StateLabelType, EdgeLabelType> &>(
                this->states.withId(src.getId()));
        auto &myDst = dynamic_cast<State<StateLabelType, EdgeLabelType> &>(
                this->states.withId(dst.getId()));
        bool added = false;
        auto ep = std::make_unique<Edge<StateLabelType, EdgeLabelType>>(mySrc, lbl, myDst);
        auto &e = *ep;
        this->edges[e.getId()] = std::move(ep);
        mySrc.addOutGoingEdge(e);
        return &e;
    };

    void removeEdge(const Edge<StateLabelType, EdgeLabelType> &e) {
        auto csrc = dynamic_cast<StateRef<StateLabelType, EdgeLabelType>>(e.getSource());
        // get a non-const version of the state
        auto &src = this->_getState(*csrc);
        src.removeOutgoingEdge(&e);
        this->edges.remove(e);
    }

    void removeState(const State<StateLabelType, EdgeLabelType> &s) {
        // remove related edges
        Abstract::SetOfEdgeRefs edgesToRemove;
        for (auto &it : this->edges) {
            auto &e = *(it.second);
            auto src = dynamic_cast<StateRef<StateLabelType, EdgeLabelType>>(e.getSource());
            auto dst = dynamic_cast<StateRef<StateLabelType, EdgeLabelType>>(e.getDestination());
            if ((src == &s) || (dst == &s)) {
                edgesToRemove.insert(&e);
            }
        }
        for (const auto *e : edgesToRemove) {
            this->removeEdge(dynamic_cast<const Edge<StateLabelType, EdgeLabelType> &>(*e));
        }
        this->states.remove(s);
    }

    // set initial state to state with label;
    void setInitialState(StateLabelType label) {
        this->setInitialState(this->states.withLabel(label));
    };

    void setInitialState(const State<StateLabelType, EdgeLabelType> &s) {
        // we are assuming s is one of our states
        this->initialStates.clear();
        this->addInitialState(s);
    };

    void addInitialState(const State<StateLabelType, EdgeLabelType> &s) {
        // we are assuming s is one of our states
        this->initialStates.insert(&s);
    };

    void addFinalState(const State<StateLabelType, EdgeLabelType> &s) {
        // we are assuming s is one of our states
        this->finalStates.insert(&s);
    };

    [[nodiscard]] StateRef<StateLabelType, EdgeLabelType> getInitialState() const override {
        if (this->initialStates.empty()) {
            throw MaxPlus::MPException("FSM has no initial state.");
        }
        auto s = this->initialStates.begin();
        return dynamic_cast<StateRef<StateLabelType, EdgeLabelType>>(*s);
    };

    [[nodiscard]] const ::MaxPlus::FSM::Abstract::SetOfStateRefs &
    getInitialStates() const override {
        return this->initialStates;
    };

    [[nodiscard]] const ::MaxPlus::FSM::Abstract::SetOfStateRefs &getFinalStates() const override {
        return this->finalStates;
    };

    StateRef<StateLabelType, EdgeLabelType> getStateLabeled(const StateLabelType &s) {
        return &(this->_getStateLabeled(s));
    };

    bool hasStateLabeled(const StateLabelType &s) {
        // try the index first
        State<StateLabelType, EdgeLabelType> *sp = this->states.withLabel(s);
        if (sp != nullptr) {
            return true;
        }
        // a linear search
        for (auto &it : this->states) {
            auto &i = *(it.second);
            auto &t = dynamic_cast<const State<StateLabelType, EdgeLabelType> &>(i);
            if ((t.stateLabel) == s) {
                this->states.addToStateIndex(s, &t);
                return true;
            }
        }
        return false;
    };

    [[nodiscard]] const SetOfStates<StateLabelType, EdgeLabelType> &getStates() const override {
        return this->states;
    };

    [[nodiscard]] ::MaxPlus::FSM::Abstract::SetOfStateRefs getStateRefs() const override {
        ::MaxPlus::FSM::Abstract::SetOfStateRefs result;
        for (const auto &s : this->states) {
            result.insert(s.second->getReference());
        }
        return result;
    };

    [[nodiscard]] const SetOfEdges<StateLabelType, EdgeLabelType> &getEdges() const override {
        return this->edges;
    };

    Abstract::SetOfStateRefs getStateRefs() {
        Abstract::SetOfStateRefs result;
        for (auto i : this->states) {
            result.insert(&(*(i.second)));
        }
        return result;
    };

    std::unique_ptr<Abstract::SetOfEdgeRefs> getEdgeRefs() {
        auto result = std::make_unique<Abstract::SetOfEdgeRefs>();
        for (auto i : this->edges) {
            result->insert(&(*(i.second)));
        }
        return result;
    };

    EdgeRef<StateLabelType, EdgeLabelType>
    getEdge(const State<StateLabelType, EdgeLabelType> &source,
            const State<StateLabelType, EdgeLabelType> &target) {
        const auto &e = static_cast<const Abstract::SetOfEdgeRefs &>(source.getOutgoingEdges());

        // collect all labels in edges of s
        for (const auto &i : e) {
            auto edge = dynamic_cast<const Edge<StateLabelType, EdgeLabelType> *>(i);
            if (&target == edge->getDestination()) {
                return edge;
            }
        }
        return nullptr;
    };

    void setEdgeLabel(const EdgeRef<StateLabelType, EdgeLabelType> &e, const EdgeLabelType &l) {
        auto ee = std::dynamic_pointer_cast<Edge<StateLabelType, EdgeLabelType>>(
                (*this->edges.find(e->getId())).second);
        ee->setLabel(l);
    }

    StateRef<StateLabelType, EdgeLabelType> checkStateLabeled(const StateLabelType &l) {
        if (this->states.hasStateWithLabel(l)) {
            return &(this->states.withLabel(l));
        }
        return nullptr;
    };

    std::unique_ptr<Abstract::SetOfStateRefs> reachableStates() {
        // use generic DFS function to add states found
        Abstract::ReachableStates rs(*this);
        rs.DoDepthFirstSearch();
        std::unique_ptr<Abstract::SetOfStateRefs> result =
                std::make_unique<Abstract::SetOfStateRefs>(rs.result);
        return result;
    };

    // check if there exists a transition e = (q1,alpha,q2)
    const Edge<StateLabelType, EdgeLabelType> *
    findEdge(StateLabelType src, EdgeLabelType lbl, StateLabelType dst) {

        // get all labels
        const SetOfStates<StateLabelType, EdgeLabelType> &allStates = this->getStates();

        for (auto iter : allStates) {
            auto s = dynamic_cast<State<StateLabelType, EdgeLabelType> &>(*(iter.second));
            if (s.stateLabel == src) {
                const StateRef<StateLabelType, EdgeLabelType> srcState = this->getStateLabeled(src);

                const auto &outgoingEdges =
                        static_cast<const Abstract::SetOfEdgeRefs &>(srcState->getOutgoingEdges());

                for (const auto &it : outgoingEdges) {
                    auto e = dynamic_cast<const Edge<StateLabelType, EdgeLabelType> *>(it);
                    auto dstState = dynamic_cast<StateRef<StateLabelType, EdgeLabelType>>(
                            e->getDestination());
                    if (e->getLabel() == lbl && dstState->getLabel() == dst) {
                        return e;
                    }
                }
            }
        }
        return nullptr;
    };

    // determinize the automaton based on edge labels only.
    // warning: original state labels are ignored
    // Using the subset construction
    std::unique_ptr<FiniteStateMachine<StateLabelType, EdgeLabelType>> determinizeEdgeLabels() {
        std::unique_ptr<FiniteStateMachine<StateLabelType, EdgeLabelType>> result =
                std::unique_ptr<FiniteStateMachine<StateLabelType, EdgeLabelType>>(
                        this->newInstance());

        // maintain map of sets of states to the corresponding new states.
        std::map<const Abstract::SetOfStateRefs, const State<StateLabelType, EdgeLabelType> *>
                newStatesMap;

        // queue of states that need to be further explored.
        std::list<std::unique_ptr<Abstract::SetOfStateRefs>> unprocessed;

        // create initial state
        std::unique_ptr<Abstract::SetOfStateRefs> initialStateSet;
        initialStateSet = std::make_unique<Abstract::SetOfStateRefs>();
        initialStateSet->insert(this->getInitialState());

        StateLabelType si = this->getInitialState()->getLabel();
        const State<StateLabelType, EdgeLabelType> *initialState = result->addState(si);
        newStatesMap[*initialStateSet] = initialState;
        result->setInitialState(si);

        // add initial state to list of unprocessed state sets
        unprocessed.push_back(initialStateSet);

        while (!unprocessed.empty()) {
            Abstract::SetOfStateRefs *Q = (unprocessed.begin())->get();
            unprocessed.erase(unprocessed.begin());

            // get all outgoing labels
            std::set<EdgeLabelType> labels;
            for (const auto *i : *Q) {
                auto s = dynamic_cast<const State<StateLabelType, EdgeLabelType> &>(*i);
                this->insertOutgoingLabels(&s, labels);
            }

            // for each label in labels get the image states into a set QNext
            for (auto l : labels) {

                // collect image state in QNext
                std::unique_ptr<Abstract::SetOfStateRefs> QNext =
                        std::make_unique<Abstract::SetOfStateRefs>();

                // for every state s in Q
                for (const auto *i : *Q) {
                    const auto &s = dynamic_cast<const State<StateLabelType, EdgeLabelType> &>(*i);
                    std::unique_ptr<Abstract::SetOfStateRefs> l_img = s.nextStatesOfEdgeLabel(l);

                    // add all l-images from s to QNext
                    for (const auto &k : *l_img) {
                        const auto *simg = k;
                        QNext->insert(simg);
                    }
                }

                // add new state in fsm if necessary
                const State<StateLabelType, EdgeLabelType> *ns = nullptr;
                if (newStatesMap.find(*QNext) == newStatesMap.end()) {
                    // state does not yet exist, make new state
                    ns = result->addState(
                            (dynamic_cast<const State<StateLabelType, EdgeLabelType> &>(
                                     *(*(QNext->begin()))))
                                    .getLabel());

                    newStatesMap[*QNext] = ns;
                    unprocessed.push_back(std::move(QNext));
                } else {
                    // state already exists, fetch from newStatesMap
                    ns = newStatesMap[*QNext];
                }

                // add an edge in the new fsm
                result->addEdge(*(newStatesMap[*Q]), l, *ns);
            }
        }

        // set initial state
        return result;
    };

    using EquivalenceMap = std::map<const Abstract::State *, Abstract::SetOfStateRefs *>;

    // minimize the automaton based on edge and state labels.
    std::unique_ptr<FiniteStateMachine<StateLabelType, EdgeLabelType>>
    minimizeEdgeLabels(bool ignoreStateLabels = false) { // NOLINT(*cognitive-complexity)
        // partition refinement algorithm

        // generate a vector of equivalence classes
        std::list<std::unique_ptr<Abstract::SetOfStateRefs>> eqClasses;

        // populate it with a set of all states
        std::unique_ptr<Abstract::SetOfStateRefs> initialClass =
                std::make_unique<Abstract::SetOfStateRefs>(this->getStateRefs());
        auto *initialClassR = initialClass.get();
        eqClasses.push_back(std::move(initialClass));

        // initially map all state to the initial class
        EquivalenceMap eqMap;
        for (const auto *si : *initialClassR) {
            auto sp = dynamic_cast<const State<StateLabelType, EdgeLabelType> *>(si);
            eqMap[sp] = initialClassR;
        }

        // partition on state labels

        bool changed = false;
        if (!ignoreStateLabels) {
            do { // NOLINT(*avoid-do-while)
                changed = false;

                std::list<std::unique_ptr<Abstract::SetOfStateRefs>> newEqClasses;

                // for every potential equivalence class
                for (const auto &ic : eqClasses) {
                    Abstract::SetOfStateRefs *_class = ic.get();

                    auto i = _class->begin();

                    // pick arbitrary state from class
                    auto s1 = dynamic_cast<const State<StateLabelType, EdgeLabelType> *>(*i);

                    std::unique_ptr<Abstract::SetOfStateRefs> equivSet =
                            std::make_unique<Abstract::SetOfStateRefs>();
                    std::unique_ptr<Abstract::SetOfStateRefs> remainingSet =
                            std::make_unique<Abstract::SetOfStateRefs>();
                    equivSet->insert(s1);

                    // check whether all other states have the same label.
                    while (++i != _class->end()) {
                        auto s2 = dynamic_cast<const State<StateLabelType, EdgeLabelType> *>(*i);
                        if (s1->getLabel() == s2->getLabel()) {
                            equivSet->insert(s2);
                        } else {
                            remainingSet->insert(s2);
                        }
                    }
                    // if not, split the class
                    auto *equivSetR = equivSet.get();
                    auto *remainingSetR = remainingSet.get();
                    if (equivSet->size() == _class->size()) {
                        newEqClasses.push_back(std::move(equivSet));
                        this->mapStates(eqMap, equivSetR);
                    } else {
                        newEqClasses.push_back(std::move(equivSet));
                        this->mapStates(eqMap, equivSetR);
                        newEqClasses.push_back(std::move(remainingSet));
                        this->mapStates(eqMap, remainingSetR);
                        changed = true;
                    }
                }
                auto tempEqClasses = eqClasses;
                eqClasses = newEqClasses;
            } while (changed);
        }

        // partition refinement on transitions
        changed = false;
        do { // NOLINT(*avoid-do-while)
            changed = false;

            std::list<std::unique_ptr<Abstract::SetOfStateRefs>> newEqClasses;

            // for every potential equivalence class
            for (const auto &ic : eqClasses) {
                Abstract::SetOfStateRefs *_class = ic.get();

                auto i = _class->begin();

                // pick arbitrary state from class
                auto s1 = dynamic_cast<const State<StateLabelType, EdgeLabelType> *>(*i);

                std::unique_ptr<Abstract::SetOfStateRefs> equivSet =
                        std::make_unique<Abstract::SetOfStateRefs>();
                std::unique_ptr<Abstract::SetOfStateRefs> remainingSet =
                        std::make_unique<Abstract::SetOfStateRefs>();
                equivSet->insert(s1);

                // check whether all other states can go with the same label to
                // the same set of other equivalence classes.
                while (++i != _class->end()) {
                    auto s2 = dynamic_cast<const State<StateLabelType, EdgeLabelType> *>(*i);
                    if (this->edgesEquivalent(eqMap, s1, s2)) {
                        equivSet->insert(s2);
                    } else {
                        remainingSet->insert(s2);
                    }
                }
                // if not, split the class
                auto *equivSetR = equivSet.get();
                auto *remainingSetR = remainingSet.get();
                if (equivSet->size() == _class->size()) {
                    newEqClasses.push_back(std::move(equivSet));
                    this->mapStates(eqMap, equivSetR);
                } else {
                    newEqClasses.push_back(std::move(equivSet));
                    this->mapStates(eqMap, equivSetR);
                    newEqClasses.push_back(std::move(remainingSet));
                    this->mapStates(eqMap, remainingSetR);
                    changed = true;
                }
            }
            auto tempEqClasses = eqClasses;
            eqClasses = newEqClasses;
        } while (changed);

        std::unique_ptr<FiniteStateMachine<StateLabelType, EdgeLabelType>> result =
                this->newInstance();

        // make a state for every equivalence class
        std::map<Abstract::SetOfStateRefs *, StateRef<StateLabelType, EdgeLabelType>> newStateMap;
        CId sid = 0;
        for (const auto &cli : eqClasses) {
            // take state label from arbitrary state from the class
            const State<StateLabelType, EdgeLabelType> *s =
                    dynamic_cast<const State<StateLabelType, EdgeLabelType> *>(*(cli->begin()));
            auto ns = result->addState(s->stateLabel);
            newStateMap[cli] = ns;
            sid++;
        }

        // make the appropriate edges
        for (const auto &cli : eqClasses) {
            // take a representative state
            const auto *s = *(cli->begin());
            const auto &es = s->getOutgoingEdges();
            // for every outgoing edge
            for (const auto *edi : es) {
                auto ed = dynamic_cast<EdgeRef<StateLabelType, EdgeLabelType>>(edi);
                result->addEdge(*(newStateMap[cli]),
                                ed->getLabel(),
                                *(newStateMap[eqMap[ed->getDestination()]]));
            }
        }

        // set initial state
        result->setInitialState(*(newStateMap[eqMap[this->getInitialState()]]));

        return result;
    }

    [[nodiscard]] bool hasDirectedCycle() const {
        ::MaxPlus::FSM::Abstract::DetectCycle DC(*this);
        return DC.checkForCycles(nullptr);
    }

private:
    void insertOutgoingLabels(const State<StateLabelType, EdgeLabelType> *s,
                              std::set<EdgeLabelType> &labels) {
        const Abstract::SetOfEdgeRefs &e = s->getOutgoingEdges();

        // collect all labels in edges of s
        for (const auto *i : e) {
            auto *ed = dynamic_cast<EdgeRef<StateLabelType, EdgeLabelType>>(i);
            labels.insert(ed->getLabel());
        }
    };

    // function only used by minimizeEdgeLabels
    bool edgesEquivalent(EquivalenceMap &m,
                         const State<StateLabelType, EdgeLabelType> *s1,
                         const State<StateLabelType, EdgeLabelType> *s2) {
        // s1 and s2 are equivalent if for every s1-a->C, s2-a->C
        // and vice versa
        std::set<EdgeLabelType> labels;
        this->insertOutgoingLabels(s1, labels);
        this->insertOutgoingLabels(s2, labels);

        // for every label, compare outgoing edges
        typename std::set<EdgeLabelType>::const_iterator k;
        for (k = labels.begin(); k != labels.end(); k++) {
            EdgeLabelType l = *k;
            std::unique_ptr<Abstract::SetOfStateRefs> ns1 = s1->nextStatesOfEdgeLabel(l);
            std::unique_ptr<Abstract::SetOfStateRefs> ns2 = s2->nextStatesOfEdgeLabel(l);
            // collect classes of states in ns1 and ns2
            std::set<Abstract::SetOfStateRefs *> cs1;
            std::set<Abstract::SetOfStateRefs *> cs2;
            for (const auto *j : *ns1) {
                const auto *s = j;
                cs1.insert(m[s]);
            }
            for (const auto *j : *ns2) {
                const auto *s = j;
                cs2.insert(m[s]);
            }

            // compare classes
            if (cs1 != cs2) {
                return false;
            }
        }
        return true;
    }

    // function only used by minimizeEdgeLabels
    void mapStates(EquivalenceMap &m, Abstract::SetOfStateRefs *sos) {
        for (const auto *i : *sos) {
            m[i] = sos;
        }
    }
};
} // namespace Labeled

namespace StateStringLabeled {

// make an FSM class with unlabeled edges, based on the labeled one with some dummy char labels
//

class State : public Labeled::State<MaxPlus::MPString, char> {
public:
    explicit State(const MaxPlus::MPString &withLabel) :
        Labeled::State<MaxPlus::MPString, char>(withLabel) {}

    const Abstract::SetOfEdgeRefs &getOutgoingEdges() {
        return dynamic_cast<const Abstract::SetOfEdgeRefs &>(
                Labeled::State<MaxPlus::MPString, char>::getOutgoingEdges());
    }
};

class FiniteStateMachine : public Labeled::FiniteStateMachine<MaxPlus::MPString, char> {
public:
    [[nodiscard]] Labeled::StateRef<MaxPlus::MPString, char> getInitialState() const override {
        return dynamic_cast<Labeled::StateRef<MaxPlus::MPString, char>>(
                Labeled::FiniteStateMachine<MaxPlus::MPString, char>::getInitialState());
    };

    void setInitialStateLabeled(const MaxPlus::MPString &sl);

    void addStateLabeled(const MaxPlus::MPString &sl);

    void addEdge(State *src, State *dst);

    void addEdgeLabeled(const MaxPlus::MPString &src, const MaxPlus::MPString &dst);

    std::unique_ptr<Abstract::SetOfStateRefs> reachableStates();
};

class Edge : public Labeled::Edge<MaxPlus::MPString, char> {};
} // namespace StateStringLabeled

namespace Product {

class FiniteStateMachine;

class State : public Abstract::State {
public:
    State(Abstract::State &s1, Abstract::State &s2, const FiniteStateMachine &ownerFsm) :
        sa(&s1), sb(&s2), fsm(&ownerFsm) {}

    virtual const Abstract::SetOfEdges &getOutgoingEdges();

private:
    Abstract::State *sa;
    Abstract::State *sb;
    const FiniteStateMachine *fsm;
    bool outgoingEdgesDone{};
};

class FiniteStateMachine : public Abstract::FiniteStateMachine {
public:
    FiniteStateMachine(const Abstract::FiniteStateMachine &fsm1,
                       const Abstract::FiniteStateMachine &fsm2) :
        fsm_a(&fsm1), fsm_b(&fsm2) {}

    [[nodiscard]] Abstract::StateRef getInitialState() const override;

    [[nodiscard]] virtual bool matchEdges(const Abstract::Edge &e1,
                                          const Abstract::Edge &e2) const = 0;

    [[nodiscard]] virtual Abstract::Edge *ensureEdge(const Abstract::Edge &e1,
                                                     const Abstract::Edge &e2) const = 0;

private:
    const Abstract::FiniteStateMachine* fsm_a;
    const Abstract::FiniteStateMachine* fsm_b;
};

} // namespace Product

} // namespace MaxPlus::FSM

#endif