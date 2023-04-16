#include "base/basic_types.h"
#include "base/analysis/mcm/mcm.h"
#include "base/analysis/mcm/mcmyto.h"
#include "graph/mpautomaton.h"
#include <memory>

using namespace MaxPlus;
using namespace Graphs;


// compute the maximum cycle ratio of delay over progress
CDouble MaxPlusAutomatonWithRewards::calculateMCR(){
    
    MCMgraph g;
    CId nId = 0;

    std::map<const ::FSM::Abstract::State*, MCMnode*> nodeMap;

    for (auto s: this->getStates()) {
        auto n = g.addNode(nId++);
        nodeMap[&(*(s.second))] = n;
    }

    CId eId = 0;
    for (auto s: this->getStates()) {
        for(auto e: (s.second)->getOutgoingEdges()) {
            auto mpae = dynamic_cast<MPAREdge*>(e);
            g.addEdge(eId++, *nodeMap[&(mpae->getSource())], *nodeMap[&(mpae->getDestination())], static_cast<CDouble>(mpae->getLabel().delay), mpae->getLabel().reward);
        }
    }

    std::shared_ptr<std::vector<const MCMedge*>> cycle;
    CDouble mcr = maxCycleRatioAndCriticalCycleYoungTarjanOrlin(g, &cycle);

    return mcr;
}

CDouble MaxPlusAutomatonWithRewards::calculateMCRAndCycle(std::shared_ptr<std::vector<const MPAREdge*>> *cycle) {
    
    MCMgraph g;

    CId nId = 0;
    std::map<const ::FSM::Abstract::State*, MCMnode*> nodeMap;

    for (auto s: this->getStates()) {
        auto n = g.addNode(nId++);
        nodeMap[&(*(s.second))] = n;
    }

    CId eId = 0;
    std::map<const MCMedge*, const MPAREdge*> edgeMap;

    for (auto s: this->getStates()) {
        for(auto e: (s.second)->getOutgoingEdges()) {
            auto mpae = dynamic_cast<MPAREdge*>(e);
            g.addEdge(eId++, *nodeMap[&(mpae->getSource())], *nodeMap[&(mpae->getDestination())], static_cast<CDouble>(mpae->getLabel().delay), mpae->getLabel().reward);
        }
    }

    std::shared_ptr<std::vector<const MCMedge*>> mcmCycle;
    CDouble mcr = maxCycleRatioAndCriticalCycleYoungTarjanOrlin(g, &mcmCycle);
    if (cycle != nullptr) {
        *cycle = std::make_shared<std::vector<const MPAREdge*>>();
        for (auto e: *mcmCycle) {
            (*cycle)->push_back(edgeMap[e]);
        }
    }

    return mcr;
}