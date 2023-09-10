#include "graph/mpautomaton.h"
#include "base/analysis/mcm/mcm.h"
#include "base/analysis/mcm/mcmyto.h"
#include "base/basic_types.h"
#include <memory>

using namespace MaxPlus;
using namespace Graphs;

// compute the maximum cycle ratio of delay over progress
CDouble MaxPlusAutomatonWithRewards::calculateMCR() {

    MCMgraph g;
    CId nId = 0;

    std::map<const ::FSM::Abstract::State *, MCMnode *> nodeMap;

    for (const auto &s : this->getStates()) {
        auto *n = g.addNode(nId++);
        nodeMap[&(*(s.second))] = n;
    }

    CId eId = 0;
    for (const auto &s : this->getStates()) {
        for (auto *e : (s.second)->getOutgoingEdges()) {
            auto *mpae = dynamic_cast<MPAREdge *>(e);
            g.addEdge(eId++,
                      *nodeMap[&(mpae->getSource())],
                      *nodeMap[&(mpae->getDestination())],
                      static_cast<CDouble>(mpae->getLabel().delay),
                      mpae->getLabel().reward);
        }
    }

    std::vector<const MCMedge *> cycle;
    CDouble mcr = maxCycleRatioAndCriticalCycleYoungTarjanOrlin(g, &cycle);

    return mcr;
}

CDouble MaxPlusAutomatonWithRewards::calculateMCRAndCycle(
        std::shared_ptr<std::vector<const MPAREdge *>> *cycle) {

    MCMgraph g;

    CId nId = 0;
    std::map<const ::FSM::Abstract::State *, MCMnode *> nodeMap;

    for (const auto &s : this->getStates()) {
        auto *n = g.addNode(nId++);
        nodeMap[&(*(s.second))] = n;
    }

    CId eId = 0;
    std::map<const MCMedge *, const MPAREdge *> edgeMap;

    for (const auto &s : this->getStates()) {
        for (auto *e : (s.second)->getOutgoingEdges()) {
            auto *mpae = dynamic_cast<MPAREdge *>(e);
            g.addEdge(eId++,
                      *nodeMap[&(mpae->getSource())],
                      *nodeMap[&(mpae->getDestination())],
                      static_cast<CDouble>(mpae->getLabel().delay),
                      mpae->getLabel().reward);
        }
    }

    std::vector<const MCMedge *> mcmCycle;
    CDouble mcr = maxCycleRatioAndCriticalCycleYoungTarjanOrlin(g, &mcmCycle);
    if (cycle != nullptr) {
        *cycle = std::make_shared<std::vector<const MPAREdge *>>();
        for (const auto *e : mcmCycle) {
            (*cycle)->push_back(edgeMap[e]);
        }
    }

    return mcr;
}

/**
 * Compute the worst-case makespan of the maxplus automaton and produce the corresponding sequence.
 * TODO : make  pair<CDouble, list<CString*>*> a struct
 */
std::pair<CDouble, std::list<CString *> *>
MakespanCalculator::calculateWorstCaseMakespan(MaxPlusAutomaton *mpa) {
    // make sure the automaton has no cycles
    if (mpa->hasDirectedCycle())
        throw CException("calculateWorstCaseMakespan: automaton has directed cycle, can not "
                         "compute makespan");

    MPASetOfStates *F = mpa->getFinalStates();
    if (F->size() < 1) {
        throw CException("calculateWorstCaseMakespan: automaton has no final state, can not "
                         "compute makespan");
    }
    MPASetOfStates *I = mpa->getInitialStates();

    // we add a dummy state and dummy 0-weight transitions to all initial states
    MPAState *dummyState = new MPAState(make_mpastatelabel(0, 0));
    CString dummyScenario = CString("dummyScenario");

    MPASetOfStates::CIter i;
    for (i = I->begin(); i != I->end(); i++) {
        MPAState *id = ((MPAState *)(*i));
        MPAEdgeLabel l = make_mpaedgelabel(0, &dummyScenario);
        l.scenario = (CString *)&dummyScenario;
        mpa->addEdge(dummyState, l, id);
    }
    std::pair<CDouble, std::list<CString *> *> makespan = bellmanFordMax(mpa, dummyState, false);
    // remove the dummy before presenting the results

    makespan.second->erase(makespan.second->begin());

    return makespan;
}

/**
 * Compute the best-case makespan of the maxplus automaton and produce the corresponding sequence.
 */
CDouble MakespanCalculator::calculateBestCaseMakespan(MPASequence **sequence) {
    throw CException("Method calculateBestCaseMakespan not implemented yet");
}

/*
 * bellman-ford algorithm on max-plus automaton mpa to find the minimum path
 * returns only makespan of accepted words
 */
