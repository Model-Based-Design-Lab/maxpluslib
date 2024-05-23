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

#ifndef MAXPLUS_BASE_FSM_H
#define MAXPLUS_BASE_FSM_H

#include "../exception/exception.h"
#include "../string/cstring.h"
#include <algorithm>
#include <list>
#include <memory>
#include <set>

namespace FSM {

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

// Edge of an FSM
class Edge : public WithUniqueID {
public:
    // constructor
    Edge(State &src, State &dst) : source(src), destination(dst) {}

    Edge(const Edge &) = default;
    Edge &operator=(const Edge &other) = delete;
    Edge(Edge &&) = default;
    Edge &operator=(Edge &&) = delete;

    // destructor
    virtual ~Edge() = default;
    ;

    // virtual access methods
    [[nodiscard]] virtual const State &getSource() const { return this->source; }
    [[nodiscard]] virtual const State &getDestination() const { return this->destination; }

private:
    // source and destination state of the edge
    State &source;
    State &destination;
};

// // Edge ordering for sets
// struct EdgeCompareLessThan {
//     bool operator()(const Edge &lhs, const Edge &rhs) const { return lhs.lessThan(rhs); };
// };

// A set of edges
// the set is assumed to have unique ownership of the edges
// TODO: wanted to make it a unique pointer, but  compiler complains we are a deleted copy
// constructor...
class SetOfEdges : public std::map<CId, std::shared_ptr<Edge>> {
public:
    void remove(const Edge &e) { this->erase(e.getId()); }
};

struct EdgeRefCompareLessThan {
    bool operator()(const Edge *lhs, const Edge *rhs) const { return lhs->lessThan(*rhs); };
};

// A set of references to edges
class SetOfEdgeRefs : public std::set<Edge *, EdgeRefCompareLessThan> {
public:
    using CIter = SetOfEdgeRefs::const_iterator;
    virtual ~SetOfEdgeRefs() = default;
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
    State(State &&)  noexcept = default;
    State &operator=(State &&) = delete;

    // add an outgoing edge to the state
    void addOutGoingEdge(Edge &e) {
        assert(&(e.getSource()) == this);
        this->outgoingEdges.insert(&e);
    }

    // access the outgoing edges
    [[nodiscard]] virtual const SetOfEdgeRefs &getOutgoingEdges() const {
        return this->outgoingEdges;
    }

    // TODO (Marc Geilen) get rid of const cast
    void removeOutgoingEdge(const Edge &e) {
        this->outgoingEdges.erase(const_cast<Edge *>(&e)); }
    void insertOutgoingEdge(Edge &e) { this->outgoingEdges.insert(&e); }

private:
    SetOfEdgeRefs outgoingEdges;
};

// A set of states
// the set is assumed to have unique ownership of the states
class SetOfStates : public std::map<CId, std::shared_ptr<State>> {
public:
    using CIter = SetOfStates::const_iterator;
    using Iter = SetOfStates::iterator;
    void remove(const State &s) { this->erase(s.getId()); }
    virtual ~SetOfStates() = default;
};

struct StateRefCompareLessThan {
    bool operator()(const State *lhs, const State *rhs) const { return lhs->lessThan(*rhs); };
};

// A set of references to states
class SetOfStateRefs : public std::set<const State *, StateRefCompareLessThan> {
public:
    using CIter = SetOfEdgeRefs::const_iterator;
    bool includesState(const State *s) { return this->find(s) != this->end(); }
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
    virtual std::shared_ptr<FiniteStateMachine> newInstance() = 0;

    [[nodiscard]] virtual State &getInitialState() const = 0;
    [[nodiscard]] virtual const SetOfStates &getInitialStates() const = 0;
    [[nodiscard]] virtual const SetOfStates &getFinalStates() const = 0;
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
        explicit DFSStackItem(const State &s) : state(s) {
            this->iter = s.getOutgoingEdges().begin();
        };

        // access state
        inline const State &getState() { return this->state; }

        inline SetOfEdgeRefs::CIter getIter() { return this->iter; }

        // test if all outgoing edges have been done
        bool atEnd() { return this->iter == this->state.getOutgoingEdges().end(); }

        // move to the next edge
        void advance() { (this->iter)++; }

    private:
        const State &state;
        SetOfEdgeRefs::CIter iter;
    };

private:
    using DfsStack = std::list<DFSStackItem>;
    DfsStack dfsStack;

public:
    virtual ~DepthFirstSearch() = default;
    using DFSStackCIter = DfsStack::const_iterator;

