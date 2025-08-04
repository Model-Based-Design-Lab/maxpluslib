/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   mcmkarp.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 8, 2005
 *
 *  Function        :   Compute the MCM for an HSDF graph using Karp's
 *                      algorithm.
 *
 *  History         :
 *      08-11-05    :   Initial version.
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

#include "base/analysis/mcm/mcmgraph.h"
#include "base/math/cmath.h"
#include <climits>
#include <cstdint>

namespace MaxPlus::Graphs {
/**
 * mcmKarp ()
 * The function computes the maximum cycle mean of an MCMgraph using Karp's
 * algorithm.
 * Note that the following assumptions are made about the MCMgraph
 * 1. it is assumed that the edge weights have integer values.
 * 2. it is assumed that all nodes in the graph are 'visible'
 * 3. it is assumed that the node have id's ranging from 0 up to the number of nodes.
 * 4. it is assumed that all nodes have an outgoing directed edge
 *
 * The critical cycle is only returned if cycle is not nullptr. Then *cycle points
 * to an array of MCMEdges of the critical cycle.
 * cycle  - pointers to arcs on maximum ratio cycle,
 *          ordered in array from top to bottom with
 *          respect to subsequent arcs on cycle
 *
 * len    - number of elements of "cycle"
 */
CDouble maximumCycleMeanKarp(MCMgraph &mcmGraph) {
    std::shared_ptr<MCMnode> u;

    // Allocate memory d[n+1][n]
    unsigned int n = mcmGraph.nrVisibleNodes();
    std::vector<std::vector<std::uint64_t>> d(n + 1, std::vector<std::uint64_t>(n));

    // Initialize
    // d[k][u], 1<=k<n+1, 0<=u<n with value -inf
    for (unsigned int k = 1; k < n + 1; k++) {
        for (unsigned int u = 0; u < n; u++) {
            d[k][u] = -INT_MAX;
        }
    }
    // d[0][u], 0<=u<n with value 0
    for (unsigned int u = 0; u < n; u++) {
        d[0][u] = 0;
    }

    // Compute the distances
    for (unsigned int k = 1; k < n + 1; k++) {
        for (const auto &v : mcmGraph.getNodes()) {
            for (auto e = v.in.begin(); e != v.in.end(); e++) {
                MCMnode *u = (*e)->src;
                d[k][v.id] = MAX(d[k][v.id], d[k - 1][u->id] + ((int)(*e)->w));
            }
        }
    }

    // Compute lambda using Karp's theorem
    CDouble l = -INT_MAX;
    for (const auto &u : mcmGraph.getNodes()) {
        CDouble ld = INT_MAX;
        for (unsigned int k = 0; k < n; k++) {
            ld = MIN(ld, (CDouble)(d[n][u.id] - d[k][u.id]) / (CDouble)(n - k));
        }
        l = MAX(l, ld);
    }

    return l;
}

CDouble maximumCycleMeanKarpGeneral(MCMgraph &g) {
    MCMgraphs sccs;
    stronglyConnectedMCMgraph(g, sccs, false);

    auto mcm = static_cast<CDouble>(-INFINITY);
    for (auto &scc : sccs) {
        std::map<CId, CId> nodeMap;
        scc->relabelNodeIds(&nodeMap);
        MCMnode *sccCriticalNode = nullptr;
        ;
        CDouble cmcm = maximumCycleMeanKarp(*scc);
        if (cmcm > mcm) {
            mcm = cmcm;
        }
    }

    return mcm;
}

/**
 * maximumCycleMeanKarpDouble ()
 * The function computes the maximum cycle mean of an MCMgGraph using Karp's
 * algorithm.
 * Note that the following assumptions are made about the MCMgraph
 * 1. it is assumed that all nodes in the graph are 'visible'
 * 2. it is assumed that the node have id's ranging from 0 up to the number of nodes.
 *
 * A critical node is only returned if criticalNode is not nullptr.
 */

CDouble maximumCycleMeanKarpDouble(MCMgraph &mcmGraph, const MCMnode **criticalNode = nullptr) {

    // Allocate memory d[n+1][n]
    unsigned int n = mcmGraph.nrVisibleNodes();
    auto d = std::vector<std::vector<CDouble>>(n + 1, std::vector<CDouble>(n));

    // Initialize
    // d[k][u], 1<=k<n+1, 0<=u<n with value -inf
    for (unsigned int k = 1; k < n + 1; k++) {
        for (unsigned int u = 0; u < n; u++) {
            d[k][u] = -DBL_MAX;
        }
    }
    // d[0][u], 0<=u<n with value 0
    for (unsigned int u = 0; u < n; u++) {
        d[0][u] = 0.0;
    }

    // Compute the distances
    for (unsigned int k = 1; k < n + 1; k++) {
        for (const auto &v : mcmGraph.getNodes()) {
            for (auto e = v.in.begin(); e != v.in.end(); e++) {
                MCMnode *u = (*e)->src;
                d[k][v.id] = MAX(d[k][v.id], d[k - 1][u->id] + ((*e)->w));
            }
        }
    }

    // Compute lambda using Karp's theorem
    CDouble l = -DBL_MAX;
    for (const auto &u : mcmGraph.getNodes()) {
        CDouble ld = DBL_MAX;
        for (unsigned int k = 0; k < n; k++) {
            CDouble nld = (d[n][u.id] - d[k][u.id]) / static_cast<CDouble>(n - k);
            if (nld < ld) {
                ld = nld;
            }
        }
        if (ld > l) {
            l = ld;
            if (criticalNode != nullptr) {
                *criticalNode = &u;
            }
        }
    }

    return l;
}

CDouble maximumCycleMeanKarpDoubleGeneral(MCMgraph &g, const MCMnode **criticalNode) {
    MCMgraphs sccs;
    stronglyConnectedMCMgraph(g, sccs, false);

    auto mcm = static_cast<CDouble>(-INFINITY);
    if (criticalNode != nullptr) {
        *criticalNode = nullptr;
    }
    for (auto &scc : sccs) {
        std::map<CId, CId> nodeMap;
        scc->relabelNodeIds(&nodeMap);
        const MCMnode *sccCriticalNode = nullptr;
        ;
        CDouble cmcm = maximumCycleMeanKarpDouble(*scc, &sccCriticalNode);
        if (cmcm > mcm) {
            mcm = cmcm;
            if (criticalNode != nullptr) {
                *criticalNode = g.getNode(nodeMap[sccCriticalNode->id]);
            }
        }
    }

    return mcm;
}

} // namespace MaxPlus::Graphs