std::pair<CDouble, std::list<CString *> *> MakespanCalculator::bellmanFordMin(
        MaxPlusAutomaton *mpa, MPAState *srcId, bool checkForNegativeCycle = false) {
    std::map<MPAState *, std::pair<CDouble, std::list<CString *> *>> result =
            bellmanFord(mpa, srcId, checkForNegativeCycle);

    MPASetOfStates *F = mpa->getFinalStates();
    CDouble minDist = 0;
    MPAState *minState = 0;
    for (std::map<MPAState *, std::pair<CDouble, std::list<CString *> *>>::iterator i =
                 result.begin();
         i != result.end();
         i++) {
        // edges are negated so we find the min
        if ((*i).second.first <= minDist) {
            MPAState *s = (*i).first;
            if (F->count(mpa->findState(s->getLabel())) > 0) {
                minState = (*i).first;
                minDist = result[minState].first;
            }
        }
    }
    return result[minState];
}
/*
 * bellman-ford algorithm on max-plus automaton mpa to find the maximum path
 * returns only makespan of accepted words
 */
std::pair<CDouble, std::list<CString *> *> MakespanCalculator::bellmanFordMax(
        MaxPlusAutomaton *mpa, MPAState *srcId, bool checkForNegativeCycle = false) {
    MPASetOfEdges *e = mpa->getEdges();
    MPASetOfStates *F = mpa->getFinalStates();
    // negate all edge weights to find the longest path
    for (MPASetOfEdges::CIter j = e->begin(); j != e->end(); j++) {
        ((MPAEdge *)*j)->label.delay = -((MPAEdge *)*j)->label.delay;
    }
    std::map<MPAState *, std::pair<CDouble, std::list<CString *> *>> result =
            bellmanFord(mpa, srcId, checkForNegativeCycle);

    CDouble minDist = 0;
    MPAState *minState = 0;
    for (std::map<MPAState *, std::pair<CDouble, std::list<CString *> *>>::iterator i =
                 result.begin();
         i != result.end();
         i++) {
        // edges are negated so we find the min
        if ((*i).second.first <= minDist) {
            MPAState *s = (*i).first;
            if (F->includeStateLabeled(s->getLabel())) {
                minState = (*i).first;
                minDist = result[minState].first;
            }
        }
    }

    // all edge weights back to positive
    for (MPASetOfEdges::CIter j = e->begin(); j != e->end(); j++) {
        ((MPAEdge *)*j)->label.delay = -((MPAEdge *)*j)->label.delay;
    }

    // negate the makespan because it is negative, we need positive
    result[minState].first = -result[minState].first;
    return result[minState];
}

std::mutex m;
std::mutex mIter;
size_t maxglobaliterations = 0;
size_t globaliterations = 0;
void bellmanFordThread(MPASetOfEdges *edges,
                       std::map<MPAState *, std::pair<CDouble, std::list<CString *> *>> &dis) {
    while (true) {
        for (MPASetOfEdges::CIter j = edges->begin(); j != edges->end(); j++) {
            MPAState *sourceId = (MPAState *)(*j)->getSource();
            if (dis[sourceId].first != INFINITY) {
                MPAState *destId = (MPAState *)(*j)->getDestination();
                CDouble weight = ((MPAEdge *)*j)->label.delay;
                if (dis[sourceId].first + weight < dis[destId].first) {
                    std::list<CString *> *scList = new list<CString *>(*dis[sourceId].second);
                    scList->push_back(((MPAEdge *)*j)->label.scenario);
                    m.lock();
                    dis[destId] = make_pair(dis[sourceId].first + weight, scList);
                    m.unlock();
                }
            }
        }
        mIter.lock();
        globaliterations++;
        mIter.unlock();

        if (globaliterations >= maxglobaliterations) {
            break;
        }
    }
}

struct EdgeData {
    size_t sourceId;
    size_t destId;
    CDouble weight;
    CString *label;
};

void bellmanFordStaticThread(CDouble *vDst, struct EdgeData *edges, size_t nrEdges) {
    while (true) {
        for (size_t i = 0; i < nrEdges; i++) {
            EdgeData *e = &(edges[i]);
            size_t sourceId = e->sourceId;
            CDouble sd = vDst[sourceId];
            if (sd != INFINITY) {
                size_t destId = e->destId;
                CDouble weight = e->weight;
                CDouble dd = vDst[destId];
                if (sd + weight < dd) {
                    vDst[destId] = sd + weight;
                }
            }
        }
        mIter.lock();
        globaliterations++;
        mIter.unlock();

        if (globaliterations >= maxglobaliterations) {
            break;
        }
    }
}