    virtual void onEnterState(const State &s){};

    virtual void onLeaveState(const State &s){};

    virtual void onTransition(const Edge &e){};

    virtual void onSimpleCycle(DfsStack &stack){};

    explicit DepthFirstSearch(FiniteStateMachine &targetFsm) : fsm(targetFsm){};

    // Execute the depth first search
    void DoDepthFirstSearch(bool fullDFS = false) {
        // store visited states
        SetOfStateRefs visitedStates;

        // put initial state on the stack
        dfsStack.emplace_back(this->fsm.getInitialState());

        while (!(dfsStack.empty())) {
            DFSStackItem &si = dfsStack.back();

            // current item complete?
            if (si.atEnd()) {
                // pop it from stack
                this->onLeaveState(si.getState());
                if (fullDFS) {
                    const auto s = si.getState();
                    assert(visitedStates.includesState(&s));
                    visitedStates.erase(&s);
                }
                dfsStack.pop_back();
            } else {
                // goto next edge
                auto *e = *(si.getIter());
                si.advance();
                bool revisit = visitedStates.includesState(&(e->getDestination()));
                if (revisit) {
                    // if target state not visited before
                    dfsStack.emplace_back(e->getDestination());
                    this->onTransition(*e);
                    this->onEnterState(e->getDestination());
                    visitedStates.insert(&(e->getDestination()));
                } else {
                    // cycle found
                    this->onSimpleCycle(dfsStack);
                }
            }
        }
    }

private:
    FiniteStateMachine &fsm;
};

// Reachable states strategy based on DFS
class ReachableStates : public DepthFirstSearch {
public:
    SetOfStateRefs result;

    explicit ReachableStates(FiniteStateMachine &targetFsm) : DepthFirstSearch(targetFsm){};

    ~ReachableStates() override = default;

    ReachableStates(const ReachableStates &) = delete;
    ReachableStates &operator=(const ReachableStates &other) = delete;
    ReachableStates(ReachableStates &&) = delete;
    ReachableStates &operator=(ReachableStates &&) = delete;

    void onEnterState(const State &s) override { this->result.insert(&s); };
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
class SetOfEdges : public Abstract::SetOfEdges {
public:
    using CIter = typename Abstract::SetOfEdges::const_iterator;
};

// template <typename StateLabelType, typename EdgeLabelType>
// class SetOfEdgeRefs : public Abstract::SetOfEdgeRefs {
// public:
//     using CIter = typename Abstract::SetOfEdgeRefs::const_iterator;
// };

template <typename StateLabelType, typename EdgeLabelType>
class SetOfStates : public Abstract::SetOfStates {
private:
    std::map<StateLabelType, State<StateLabelType, EdgeLabelType> *> stateIndex;

public:
    using CIter = typename SetOfStates::const_iterator;
    State<StateLabelType, EdgeLabelType> *withLabel(StateLabelType l) {
        if (this->stateIndex.find(l) != this->stateIndex.end()) {
            return this->stateIndex[l];
        }
        return nullptr;
    }

    void addToStateIndex(StateLabelType l, State<StateLabelType, EdgeLabelType> *s) {
        this->stateIndex[l] = s;
    }
};

template <typename StateLabelType, typename EdgeLabelType>
class SetOfStateRefs : public Abstract::SetOfStateRefs {
public:
    using CIter = typename SetOfStateRefs::const_iterator;
};

template <typename StateLabelType, typename EdgeLabelType> class State : public Abstract::State {
public:
    StateLabelType stateLabel;

    explicit State(const StateLabelType &withLabel) : Abstract::State() {
        this->stateLabel = withLabel;
    }

    [[nodiscard]] const StateLabelType &getLabel() const { return this->stateLabel; }

    // return all next states reachable via an edge labelled l
    [[nodiscard]] std::shared_ptr<Abstract::SetOfStateRefs>
    nextStatesOfEdgeLabel(const EdgeLabelType l) const {

        auto result = std::make_shared<Abstract::SetOfStateRefs>();
        auto es = static_cast<const Abstract::SetOfEdgeRefs &>(this->getOutgoingEdges());
        for (const auto &i : es) {
            auto *e = static_cast<Edge<StateLabelType, EdgeLabelType> *>(i);
            if (e->getLabel() == l) {
                result->insert(&(e->getDestination()));
            }
        }
        return result;
    }

    // return an arbitrary next state reachable via an edge labelled l
    // or null if no such state exists
    State<StateLabelType, EdgeLabelType> *nextStateOfEdgeLabel(const EdgeLabelType l) {
        auto es = static_cast<const Abstract::SetOfEdgeRefs &>(this->getOutgoingEdges());
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
    Abstract::SetOfEdges edges;
    // State<StateLabelType, EdgeLabelType> *initialState;
    SetOfStates<StateLabelType, EdgeLabelType> initialStates;
    SetOfStates<StateLabelType, EdgeLabelType> finalStates;

public:
    FiniteStateMachine() : Abstract::FiniteStateMachine() {};

    ~FiniteStateMachine() override = default;

    FiniteStateMachine(const FiniteStateMachine &) = default;
    FiniteStateMachine &operator=(const FiniteStateMachine &other) = default;
    FiniteStateMachine(FiniteStateMachine &&) noexcept = default;
    FiniteStateMachine &operator=(FiniteStateMachine &&) noexcept = default;

    std::shared_ptr<Abstract::FiniteStateMachine> newInstance() override {
        return std::make_shared<FiniteStateMachine<StateLabelType, EdgeLabelType>>();
    }

    // add state with the given label
    State<StateLabelType, EdgeLabelType> *addState(StateLabelType label) {
        bool added = false;
        typename SetOfStates<StateLabelType, EdgeLabelType>::CIter i;
        auto sp = std::make_shared<State<StateLabelType, EdgeLabelType>>(label);
        auto &s = *sp;
        this->states[sp->getId()] = std::move(sp);
        return &s;
    };

    Edge<StateLabelType, EdgeLabelType> *addEdge(const State<StateLabelType, EdgeLabelType> &src,
                                                 EdgeLabelType lbl,
                                                 const State<StateLabelType, EdgeLabelType> &dst) {
        // lookup state again to drop const qualifier
        auto &mySrc =
                dynamic_cast<State<StateLabelType, EdgeLabelType> &>(*(this->states[src.getId()]));
        auto &myDst =
                dynamic_cast<State<StateLabelType, EdgeLabelType> &>(*(this->states[dst.getId()]));
        bool added = false;
        auto ep = std::make_shared<Edge<StateLabelType, EdgeLabelType>>(mySrc, lbl, myDst);
        auto &e = *ep;
        this->edges[e.getId()] = std::move(ep);
        mySrc.addOutGoingEdge(e);
        return &e;
    };

    void removeEdge(const Edge<StateLabelType, EdgeLabelType> &e) {
        auto csrc = dynamic_cast<const State<StateLabelType, EdgeLabelType> &>(e.getSource());
        // get a non-const version of the state
        auto src = this->getStateLabeled(csrc.getLabel());
        src.removeOutgoingEdge(e);
        this->edges.remove(e);
    }

    void removeState(const State<StateLabelType, EdgeLabelType> &s) {
        // remove related edges
        Abstract::SetOfEdgeRefs edgesToRemove;
        for (auto &it : this->edges) {
            auto &e = *(it.second);
            auto src = dynamic_cast<const State<StateLabelType, EdgeLabelType> &>(e.getSource());
            auto dst =
                    dynamic_cast<const State<StateLabelType, EdgeLabelType> &>(e.getDestination());
            if ((src == s) || (dst == s)) {
                edgesToRemove.insert(&e);
            }
        }
        for (auto *e : edgesToRemove) {
            this->removeEdge(dynamic_cast<const Edge<StateLabelType, EdgeLabelType> &>(*e));
        }
        this->states.remove(s);
    }

    // set initial state to state with label;
    void setInitialState(StateLabelType label) {
        this->setInitialState(*(this->states.withLabel(label)));
    };

    void setInitialState(State<StateLabelType, EdgeLabelType> &s) {
        // we are assuming s is one of our states
        this->initialStates.clear();
        this->addInitialState(s);
    };

    void addInitialState(State<StateLabelType, EdgeLabelType> &s) {
        // we are assuming s is one of our states
        this->initialStates.emplace(s.getId(), &s);
    };

    void addFinalState(State<StateLabelType, EdgeLabelType> &s) {
        // we are assuming s is one of our states
        this->finalStates.emplace(s.getId(), &s);
    };


    [[nodiscard]] State<StateLabelType, EdgeLabelType>& getInitialState() const override {
        if (this->initialStates.empty()) {
            throw CException("FSM has no initial state.");
        }
        auto xx = (this->initialStates.begin())->second;
        return dynamic_cast<State<StateLabelType, EdgeLabelType>&>(*xx);
    };

    [[nodiscard]] const ::FSM::Abstract::SetOfStates &getInitialStates() const override {
        return this->initialStates;
    };

    [[nodiscard]] const ::FSM::Abstract::SetOfStates &getFinalStates() const override {
        return this->finalStates;
    };

    State<StateLabelType, EdgeLabelType> &getStateLabeled(const StateLabelType &s) {
        // try the index first
        State<StateLabelType, EdgeLabelType> *sp = this->states.withLabel(s);
        if (sp != nullptr) {
            return *sp;
        }
        // for now just a linear search
        // TODO: use index?
        for (auto &it : this->states) {
            auto &i = *(it.second);
            auto &t = dynamic_cast<const State<StateLabelType, EdgeLabelType> &>(i);
            // TODO: remove const_cast set of states provides only const iterator, but states are
            // identified only by ID.
            auto &ct = const_cast<State<StateLabelType, EdgeLabelType> &>(t);
            if ((ct.stateLabel) == s) {
                // TODO: manage index inside SetOfStates
                this->states.addToStateIndex(s, &ct);
                return ct;
            }
        }
        throw CException("error - state not found in FiniteStateMachine::getStateLabeled");
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
            // TODO: remove const_cast set of states provides only const iterator, but states are
            // identified only by ID.
            auto &ct = const_cast<State<StateLabelType, EdgeLabelType> &>(t);
            if ((ct.stateLabel) == s) {
                // TODO: manage index inside SetOfStates
                this->states.addToStateIndex(s, &ct);
                return true;
            }
        }
        return false;
    };

    [[nodiscard]] const SetOfStates<StateLabelType, EdgeLabelType> &getStates() const { return this->states; };
    Abstract::SetOfStateRefs getStateRefs() {
        Abstract::SetOfStateRefs result;
        for (auto i : this->states) {
            result.insert(&(*(i.second)));
        }
        return result;
    };

    Abstract::SetOfEdges &getEdges() { return this->edges; };

    Edge<StateLabelType, EdgeLabelType> *
    getEdge(const State<StateLabelType, EdgeLabelType> &source,
            const State<StateLabelType, EdgeLabelType> &target) {
        auto e = static_cast<const Abstract::SetOfEdgeRefs &>(source.getOutgoingEdges());

        // collect all labels in edges of s
        for (const auto &i : e) {
            auto edge = dynamic_cast<Edge<StateLabelType, EdgeLabelType> *>(i);
            if (target == edge->getDestination()) {
                return edge;
            }
        }
        return nullptr;
    };

    State<StateLabelType, EdgeLabelType> *checkStateLabeled(const StateLabelType &s) {
        // try the index first
        if (this->states.withLabel(s) != nullptr) {
            return this->states.withLabel(s);
        }
        // for now just a linear search
        auto i = this->states.begin();
        while (i != this->states.end()) {
            auto &t = dynamic_cast<State<StateLabelType, EdgeLabelType> &>(*((*i).second));
            if ((t.stateLabel) == s) {
                this->states.addToStateIndex(s, &t);
                return &t;
            }
            i++;
        }
        return nullptr;
    };

    std::shared_ptr<Abstract::SetOfStateRefs> reachableStates() {
        // use generic DFS function to add states found
        Abstract::ReachableStates rs(*this);
        rs.DoDepthFirstSearch();
        std::shared_ptr<Abstract::SetOfStateRefs> result =
                std::make_shared<Abstract::SetOfStateRefs>(rs.result);
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
                State<StateLabelType, EdgeLabelType> &srcState = this->getStateLabeled(src);

                const auto &outgoingEdges =
                        static_cast<const Abstract::SetOfEdgeRefs &>(srcState.getOutgoingEdges());

                for (const auto &it : outgoingEdges) {
                    auto e = dynamic_cast<const Edge<StateLabelType, EdgeLabelType> *>(it);
                    auto dstState = dynamic_cast<const State<StateLabelType, EdgeLabelType> &>(
                            e->getDestination());
                    if (e->getLabel() == lbl && dstState.getLabel() == dst) {
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
    std::shared_ptr<FiniteStateMachine<StateLabelType, EdgeLabelType>> determinizeEdgeLabels() {
        std::shared_ptr<FiniteStateMachine<StateLabelType, EdgeLabelType>> result =
                std::dynamic_pointer_cast<FiniteStateMachine<StateLabelType, EdgeLabelType>>(
                        this->newInstance());

        // maintain map of sets of states to the corresponding new states.
        std::map<const Abstract::SetOfStateRefs, const State<StateLabelType, EdgeLabelType> *>
                newStatesMap;

        // queue of states that need to be further explored.
        std::list<std::shared_ptr<Abstract::SetOfStateRefs>> unprocessed;

        // create initial state
        std::shared_ptr<Abstract::SetOfStateRefs> initialStateSet;
        initialStateSet = std::make_shared<Abstract::SetOfStateRefs>();
        initialStateSet->insert(&(this->getInitialState()));

        StateLabelType si = this->getInitialState().getLabel();
        const State<StateLabelType, EdgeLabelType> *initialState = result->addState(si);
        newStatesMap[*initialStateSet] = initialState;
        result->setInitialState(si);

        // add initial state to list of unprocessed state sets
        unprocessed.push_back(initialStateSet);

        while (!unprocessed.empty()) {
            std::shared_ptr<Abstract::SetOfStateRefs> Q = *(unprocessed.begin());
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
                std::shared_ptr<Abstract::SetOfStateRefs> QNext =
                        std::make_shared<Abstract::SetOfStateRefs>();

                // for every state s in Q
                for (const auto *i : *Q) {
                    const auto &s = dynamic_cast<const State<StateLabelType, EdgeLabelType> &>(*i);
                    std::shared_ptr<Abstract::SetOfStateRefs> l_img = s.nextStatesOfEdgeLabel(l);

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
                    unprocessed.push_back(QNext);
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

    using EquivalenceMap =
            std::map<const Abstract::State *, std::shared_ptr<Abstract::SetOfStateRefs>>;

    // minimize the automaton based on edge and state labels.
    std::shared_ptr<FiniteStateMachine<StateLabelType, EdgeLabelType>>
    minimizeEdgeLabels(bool ignoreStateLabels = false) {
        // partition refinement algorithm

        // generate a vector of equivalence classes
        std::list<std::shared_ptr<Abstract::SetOfStateRefs>> eqClasses;

        // populate it with s set of all states
        std::shared_ptr<Abstract::SetOfStateRefs> initialClass =
                std::make_shared<Abstract::SetOfStateRefs>(this->getStateRefs());
        eqClasses.push_back(initialClass);

        // initially map all state to the initial class
        EquivalenceMap eqMap;
        for (const auto *si : *initialClass) {
            auto sp = dynamic_cast<const State<StateLabelType, EdgeLabelType> *>(si);
            eqMap[sp] = initialClass;
        }

        // partition on state labels

        bool changed = false;
        if (!ignoreStateLabels) {
            do {
                changed = false;

                std::list<std::shared_ptr<Abstract::SetOfStateRefs>> newEqClasses;

                // for every potential equivalence class
                for (const auto &ic : eqClasses) {
                    std::shared_ptr<Abstract::SetOfStateRefs> _class = ic;

                    auto i = _class->begin();

                    // pick arbitrary state from class
                    auto s1 = dynamic_cast<const State<StateLabelType, EdgeLabelType> *>(*i);

                    std::shared_ptr<Abstract::SetOfStateRefs> equivSet =
                            std::make_shared<Abstract::SetOfStateRefs>();
                    std::shared_ptr<Abstract::SetOfStateRefs> remainingSet =
                            std::make_shared<Abstract::SetOfStateRefs>();
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
                    if (equivSet->size() == _class->size()) {
                        newEqClasses.push_back(equivSet);
                        this->mapStates(eqMap, equivSet);
                    } else {
                        newEqClasses.push_back(equivSet);
                        this->mapStates(eqMap, equivSet);
                        newEqClasses.push_back(remainingSet);
                        this->mapStates(eqMap, remainingSet);
                        changed = true;
                    }
                }
                auto tempEqClasses = eqClasses;
                eqClasses = newEqClasses;
            } while (changed);
        }

        // partition refinement on transitions
        changed = false;
        do {
            changed = false;

            std::list<std::shared_ptr<Abstract::SetOfStateRefs>> newEqClasses;

            // for every potential equivalence class
            for (const auto &ic : eqClasses) {
                std::shared_ptr<Abstract::SetOfStateRefs> _class = ic;

                auto i = _class->begin();

                // pick arbitrary state from class
                auto s1 = dynamic_cast<const State<StateLabelType, EdgeLabelType> *>(*i);

                std::shared_ptr<Abstract::SetOfStateRefs> equivSet =
                        std::make_shared<Abstract::SetOfStateRefs>();
                std::shared_ptr<Abstract::SetOfStateRefs> remainingSet =
                        std::make_shared<Abstract::SetOfStateRefs>();
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
                if (equivSet->size() == _class->size()) {
                    newEqClasses.push_back(equivSet);
                    this->mapStates(eqMap, equivSet);
                } else {
                    newEqClasses.push_back(equivSet);
                    this->mapStates(eqMap, equivSet);
                    newEqClasses.push_back(remainingSet);
                    this->mapStates(eqMap, remainingSet);
                    changed = true;
                }
            }
            auto tempEqClasses = eqClasses;
            eqClasses = newEqClasses;
        } while (changed);

        auto result = std::dynamic_pointer_cast<FiniteStateMachine<StateLabelType, EdgeLabelType>>(
                this->newInstance());

        // make a state for every equivalence class
        std::map<std::shared_ptr<Abstract::SetOfStateRefs>, State<StateLabelType, EdgeLabelType> *>
                newStateMap;
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
            auto es = s->getOutgoingEdges();
            // for every outgoing edge
            for (auto *edi : es) {
                auto ed = dynamic_cast<Edge<StateLabelType, EdgeLabelType> *>(edi);
                result->addEdge(*(newStateMap[cli]),
                                ed->getLabel(),
                                *(newStateMap[eqMap[&(ed->getDestination())]]));
            }
        }

        // set initial state
        result->setInitialState(*(newStateMap[eqMap[&(this->getInitialState())]]));

        return result;
    }

private:
    void insertOutgoingLabels(const State<StateLabelType, EdgeLabelType> *s,
                              std::set<EdgeLabelType> &labels) {
        const Abstract::SetOfEdgeRefs &e = s->getOutgoingEdges();

        // collect all labels in edges of s
        for (auto *i : e) {
            auto *ed = dynamic_cast<Edge<StateLabelType, EdgeLabelType> *>(i);
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
            std::shared_ptr<Abstract::SetOfStateRefs> ns1 = s1->nextStatesOfEdgeLabel(l);
            std::shared_ptr<Abstract::SetOfStateRefs> ns2 = s2->nextStatesOfEdgeLabel(l);
            // collect classes of states in ns1 and ns2
            std::set<std::shared_ptr<Abstract::SetOfStateRefs>> cs1;
            std::set<std::shared_ptr<Abstract::SetOfStateRefs>> cs2;
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
    void mapStates(EquivalenceMap &m, std::shared_ptr<Abstract::SetOfStateRefs> &sos) {
        for (const auto *i : *sos) {
            m[i] = sos;
        }
    }
};
} // namespace Labeled

namespace StateStringLabeled {

// make an FSM class with unlabeled edges, based on the labeled one with some dummy char labels
//

class State : public Labeled::State<CString, char> {
public:
    explicit State(const CString &withLabel) : Labeled::State<CString, char>(withLabel) {}

    const Abstract::SetOfEdgeRefs &getOutgoingEdges() {
        return dynamic_cast<const Abstract::SetOfEdgeRefs &>(
                Labeled::State<CString, char>::getOutgoingEdges());
    }
};

class FiniteStateMachine : public Labeled::FiniteStateMachine<CString, char> {
public:
    [[nodiscard]] State &getInitialState() const override {
        return dynamic_cast<State &>(Labeled::FiniteStateMachine<CString, char>::getInitialState());
    };

    void setInitialStateLabeled(const CString &sl);

    void addStateLabeled(const CString &sl);

    void addEdge(State *src, State *dst);

    void addEdgeLabeled(const CString &src, const CString &dst);

    std::shared_ptr<Abstract::SetOfStateRefs> reachableStates();
};

class Edge : public Labeled::Edge<CString, char> {};
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
        fsm_a(fsm1), fsm_b(fsm2) {}

    [[nodiscard]] State &getInitialState() const override;

    [[nodiscard]] virtual bool matchEdges(const Abstract::Edge &e1,
                                          const Abstract::Edge &e2) const = 0;

    [[nodiscard]] virtual std::shared_ptr<Abstract::Edge>
    ensureEdge(const Abstract::Edge &e1, const Abstract::Edge &e2) const = 0;

private:
    const Abstract::FiniteStateMachine &fsm_a;
    const Abstract::FiniteStateMachine &fsm_b;
};

} // namespace Product

} // namespace FSM

#endif