void bellmanFordStaticSequential(CDouble *vDst,
                                 struct EdgeData *edges,
                                 size_t nrVertices,
                                 size_t nrEdges) {
    for (size_t k = 0; k < nrVertices; k++) {
        size_t i = 0;
        for (; i < nrEdges; i++) {
            EdgeData *e = &(edges[i]);
            size_t sourceId = e->sourceId;
            CDouble sd = vDst[sourceId];
            if (sd != INFINITY) {
                size_t destId = e->destId;
                CDouble weight = e->weight;
                CDouble dd = vDst[destId];
                if (sd + weight < dd) {
                    vDst[destId] = sd + weight;
                    break;
                }
            }
        }
        // we reached the end without updates
        if (i == nrEdges) {
            break;
        }
        for (; i < nrEdges; i++) {
            EdgeData *e = &(edges[i]);
            size_t sourceId = e->sourceId;
            CDouble sd = vDst[sourceId];
            if (sd != INFINITY) {
                size_t destId = e->destId;
                CDouble weight = e->weight;
                CDouble dd = vDst[destId];
                if (sd + weight < dd) {
                    vDst[destId] = sd + weight;
                }
            }
        }
    }
}

void bellmanFordFindCriticalEdges(CDouble *vDst,
                                  struct EdgeData *edges,
                                  size_t *critical,
                                  size_t nrEdges) {
    for (size_t i = 0; i < nrEdges; i++) {
        EdgeData *e = &(edges[i]);
        size_t sourceId = e->sourceId;
        CDouble sd = vDst[sourceId];
        size_t destId = e->destId;
        CDouble weight = e->weight;
        CDouble dd = vDst[destId];
        if (sd + weight <= dd + MP_EPSILON) {
            if (sourceId
                != destId) { // hack: remove if loop on first state labelled DummyState is fixed
                critical[destId] = i;
            }
        }
    }
}

/*
 * bellman-ford algorithm on max-plus automaton mpa
 */
std::map<MPAState *, std::pair<CDouble, std::list<CString *> *>> MakespanCalculator::bellmanFord(
        MaxPlusAutomaton *mpa, MPAState *srcId, bool checkForNegativeCycle = false) {

    MPASetOfEdges *edges = mpa->getEdges();
    MPASetOfStates *s = mpa->getStates();
    size_t nv = mpa->getStates()->size();

    // make static tables
    std::map<CId, size_t> vertexMap;
    MPASetOfStates *states = mpa->getStates();
    CDouble *vDist = new CDouble[nv];
    size_t i = 0;
    for (MPASetOfStates::CIter j = states->begin(); j != states->end(); j++) {
        auto s = *j;
        vDist[i] = INFINITY;
        vertexMap[s->getId()] = i;
        i++;
    }
    // distance of source vertex from source vertex is 0
    vDist[vertexMap[srcId->getId()]] = 0;

    struct EdgeData *edgesArray = new struct EdgeData[edges->size()];
    i = 0;
    for (MPASetOfEdges::CIter j = edges->begin(); j != edges->end(); j++) {
        auto e = ((MPAEdge *)*j);
        edgesArray[i].sourceId = vertexMap[e->getSource()->getId()];
        edgesArray[i].destId = vertexMap[e->getDestination()->getId()];
        edgesArray[i].weight = e->label.delay;
        edgesArray[i].label = e->label.scenario;
        i++;
    }
    size_t nrEdges = i;

    bellmanFordStaticSequential(vDist, edgesArray, nv, nrEdges);

    // checking if negative cycle is present
    if (checkForNegativeCycle) {
        std::cout << "checking for negative cycle" << std::endl;
        for (size_t i = 0; i < nrEdges; i++) {
            EdgeData *e = &(edgesArray[i]);
            if (vDist[e->sourceId] + e->weight < vDist[e->destId]) {
                throw CException("in bellman-ford algorithm: Negative cycle found!");
            }
        }
    }

    size_t *critical = new size_t[nv];
    for (size_t i = 0; i < nv; i++) {
        critical[i] = SIZE_MAX;
    }
    critical[0] = SIZE_MAX - 1;
    bellmanFordFindCriticalEdges(vDist, edgesArray, critical, nrEdges);

    // destination, distance, list of edges to get there
    std::map<MPAState *, pair<CDouble, list<CString *> *>> dis;

    // copy results from BF analysis
    for (MPASetOfStates::CIter i = s->begin(); i != s->end(); i++) {
        auto st = *i;
        size_t sid = vertexMap[st->getId()];
        auto l = new std::list<CString *>();
        size_t lid = sid;
        while (critical[lid] != SIZE_MAX - 1) {
            l->push_back(edgesArray[critical[lid]].label);
            lid = edgesArray[critical[lid]].sourceId;
        }
        l->reverse();
        dis[(MPAState *)*i] = make_pair(vDist[sid], l);
    }

    delete[] vDist;
    delete[] edgesArray;
    delete[] critical;

    // delete (e);
    return dis;
}